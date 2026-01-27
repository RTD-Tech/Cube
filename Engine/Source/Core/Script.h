#ifndef GAME_INTERFACE_H
#define GAME_INTERFACE_H

#include "Renderer/Renderer.h"
#include "Renderer/EntityManager.h"
#include "Input/Input.h"
#include "Physics/Physics.h"
#include "Timeline.h"
#include "Networking/NetworkManager.h"
#include "NetworkMode.h"
#include "Memory/PoolAllocator.h"
#include "Audio/AudioManager.h"
#include "SceneManager.h"
#include <vector>
#include <string>

namespace SquareCore
{
    // Forward declarations
    class Server;
    class ServerInputManager;

    class Script
    {
    public:
        virtual ~Script() = default;

        // Runs before the main game loop at the start of the game
        virtual void OnStart() {}

        // Runs for every frame
        virtual void OnUpdate(float deltaTime) {}

        virtual void OnExit() {}

        // Optional callbacks for server mode
        virtual void OnClientConnected(uint32_t clientID) {}

        virtual void OnClientDisconnected(uint32_t clientID) {}

        // Set the internal renderer reference (for use in the engine core only)
        void SetRenderer(Renderer* renderer) { this->rendererRef = renderer; }
        // Set the internal input system reference (for use in the engine core only)
        void SetInput(Input* input) { this->inputRef = input; }
        // Set the internal physics system reference (for use in the engine core only)
        void SetPhysicsRef(Physics* physics) { this->physicsRef = physics; }
        // Set the internal entity manager reference (for use in the engine core only)
        void SetEntityManager(EntityManager* entityManager) { this->entityManagerRef = entityManager; }
        // Set timeline reference (for use in engine core only)
        void SetTimeline(Timeline* timeline) { this->timelineRef = timeline; }
        // Set network manager reference (for use in engine core only)
        void SetNetworkManager(NetworkManager* networkManager) { this->networkManagerRef = networkManager; }
        // Set server input manager reference (for server mode only)
        void SetInputManager(ServerInputManager* inputManager) { this->serverInputManagerRef = inputManager; }
        // Set server reference (for server mode only)
        void SetServerRef(Server* server) { this->serverRef = server; }
        // Set network mode (for use in engine core only)
        void SetMode(NetworkMode mode) { this->currentMode = mode; }
        // Set headless server flag (for server mode only)
        void SetHeadlessServer(bool headless) { headlessServer = headless; }
        // Set pool allocator reference (for use in engine core only)
        void SetPoolAllocator(PoolAllocator* allocator) { this->allocatorRef = allocator; }
        void SetAudioManager(AudioManager* audioManager) { this->audioManagerRef = audioManager; }
        void SetUIManager(UIManager* uiManager) { this->uiManagerRef = uiManager; }
        void SetSceneManager(SceneManager* sceneManager) { this->sceneManagerRef = sceneManager; }

    protected:
        void Quit();
        // Add an entity to the scene
        uint32_t AddEntity(const char* spritePath, float Xpos = 0.0f, float Ypos = 0.0f, float rotation = 0.0f,
                           float Xscale = 1.0f, float Yscale = 1.0f, bool physEnabled = false, std::vector<std::string> tags = {});
        // Add an animated entity to the scene
        uint32_t AddAnimatedEntity(const char* spritePath, int totalFrames, float fps, float Xpos = 0.0f,
            float Ypos = 0.0f, float rotation = 0.0f, float Xscale = 1.0f, float Yscale = 1.0f, bool physEnabled = false);
        // Add a spriteless entity to the scene
        uint32_t AddSpritelessEntity(float width, float height, RGBA color, float Xpos = 0.0f, float Ypos = 0.0f,
            float rotation = 0.0f, float Xscale = 1.0f, float Yscale = 1.0f, bool physEnabled = false);
        
        std::vector<uint32_t> GetAllEntitiesWithTag(std::string tag);
        uint32_t GetFirstEntityWithTag(std::string tag);
        
