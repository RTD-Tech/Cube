#include "EntityManager.h"

#include "algorithm"
#include <iostream>

namespace CubeCore
{
    EntityManager::EntityManager()
    {
    }

    EntityManager::~EntityManager()
    {
        std::lock_guard<std::mutex> lock(entityMutex);
        for (auto& entity : entities)
        {
            if (entity.spriteSheet != nullptr)
            {
                //SDL_DestroyTexture(entity.spriteSheet);
            }
        }
    }

    uint32_t EntityManager::AddEntity(const char* spritePath, float Xpos, float Ypos, float rotation,
                                      float Xscale, float Yscale, bool physEnabled, std::vector<std::string> tags)
    {
        if (!spritePath || spritePath[0] == '\0')
        {
            std::cout << "AddEntity: Invalid sprite path" << std::endl;
            return 0;
        }

        std::lock_guard<std::mutex> lock(entityMutex);

        TextureInfo textureInfo = LoadTexture(spritePath);
        if (!textureInfo.texture && textureInfo.width == 0.0f && textureInfo.height == 0.0f)
        {
            return 0;
        }

        Entity newEntity;
        newEntity.ID = nextEntityID++;
        newEntity.spritePath = spritePath;
        newEntity.spriteSheet = textureInfo.texture;
        newEntity.spriteWidth = textureInfo.width;
        newEntity.spriteHeight = textureInfo.height;
        newEntity.position = Vec2(Xpos, Ypos);
        newEntity.rotation = rotation;
        newEntity.scale = Vec2(Xscale, Yscale);
        newEntity.physApplied = physEnabled;
        newEntity.tags = tags;

        entities.push_back(newEntity);
        idToIndex[newEntity.ID] = entities.size() - 1;

        return newEntity.ID;
    }

    uint32_t EntityManager::AddAnimatedEntity(const char* spritePath, int totalFrames, float fps,
                                              float Xpos, float Ypos, float rotation, float Xscale, float Yscale,
                                              bool physEnabled, std::vector<std::string> tags)
    {
        if (!spritePath || spritePath[0] == '\0')
        {
            std::cout << "AddAnimatedEntity: Invalid sprite path" << std::endl;
            return 0;
        }
        if (totalFrames <= 0)
        {
            std::cout << "AddAnimatedEntity: Invalid totalFrames value: " << totalFrames << std::endl;
            return 0;
        }
        if (fps <= 0.0f)
        {
            std::cout << "AddAnimatedEntity: Invalid fps value: " << fps << std::endl;
            return 0;
        }

        std::lock_guard<std::mutex> lock(entityMutex);

        TextureInfo textureInfo = LoadTexture(spritePath);
        if (!textureInfo.texture && textureInfo.width == 0.0f && textureInfo.height == 0.0f)
        {
            return 0;
        }

        Entity newEntity;
        newEntity.ID = nextEntityID++;
        newEntity.spritePath = spritePath;
        newEntity.spriteSheet = textureInfo.texture;
        newEntity.spriteWidth = textureInfo.width;
        newEntity.spriteHeight = textureInfo.height;
        newEntity.position = Vec2(Xpos, Ypos);
        newEntity.rotation = rotation;
        newEntity.scale = Vec2(Xscale, Yscale);
        newEntity.physApplied = physEnabled;
        newEntity.tags = tags;

        newEntity.totalFrames = totalFrames;
        newEntity.fps = fps;
        newEntity.currentFrame = 0;
        newEntity.elapsedTime = 0.0f;

        entities.push_back(newEntity);
        idToIndex[newEntity.ID] = entities.size() - 1;

        return newEntity.ID;
    }

    uint32_t EntityManager::AddSpritelessEntity(float width, float height, RGBA color, float Xpos, float Ypos,
                                                float rotation, float Xscale, float Yscale, bool physEnabled)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        Entity newEntity;
        newEntity.ID = nextEntityID++;
        newEntity.isSpriteless = true;
        newEntity.spritelessWidth = width;
        newEntity.spritelessHeight = height;
        newEntity.spritelessColor = color;
        newEntity.position = Vec2(Xpos, Ypos);
        newEntity.rotation = rotation;
        newEntity.scale = Vec2(Xscale, Yscale);
        newEntity.physApplied = physEnabled;

        newEntity.collider.size = Vec2(width * Xscale, height * Yscale);
        newEntity.collider.type = ColliderType::SOLID;

        newEntity.spriteSheet = nullptr;
        newEntity.spritePath = "";
        newEntity.spriteWidth = width;
        newEntity.spriteHeight = height;

        entities.push_back(newEntity);
        idToIndex[newEntity.ID] = entities.size() - 1;

