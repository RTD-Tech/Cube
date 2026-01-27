#include "Script.h"
#include "Networking/Server.h"
#include "Networking/ServerInputManager.h"
#include "Networking/NetworkProtocol.h"

namespace SquareCore
{
    void Script::Quit()
    {
        SDL_Event quit_event;
        quit_event.type = SDL_EVENT_QUIT;
        SDL_PushEvent(&quit_event);
    }
    
    std::vector<uint32_t> Script::GetAllEntitiesWithTag(std::string tag)
    {
        if (entityManagerRef) return entityManagerRef->GetAllEntitiesWithTag(tag);
        return {};
    }

    uint32_t Script::GetFirstEntityWithTag(std::string tag)
    {
        if (entityManagerRef) return entityManagerRef->GetFirstEntityWithTag(tag);
        return 0;
    }

    uint32_t Script::AddEntity(const char* spritePath, float Xpos, float Ypos, float rotation,
                                      float Xscale, float Yscale, bool physEnabled, std::vector<std::string> tags)
    {
        if (entityManagerRef)
        {
            return entityManagerRef->AddEntity(spritePath, Xpos, Ypos, rotation, Xscale, Yscale, physEnabled, tags);
        }
        return 0;
    }

    uint32_t Script::AddAnimatedEntity(const char* spritePath, int totalFrames, float fps,
                                              float Xpos, float Ypos, float rotation, float Xscale, float Yscale,
                                              bool physEnabled)
    {
        if (entityManagerRef)
        {
            return entityManagerRef->AddAnimatedEntity(spritePath, totalFrames, fps, Xpos, Ypos, rotation, Xscale,
                                                       Yscale, physEnabled);
        }
        return 0;
    }

    uint32_t Script::AddSpritelessEntity(float width, float height, RGBA color, float Xpos, float Ypos,
                                                float rotation, float Xscale, float Yscale, bool physEnabled)
    {
        if (entityManagerRef)
        {
            return entityManagerRef->AddSpritelessEntity(width, height, color, Xpos, Ypos, rotation, Xscale, Yscale,
                                                         physEnabled);
        }
        return 0;
    }

    void Script::RemoveEntity(uint32_t entityID)
    {
        if (entityManagerRef && physicsRef)
        {
            physicsRef->DestroyBody(entityID);
            entityManagerRef->RemoveEntity(entityID);
        }
    }

    void Script::UpdateEntityPosition(uint32_t entityID, float newX, float newY)
    {
        if (entityManagerRef)
        {
            entityManagerRef->UpdateEntityPosition(entityID, newX, newY);
        }
    }

    std::vector<std::pair<uint32_t, int>> Script::GetEntityCollisions(uint32_t entityID)
    {
        if (entityManagerRef)
        {
            std::vector<std::pair<uint32_t, int>> result;
            bool success = entityManagerRef->GetEntityProperty(entityID, [&result](const Entity& entity)
            {
                result = entity.collider.GetCollisions();
            });
            if (success)
            {
                return result;
            }
        }
        return {};
    }

    void Script::FlipSprite(uint32_t entityID, bool flipX, bool flipY)
    {
        if (entityManagerRef)
        {
            entityManagerRef->FlipSprite(entityID, flipX, flipY);
        }
    }

    bool Script::GetFlipX(uint32_t entityID)
    {
        if (entityManagerRef)
        {
            return entityManagerRef->GetFlipX(entityID);
        }
        return false;
    }

    bool Script::GetFlipY(uint32_t entityID)
    {
        if (entityManagerRef)
        {
            return entityManagerRef->GetFlipY(entityID);
        }
        return false;
    }

    bool Script::GetFlipState(uint32_t entityID, bool& flipX, bool& flipY)
    {
        if (entityManagerRef)
        {
            return entityManagerRef->GetFlipState(entityID, flipX, flipY);
        }
        flipX = false;
        flipY = false;
        return false;
    }

    void Script::ToggleFlipX(uint32_t entityID)
    {
        if (entityManagerRef)
        {
            entityManagerRef->ToggleFlipX(entityID);
        }
    }

    void Script::ToggleFlipY(uint32_t entityID)
    {
        if (entityManagerRef)
        {
            entityManagerRef->ToggleFlipY(entityID);
        }
    }

