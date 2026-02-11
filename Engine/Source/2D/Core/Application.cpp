#include "Application.h"
#include <chrono>
#include <iostream>
#include <csignal>

namespace CubeCore
{
    std::atomic<bool>* g_running = nullptr;

    Application::Application()
    {
        // Initialize the internal window class object
        window = Window();

        // Initialize the physics class object
        physics = Physics();
    }

    Application::~Application()
    {
        // Ensure threads are properly stopped
        running = false;
        renderCondition.notify_all();

        if (physicsThread.joinable())
        {
            physicsThread.join();
        }
        if (renderThread.joinable())
        {
            renderThread.join();
        }
    }

    void Application::Init()
    {

    }

    void Application::PhysicsThreadFunction()
    {
        while (running)
        {
            // Update physics system
            float effectiveTimestep = timeline.CalculateEffectiveTime(FIXED_TIMESTEP);
            physics.Update(effectiveTimestep);

            // Sleep to maintain 60 Hz update rate
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

    void Application::RenderThreadFunction()
    {
        // Initialize renderer
        renderer.Init(window.GetNativeWindow());
        uiManager.SetTextEngine(renderer.GetTextEngine());
        uiManager.SetInput(&input);
        renderer.SetUIManager(&uiManager);
        // Initialize entity manager
        entityManager.SetRenderer(renderer.GetRenderer());
        entityManager.SetPhysics(&physics);
        sceneManager.SetEntityManager(&entityManager);
        sceneManager.SetUIManager(&uiManager);
        sceneManager.SetPhysics(&physics);
        sceneManager.SetRenderer(&renderer);

        for (auto* script : scripts)
            script->OnStart();

        rendererInitialized.store(true);

        // Last frame time
        auto lastTime = std::chrono::high_resolution_clock::now();

        while (running)
        {
            // Wait for a render signal from the main thread
            std::unique_lock<std::mutex> lock(renderMutex);
            renderCondition.wait(lock, [this] { return renderReady.load() || !running.load(); });

            // Stop render thread if the application was just stopped
            if (!running)
            {
                break;
            }

            // Calculate delta time
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;
            deltaTime = std::min(deltaTime, MAX_FRAME_TIME);

            float effectiveDeltaTime = timeline.CalculateEffectiveTime(deltaTime);

            // Reset render signal
            renderReady = false;
            // Release lock allow rendering events
            lock.unlock();

            input.ProcessEvents();
            
            uiManager.Update();

            // Update animations
            entityManager.UpdateAnimations(effectiveDeltaTime);

            // Update timeline
            timeline.Update(deltaTime);

            // Update game logic
            for (auto* script : scripts)
                script->OnUpdate(effectiveDeltaTime);
            
            input.EndFrame();

            // Render the frame
            renderer.BeginFrame(effectiveDeltaTime, entityManager);
            renderer.RenderUI();
            renderer.EndFrame();
        }

        for (auto* script : scripts)
            script->OnExit();
    }

    void Application::PushScript(Script* script)
    {
        if (script)
            scripts.push_back(script);
    }

    void Application::Run()
    {
        // Initialize engine systems
        Init();

        auto& localScripts = scripts;

        // Set up game references
        for (auto* script : localScripts)
        {
            script->SetPhysicsRef(&physics);
            script->SetRenderer(&renderer);
            script->SetInput(&input);
            script->SetEntityManager(&entityManager);
            script->SetTimeline(&timeline);
            script->SetAudioManager(&audioManager);
            script->SetUIManager(&uiManager);
            script->SetSceneManager(&sceneManager);
        }
        
        physics.SetEntityManager(&entityManager);
        physics.Initialize();

        // Start worker threads
        physicsThread = std::thread(&Application::PhysicsThreadFunction, this);
        renderThread = std::thread(&Application::RenderThreadFunction, this);

        while (!rendererInitialized.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // Main update loop
        bool done = false;
        while (!done && running)
        {
            // Handle SDL events
            // SDL_Event event;
            // while (SDL_PollEvent(&event))
            // {
            //     switch (event.type)
            //     {
            //     case SDL_EVENT_QUIT:
            //         done = true;
            //         break;
            //     case SDL_EVENT_KEY_DOWN:
            //         if (!event.key.repeat)
            //             input.QueueKeyEvent(event.key.scancode, true);
            //         break;
            //     case SDL_EVENT_KEY_UP:
            //         input.QueueKeyEvent(event.key.scancode, false);
            //         break;
            //     case SDL_EVENT_MOUSE_BUTTON_DOWN:
            //         input.QueueMouseButtonEvent(event.button.button - 1, true);
            //         break;
            //     case SDL_EVENT_MOUSE_BUTTON_UP:
            //         input.QueueMouseButtonEvent(event.button.button - 1, false);
            //         break;
            //     case SDL_EVENT_MOUSE_MOTION:
            //         input.QueueMouseMove(event.motion.x, event.motion.y);
            //         break;
            //     case SDL_EVENT_MOUSE_WHEEL:
            //         input.QueueMouseScroll(event.wheel.x, event.wheel.y);
            //         break;
            //     }
            // }

            // Signal render thread to render this frame
            {
                std::lock_guard<std::mutex> lock(renderMutex);
                renderReady = true;
            }
            renderCondition.notify_one();
        }

        // Signal threads to stop
        running = false;
        renderCondition.notify_all();

        // Wait for threads to finish
        if (physicsThread.joinable())
        {
            physicsThread.join();
        }
        if (renderThread.joinable())
        {
            renderThread.join();
        }
        
        physics.Shutdown();
    }
}