        // Removes an entity from the screen
        void RemoveEntity(uint32_t entityID);
        // Update an entity's position given its ID
        void UpdateEntityPosition(uint32_t entityID, float newX, float newY);
        // Returns an entity's collisions given its ID
        /*
         0 - top
         1 - right
         2 - bottom
         3 - left
         */
        std::vector<std::pair<uint32_t, int>> GetEntityCollisions(uint32_t entityID);
        // Flip an entity's sprite
        void FlipSprite(uint32_t entityID, bool flipX, bool flipY);
        // Returns the X-axis flip state of an entity's sprite
        bool GetFlipX(uint32_t entityID);
        // Get the Y-axis flip state of an entity's sprite
        bool GetFlipY(uint32_t entityID);
        // Get the flip state of an entity's sprite
        bool GetFlipState(uint32_t entityID, bool& flipX, bool& flipY);
        // Toggles the X-axis flip state of an entity's sprite
        void ToggleFlipX(uint32_t entityID);
        // Toggles the Y-axis flip state of an entity's sprite
        void ToggleFlipY(uint32_t entityID);
        // Sets an entity's collider type
        void SetColliderType(uint32_t entityID, ColliderType type);
        void SetEntityColor(uint32_t entityID, RGBA color);
        void SetEntityVisible(uint32_t entityID, bool visible);
        void SetEntityPersistent(uint32_t entityID, bool persistent);
        void ResetAnimation(uint32_t entityID);
        void SetAnimationFPS(uint32_t entityID, float fps);
        void SetAnimationFrame(uint32_t entityID, int frame);
        bool IsAnimationComplete(uint32_t entityID) const;
        int GetTotalFrames(uint32_t entityID) const;

        bool EntityHasTag(uint32_t entityID, std::string tag);
        void AddTagToEntity(uint32_t entityID, std::string tag);
        void RemoveTagFromEntity(uint32_t entityID, std::string tag);
        
        void AddPropertyToEntity(uint32_t entityID, Property* property);
        std::vector<Property*> GetAllEntityProperties(uint32_t entityID);

        void SetZIndex(uint32_t entityID, int zIndex);
        int GetZIndex(uint32_t entityID);
        
        bool EntityExists(uint32_t ID) const;

        bool GetKeyPressed(SDL_Scancode key);
        bool GetKeyHeld(SDL_Scancode key);
        bool GetKeyReleased(SDL_Scancode key);
        bool GetMouseButtonPressed(int button);
        bool GetMouseButtonHeld(int button);
        bool GetMouseButtonReleased(int button);
        Vec2 GetMousePosition();
        Vec2 GetMouseDelta();
        Vec2 GetMouseScroll();

        void SetBackgroundColor(RGBA color);

        void SetPhysicsEnabled(uint32_t entityID, bool enabled);
        void SetColliderBox(uint32_t entityID, float halfWidth, float halfHeight);
        void SetColliderCircle(uint32_t entityID, float radius);
        void SetColliderCapsule(uint32_t entityID, float height, float radius);
        void SetColliderPolygon(uint32_t entityID, std::vector<Vec2> vertices);
        // Set gravity
        void SetGravity(float gravity);
        // Get Gravity
        float GetGravity();
        // Applies a force to an entity
        void ApplyForce(uint32_t entityID, float forceX, float forceY);
        // Applies an impulse to an entity
        void ApplyImpulse(uint32_t entityID, float impulseX, float impulseY);
        // Sets an entity's velocity
        void SetVelocity(uint32_t entityID, float velX, float velY);
        void SetMass(uint32_t entityID, float mass);
        void SetDrag(uint32_t entityID, float drag);
        void SetGravityScale(uint32_t entityID, float gravityScale);
        void SetFixedRotation(uint32_t entityID, bool fixed);
        // Gets an entity's velocity
        Vec2 GetVelocity(uint32_t entityID);
        // Sets an entity's position
        void SetPosition(uint32_t entityID, float newX, float newY);
        
        // Gets an entity's position
        Vec2 GetPosition(uint32_t entityID);
        
        void SetScale(uint32_t entityID, const Vec2& scale);
        Vec2 GetScale(uint32_t entityID);
        void SetRotation(uint32_t entityID, float rotation);
        float GetRotation(uint32_t entityID);
        void SetMouseVisible(bool visible);

        // Toggles scaling mode
        void ToggleScalingMode();
        // Toggles collision debug boxes
        void ToggleDebugCollisions();