    void Script::SetColliderType(uint32_t entityID, ColliderType type)
    {
        if (entityManagerRef)
        {
            entityManagerRef->SetColliderType(entityID, type);
        }
    }

    void Script::SetEntityColor(uint32_t entityID, RGBA color)
    {
        if (entityManagerRef) entityManagerRef->SetColor(entityID, color);
    }

    void Script::SetEntityVisible(uint32_t entityID, bool visible)
    {
        if (entityManagerRef) entityManagerRef->SetVisible(entityID, visible);
    }

    void Script::SetEntityPersistent(uint32_t entityID, bool persistent)
    {
        if (entityManagerRef) entityManagerRef->SetEntityPersistent(entityID, persistent);
    }

    void Script::ResetAnimation(uint32_t entityID)
    {
        if (entityManagerRef) entityManagerRef->ResetAnimation(entityID);
    }

    void Script::SetAnimationFPS(uint32_t entityID, float fps)
    {
        if (entityManagerRef) entityManagerRef->SetAnimationFPS(entityID, fps);
    }

    void Script::SetAnimationFrame(uint32_t entityID, int frame)
    {
        if (entityManagerRef) entityManagerRef->SetAnimationFrame(entityID, frame);
    }

    bool Script::IsAnimationComplete(uint32_t entityID) const
    {
        if (entityManagerRef) return entityManagerRef->IsAnimationComplete(entityID);
        return false;
    }

    int Script::GetTotalFrames(uint32_t entityID) const
    {
        if (entityManagerRef) return entityManagerRef->GetTotalFrames(entityID);
        return 0;
    }

    bool Script::EntityHasTag(uint32_t entityID, std::string tag)
    {
        if (entityManagerRef) return entityManagerRef->EntityHasTag(entityID, tag);
        return false;
    }

    void Script::AddTagToEntity(uint32_t entityID, std::string tag)
    {
        if (entityManagerRef) entityManagerRef->AddTagToEntity(entityID, tag);
    }

    void Script::RemoveTagFromEntity(uint32_t entityID, std::string tag)
    {
        if (entityManagerRef) entityManagerRef->RemoveTagFromEntity(entityID, tag);
    }

    void Script::AddPropertyToEntity(uint32_t entityID, Property* property)
    {
        if (entityManagerRef) entityManagerRef->AddPropertyToEntity(entityID, property);
    }

    std::vector<Property*> Script::GetAllEntityProperties(uint32_t entityID)
    {
        if (entityManagerRef) return entityManagerRef->GetAllEntityProperties(entityID);
        return {};
    }

    void Script::SetZIndex(uint32_t entityID, int zIndex)
    {
        if (entityManagerRef) entityManagerRef->SetZIndex(entityID, zIndex);
    }

    int Script::GetZIndex(uint32_t entityID)
    {
        if (entityManagerRef) return entityManagerRef->GetZIndex(entityID);
        return 0;
    }

    bool Script::EntityExists(uint32_t ID) const
    {
        if (entityManagerRef) return entityManagerRef->EntityExists(ID);
        return false;
    }

    bool Script::GetKeyPressed(SDL_Scancode key)
    {
        if (inputRef)
        {
            return inputRef->GetKeyPressed(key);
        }
        return false;
    }

    bool Script::GetKeyHeld(SDL_Scancode key)
    {
        if (inputRef)
        {
            return inputRef->GetKeyHeld(key);
        }
        return false;
    }

    bool Script::GetKeyReleased(SDL_Scancode key)
    {
        if (inputRef)
        {
            return inputRef->GetKeyReleased(key);
        }
        return false;
    }

    bool Script::GetMouseButtonPressed(int button)
    {
        if (inputRef)
        {
            return inputRef->GetMouseButtonPressed(button);
        }
        return false;
    }

    bool Script::GetMouseButtonHeld(int button)
    {
        if (inputRef)
        {
            return inputRef->GetMouseButtonHeld(button);
        }
        return false;
    }

    bool Script::GetMouseButtonReleased(int button)
    {
        if (inputRef)
        {
            return inputRef->GetMouseButtonReleased(button);
        }
        return false;
    }

    Vec2 Script::GetMousePosition()
    {
        if (inputRef)
        {
            return inputRef->GetMousePosition();
        }
        return {};
    }

