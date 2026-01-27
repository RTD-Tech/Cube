#ifndef NETWORKPROTOCOL_H
#define NETWORKPROTOCOL_H

#include "Math/Math.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <cstdint>

namespace SquareCore {

// Generic input state
struct InputState {
    uint32_t clientID = 0;
    std::unordered_map<std::string, bool> buttons;
    std::unordered_map<std::string, float> axes;
    uint64_t timestamp = 0;

    // Serialization
    std::string Serialize() const {
        std::ostringstream oss;
        oss << clientID << " " << timestamp << " " << buttons.size() << " " << axes.size();

        for (const auto& [key, value] : buttons) {
            oss << " " << key << " " << (value ? 1 : 0);
        }

        for (const auto& [key, value] : axes) {
            oss << " " << key << " " << value;
        }

        return oss.str();
    }

    static InputState Deserialize(const std::string& data) {
        InputState input;
        std::istringstream iss(data);

        size_t buttonCount, axesCount;
        iss >> input.clientID >> input.timestamp >> buttonCount >> axesCount;

        for (size_t i = 0; i < buttonCount; ++i) {
            std::string key;
            int value;
            iss >> key >> value;
            input.buttons[key] = (value != 0);
        }

        for (size_t i = 0; i < axesCount; ++i) {
            std::string key;
            float value;
            iss >> key >> value;
            input.axes[key] = value;
        }

        return input;
    }
};

// Generic entity snapshot
struct EntitySnapshot {
    uint32_t entityID = 0;
    Vec2 position = Vec2::zero();
    Vec2 velocity = Vec2::zero();
    Vec2 scale = Vec2::one();
    float rotation = 0.0f;
    bool flipX = false;
    bool flipY = false;
    int currentFrame = 0;

    // Serialization
    std::string Serialize() const {
        std::ostringstream oss;
        oss << entityID << " "
            << position.x << " " << position.y << " "
            << velocity.x << " " << velocity.y << " "
            << scale.x << " " << scale.y << " "
            << rotation << " "
            << (flipX ? 1 : 0) << " " << (flipY ? 1 : 0) << " "
            << currentFrame;
        return oss.str();
    }

    static EntitySnapshot Deserialize(std::istringstream& iss) {
        EntitySnapshot snapshot;
        int flipXInt, flipYInt;

        iss >> snapshot.entityID
            >> snapshot.position.x >> snapshot.position.y
            >> snapshot.velocity.x >> snapshot.velocity.y
            >> snapshot.scale.x >> snapshot.scale.y
            >> snapshot.rotation
            >> flipXInt >> flipYInt
            >> snapshot.currentFrame;

        snapshot.flipX = (flipXInt != 0);
        snapshot.flipY = (flipYInt != 0);

        return snapshot;
    }
};

// Complete game state snapshot
struct GameStateSnapshot {
    std::vector<EntitySnapshot> entities;
    std::unordered_map<uint32_t, uint32_t> playerEntityBindings;  // clientID -> entityID
    uint64_t timestamp = 0;

    // Serialization
    std::string Serialize() const {
        std::ostringstream oss;
        oss << timestamp << " " << entities.size() << " " << playerEntityBindings.size();

        for (const auto& entity : entities) {
            oss << " " << entity.Serialize();
        }

        for (const auto& [clientID, entityID] : playerEntityBindings) {
            oss << " " << clientID << " " << entityID;
        }

        return oss.str();
    }

    static GameStateSnapshot Deserialize(const std::string& data) {
        GameStateSnapshot snapshot;
        std::istringstream iss(data);

        size_t entityCount, bindingCount;
        iss >> snapshot.timestamp >> entityCount >> bindingCount;

        for (size_t i = 0; i < entityCount; ++i) {
            snapshot.entities.push_back(EntitySnapshot::Deserialize(iss));
        }

        for (size_t i = 0; i < bindingCount; ++i) {
            uint32_t clientID, entityID;
            iss >> clientID >> entityID;
            snapshot.playerEntityBindings[clientID] = entityID;
        }

        return snapshot;
    }
};

// Entity spawn information
struct EntitySpawnInfo {
    uint32_t entityID = 0;
    std::string spritePath;
    int totalFrames = 1;
    float fps = 0.0f;
    Vec2 position = Vec2::zero();
    Vec2 scale = Vec2::one();
    float rotation = 0.0f;
    bool physEnabled = false;
    int colliderType = 0;
    uint32_t ownerClientID = 0;  // 0 = shared, non-zero = owned by that client

    // Serialization
    std::string Serialize() const {
        std::ostringstream oss;
        oss << entityID << " "
            << "\"" << spritePath << "\" "
            << totalFrames << " "
            << fps << " "
            << position.x << " " << position.y << " "
            << scale.x << " " << scale.y << " "
            << rotation << " "
            << (physEnabled ? 1 : 0) << " "
            << colliderType << " "
            << ownerClientID;
        return oss.str();
    }

    static EntitySpawnInfo Deserialize(const std::string& data) {
        EntitySpawnInfo info;
        std::istringstream iss(data);
        int physInt;

        // Read entityID
        iss >> info.entityID;

        // Read quoted sprite path
        iss.ignore();  // Skip space before quote
        std::getline(iss, info.spritePath, '"');
        std::getline(iss, info.spritePath, '"');

        // Read remaining fields
        iss >> info.totalFrames
            >> info.fps
            >> info.position.x >> info.position.y
            >> info.scale.x >> info.scale.y
            >> info.rotation
            >> physInt
            >> info.colliderType
            >> info.ownerClientID;

        info.physEnabled = (physInt != 0);

        return info;
    }
};

// Message types for network protocol
enum class MessageType {
    CONNECT,
    DISCONNECT,
    INPUT,              // Client -> Server
    GAME_STATE,         // Server -> Client
    SPAWN_ENTITY,       // Server -> Client (spawn new entity)
    DESPAWN_ENTITY      // Server -> Client (remove entity)
};

// Helper to create protocol messages
inline std::string CreateMessage(MessageType type, const std::string& payload = "") {
    std::ostringstream oss;
    oss << static_cast<int>(type);
    if (!payload.empty()) {
        oss << " " << payload;
    }
    return oss.str();
}

// Helper to parse protocol messages
inline bool ParseMessage(const std::string& message, MessageType& type, std::string& payload) {
    std::istringstream iss(message);
    int typeInt;

    if (!(iss >> typeInt)) {
        return false;
    }

    type = static_cast<MessageType>(typeInt);

    // Get the rest of the message as payload
    if (iss.peek() == ' ') {
        iss.ignore();
    }
    std::getline(iss, payload);

    return true;
}

}

#endif
