#ifndef PHYSICS_H
#define PHYSICS_H

#include "Math/Math.h"
#include "PhysicsTypes.h"
#include "Renderer/Entity.h"
#include "Renderer/EntityManager.h"
#include <box2d/box2d.h>
#include <vector>
#include <unordered_map>

namespace SquareCore{

class EntityManager;

class Physics {
public:
    Physics();
    ~Physics();
    
    void Initialize();
    void Shutdown();
    
    void SetEntityManager(EntityManager* entityManager) { entityManagerRef = entityManager; }
    
    void Update(float fixedDeltaTime);
    
    void SetGravity(float gravity);
    float GetGravity() const { return gravityAmount; }
    Vec2 GetGravityVector() const;
    
    void CreateBody(uint32_t entityID);
    void DestroyBody(uint32_t entityID);
    void ClearBodies();
    
    void SetColliderBox(uint32_t entityID, float halfWidth, float halfHeight);
    void SetColliderCircle(uint32_t entityID, float radius, Vec2 center = Vec2::zero());
    void SetColliderCapsule(uint32_t entityID, Vec2 center1, Vec2 center2, float radius);
    void SetColliderPolygon(uint32_t entityID, std::vector<Vec2>& vertices);
    
    void SetColliderPosition(uint32_t entityID, Vec2 position);
    void SetColliderRotation(uint32_t entityID, float rotation);
    void SetColliderScale(uint32_t entityID, Vec2 scale);
    
    void ApplyForce(uint32_t entityID, const Vec2& force);
    void ApplyImpulse(uint32_t entityID, const Vec2& impulse);
    void SetVelocity(uint32_t entityID, const Vec2& velocity);
    
    void SetMass(uint32_t entityID, float mass);
    void SetDrag(uint32_t entityID, float drag);
    void SetGravityScale(uint32_t entityID, float gravityScale);
    void SetFixedRotation(uint32_t entityID, bool fixed);

private:
    b2WorldId worldId;
    
    EntityManager* entityManagerRef = nullptr;
    
    float gravityAmount = -981.0f;
    
    std::unordered_map<uint32_t, std::vector<CollisionInfo>> collisionMap;
    std::unordered_map<int64_t, uint32_t> shapeToEntityMap;

    void CreateBodyInternal(Entity& entity);
    void DestroyBodyInternal(Entity& entity);
    void SetColliderShape(uint32_t entityID, const ColliderShapeData& shapeData);
    void SyncBodyToEntity(Entity& entity);
    void SyncEntityToBody(Entity& entity);
    void UpdateCollisions(std::vector<Entity>& entities);
    
    void ProcessContactEvents(std::vector<Entity>& entities);
    void ProcessSensorEvents();
    
    int ComputeCollisionSide(const b2Vec2& normal) const;
    uint32_t GetEntityFromShape(b2ShapeId shapeId) const;
    
    void RegisterShape(b2ShapeId shapeId, uint32_t entityID);
    void UnregisterShape(b2ShapeId shapeId);
    b2ShapeId CreateShapeForBody(b2BodyId bodyId, const Entity& entity);
    
    Vec2 ToMeters(const Vec2& val);
    Vec2 ToCentimeters(const Vec2& val);
    float ToMeters(float centimeters);
    float ToCentimeters(float meters);
    float ToRadians(float degrees) const;
    float ToDegrees(float radians) const;
};

}

#endif