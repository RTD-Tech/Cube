#ifndef ENTITIES_H
#define ENTITIES_H

#include "Math/Math.h"
#include "UI/Color.h"
#include "Physics/PhysicsTypes.h"
#include <SDL3/SDL_render.h>
#include <box2d/box2d.h>
#include <vector>
#include <string>

namespace SquareCore {

// Enum for collider types
enum class ColliderType {
    NONE,     // No collision detection
    SOLID,    // Full physics collision with position resolution
    TRIGGER   // Detection only, no position resolution
};

// Struct to define entity collider properties
struct Collider {
    ColliderType type = ColliderType::SOLID;

    // Collision properties
    bool enabled = true;
    Vec2 offset = Vec2::zero();        // Offset from the center of the entity
    Vec2 size = Vec2::zero();          // Custom size (0,0 = use sprite size)

    // Add a collision record to the collision vector for this entity
    void AddCollision(uint32_t entityID, int side);
    // Check if this entity is colliding with another entity
    bool IsCollidingWith(uint32_t entityID) const;
    // Check if this entity has a collision on a specific side
    bool HasCollisionOnSide(int side) const;
    // Clear all collision records for this entity
    void ClearCollisions();
    // Get all collision records for this entity
    const std::vector<std::pair<uint32_t, int>>& GetCollisions() const;

private:
    // Vector to store collision records for this entity
    std::vector<std::pair<uint32_t, int>> collisions;
};

struct Property
{
    virtual ~Property();
};

struct PhysicsHandle
{
    b2BodyId bodyId;
    b2ShapeId shapeId;
    bool isValid = false;
};

// Data-only struct that defines variables for entities
struct Entity {
    uint32_t ID = 0;                   // Internal identifier (default 0 for invalid entity)
    std::string spritePath;            // Path to sprite file (for replication)
    SDL_Texture* spriteSheet;          // Spritesheet to use for the entity sprite
    float spriteWidth;                 // Width of sprite frame(s)
    float spriteHeight;                // Height of sprite frame(s)

    bool visible = true;
    bool persistent = false;           // If true, this entity will survive scene transitions

    // Sprite animation
    int currentFrame = 0;              // Current animation frame
    int totalFrames = 1;               // Total frames in the animation
    float fps = 0.0f;                  // Frames per second
    float elapsedTime = 0.0f;          // Time tracking for animations
    
    // Spriteless rendering support
    bool isSpriteless = false;         // Flag to indicate this is a spriteless entity
    RGBA spritelessColor = RGBA(0, 0, 0, 255);
    float spritelessWidth = 10.0f;      // Width of spriteless entity
    float spritelessHeight = 10.0f;     // Height of spriteless entity

    RGBA color = RGBA(255, 255, 255, 255);
    int zIndex = 0;

    // Transform
    Vec2 position = Vec2::zero();      // Position (default: Vec2::zero())
    float rotation = 0.0f;             // Rotation in degrees (default: 0.0)
    Vec2 scale = Vec2::one();          // Scale (default: Vec2::one())
    bool flipX = false;                // Horizontal flip
    bool flipY = false;                // Vertical flip

    // Physics
    Vec2 velocity = Vec2::zero();      // Velocity vector
    Vec2 acceleration = Vec2::zero();  // Acceleration vector
    bool physApplied = false;          // Whether physics is applied to this entity
    float mass = 1.0f;                 // Mass for physics calculations
    float drag = 0.0f;                 // Air resistance/drag coefficient
    float gravityScale = 1.0f;         // Gravity scale multiplier for this entity
    bool fixedRotation = true;

    // Collision
    Collider collider;                 // The collider for this entity
    PhysicsHandle physicsHandle;
    ColliderShapeData shapeData;

    std::vector<std::string> tags;
    std::vector<Property*> properties;
};

}

#endif