#ifndef SERVER_H
#define SERVER_H

#include "ServerInputManager.h"
#include "NetworkProtocol.h"
#include "Renderer/EntityManager.h"
#include "Physics/Physics.h"
#include "Core/Timeline.h"
#include <unordered_map>
#include <string>
#include <chrono>
#include <mutex>
#include <atomic>
#include <thread>
#include <queue>

namespace SquareCore {

// Forward declaration
class Script;

// Client connection data
struct ClientConnection {
    uint32_t clientID;
    std::thread thread;
    std::atomic<bool> active{true};

    // Message queues for entity spawn/despawn
    std::vector<EntitySpawnInfo> spawnQueue;
    std::vector<uint32_t> despawnQueue;
    std::mutex queueMutex;
};

class Server {
public:
    Server();
    ~Server();

    // Starts the server with game logic instance
    void Start(const std::vector<Script*>& scripts);
    // Stops the server gracefully
    void Stop();

    // Get server's entity manager (for game logic access)
    EntityManager& GetEntityManager() { return serverEntityManager; }
    // Get server's physics system (for game logic access)
    Physics& GetPhysics() { return serverPhysics; }
    // Get server's timeline (for game logic access)
    Timeline& GetTimeline() { return serverTimeline; }
    // Get server's input manager (for game logic access)
    ServerInputManager& GetInputManager() { return inputManager; }

    // Mark an entity as controlled by a client
    void RegisterPlayerEntity(uint32_t clientID, uint32_t entityID);
    // Unregister a player entity
    void UnregisterPlayerEntity(uint32_t clientID);

    // Get connected client IDs
    std::vector<uint32_t> GetConnectedClients() const;
    // Get player entity ID for a client
    uint32_t GetPlayerEntityForClient(uint32_t clientID) const;

    // Entity spawn/despawn broadcasting
    void BroadcastEntitySpawn(const EntitySpawnInfo& spawnInfo, uint32_t ownerClientID = 0, uint32_t excludeClientID = 0);
    void BroadcastEntityDespawn(uint32_t entityID, uint32_t excludeClientID = 0);

private:
    // Game simulation components
    EntityManager serverEntityManager;
    Physics serverPhysics;
    Timeline serverTimeline;
    ServerInputManager inputManager;
    std::vector<Script*> scripts;

    // Client management
    std::unordered_map<uint32_t, uint32_t> clientPlayerMap;  // clientID -> player entityID
    mutable std::mutex clientPlayerMutex;

    std::vector<std::unique_ptr<ClientConnection>> clientConnections;
    mutable std::mutex clientConnectionsMutex;

    // Next available client ID
    std::atomic<uint32_t> nextClientID{1};

    // Server running state
    std::atomic<bool> running{false};

    // Game state queue for sending to clients
    struct GameStatePacket {
        GameStateSnapshot snapshot;
        std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;
    };
    std::queue<GameStatePacket> stateQueue;
    mutable std::mutex stateQueueMutex;

    // Main simulation loop (runs game logic at 60Hz)
    void SimulationLoop();

    // Per-client thread function
    void ClientThread(uint32_t clientID, void* socket);

    // Handle client connection
    uint32_t HandleConnect(void* socket);
    // Handle client disconnection
    void HandleDisconnect(uint32_t clientID);

    // Serialize current game state
    GameStateSnapshot CaptureGameState();

    // Send world state to newly connected client
    void SendWorldStateToClient(uint32_t clientID);

    // Socket management
    void InitializeSockets();
    void CleanupSockets();

    // Connection listener thread
    void ConnectionListenerThread();

    // Fixed timestep for simulation
    static constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
};

}

#endif