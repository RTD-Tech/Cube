#include "Client.h"
#include <zmq/zmq.hpp>
#include <iostream>
#include <sstream>
#include <chrono>

namespace SquareCore {

// Global ZMQ context and socket
static zmq::context_t context(1);
static zmq::socket_t* clientSocket = nullptr;

Client::Client() {
    lastUpdate = std::chrono::steady_clock::now();
}

Client::~Client() {
    Disconnect();
}

bool Client::Connect(const std::string& serverAddress) {
    if (connected.load()) {
        std::cout << "Client is already connected\n";
        return true;
    }

    std::cout << "Connecting to server at " << serverAddress << "\n";

    try {
        InitializeSockets(serverAddress);

        // Send connection request
        std::string connectMsg = CreateMessage(MessageType::CONNECT, "");
        zmq::message_t request(connectMsg.size());
        memcpy(request.data(), connectMsg.data(), connectMsg.size());

        if (!clientSocket->send(request, zmq::send_flags::none)) {
            std::cout << "Failed to send CONNECT request\n";
            CleanupSockets();
            return false;
        }

        // Wait for response
        zmq::message_t reply;
        auto result = clientSocket->recv(reply, zmq::recv_flags::none);

        if (result) {
            std::string response(static_cast<char*>(reply.data()), reply.size());
            std::cout << "Server response: " << response << "\n";

            std::istringstream iss(response);
            std::string status;
            uint32_t assignedId;

            if (iss >> status >> assignedId && status == "CONNECTED") {
                clientId = assignedId;
                connected = true;

                std::cout << "Connected successfully! Client ID: " << assignedId << "\n";

                // Close connection socket and reconnect to dedicated port
                CleanupSockets();
                std::string dedicatedAddress = serverAddress + ":" + std::to_string(5556 + assignedId);

                clientSocket = new zmq::socket_t(context, zmq::socket_type::req);
                clientSocket->connect("tcp://" + dedicatedAddress);

                int timeout = 1000;
                clientSocket->set(zmq::sockopt::rcvtimeo, timeout);
                clientSocket->set(zmq::sockopt::sndtimeo, timeout);

                return true;
            } else {
                std::cout << "Invalid response format: " << response << "\n";
            }
        } else {
            std::cout << "No response from server\n";
        }

    } catch (const zmq::error_t& e) {
        std::cout << "ZMQ error during connection: " << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cout << "Error during connection: " << e.what() << "\n";
    }

    CleanupSockets();
    return false;
}

void Client::Disconnect() {
    if (!connected.load()) {
        return;
    }

    std::cout << "Disconnecting from server...\n";
    disconnecting = true;

    // Send disconnect message
    if (clientSocket && clientId.load() != 0) {
        std::lock_guard<std::mutex> lock(socketMutex);

        try {
            std::string disconnectMsg = CreateMessage(MessageType::DISCONNECT, "");
            zmq::message_t request(disconnectMsg.size());
            memcpy(request.data(), disconnectMsg.data(), disconnectMsg.size());

            clientSocket->send(request, zmq::send_flags::none);
        } catch (const std::exception& e) {
            std::cout << "Error sending disconnect: " << e.what() << "\n";
        }
    }

    connected = false;
    clientId = 0;
    disconnecting = false;

    CleanupSockets();
    std::cout << "Disconnected from server\n";
}

void Client::Update() {
    if (!connected.load() || disconnecting.load()) {
        return;
    }

    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate);

    if (timeSinceLastUpdate.count() >= UPDATE_INTERVAL_MS) {
        SendInputAndReceiveState();
        lastUpdate = now;
    }
}

