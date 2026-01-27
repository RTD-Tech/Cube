#include "Server.h"
#include "Core/Script.h"
#include <zmq/zmq.hpp>
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

namespace SquareCore {

// Global ZMQ context
static zmq::context_t context(1);
static zmq::socket_t* acceptSocket = nullptr;

Server::Server() {
}

Server::~Server() {
    Stop();
}

void Server::Start(const std::vector<Script*>& scripts) {
    if (running.load()) {
        std::cout << "Server is already running\n";
        return;
    }

    if (scripts.empty()) {
        std::cout << "Error: Cannot start server without game logic instance\n";
        return;
    }

    this->scripts = scripts;
    running = true;

    std::cout << "Starting server...\n";

    try {
        InitializeSockets();

        // Start connection listener thread
        std::thread listener(&Server::ConnectionListenerThread, this);
        listener.detach();

        // Run simulation loop in main thread
        SimulationLoop();

    } catch (const std::exception& e) {
        std::cout << "Failed to start server: " << e.what() << "\n";
        CleanupSockets();
        running = false;
    }
}

void Server::Stop() {
    if (!running.load()) {
        return;
    }

    std::cout << "Stopping server...\n";
    running = false;

    // Signal all client threads to stop
    {
        std::lock_guard<std::mutex> lock(clientConnectionsMutex);
        for (auto& conn : clientConnections) {
            conn->active = false;
        }
    }

    // Give threads time to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Join all client threads
    {
        std::lock_guard<std::mutex> lock(clientConnectionsMutex);
        for (auto& conn : clientConnections) {
            if (conn->thread.joinable()) {
                conn->thread.join();
            }
        }
        clientConnections.clear();
    }

    CleanupSockets();
    std::cout << "Server stopped successfully\n";
}

void Server::InitializeSockets() {
    try {
        acceptSocket = new zmq::socket_t(context, zmq::socket_type::rep);
        acceptSocket->bind("tcp://*:5555");

        int timeout = 100;
        acceptSocket->set(zmq::sockopt::rcvtimeo, timeout);

    } catch (const zmq::error_t& e) {
        CleanupSockets();
        throw std::runtime_error("Failed to initialize sockets: " + std::string(e.what()));
    }
}

void Server::CleanupSockets() {
    if (acceptSocket) {
        try {
            acceptSocket->close();
            delete acceptSocket;
            acceptSocket = nullptr;
        } catch (const std::exception& e) {
            std::cout << "Error closing accept socket: " << e.what() << "\n";
        }
    }
}

void Server::ConnectionListenerThread() {
    std::cout << "Connection listener started on port 5555\n";

    while (running.load()) {
        try {
            if (!acceptSocket) {
                break;
            }

            // Listen for new connection requests
            zmq::message_t request;
            auto result = acceptSocket->recv(request, zmq::recv_flags::dontwait);

            if (result) {
                std::string requestStr(static_cast<char*>(request.data()), request.size());

                MessageType msgType;
                std::string payload;
                if (!ParseMessage(requestStr, msgType, payload)) {
                    std::cout << "Failed to parse message: " << requestStr << "\n";
                    continue;
                }

                if (msgType == MessageType::CONNECT) {
                    // Handle new client connection
                    uint32_t newClientID = HandleConnect(acceptSocket);

                    std::ostringstream oss;
                    oss << "CONNECTED " << newClientID;
                    std::string response = oss.str();

                    zmq::message_t reply(response.size());
                    memcpy(reply.data(), response.data(), response.size());
                    acceptSocket->send(reply, zmq::send_flags::none);

                    std::cout << "Client " << newClientID << " connected\n";
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        } catch (const zmq::error_t& e) {
            if (e.num() != ETERM && running.load()) {
                std::cout << "ZMQ error in connection listener: " << e.what() << "\n";
            }
        } catch (const std::exception& e) {
            std::cout << "Error in connection listener: " << e.what() << "\n";
        }
    }

    std::cout << "Connection listener stopped\n";
}

uint32_t Server::HandleConnect(void* socket) {
    uint32_t clientID = nextClientID.fetch_add(1);

    // Create dedicated socket for this client
    zmq::socket_t* clientSocket = new zmq::socket_t(context, zmq::socket_type::rep);
    std::string address = "tcp://*:" + std::to_string(5556 + clientID);
    clientSocket->bind(address);

    int timeout = 100;
    clientSocket->set(zmq::sockopt::rcvtimeo, timeout);

    // Create client connection
    auto conn = std::make_unique<ClientConnection>();
    conn->clientID = clientID;
    conn->active = true;

    {
        std::lock_guard<std::mutex> lock(clientConnectionsMutex);
        clientConnections.push_back(std::move(conn));
    }

    // Send current world state to new client
    SendWorldStateToClient(clientID);

    // Notify game logic (spawn player and broadcast to all clients)
    for (auto* script : scripts) 
        script->OnClientConnected(clientID);

    // NOW start the client thread (world state is already queued)
    {
        std::lock_guard<std::mutex> lock(clientConnectionsMutex);
        for (auto& c : clientConnections) {
            if (c->clientID == clientID) {
                c->thread = std::thread(&Server::ClientThread, this, clientID, clientSocket);
                break;
            }
        }
    }

    return clientID;
}

void Server::HandleDisconnect(uint32_t clientID) {
    // Remove player entity
    {
        std::lock_guard<std::mutex> lock(clientPlayerMutex);
        auto it = clientPlayerMap.find(clientID);
        if (it != clientPlayerMap.end()) {
            uint32_t entityID = it->second;
            serverEntityManager.RemoveEntity(entityID);
            clientPlayerMap.erase(it);
        }
    }

    // Notify game logic
    for (auto* script : scripts) 
        script->OnClientDisconnected(clientID);

    std::cout << "Client " << clientID << " disconnected\n";
}

void Server::ClientThread(uint32_t clientID, void* socketPtr) {
    zmq::socket_t* clientSocket = static_cast<zmq::socket_t*>(socketPtr);
    std::cout << "Client thread started for client " << clientID << "\n";

    // Find this client's connection
    ClientConnection* conn = nullptr;
    {
        std::lock_guard<std::mutex> lock(clientConnectionsMutex);
        for (auto& c : clientConnections) {
            if (c->clientID == clientID) {
                conn = c.get();
                break;
            }
        }
    }

    while (running.load() && conn && conn->active.load()) {
        try {
            // Receive input from client
            zmq::message_t request;
            auto result = clientSocket->recv(request, zmq::recv_flags::dontwait);

            if (result) {
                std::string requestStr(static_cast<char*>(request.data()), request.size());

                MessageType msgType;
                std::string payload;
                if (ParseMessage(requestStr, msgType, payload)) {
                    if (msgType == MessageType::INPUT) {
                        // Parse and queue input
                        InputState input = InputState::Deserialize(payload);
                        input.clientID = clientID;
                        inputManager.QueueInput(input);
                    } else if (msgType == MessageType::DISCONNECT) {
                        HandleDisconnect(clientID);
                        conn->active = false;
                        break;
                    }
                }

                // Build response with queued messages and game state
                std::ostringstream response;

                // Get and send queued spawn/despawn messages
                {
                    std::lock_guard<std::mutex> queueLock(conn->queueMutex);

                    // Send spawn messages
                    for (const auto& spawnInfo : conn->spawnQueue) {
                        response << CreateMessage(MessageType::SPAWN_ENTITY, spawnInfo.Serialize()) << "\n";
                    }
                    conn->spawnQueue.clear();

                    // Send despawn messages
                    for (uint32_t entityID : conn->despawnQueue) {
                        response << CreateMessage(MessageType::DESPAWN_ENTITY, std::to_string(entityID)) << "\n";
                    }
                    conn->despawnQueue.clear();
                }

                // Send latest game state
                GameStateSnapshot latestState;
                {
                    std::lock_guard<std::mutex> lock(stateQueueMutex);
                    if (!stateQueue.empty()) {
                        latestState = stateQueue.back().snapshot;
                    }
                }

                response << CreateMessage(MessageType::GAME_STATE, latestState.Serialize());

                std::string responseStr = response.str();
                zmq::message_t reply(responseStr.size());
                memcpy(reply.data(), responseStr.data(), responseStr.size());
                clientSocket->send(reply, zmq::send_flags::none);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(16));

        } catch (const zmq::error_t& e) {
            if (e.num() != ETERM && conn->active.load()) {
                std::cout << "ZMQ error in client thread " << clientID << ": " << e.what() << "\n";
            }
        } catch (const std::exception& e) {
            std::cout << "Error in client thread " << clientID << ": " << e.what() << "\n";
        }
    }

    // Cleanup
    try {
        clientSocket->close();
        delete clientSocket;
    } catch (const std::exception& e) {
        std::cout << "Error closing client socket: " << e.what() << "\n";
    }

    std::cout << "Client thread stopped for client " << clientID << "\n";
}

void Server::SimulationLoop() {
    std::cout << "Server simulation loop started\n";

    auto lastTime = std::chrono::high_resolution_clock::now();
    float accumulator = 0.0f;

    while (running.load()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        accumulator += deltaTime;

        // Fixed timestep updates
        while (accumulator >= FIXED_TIMESTEP) {
            // Apply timeline scaling
            float effectiveTimestep = serverTimeline.CalculateEffectiveTime(FIXED_TIMESTEP);

            // Update timeline
            serverTimeline.Update(FIXED_TIMESTEP);

            // Update physics
            serverPhysics.Update(effectiveTimestep);

            // Update animations
            serverEntityManager.UpdateAnimations(effectiveTimestep);

            // Call game logic
            for (auto* script : scripts) 
                script->OnUpdate(effectiveTimestep);

            // Capture game state
            GameStateSnapshot snapshot = CaptureGameState();
            snapshot.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                currentTime.time_since_epoch()).count();

            // Push to state queue
            {
                std::lock_guard<std::mutex> lock(stateQueueMutex);
                GameStatePacket packet;
                packet.snapshot = snapshot;
                packet.timestamp = currentTime;
                stateQueue.push(packet);

                // Keep only latest 5 states
                while (stateQueue.size() > 5) {
                    stateQueue.pop();
                }
            }

            accumulator -= FIXED_TIMESTEP;
        }

        // Sleep to maintain ~60Hz
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    std::cout << "Server simulation loop stopped\n";
}

GameStateSnapshot Server::CaptureGameState() {
    GameStateSnapshot snapshot;

    // Get all entities from server's entity manager
    std::vector<Entity> entities = serverEntityManager.GetEntitiesCopy();

    for (const Entity& entity : entities) {
        EntitySnapshot entitySnap;
        entitySnap.entityID = entity.ID;
        entitySnap.position = entity.position;
        entitySnap.velocity = entity.velocity;
        entitySnap.scale = entity.scale;
        entitySnap.rotation = entity.rotation;
        entitySnap.flipX = entity.flipX;
        entitySnap.flipY = entity.flipY;
        entitySnap.currentFrame = entity.currentFrame;

        snapshot.entities.push_back(entitySnap);
    }

    // Add player bindings
    {
        std::lock_guard<std::mutex> lock(clientPlayerMutex);
        snapshot.playerEntityBindings = clientPlayerMap;
    }

    return snapshot;
}

void Server::RegisterPlayerEntity(uint32_t clientID, uint32_t entityID) {
    std::lock_guard<std::mutex> lock(clientPlayerMutex);
    clientPlayerMap[clientID] = entityID;
    std::cout << "Registered player entity " << entityID << " for client " << clientID << "\n";
}

void Server::UnregisterPlayerEntity(uint32_t clientID) {
    std::lock_guard<std::mutex> lock(clientPlayerMutex);
    auto it = clientPlayerMap.find(clientID);
    if (it != clientPlayerMap.end()) {
        std::cout << "Unregistered player entity " << it->second << " for client " << clientID << "\n";
        clientPlayerMap.erase(it);
    }
}

std::vector<uint32_t> Server::GetConnectedClients() const {
    std::lock_guard<std::mutex> lock(clientConnectionsMutex);
    std::vector<uint32_t> clients;
    for (const auto& conn : clientConnections) {
        if (conn->active.load()) {
            clients.push_back(conn->clientID);
        }
    }
    return clients;
}

uint32_t Server::GetPlayerEntityForClient(uint32_t clientID) const {
    std::lock_guard<std::mutex> lock(clientPlayerMutex);
    auto it = clientPlayerMap.find(clientID);
    if (it != clientPlayerMap.end()) {
        return it->second;
    }
    return 0;
}

void Server::BroadcastEntitySpawn(const EntitySpawnInfo& spawnInfo, uint32_t ownerClientID, uint32_t excludeClientID) {
    std::lock_guard<std::mutex> lock(clientConnectionsMutex);

    // Create a copy with owner set
    EntitySpawnInfo spawnInfoWithOwner = spawnInfo;
    spawnInfoWithOwner.ownerClientID = ownerClientID;

    for (auto& conn : clientConnections) {
        if (conn->active.load() && conn->clientID != excludeClientID) {
            std::lock_guard<std::mutex> queueLock(conn->queueMutex);
            conn->spawnQueue.push_back(spawnInfoWithOwner);
        }
    }

    std::cout << "Broadcasted entity spawn (ID: " << spawnInfo.entityID << ") to "
              << clientConnections.size() << " clients (owner: " << ownerClientID << ")\n";
}

void Server::BroadcastEntityDespawn(uint32_t entityID, uint32_t excludeClientID) {
    std::lock_guard<std::mutex> lock(clientConnectionsMutex);

    for (auto& conn : clientConnections) {
        if (conn->active.load() && conn->clientID != excludeClientID) {
            std::lock_guard<std::mutex> queueLock(conn->queueMutex);
            conn->despawnQueue.push_back(entityID);
        }
    }

    std::cout << "Broadcasted entity despawn (ID: " << entityID << ") to "
              << clientConnections.size() << " clients\n";
}

void Server::SendWorldStateToClient(uint32_t clientID) {
    // Get all entities from entity manager
    std::vector<Entity> entities = serverEntityManager.GetEntitiesCopy();

    std::cout << "Sending world state to client " << clientID
              << " (" << entities.size() << " entities)\n";

    // For each entity, create a spawn message and queue it
    for (const Entity& entity : entities) {
        EntitySpawnInfo spawnInfo;
        spawnInfo.entityID = entity.ID;
        spawnInfo.spritePath = entity.spritePath;
        spawnInfo.totalFrames = entity.totalFrames;
        spawnInfo.fps = entity.fps;
        spawnInfo.position = entity.position;
        spawnInfo.scale = entity.scale;
        spawnInfo.rotation = entity.rotation;
        spawnInfo.physEnabled = entity.physApplied;
        spawnInfo.colliderType = static_cast<int>(entity.collider.type);

        // Queue this spawn for the client
        std::lock_guard<std::mutex> lock(clientConnectionsMutex);
        for (auto& conn : clientConnections) {
            if (conn->clientID == clientID) {
                std::lock_guard<std::mutex> queueLock(conn->queueMutex);
                conn->spawnQueue.push_back(spawnInfo);
                break;
            }
        }
    }
}

}