        return newEntity.ID;
    }

    void EntityManager::RemoveEntity(uint32_t entityID)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it == idToIndex.end())
        {
            return;
        }

        size_t index = it->second;
        
        if (entities[index].physicsHandle.isValid && physicsRef) {
            uint32_t idToDestroy = entityID;
            entityMutex.unlock();
            physicsRef->DestroyBody(idToDestroy);
            entityMutex.lock();
            
            it = idToIndex.find(entityID);
            if (it == idToIndex.end()) {
                return;
            }
            index = it->second;
        }

        if (entities[index].spriteSheet != nullptr)
        {
            //SDL_DestroyTexture(entities[index].spriteSheet);
        }

        if (index < entities.size() - 1)
        {
            std::swap(entities[index], entities.back());
        }
        entities.pop_back();

        idToIndex.erase(it);
        UpdateIndexMap();
    }

    void EntityManager::ClearEntities()
    {
        std::lock_guard<std::mutex> lock(entityMutex);
        
        for (int i = static_cast<int>(entities.size()) - 1; i >= 0; --i) {
            if (!entities[i].persistent) {
                if (entities[i].spriteSheet) {
                    //SDL_DestroyTexture(entities[i].spriteSheet);
                }
                entities.erase(entities.begin() + i);
            }
        }

        UpdateIndexMap();
    }

    std::vector<Entity> EntityManager::GetEntitiesCopy() const
    {
        std::lock_guard<std::mutex> lock(entityMutex);
        return entities;
    }

    Entity* EntityManager::GetEntityByID(uint32_t ID)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(ID);
        if (it == idToIndex.end())
        {
            return nullptr;
        }

        return &entities[it->second];
    }

    std::vector<uint32_t> EntityManager::GetAllEntitiesWithTag(std::string tag)
    {
        std::lock_guard<std::mutex> lock(entityMutex);
        std::vector<uint32_t> entityIDs;

        for (Entity& entity : entities)
        {
            for (const auto& entityTag : entity.tags)
            {
                if (entityTag == tag)
                {
                    entityIDs.push_back(entity.ID);
                    break;
                }
            }
        }

        return entityIDs;
    }

    uint32_t EntityManager::GetFirstEntityWithTag(std::string tag)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        for (Entity& entity : entities)
        {
            for (const auto& entityTag : entity.tags)
            {
                if (entityTag == tag)
                {
                    return entity.ID;
                }
            }
        }

        return 0;
    }

    size_t EntityManager::GetEntityCount() const
    {
        std::lock_guard<std::mutex> lock(entityMutex);
        return entities.size();
    }

    bool EntityManager::EntityExists(uint32_t ID) const
    {
        std::lock_guard<std::mutex> lock(entityMutex);
        return idToIndex.find(ID) != idToIndex.end();
    }

    bool EntityManager::GetEntityProperty(uint32_t ID, std::function<void(const Entity&)> accessor) const
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(ID);
        if (it == idToIndex.end())
        {
            return false;
        }

        accessor(entities[it->second]);
        return true;
    }

    void EntityManager::UpdateEntityPosition(uint32_t entityID, float newX, float newY)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            entities[it->second].position = Vec2(newX, newY);
        }
        else
        {
            std::cout << "UpdateEntityPosition: Entity ID " << entityID << " not found" << std::endl;
        }
    }

    void EntityManager::FlipSprite(uint32_t entityID, bool flipX, bool flipY)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            entities[it->second].flipX = flipX;
            entities[it->second].flipY = flipY;
        }
        else
        {
            std::cout << "FlipSprite: Entity ID " << entityID << " not found" << std::endl;
        }
    }

    void EntityManager::SetPosition(uint32_t entityID, const Vec2& position)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            entities[it->second].position = position;
            
            if (physicsRef && entities[it->second].physicsHandle.isValid)
            {
                entityMutex.unlock();
                physicsRef->SetColliderPosition(entityID, position);
                entityMutex.lock();
            }
        }
        else
        {
            std::cout << "SetPosition: Entity ID " << entityID << " not found" << std::endl;
        }
    }

    Vec2 EntityManager::GetPosition(uint32_t entityID)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            return entities[it->second].position;
        }
        else
        {
            std::cout << "GetPosition: Entity ID " << entityID << " not found" << std::endl;
            return Vec2::zero();
        }
    }

    void EntityManager::SetScale(uint32_t entityID, const Vec2& scale)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            entities[it->second].scale = scale;
            
            if (physicsRef && entities[it->second].physicsHandle.isValid)
            {
                entityMutex.unlock();
                physicsRef->SetColliderScale(entityID, scale);
                entityMutex.lock();
            }
        }
        else
        {
            std::cout << "SetScale: Entity ID " << entityID << " not found" << std::endl;
        }
    }
    
    Vec2 EntityManager::GetScale(uint32_t entityID)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            return entities[it->second].scale;
        }
        else
        {
            std::cout << "GetScale: Entity ID " << entityID << " not found" << std::endl;
            return Vec2::zero();
        }
    }

    void EntityManager::SetRotation(uint32_t entityID, float rotation)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            entities[it->second].rotation = rotation;
            
            if (physicsRef && entities[it->second].physicsHandle.isValid)
            {
                entityMutex.unlock();
                physicsRef->SetColliderRotation(entityID, rotation);
                entityMutex.lock();
            }
        }
        else
        {
            std::cout << "SetRotation: Entity ID " << entityID << " not found" << std::endl;
        }
    }
    
    float EntityManager::GetRotation(uint32_t entityID)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            return entities[it->second].rotation;
        }
        else
        {
            std::cout << "GetRotation: Entity ID " << entityID << " not found" << std::endl;
            return 0.0f;
        }
    }

    bool EntityManager::GetFlipX(uint32_t entityID) const
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            return entities[it->second].flipX;
        }
        else
        {
            std::cout << "GetFlipX: Entity ID " << entityID << " not found" << std::endl;
            return false;
        }
    }

    bool EntityManager::GetFlipY(uint32_t entityID) const
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            return entities[it->second].flipY;
        }
        else
        {
            std::cout << "GetFlipY: Entity ID " << entityID << " not found" << std::endl;
            return false;
        }
    }

    bool EntityManager::GetFlipState(uint32_t entityID, bool& flipX, bool& flipY) const
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            flipX = entities[it->second].flipX;
            flipY = entities[it->second].flipY;
            return true;
        }
        else
        {
            std::cout << "GetFlipState: Entity ID " << entityID << " not found" << std::endl;
            flipX = false;
            flipY = false;
            return false;
        }
    }

    void EntityManager::ToggleFlipX(uint32_t entityID)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            entities[it->second].flipX = !entities[it->second].flipX;
        }
        else
        {
            std::cout << "ToggleFlipX: Entity ID " << entityID << " not found" << std::endl;
        }
    }

    void EntityManager::ToggleFlipY(uint32_t entityID)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            entities[it->second].flipY = !entities[it->second].flipY;
        }
        else
        {
            std::cout << "ToggleFlipY: Entity ID " << entityID << " not found" << std::endl;
        }
    }

    void EntityManager::SetColliderType(uint32_t entityID, ColliderType type)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            entities[it->second].collider.type = type;
            
            if (physicsRef && entities[it->second].physicsHandle.isValid && type != ColliderType::NONE)
            {
                entityMutex.unlock();
                physicsRef->DestroyBody(entityID);
                physicsRef->CreateBody(entityID);
                entityMutex.lock();
            }
            else if (physicsRef && entities[it->second].physicsHandle.isValid && type == ColliderType::NONE)
            {
                entityMutex.unlock();
                physicsRef->DestroyBody(entityID);
                entityMutex.lock();
            }
        }
        else
        {
            std::cout << "SetColliderType: Entity ID " << entityID << " not found" << std::endl;
        }
    }

    void EntityManager::SetColor(uint32_t entityID, RGBA color)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end()) {
            entities[it->second].color = color;
        }
    }

    void EntityManager::SetEntityPersistent(uint32_t entityID, bool persistent)
    {
        std::lock_guard<std::mutex> lock(entityMutex);
        
        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end()) {
            entities[it->second].persistent = persistent;
        }
    }

    bool EntityManager::EntityHasTag(uint32_t entityID, std::string tag)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            return std::find(entities[it->second].tags.begin(), entities[it->second].tags.end(), tag) != entities[it->second].tags.end();
        }
        
        std::cout << "EntityHasTag: Entity ID " << entityID << " not found" << std::endl;
        return false;
    }

    void EntityManager::SetPhysicsEnabled(uint32_t entityID, bool enabled)
    {
        std::lock_guard<std::mutex> lock(entityMutex);
        
        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            entities[it->second].physApplied = enabled;
            
            if (physicsRef && entities[it->second].physicsHandle.isValid)
            {
                entityMutex.unlock();
                physicsRef->DestroyBody(entityID);
                physicsRef->CreateBody(entityID);
                entityMutex.lock();
            }
        }
    }

    void EntityManager::SetVisible(uint32_t entityID, bool visible)
    {
        std::lock_guard<std::mutex> lock(entityMutex);
        
        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            entities[it->second].visible = visible;
        }
        
    }

    void EntityManager::ResetAnimation(uint32_t entityID)
    {
        std::lock_guard<std::mutex> lock(entityMutex);
        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            entities[it->second].currentFrame = 0;
            entities[it->second].elapsedTime = 0.0f;
        }
    }

    void EntityManager::SetAnimationFPS(uint32_t entityID, float fps)
    {
        std::lock_guard<std::mutex> lock(entityMutex);
        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            Entity& entity = entities[it->second];
            entity.fps = fps;
        }
    }

    void EntityManager::SetAnimationFrame(uint32_t entityID, int frame)
    {
        std::lock_guard<std::mutex> lock(entityMutex);
        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            Entity& entity = entities[it->second];
            if (frame >= 0 && frame < entity.totalFrames)
            {
                entity.currentFrame = frame;
            }
        }
    }

    bool EntityManager::IsAnimationComplete(uint32_t entityID) const
    {
        std::lock_guard<std::mutex> lock(entityMutex);
        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            const Entity& entity = entities[it->second];
            return entity.currentFrame >= entity.totalFrames - 1;
        }
        return false;
    }

    int EntityManager::GetTotalFrames(uint32_t entityID) const
    {
        std::lock_guard<std::mutex> lock(entityMutex);
        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            return entities[it->second].totalFrames;
        }
        return 0;
    }

    void EntityManager::SetZIndex(uint32_t entityID, int zIndex)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            entities[it->second].zIndex = zIndex;
        }
        else
        {
            std::cout << "SetZIndex: Entity ID " << entityID << " not found" << std::endl;
        }
    }

    int EntityManager::GetZIndex(uint32_t entityID)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            return entities[it->second].zIndex;
        }
        
        std::cout << "GetZIndex: Entity ID " << entityID << " not found" << std::endl;
        return 0;
    }

    void EntityManager::AddTagToEntity(uint32_t entityID, std::string tag)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            entities[it->second].tags.push_back(tag);
        }
        else
        {
            std::cout << "AddTagToEntity: Entity ID " << entityID << " not found" << std::endl;
        }
    }

    void EntityManager::RemoveTagFromEntity(uint32_t entityID, std::string tag)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            auto& tagVec = entities[it->second].tags;
            auto tagIt = std::find(tagVec.begin(), tagVec.end(), tag);
            if (tagIt != tagVec.end())
            {
                tagVec.erase(tagIt);
            }
        }
        else
        {
            std::cout << "RemoveTagFromEntity: Entity ID " << entityID << " not found" << std::endl;
        }
    }

    std::vector<Property*> EntityManager::GetAllEntityProperties(uint32_t entityID)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        auto it = idToIndex.find(entityID);
        if (it != idToIndex.end())
        {
            return entities[it->second].properties;
        }
        
        std::cout << "GetAllEntityProperties: Entity ID " << entityID << " not found" << std::endl;
        return {};
    }

    void EntityManager::UpdateAnimations(float deltaTime)
    {
        std::lock_guard<std::mutex> lock(entityMutex);

        for (auto& entity : entities)
        {
            if (entity.totalFrames > 1)
            {
                entity.elapsedTime += deltaTime;
                float frameTime = 1.0f / entity.fps;

                while (entity.elapsedTime >= frameTime)
                {
                    entity.currentFrame = (entity.currentFrame + 1) % entity.totalFrames;
                    entity.elapsedTime -= frameTime;
                }
            }
        }
    }

    void EntityManager::UpdateIndexMap()
    {
        idToIndex.clear();
        for (size_t i = 0; i < entities.size(); ++i)
        {
            idToIndex[entities[i].ID] = i;
        }
    }

    TextureInfo EntityManager::LoadTexture(const char* spritePath)
    {
        TextureInfo result = {nullptr, 0.0f, 0.0f};

        if (headlessMode)
        {
           // SDL_Surface* spriteSheet = IMG_Load(spritePath);

            if (!spriteSheet)
            {
                std::cout << "Failed to load image " << spritePath << std::endl;
                return result;
            }

            result.width = static_cast<float>(spriteSheet->w);
            result.height = static_cast<float>(spriteSheet->h);
            result.texture = nullptr;

            //SDL_DestroySurface(spriteSheet);

            return result;
        }

        if (!rendererRef)
        {
            std::cout << "Renderer not set in EntityManager" << std::endl;
            return result;
        }

        //SDL_Surface* spriteSheet = IMG_Load(spritePath);

        if (!spriteSheet)
        {
            std::cout << "Failed to load image " << spritePath << ": "<< std::endl;
            return result;
        }

        result.width = static_cast<float>(spriteSheet->w);
        result.height = static_cast<float>(spriteSheet->h);

        //result.texture = SDL_CreateTextureFromSurface(rendererRef, spriteSheet);

        //SDL_DestroySurface(spriteSheet);

        if (!result.texture)
        {
            std::cout << "Failed to create texture: " << std::endl;
            result.width = 0.0f;
            result.height = 0.0f;
            return result;
        }

        return result;
    }
}