    Vec2 Script::GetMouseDelta()
    {
        if (inputRef) return inputRef->GetMouseDelta();
        return {};
    }

    Vec2 Script::GetMouseScroll()
    {
        if (inputRef) return inputRef->GetMouseScroll();
        return {};
    }

    void Script::SetPhysicsEnabled(uint32_t entityID, bool enabled)
    {
        if (entityManagerRef) entityManagerRef->SetPhysicsEnabled(entityID, enabled);
    }

    void Script::SetBackgroundColor(RGBA color)
    {
        if (rendererRef) rendererRef->SetBackgroundColor(color);
    }

    void Script::SetMouseVisible(bool visible)
    {
        if (visible) SDL_ShowCursor();
        else SDL_HideCursor();
    }
    
    void Script::SetColliderBox(uint32_t entityID, float halfWidth, float halfHeight)
    {
        if (physicsRef)
        {
            physicsRef->SetColliderBox(entityID, halfWidth, halfHeight);
        }
    }

    void Script::SetColliderCircle(uint32_t entityID, float radius)
    {
        if (physicsRef)
        {
            physicsRef->SetColliderCircle(entityID, radius);
        }
    }

    void Script::SetColliderCapsule(uint32_t entityID, float height, float radius)
    {
        if (physicsRef)
        {
            float halfHeight = height / 2.0f - radius;
            Vec2 c1(0.0f, -halfHeight);
            Vec2 c2(0.0f, halfHeight);
            physicsRef->SetColliderCapsule(entityID, c1, c2, radius);
        }
    }
    
    void Script::SetColliderPolygon(uint32_t entityID, std::vector<Vec2> vertices)
    {
        if (physicsRef)
        {
            physicsRef->SetColliderPolygon(entityID, vertices);
        }
    }

    void Script::SetGravity(float gravity)
    {
        if (physicsRef) physicsRef->SetGravity(gravity);
    }

    float Script::GetGravity()
    {
        if (physicsRef) return physicsRef->GetGravity();
        return 0.0f;
    }

    void Script::ApplyForce(uint32_t entityID, float forceX, float forceY)
    {
        if (physicsRef)
        {
            physicsRef->ApplyForce(entityID, Vec2(forceX, forceY));
        }
    }

    void Script::ApplyImpulse(uint32_t entityID, float impulseX, float impulseY)
    {
        if (physicsRef)
        {
            physicsRef->ApplyImpulse(entityID, Vec2(impulseX, impulseY));
        }
    }

    void Script::SetVelocity(uint32_t entityID, float velX, float velY)
    {
        if (physicsRef)
        {
            physicsRef->SetVelocity(entityID, Vec2(velX, velY));
        }
    }

    void Script::SetMass(uint32_t entityID, float mass)
    {
        if (physicsRef)
        {
            physicsRef->SetMass(entityID, mass);
        }
    }

    void Script::SetDrag(uint32_t entityID, float drag)
    {
        if (physicsRef)
        {
            physicsRef->SetDrag(entityID, drag);
        }
    }

    void Script::SetGravityScale(uint32_t entityID, float gravityScale)
    {
        if (physicsRef)
        {
            physicsRef->SetGravityScale(entityID, gravityScale);
        }
    }

    void Script::SetFixedRotation(uint32_t entityID, bool fixed)
    {
        if (physicsRef)
        {
            physicsRef->SetFixedRotation(entityID, fixed);
        }
    }

    Vec2 Script::GetVelocity(uint32_t entityID)
    {
        if (entityManagerRef)
        {
            Entity* entity = entityManagerRef->GetEntityByID(entityID);
            if (entity) return entity->velocity;
        }
        return Vec2::zero();
    }

    void Script::SetPosition(uint32_t entityID, float newX, float newY)
    {
        if (entityManagerRef) entityManagerRef->SetPosition(entityID, Vec2(newX, newY));
    }

    Vec2 Script::GetPosition(uint32_t entityID)
    {
        if (entityManagerRef)
        {
            return entityManagerRef->GetPosition(entityID);
        }
        return Vec2::zero();
    }

    void Script::SetScale(uint32_t entityID, const Vec2& scale)
    {
        if (entityManagerRef) entityManagerRef->SetScale(entityID, scale);
    }

