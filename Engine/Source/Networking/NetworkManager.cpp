#include "NetworkManager.h"
#include <iostream>

namespace SquareCore {

NetworkManager::NetworkManager() {
}

NetworkManager::~NetworkManager() {
    Disconnect();
}

bool NetworkManager::Connect(const std::string& serverAddress) {
    if (!client.Connect(serverAddress)) {
        std::cout << "NetworkManager: Failed to connect to server\n";
        return false;
    }
    std::cout << "NetworkManager: Connected to server\n";
    return true;
}

void NetworkManager::Disconnect() {
    client.Disconnect();
    serverToLocalEntityMap.clear();
    localToServerEntityMap.clear();
    entitySpriteInfo.clear();
}

void NetworkManager::Update() {
    if (!IsConnected()) {
        return;
    }

    // Update client networking
    client.Update();

    // Process entity spawn/despawn messages
    ProcessPendingSpawns();
    ProcessPendingDespawns();

    // Get latest game state from server
    GameStateSnapshot snapshot = client.GetLatestGameState();

    // Synchronize local entities with server state
    if (!snapshot.entities.empty()) {
        SyncEntitiesFromServer(snapshot);
    }
}

bool NetworkManager::IsConnected() const {
    return client.IsConnected();
}

void NetworkManager::SendInput(const std::unordered_map<std::string, bool>& buttons,
                                const std::unordered_map<std::string, float>& axes) {
    if (IsConnected()) {
        client.SendInput(buttons, axes);
    }
}

uint32_t NetworkManager::GetClientId() const {
    return client.GetClientId();
}

void NetworkManager::ProcessPendingSpawns() {
    if (!entityManagerRef) {
        return;
    }

    // Get spawn messages from client
    std::vector<EntitySpawnInfo> spawns = client.GetPendingSpawns();

    for (const EntitySpawnInfo& spawnInfo : spawns) {
        // Check if we've already spawned this server entity ID
        if (serverToLocalEntityMap.count(spawnInfo.entityID) > 0) {
            continue;  // Already spawned
        }

        // Create entity based on whether it's animated or not
        uint32_t localEntityID = 0;
        if (spawnInfo.totalFrames > 1) {
            // Animated entity
            localEntityID = entityManagerRef->AddAnimatedEntity(
                spawnInfo.spritePath.c_str(),
                spawnInfo.totalFrames,
                spawnInfo.fps,
                spawnInfo.position.x,
                spawnInfo.position.y,
                spawnInfo.rotation,
                spawnInfo.scale.x,
                spawnInfo.scale.y,
                spawnInfo.physEnabled
            );
        } else {
            // Static entity
            localEntityID = entityManagerRef->AddEntity(
                spawnInfo.spritePath.c_str(),
                spawnInfo.position.x,
                spawnInfo.position.y,
                spawnInfo.rotation,
                spawnInfo.scale.x,
                spawnInfo.scale.y,
                spawnInfo.physEnabled
            );
        }

        // Map server ID to local ID
        if (localEntityID != 0) {
            serverToLocalEntityMap[spawnInfo.entityID] = localEntityID;
            localToServerEntityMap[localEntityID] = spawnInfo.entityID;

            entityManagerRef->SetColliderType(localEntityID, static_cast<ColliderType>(spawnInfo.colliderType));
            spawnedEntities.insert(localEntityID);

            // Check if this entity is owned by us
            if (spawnInfo.ownerClientID == GetClientId() && spawnInfo.ownerClientID != 0) {
                localPlayerEntityId = localEntityID;
                std::cout << "NetworkManager: This is our player entity! Local ID: " << localEntityID
                          << " (Server ID: " << spawnInfo.entityID << ")\n";
            }

            std::cout << "NetworkManager: Spawned entity - Server ID: " << spawnInfo.entityID
                      << " -> Local ID: " << localEntityID
                      << " (" << spawnInfo.spritePath << ")\n";
        }
    }
}

void NetworkManager::ProcessPendingDespawns() {
    if (!entityManagerRef) {
        return;
    }

    // Get despawn messages from client
    std::vector<uint32_t> despawns = client.GetPendingDespawns();

    for (uint32_t serverEntityID : despawns) {
        // Translate server entity ID to local entity ID
        if (serverToLocalEntityMap.count(serverEntityID) == 0) {
            continue;  // Not found in mapping
        }

        uint32_t localEntityID = serverToLocalEntityMap[serverEntityID];

        if (spawnedEntities.count(localEntityID) > 0) {
            entityManagerRef->RemoveEntity(localEntityID);
            spawnedEntities.erase(localEntityID);

            // Clean up mappings
            serverToLocalEntityMap.erase(serverEntityID);
            localToServerEntityMap.erase(localEntityID);

            std::cout << "NetworkManager: Despawned entity - Server ID: " << serverEntityID
                      << " (was Local ID: " << localEntityID << ")\n";
        }
    }
}

void NetworkManager::SyncEntitiesFromServer(const GameStateSnapshot& snapshot) {
    if (!entityManagerRef) {
        return;
    }

    // Update all entities from server snapshot
    for (const EntitySnapshot& entitySnap : snapshot.entities) {
        // Translate server entity ID to local entity ID
        if (serverToLocalEntityMap.count(entitySnap.entityID) == 0) {
            continue;  // Entity not spawned yet
        }

        uint32_t localEntityID = serverToLocalEntityMap[entitySnap.entityID];

        // Check if entity exists locally (by local ID)
        Entity* entity = entityManagerRef->GetEntityByID(localEntityID);

        if (entity) {
            // Update entity transform from server
            entity->position = entitySnap.position;
            entity->velocity = entitySnap.velocity;
            entity->scale = entitySnap.scale;
            entity->rotation = entitySnap.rotation;
            entity->flipX = entitySnap.flipX;
            entity->flipY = entitySnap.flipY;
            entity->currentFrame = entitySnap.currentFrame;
        }
    }
}

}