void Client::SendInput(const std::unordered_map<std::string, bool>& buttons,
                       const std::unordered_map<std::string, float>& axes) {
    if (!connected.load()) {
        return;
    }

    std::lock_guard<std::mutex> lock(inputMutex);
    pendingInput.clientID = clientId.load();
    pendingInput.buttons = buttons;
    pendingInput.axes = axes;
    pendingInput.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

GameStateSnapshot Client::GetLatestGameState() {
    std::lock_guard<std::mutex> lock(stateMutex);
    return latestState;
}

std::vector<EntitySpawnInfo> Client::GetPendingSpawns() {
    std::lock_guard<std::mutex> lock(pendingMessagesMutex);
    std::vector<EntitySpawnInfo> spawns = std::move(pendingSpawns);
    pendingSpawns.clear();
    return spawns;
}

std::vector<uint32_t> Client::GetPendingDespawns() {
    std::lock_guard<std::mutex> lock(pendingMessagesMutex);
    std::vector<uint32_t> despawns = std::move(pendingDespawns);
    pendingDespawns.clear();
    return despawns;
}

void Client::InitializeSockets(const std::string& serverAddress) {
    try {
        clientSocket = new zmq::socket_t(context, zmq::socket_type::req);
        std::string address = "tcp://" + serverAddress + ":5555";
        clientSocket->connect(address);

        int timeout = 5000;
        clientSocket->set(zmq::sockopt::rcvtimeo, timeout);
        clientSocket->set(zmq::sockopt::sndtimeo, timeout);

    } catch (const zmq::error_t& e) {
        CleanupSockets();
        throw std::runtime_error("Failed to initialize client socket: " + std::string(e.what()));
    }
}

void Client::CleanupSockets() {
    if (clientSocket) {
        try {
            clientSocket->close();
            delete clientSocket;
            clientSocket = nullptr;
        } catch (const std::exception& e) {
            std::cout << "Error closing client socket: " << e.what() << "\n";
        }
    }
}

void Client::SendInputAndReceiveState() {
    if (!clientSocket || !connected.load() || disconnecting.load()) {
        return;
    }

    std::lock_guard<std::mutex> lock(socketMutex);

    try {
        // Get pending input
        InputState inputToSend;
        {
            std::lock_guard<std::mutex> inputLock(inputMutex);
            inputToSend = pendingInput;
        }

        // Send input to server
        std::string inputMsg = CreateMessage(MessageType::INPUT, inputToSend.Serialize());
        zmq::message_t request(inputMsg.size());
        memcpy(request.data(), inputMsg.data(), inputMsg.size());

        if (clientSocket->send(request, zmq::send_flags::none)) {
            // Receive game state response
            zmq::message_t reply;
            auto result = clientSocket->recv(reply, zmq::recv_flags::none);

            if (result) {
                std::string response(static_cast<char*>(reply.data()), reply.size());

                // Server sends multiple messages separated by newlines
                std::istringstream responseStream(response);
                std::string line;

                while (std::getline(responseStream, line)) {
                    if (line.empty()) continue;

                    MessageType msgType;
                    std::string payload;
                    if (ParseMessage(line, msgType, payload)) {
                        if (msgType == MessageType::SPAWN_ENTITY) {
                            // Parse and queue entity spawn
                            EntitySpawnInfo spawnInfo = EntitySpawnInfo::Deserialize(payload);
                            {
                                std::lock_guard<std::mutex> msgLock(pendingMessagesMutex);
                                pendingSpawns.push_back(spawnInfo);
                            }
                        }
                        else if (msgType == MessageType::DESPAWN_ENTITY) {
                            // Parse and queue entity despawn
                            uint32_t entityID = std::stoul(payload);
                            {
                                std::lock_guard<std::mutex> msgLock(pendingMessagesMutex);
                                pendingDespawns.push_back(entityID);
                            }
                        }
                        else if (msgType == MessageType::GAME_STATE) {
                            // Parse game state
                            GameStateSnapshot newState = GameStateSnapshot::Deserialize(payload);

                            // Update latest state
                            {
                                std::lock_guard<std::mutex> stateLock(stateMutex);
                                latestState = newState;
                            }
                        }
                    }
                }
            }
        }

    } catch (const zmq::error_t& e) {
        if (e.num() != ETERM) {
            std::cout << "ZMQ error in send/receive: " << e.what() << "\n";
        }
    } catch (const std::exception& e) {
        std::cout << "Error in send/receive: " << e.what() << "\n";
    }
}

}