    Vec2 Script::GetScale(uint32_t entityID)
    {
        if (entityManagerRef)
        {
            return entityManagerRef->GetScale(entityID);
        }
        return Vec2::zero();
    }

    void Script::SetRotation(uint32_t entityID, float rotation)
    {
        if (entityManagerRef) entityManagerRef->SetRotation(entityID, rotation);
    }

    float Script::GetRotation(uint32_t entityID)
    {
        if (entityManagerRef)
        {
            return entityManagerRef->GetRotation(entityID);
        }
        return 0.0f;
    }

    void Script::ToggleScalingMode()
    {
        if (rendererRef) rendererRef->ToggleScalingMode();
    }

    void Script::ToggleDebugCollisions()
    {
        if (rendererRef) rendererRef->ToggleDebugCollisions();
    }

    void Script::SetCameraPosition(const Vec2& pos)
    {
        if (rendererRef) rendererRef->GetCamera().SetPosition(pos);
    }

    void Script::SetCameraPosition(float x, float y)
    {
        SetCameraPosition(Vec2(x, y));
    }

    Vec2 Script::GetCameraPosition() const
    {
        if (rendererRef) return rendererRef->GetCamera().GetPosition();
        return Vec2::zero();
    }

    void Script::MoveCamera(const Vec2& delta)
    {
        if (rendererRef) rendererRef->GetCamera().Move(delta);
    }

    void Script::MoveCamera(float deltaX, float deltaY)
    {
        MoveCamera(Vec2(deltaX, deltaY));
    }

    void Script::SnapCameraToPosition(const Vec2& pos)
    {
        if (rendererRef) rendererRef->GetCamera().SnapToPosition(pos);
    }

    void Script::SetCameraZoom(float zoom)
    {
        if (rendererRef) rendererRef->GetCamera().SetZoom(zoom);
    }

    float Script::GetCameraZoom() const
    {
        if (rendererRef) return rendererRef->GetCamera().GetZoom();
        return 1.0f;
    }

    void Script::SetCameraZoomLimits(float min, float max)
    {
        if (rendererRef)
        {
            rendererRef->GetCamera().SetZoomLimits(min, max);
        }
    }

    void Script::SetCameraBounds(const Vec2& min, const Vec2& max)
    {
        if (rendererRef)
        {
            rendererRef->GetCamera().SetBounds(min, max);
        }
    }

    void Script::SetCameraBounds(float minX, float minY, float maxX, float maxY)
    {
        SetCameraBounds(Vec2(minX, minY), Vec2(maxX, maxY));
    }

    void Script::EnableCameraBounds(bool enable)
    {
        if (rendererRef)
        {
            rendererRef->GetCamera().EnableBounds(enable);
        }
    }

    void Script::FollowCameraTarget(const Vec2& targetPos, float smoothing, float deltaTime)
    {
        if (rendererRef)
        {
            rendererRef->GetCamera().FollowTarget(targetPos, smoothing, deltaTime);
        }
    }

    void Script::FollowCameraTarget(float targetX, float targetY, float smoothing, float deltaTime)
    {
        FollowCameraTarget(Vec2(targetX, targetY), smoothing, deltaTime);
    }

    void Script::SetCameraDeadZone(float width, float height)
    {
        if (rendererRef)
        {
            rendererRef->GetCamera().SetFollowDeadZone(width, height);
        }
    }

    bool Script::IsCameraInDeadZone(const Vec2& targetPos) const
    {
        if (rendererRef)
        {
            return rendererRef->GetCamera().IsInDeadZone(targetPos);
        }
        return false;
    }

    Vec2 Script::GetVisibleWorldMin() const
    {
        if (rendererRef)
        {
            return rendererRef->GetCamera().GetVisibleWorldMin();
        }
        return Vec2::zero();
    }

    Vec2 Script::GetVisibleWorldMax() const
    {
        if (rendererRef)
        {
            return rendererRef->GetCamera().GetVisibleWorldMax();
        }
        return Vec2::zero();
    }

    bool Script::IsWorldPositionVisible(const Vec2& worldPos, const Vec2& size) const
    {
        if (rendererRef)
        {
            return rendererRef->GetCamera().IsVisible(worldPos, size);
        }
        return false;
    }