        // Sets camera position
        void SetCameraPosition(const Vec2& pos);
        // Sets camera position
        void SetCameraPosition(float x, float y);
        // Gets camera position
        Vec2 GetCameraPosition() const;
        // Moves camera
        void MoveCamera(const Vec2& delta);
        // Moves camera
        void MoveCamera(float deltaX, float deltaY);
        // Moves camera to a new position instantly
        void SnapCameraToPosition(const Vec2& pos);
        // Sets camera zoom
        void SetCameraZoom(float zoom);
        // Gets camera zoom
        float GetCameraZoom() const;
        // Sets camera zoom limits
        void SetCameraZoomLimits(float min, float max);
        // Sets camera bounds
        void SetCameraBounds(const Vec2& min, const Vec2& max);
        // Sets camera bounds
        void SetCameraBounds(float minX, float minY, float maxX, float maxY);
        // Enables/disables camera bounds
        void EnableCameraBounds(bool enable);
        // Sets camera to follow a target position with smoothing
        void FollowCameraTarget(const Vec2& targetPos, float smoothing, float deltaTime);
        // Sets camera to follow a target position with smoothing
        void FollowCameraTarget(float targetX, float targetY, float smoothing, float deltaTime);
        // Sets camera follow dead zone
        void SetCameraDeadZone(float width, float height);
        // Checks if the camera is in the dead zone
        bool IsCameraInDeadZone(const Vec2& targetPos) const;
        // Camera viewport query (what world area is visible on screen)
        Vec2 GetVisibleWorldMin() const;
        // Camera viewport query (what world area is visible on screen)
        Vec2 GetVisibleWorldMax() const;
        // Camera viewport query (what world area is visible on screen)
        bool IsWorldPositionVisible(const Vec2& worldPos, const Vec2& size) const;
        Vec2 ScreenToWorld(const Vec2& screenPos) const;

        // Get current game time
        float GetCurrentTime() const;
        // Sets the internal engine time scale to a custom time scale
        void SetTimeScale(float scale);
        // Gets the internal engine time scale
        float GetTimeScale() const;
        // Increases the internal engine time scale to the next default setting (eg. 1.0x -> 2.0x)
        void IncreaseTimeScale();
        // Decreases the internal engine time scale to the next default setting (eg. 1.0x -> 0.5x)
        void DecreaseTimeScale();
        // Sets the internal engine pause state
        void SetPaused(bool isPaused);
        // Returns the current engine pause state
        bool IsPaused() const;

        // Mode detection
        bool IsServer() const { return currentMode == NetworkMode::SERVER; }
        bool IsClient() const { return currentMode == NetworkMode::CLIENT; }
        bool IsStandalone() const { return currentMode == NetworkMode::STANDALONE; }
        bool IsHeadlessServer() const { return headlessServer; }

        // Gets the input state of a connected client
        InputState GetInputForClient(uint32_t clientID);
        // Gets the IDs of all connected clients
        std::vector<uint32_t> GetConnectedClients();
        // Gets the entity id of a connected client
        uint32_t GetPlayerEntityForClient(uint32_t clientID);
        // Sets the entity id of a connected client (useful for listen-server host player)
        void RegisterPlayerEntity(uint32_t clientID, uint32_t entityID);
        // Broadcasts entity spawns to connected clients
        void BroadcastEntitySpawn(uint32_t entityID, uint32_t ownerClientID = 0, uint32_t excludeClientID = 0);
        // Broadcasts entity despawns to connected clients
        void BroadcastEntityDespawn(uint32_t entityID, uint32_t excludeClientID = 0);

        // Sends client input states to the server
        void SendInputToServer(const std::unordered_map<std::string, bool>& buttons);
        // Gets the local player's client ID
        uint32_t GetLocalClientId();
        // Gets the local player's entity ID
        uint32_t GetLocalPlayerEntity();

        // Allocates a slot from the memory pool and returns its ID (-1 if pool is full)
        int Alloc();
        // Frees a specific memory slot by ID, making it available for reuse
        void FreeSlot(int id);
        // Gets a pointer to the memory at the specified slot ID (nullptr if invalid)
        void* GetPointer(int id);
        // Returns the number of currently allocated memory slots
        int GetUsed();
        // Returns the total number of memory slots in the pool
        int GetTotal();
        // Returns the percentage of used memory slots (0.0-100.0)
        float GetUsedPercent();

