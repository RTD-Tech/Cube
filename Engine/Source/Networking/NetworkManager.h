#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "Client.h"
#include "NetworkProtocol.h"
#include "Renderer/EntityManager.h"
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace SquareCore {

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    // Connects the client to a server
    bool Connect(const std::string& serverAddress);
    // Disconnects the client from a server
    void Disconnect();
    // Updates the local client
    void Update();
    // Returns if the local client is connected to a server
    bool IsConnected() const;

    // Set EntityManager reference for entity manipulation
    void SetEntityManager(EntityManager* entityManager) { entityManagerRef = entityManager; }

    // Send input to server (client mode)
    void SendInput(const std::unordered_map<std::string, bool>& buttons,
                   const std::unordered_map<std::string, float>& axes = {});

    // Get client ID
    uint32_t GetClientId() const;

    // Get local player entity ID (client mode)
    uint32_t GetLocalPlayerEntity() const { return localPlayerEntityId; }

private:
    // Client instance for server communication
    Client client;

    // EntityManager reference for creating/updating networked entities
    EntityManager* entityManagerRef = nullptr;

    // Map server entity IDs to local entity IDs
    std::unordered_map<uint32_t, uint32_t> serverToLocalEntityMap;
    std::unordered_map<uint32_t, uint32_t> localToServerEntityMap;

    // Track which entity is the local player
    uint32_t localPlayerEntityId = 0;

    // Track entities spawned by network
    std::unordered_set<uint32_t> spawnedEntities;

    // Synchronize entities from server state
    void SyncEntitiesFromServer(const GameStateSnapshot& snapshot);

    // Process pending spawn/despawn messages from server
    void ProcessPendingSpawns();
    void ProcessPendingDespawns();

    // Sprite sheet cache
    struct SpriteInfo {
        std::string path;
        int totalFrames;
        float fps;
    };
    std::unordered_map<uint32_t, SpriteInfo> entitySpriteInfo;
};

}

#endif