    Vec2 Script::ScreenToWorld(const Vec2& screenPos) const
    {
        if (rendererRef)
            return rendererRef->ScreenToWorld(screenPos);
        return screenPos;
    }

    float Script::GetCurrentTime() const
    {
        if (timelineRef)
        {
            return timelineRef->GetCurrentTime();
        }
        return 0.0f;
    }

    void Script::SetTimeScale(float scale)
    {
        if (timelineRef)
        {
            timelineRef->SetTimeScale(scale);
        }
    }

    float Script::GetTimeScale() const
    {
        if (timelineRef)
        {
            return timelineRef->GetTimeScale();
        }
        return 1.0f;
    }

    void Script::IncreaseTimeScale()
    {
        if (timelineRef)
        {
            timelineRef->IncreaseTimeScale();
        }
    }

    void Script::DecreaseTimeScale()
    {
        if (timelineRef)
        {
            timelineRef->DecreaseTimeScale();
        }
    }

    void Script::SetPaused(bool isPaused)
    {
        if (timelineRef)
        {
            timelineRef->SetPaused(isPaused);
        }
    }

    bool Script::IsPaused() const
    {
        if (timelineRef)
        {
            return timelineRef->IsPaused();
        }
        return false;
    }

    InputState Script::GetInputForClient(uint32_t clientID)
    {
        if (serverInputManagerRef)
        {
            return serverInputManagerRef->GetInputForClient(clientID);
        }
        return {};
    }

    std::vector<uint32_t> Script::GetConnectedClients()
    {
        if (serverRef)
        {
            return serverRef->GetConnectedClients();
        }
        return {};
    }

    uint32_t Script::GetPlayerEntityForClient(uint32_t clientID)
    {
        if (serverRef)
        {
            return serverRef->GetPlayerEntityForClient(clientID);
        }
        return 0;
    }

    void Script::RegisterPlayerEntity(uint32_t clientID, uint32_t entityID)
    {
        if (serverRef)
        {
            serverRef->RegisterPlayerEntity(clientID, entityID);
        }
    }

    void Script::SendInputToServer(const std::unordered_map<std::string, bool>& buttons)
    {
        if (networkManagerRef)
        {
            networkManagerRef->SendInput(buttons);
        }
    }

    uint32_t Script::GetLocalClientId()
    {
        if (networkManagerRef)
        {
            return networkManagerRef->GetClientId();
        }
        return 0;
    }

    uint32_t Script::GetLocalPlayerEntity()
    {
        if (networkManagerRef)
        {
            return networkManagerRef->GetLocalPlayerEntity();
        }
        return 0;
    }

    void Script::BroadcastEntitySpawn(uint32_t entityID, uint32_t ownerClientID, uint32_t excludeClientID)
    {
        if (serverRef && entityManagerRef)
        {
            Entity* entity = entityManagerRef->GetEntityByID(entityID);
            if (entity)
            {
                EntitySpawnInfo spawnInfo;
                spawnInfo.entityID = entity->ID;
                spawnInfo.spritePath = entity->spritePath;
                spawnInfo.totalFrames = entity->totalFrames;
                spawnInfo.fps = entity->fps;
                spawnInfo.position = entity->position;
                spawnInfo.scale = entity->scale;
                spawnInfo.rotation = entity->rotation;
                spawnInfo.physEnabled = entity->physApplied;
                spawnInfo.colliderType = static_cast<int>(entity->collider.type);

                serverRef->BroadcastEntitySpawn(spawnInfo, ownerClientID, excludeClientID);
            }
        }
    }

    void Script::BroadcastEntityDespawn(uint32_t entityID, uint32_t excludeClientID)
    {
        if (serverRef)
        {
            serverRef->BroadcastEntityDespawn(entityID, excludeClientID);
        }
    }

    int Script::Alloc()
    {
        if (allocatorRef)
        {
            return allocatorRef->Alloc();
        }
        return -1;
    }

    void Script::FreeSlot(int id)
    {
        if (allocatorRef)
        {
            allocatorRef->FreeSlot(id);
        }
    }

    void* Script::GetPointer(int id)
    {
        if (allocatorRef)
        {
            return allocatorRef->GetPointer(id);
        }
        return nullptr;
    }