        uint32_t AddAudioClip(const std::string& path);
        void PlayAudioClip(uint32_t id);
        void PauseAudioClip(uint32_t id);
        void StopAudioClip(uint32_t id);
        void SetAudioVolume(uint32_t id, float volume);
        float GetAudioVolume(uint32_t id);
        void SetAudioMasterVolume(float volume);
        float GetAudioMasterVolume();
        void SetAudioLooping(uint32_t id, bool loop);
        bool GetAudioLooping(uint32_t id);
        void SetAudioPlayOnAwake(uint32_t id, bool playOnAwake);
        bool GetAudioPlayOnAwake(uint32_t id);
        void SetAudioSpatialization(uint32_t id, bool spatialization);
        bool IsAudioPlaying(uint32_t id);
        bool IsAudioPaused(uint32_t id);
        bool IsAudioStopped(uint32_t id);

        uint32_t AddUIRect(float x_pos = 0.0f, float y_pos = 0.0f, float width = 100.0f, float height = 50.0f, RGBA color = RGBA(255, 255, 255, 255), std::string text = "", Border border = {RGBA(0, 0, 0, 255), 1.0f, 0.0f}, const std::string& fontPath = "", float fontSize = 16.0f, RGBA textColor = RGBA(0, 0, 0, 255));
        uint32_t AddUIButton(float x_pos = 0.0f, float y_pos = 0.0f, float width = 100.0f, float height = 50.0f, RGBA color = RGBA(255, 255, 255, 255), std::string text = "", Border border = {RGBA(0, 0, 0, 255), 1.0f, 0.0f}, std::function<void()> onPress = nullptr, const std::string& fontPath = "", float fontSize = 16.0f, RGBA textColor = RGBA(0, 0, 0, 255));
        uint32_t AddUIText(float x_pos, float y_pos, float fontSize, RGBA color,
                     const std::string& fontPath, const std::string& text);
        void SetUIText(uint32_t textID, const std::string& text);
        void RemoveUIElement(uint32_t elementID);
        void SetUIButtonCallback(uint32_t buttonID, std::function<void()> onPress);
        void SetUIElementVisible(uint32_t elementID, bool visible);
        void SetUIElementPersistent(uint32_t elementID, bool persistent);
        void SetUIElementPosition(uint32_t elementID, float x_pos, float y_pos);
        void SetUIElementColor(uint32_t elementID, RGBA color);
        void SetUIElementZIndex(uint32_t elementID, int zIndex);
        void SetUITextColor(uint32_t elementID, RGBA color);
        void SetElementSprite(uint32_t elementID, std::string spritePath);
        void SetButtonSprites(uint32_t elementID, std::string spritePath, std::string hoveredSpritePath, std::string pressedSpritePath);
        
        void AddTagToUIElement(uint32_t elementID, const std::string& tag);
        void RemoveTagFromUIElement(uint32_t elementID, const std::string& tag);
        std::vector<uint32_t> GetAllUIElementsWithTag(std::string tag);
        uint32_t GetFirstUIElementWithTag(std::string tag);
        bool UIElementHasTag(uint32_t elementID, std::string tag);
        Vec2 GetTextSize(uint32_t elementID);
        
        bool SaveScene(const std::string& filepath);
        bool LoadScene(const std::string& filepath);

    private:
        // Internal renderer reference (internal use only)
        Renderer* rendererRef = nullptr;
        // Internal input reference (internal use only)
        Input* inputRef = nullptr;
        // Internal physics reference (internal use only)
        Physics* physicsRef = nullptr;
        // Internal entity manager reference (internal use only)
        EntityManager* entityManagerRef = nullptr;
        // Internal timeline reference (internal use only)
        Timeline* timelineRef = nullptr;
        // Internal network manager reference (internal use only)
        NetworkManager* networkManagerRef = nullptr;
        // Internal server input manager reference (internal use and server mode only)
        ServerInputManager* serverInputManagerRef = nullptr;
        // Internal server reference (internal use and server mode only)
        Server* serverRef = nullptr;
        // Memory reference (internal use only)
        PoolAllocator* allocatorRef = nullptr;
        AudioManager* audioManagerRef = nullptr;
        UIManager* uiManagerRef = nullptr;
        SceneManager* sceneManagerRef = nullptr;

        // Current network mode
        NetworkMode currentMode = NetworkMode::STANDALONE;
        // Headless server flag (server mode only)
        bool headlessServer = false;
    };
}

#endif