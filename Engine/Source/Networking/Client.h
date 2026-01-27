#ifndef CLIENT_H
#define CLIENT_H

#include "NetworkProtocol.h"
#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <atomic>

namespace SquareCore {

class Client {
public:
    Client();
    ~Client();

    // Connect to server
    bool Connect(const std::string& serverAddress);
    // Disconnect from server gracefully
    void Disconnect();
    // Update client networking
    void Update();

    // Send input to server (thread-safe)
    void SendInput(const std::unordered_map<std::string, bool>& buttons,
                   const std::unordered_map<std::string, float>& axes = {});

    // Get latest game state from server (thread-safe)
    GameStateSnapshot GetLatestGameState();

    // Get and clear pending entity spawn messages (thread-safe)
    std::vector<EntitySpawnInfo> GetPendingSpawns();
    // Get and clear pending entity despawn messages (thread-safe)
    std::vector<uint32_t> GetPendingDespawns();

    // Check if connected to server
    bool IsConnected() const { return connected.load() && clientId.load() != 0; }
    // Get this client's ID
    uint32_t GetClientId() const { return clientId.load(); }

private:
    // Thread-safe connection state
    std::atomic<bool> connected{false};
    std::atomic<uint32_t> clientId{0};
    std::atomic<bool> disconnecting{false};

    // Latest input to send
    InputState pendingInput;
    mutable std::mutex inputMutex;

    // Latest game state received
    GameStateSnapshot latestState;
    mutable std::mutex stateMutex;

    // Pending entity spawn/despawn messages
    std::vector<EntitySpawnInfo> pendingSpawns;
    std::vector<uint32_t> pendingDespawns;
    mutable std::mutex pendingMessagesMutex;

    // Mutex for socket synchronization
    mutable std::mutex socketMutex;

    // Connection timing
    std::chrono::time_point<std::chrono::steady_clock> lastUpdate;
    static constexpr int UPDATE_INTERVAL_MS = 16;

    // Send input and receive game state
    void SendInputAndReceiveState();

    // Socket management
    void InitializeSockets(const std::string& serverAddress);
    void CleanupSockets();

};

}

#endif