    int Script::GetUsed()
    {
        if (allocatorRef)
        {
            return allocatorRef->GetUsed();
        }
        return -1;
    }

    int Script::GetTotal()
    {
        if (allocatorRef)
        {
            return allocatorRef->GetTotal();
        }
        return -1;
    }

    float Script::GetUsedPercent()
    {
        if (allocatorRef)
        {
            return allocatorRef->GetUsedPercent();
        }
        return 0.0f;
    }

    uint32_t Script::AddAudioClip(const std::string& path)
    {
        if (audioManagerRef)
        {
            return audioManagerRef->AddAudioClip(path);
        }
        return 0;
    }

    void Script::PlayAudioClip(uint32_t id)
    {
        if (audioManagerRef)
        {
            return audioManagerRef->PlayAudioClip(id);
        }
        return;
    }

    void Script::PauseAudioClip(uint32_t id)
    {
        if (audioManagerRef)
        {
            return audioManagerRef->PauseAudioClip(id);
        }
        return;
    }

    void Script::StopAudioClip(uint32_t id)
    {
        if (audioManagerRef)
        {
            return audioManagerRef->StopAudioClip(id);
        }
        return;
    }

    void Script::SetAudioVolume(uint32_t id, float volume)
    {
        if (audioManagerRef)
        {
            return audioManagerRef->SetAudioVolume(id, volume);
        }
        return;
    }

    float Script::GetAudioVolume(uint32_t id)
    {
        if (audioManagerRef)
        {
            return audioManagerRef->GetAudioVolume(id);
        }
        return 0.0f;
    }

    void Script::SetAudioMasterVolume(float volume)
    {
        if (audioManagerRef)
        {
            return audioManagerRef->SetAudioMasterVolume(volume);
        }
        return;
    }

    float Script::GetAudioMasterVolume()
    {
        if (audioManagerRef)
        {
            return audioManagerRef->GetAudioMasterVolume();
        }
        return 0.0f;
    }

    void Script::SetAudioLooping(uint32_t id, bool loop)
    {
        if (audioManagerRef)
        {
            return audioManagerRef->SetAudioLooping(id, loop);
        }
        return;
    }

    bool Script::GetAudioLooping(uint32_t id)
    {
        if (audioManagerRef)
        {
            return audioManagerRef->GetAudioLooping(id);
        }
        return false;
    }

    void Script::SetAudioPlayOnAwake(uint32_t id, bool playOnAwake)
    {
        if (audioManagerRef)
        {
            audioManagerRef->SetAudioPlayOnAwake(id, playOnAwake);
        }
        return;
    }

    bool Script::GetAudioPlayOnAwake(uint32_t id)
    {
        if (audioManagerRef)
        {
            return audioManagerRef->GetAudioPlayOnAwake(id);
        }
        return false;
    }

    void Script::SetAudioSpatialization(uint32_t id, bool spatialization)
    {
        if (audioManagerRef)
        {
            return audioManagerRef->SetAudioSpatialization(id, spatialization);
        }
    }

    bool Script::IsAudioPlaying(uint32_t id)
    {
        if (audioManagerRef)
        {
            return audioManagerRef->IsAudioPlaying(id);
        }
        return false;
    }

    bool Script::IsAudioPaused(uint32_t id)
    {
        if (audioManagerRef)
        {
            return audioManagerRef->IsAudioPaused(id);
        }
        return false;
    }

    bool Script::IsAudioStopped(uint32_t id)
    {
        if (audioManagerRef)
        {
            audioManagerRef->IsAudioStopped(id);
        }
        return false;
    }

    uint32_t Script::AddUIRect(float x_pos, float y_pos, float width, float height, RGBA color, std::string text, Border border, const std::string& fontPath, float fontSize, RGBA textColor)
    {
        if (!uiManagerRef) return 0;
        return uiManagerRef->AddRect(x_pos, y_pos, width, height, color, text, border, fontPath, fontSize, textColor);
    }

    uint32_t Script::AddUIButton(float x_pos, float y_pos, float width, float height, RGBA color, std::string text, Border border, std::function<void()> onPress, const std::string& fontPath, float fontSize, RGBA textColor)
    {
        if (!uiManagerRef) return 0;
        return uiManagerRef->AddButton(x_pos, y_pos, width, height, color, text, border, onPress, fontPath, fontSize, textColor);
    }

