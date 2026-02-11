#ifndef APPLICATION_H
#define APPLICATION_H

#include "Window.h"
#include "Script.h"
#include "Renderer/Renderer.h"
#include "Input/Input.h"
#include "Physics/Physics.h"
#include "Renderer/EntityManager.h"
#include "Timeline.h"
#include "Audio/AudioManager.h"
#include "UI/UIManager.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <string>
#include <iostream>

namespace CubeCore
{
    // Global running flag for signal handling (only for headless server)
    inline extern std::atomic<bool>* g_running;

    class Application {
    public:
        Application();
        ~Application();

        // Initializes engine resources
        void Init();
        // Starts the core application loop (standalone mode)
        void Run();
        // Pushes a script to the script stack
        void PushScript(Script* script);

        // Provides access to the entity manager
        EntityManager& GetEntityManager() { return entityManager; }
        UIManager& GetUIManager() { return uiManager; }

    private:
        Window window;
        Renderer renderer;
        Input input;
        Physics physics;
        EntityManager entityManager;
        Timeline timeline;
        AudioManager audioManager;
        UIManager uiManager;
        SceneManager sceneManager;
    
        // Script collection for game logic
        std::vector<Script*> scripts;

        // Atomic boolean to control thread loops
        std::atomic<bool> running{true};
        // Physics thread reference
        std::thread physicsThread;
        // Render thread reference
        std::thread renderThread;

        // Mutex for renderer synchronization
        std::mutex renderMutex;
        // Condition variable for renderer synchronization
        std::condition_variable renderCondition;
        // Atomic boolean to signal renderer initialization complete
        std::atomic<bool> rendererInitialized{false};
        // Atomic boolean to control the render thread loop
        std::atomic<bool> renderReady{false};

        // Physics thread function
        void PhysicsThreadFunction();
        // Render thread function
        void RenderThreadFunction();

        // Fixed 60 Hz timestep for physics updates
        static constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
        // Maximum frame time for rendering
        static constexpr float MAX_FRAME_TIME = 0.25f;
    };
}

#endif