    uint32_t Script::AddUIText(float x_pos, float y_pos, float fontSize, RGBA color, const std::string& fontPath,
                                      const std::string& text)
    {
        if (!uiManagerRef) return 0;
        return uiManagerRef->AddText(x_pos, y_pos, fontSize, color, fontPath, text);
    }

    void Script::SetUIText(uint32_t textID, const std::string& text)
    {
        if (!uiManagerRef) return;
        uiManagerRef->SetUIText(textID, text);
    }

    void Script::RemoveUIElement(uint32_t elementID)
    {
        if (!uiManagerRef) return;
        uiManagerRef->DeleteElement(elementID);
    }

    void Script::SetUIButtonCallback(uint32_t buttonID, std::function<void()> onPress)
    {
        if (!uiManagerRef) return;
        uiManagerRef->SetButtonCallback(buttonID, onPress);
    }

    void Script::SetUIElementVisible(uint32_t elementID, bool visible)
    {
        if (!uiManagerRef) return;
        uiManagerRef->SetElementVisible(elementID, visible);
    }
    
    void Script::SetUIElementPersistent(uint32_t elementID, bool persistent)
    {
        if (!uiManagerRef) return;
        uiManagerRef->SetElementPersistent(elementID, persistent);
    }

    void Script::SetUIElementPosition(uint32_t elementID, float x_pos, float y_pos)
    {
        if (!uiManagerRef) return;
        uiManagerRef->SetElementPosition(elementID, x_pos, y_pos);
    }

    void Script::SetUIElementColor(uint32_t elementID, RGBA color)
    {
        if (!uiManagerRef) return;
        uiManagerRef->SetElementColor(elementID, color);
    }

    void Script::SetUIElementZIndex(uint32_t elementID, int zIndex)
    {
        if (!uiManagerRef) return;
        uiManagerRef->SetElementZIndex(elementID, zIndex);
    }

    void Script::SetUITextColor(uint32_t elementID, RGBA color)
    {
        if (!uiManagerRef) return;
        uiManagerRef->SetTextColor(elementID, color);
    }

    void Script::SetElementSprite(uint32_t elementID, std::string spritePath)
    {
        if (!uiManagerRef) return;
        uiManagerRef->SetElementSprite(elementID, spritePath);
    }

    void Script::SetButtonSprites(uint32_t elementID, std::string spritePath, std::string hoveredSpritePath,
        std::string pressedSpritePath)
    {
        if (!uiManagerRef) return;
        uiManagerRef->SetButtonSprites(elementID, spritePath, hoveredSpritePath, pressedSpritePath);
    }

    void Script::AddTagToUIElement(uint32_t elementID, const std::string& tag)
    {
        if (!uiManagerRef) return;
        return uiManagerRef->AddTagToUIElement(elementID, tag);
    }

    void Script::RemoveTagFromUIElement(uint32_t elementID, const std::string& tag)
    {
        if (!uiManagerRef) return;
        return uiManagerRef->RemoveTagFromUIElement(elementID, tag);
    }

    std::vector<uint32_t> Script::GetAllUIElementsWithTag(std::string tag)
    {
        if (!uiManagerRef) return {};
        return uiManagerRef->GetAllUIElementsWithTag(tag);
    }

    uint32_t Script::GetFirstUIElementWithTag(std::string tag)
    {
        if (!uiManagerRef) return 0;
        return uiManagerRef->GetFirstUIElementWithTag(tag);
    }

    bool Script::UIElementHasTag(uint32_t elementID, std::string tag)
    {
        if (!uiManagerRef) return false;
        return uiManagerRef->UIElementHasTag(elementID, tag);
    }

    Vec2 Script::GetTextSize(uint32_t elementID)
    {
        if (uiManagerRef) return uiManagerRef->GetTextSize(elementID);
        return Vec2::zero();
    }

    bool Script::SaveScene(const std::string& filepath) {
        if (!sceneManagerRef) return false;
        return sceneManagerRef->SaveScene(filepath);
    }

    bool Script::LoadScene(const std::string& filepath)
    {
        if (!sceneManagerRef) return false;
        return sceneManagerRef->LoadScene(filepath);
    }
}