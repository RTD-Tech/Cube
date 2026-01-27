#include "Application.h"
#include "Networking/Server.h"
#include <chrono>
#include <iostream>
#include <csignal>

namespace SquareCore
{
    std::atomic<bool>* g_running = nullptr;

    Application::Application()
        : allocator(32, 200)
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
        if (networkThread.joinable())
        {
            networkThread.join();
        }
    }

    void Application::Init()
    {
        // Initialize SDL video subsystem
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            std::cout << "Error initializing SDL\n";
        }
        
        if (!TTF_Init()) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,"Error initializing SDL_ttf: %s\n", SDL_GetError());
        }

        // Create the application window
        window.SetNativeWindow(
            SDL_CreateWindow(
                window.GetTitle().c_str(),
                window.GetWidth(),
                window.GetHeight(),
                SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_MOUSE_GRABBED
            )
        );

        if (window.GetNativeWindow() == nullptr)
        {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error creating SDL window: %s\n", SDL_GetError());
        }
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

    void Application::RenderThreadFunction_ListenServer(Server* server)
    {
        // Initialize renderer
        renderer.Init(window.GetNativeWindow());
        uiManager.SetTextEngine(renderer.GetTextEngine());
        uiManager.SetInput(&input);
        // Initialize entity manager
        entityManager.SetRenderer(renderer.GetRenderer());
        sceneManager.SetEntityManager(&entityManager);
        sceneManager.SetUIManager(&uiManager);
        sceneManager.SetRenderer(&renderer);
        server->GetEntityManager().SetRenderer(renderer.GetRenderer());

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
            
            input.EndFrame();

            // Render the frame
            renderer.BeginFrame(effectiveDeltaTime, server->GetEntityManager());
            renderer.RenderUI();
            renderer.EndFrame();
        }
    }

    void Application::NetworkThreadFunction()
    {
        while (running)
        {
            // Update networking system
            networkManager.Update();

            // Sleep duration scaled by timeline to simulate different update rates
            float baseIntervalMs = 16.0f;
            float timeScale = timeline.GetTimeScale();
            float adjustedInterval = baseIntervalMs / timeScale;

            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(adjustedInterval)));
        }
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

        // Set mode
        currentMode = NetworkMode::STANDALONE;

        // Set up game references
        for (auto* script : localScripts)
        {
            script->SetPhysicsRef(&physics);
            script->SetRenderer(&renderer);
            script->SetInput(&input);
            script->SetEntityManager(&entityManager);
            script->SetTimeline(&timeline);
            script->SetMode(NetworkMode::STANDALONE);
            script->SetPoolAllocator(&allocator);
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
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                case SDL_EVENT_QUIT:
                    done = true;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    if (!event.key.repeat)
                        input.QueueKeyEvent(event.key.scancode, true);
                    break;
                case SDL_EVENT_KEY_UP:
                    input.QueueKeyEvent(event.key.scancode, false);
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    input.QueueMouseButtonEvent(event.button.button - 1, true);
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    input.QueueMouseButtonEvent(event.button.button - 1, false);
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    input.QueueMouseMove(event.motion.x, event.motion.y);
                    break;
                case SDL_EVENT_MOUSE_WHEEL:
                    input.QueueMouseScroll(event.wheel.x, event.wheel.y);
                    break;
                }
            }

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

        // Clean up SDL resources
        SDL_DestroyRenderer(renderer.GetRenderer());
        SDL_DestroyWindow(window.GetNativeWindow());
        TTF_Quit();
        SDL_Quit();
    }

    void Application::RunServer(bool headless)
    {
        auto& localScripts = scripts;
        
        if (localScripts.empty())
        {
            std::cout << "Error: Cannot start server without game logic\n";
            return;
        }

        currentMode = NetworkMode::SERVER;

        if (!headless)
        {
            // Initialize engine systems for listen-server mode
            Init();
        }

        std::cout << "Starting server in " << (headless ? "headless" : "listen-server") << " mode...\n";

        // Initialize server
        Server server;

        // Set up game references to server's systems
        for (auto* script : localScripts)
        {
            script->SetEntityManager(&server.GetEntityManager());
            script->SetPhysicsRef(&server.GetPhysics());
            script->SetTimeline(&server.GetTimeline());
            script->SetInputManager(&server.GetInputManager());
            script->SetMode(NetworkMode::SERVER);
            script->SetServerRef(&server);
            script->SetHeadlessServer(headless);
            script->SetPoolAllocator(&allocator);
        }

        // Enable headless mode only for dedicated servers
        server.GetEntityManager().SetHeadlessMode(headless);

        // Set renderer for listen-server entity manager
        if (headless)
        {
            // Initialize game logic
            for (auto* script : localScripts)
                script->OnStart();

            std::cout << "Game initialized, starting server...\n";

            // Set up signal handler
            g_running = &running;
            (void)std::signal(SIGINT, ServerSignalHandler);

            // Start server (blocks in simulation loop)
            std::thread serverThread([&server, localScripts]()
            {
                server.Start(localScripts);
            });

            // Shutdown monitor thread - watches running flag and stops server
            std::thread shutdownMonitor([this, &server]()
            {
                while (running.load())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                // Running flag set to false, stop the server
                std::cout << "Shutdown requested, stopping server...\n";
                server.Stop();
            });

            // Wait for server thread to complete
            std::cout << "Headless server running. Press Ctrl+C to stop.\n";

            if (serverThread.joinable())
            {
                serverThread.join();
            }

            if (shutdownMonitor.joinable())
            {
                shutdownMonitor.join();
            }

            // Restore default signal handler
            (void)std::signal(SIGINT, SIG_DFL);
            g_running = nullptr;
        }
        else
        {
            // Listen-server with local rendering
            for (auto* script : localScripts)
            {
                script->SetRenderer(&renderer);
                script->SetInput(&input);
            }

            // Start render thread with server's EntityManager
            renderThread = std::thread(&Application::RenderThreadFunction_ListenServer, this, &server);

            while (!rendererInitialized.load())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            // Start server (blocks in simulation loop)
            std::thread serverThread([&server, localScripts]()
            {
                server.Start(localScripts);
            });

            // Main event loop
            bool done = false;
            while (!done && running)
            {
                SDL_Event event;
                while (SDL_PollEvent(&event))
                {
                    switch (event.type)
                    {
                    case SDL_EVENT_KEY_DOWN:
                        if (!event.key.repeat)
                            input.QueueKeyEvent(event.key.scancode, true);
                        break;
                    case SDL_EVENT_KEY_UP:
                        input.QueueKeyEvent(event.key.scancode, false);
                        break;
                    case SDL_EVENT_MOUSE_BUTTON_DOWN:
                        input.QueueMouseButtonEvent(event.button.button - 1, true);
                        break;
                    case SDL_EVENT_MOUSE_BUTTON_UP:
                        input.QueueMouseButtonEvent(event.button.button - 1, false);
                        break;
                    case SDL_EVENT_MOUSE_MOTION:
                        input.QueueMouseMove(event.motion.x, event.motion.y);
                        break;
                    case SDL_EVENT_MOUSE_WHEEL:
                        input.QueueMouseScroll(event.wheel.x, event.wheel.y);
                        break;
                    }
                }

                // Signal render thread
                {
                    std::lock_guard<std::mutex> lock(renderMutex);
                    renderReady = true;
                }
                renderCondition.notify_one();

                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }

            running = false;
            renderCondition.notify_all();

            if (renderThread.joinable())
            {
                renderThread.join();
            }

            // Stop server and wait for server thread to finish
            server.Stop();
            if (serverThread.joinable())
            {
                serverThread.join();
            }

            SDL_DestroyRenderer(renderer.GetRenderer());
            SDL_DestroyWindow(window.GetNativeWindow());
            TTF_Quit();
            SDL_Quit();
        }
    }

    void Application::RunClient(const std::string& serverAddress)
    {
        // Initialize engine systems
        Init();

        auto& localScripts = scripts;

        // Set mode
        currentMode = NetworkMode::CLIENT;

        // Set up game references
        for (auto* script: localScripts)
        {
            script->SetPhysicsRef(&physics);
            script->SetRenderer(&renderer);
            script->SetInput(&input);
            script->SetEntityManager(&entityManager);
            script->SetTimeline(&timeline);
            script->SetMode(NetworkMode::CLIENT);
            script->SetPoolAllocator(&allocator);
            script->SetAudioManager(&audioManager);
            script->SetUIManager(&uiManager);
        }
        
        physics.SetEntityManager(&entityManager);
        physics.Initialize();

        // Start worker threads
        physicsThread = std::thread(&Application::PhysicsThreadFunction, this);
        renderThread = std::thread(&Application::RenderThreadFunction, this);
        networkThread = std::thread(&Application::NetworkThreadFunction, this);

        while (!rendererInitialized.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // Initialize NetworkManager and connect to server
        networkManager.SetEntityManager(&entityManager);
        if (!networkManager.Connect(serverAddress))
        {
            std::cout << "Failed to connect to server at " << serverAddress << "\n";
            return;
        }

        // Set network manager reference for game
        for (auto* script : localScripts)
            script->SetNetworkManager(&networkManager);

        // Main update loop
        bool done = false;
        while (!done && running)
        {
            // Handle SDL quit event
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                case SDL_EVENT_KEY_DOWN:
                    if (!event.key.repeat)
                        input.QueueKeyEvent(event.key.scancode, true);
                    break;
                case SDL_EVENT_KEY_UP:
                    input.QueueKeyEvent(event.key.scancode, false);
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    input.QueueMouseButtonEvent(event.button.button - 1, true);
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    input.QueueMouseButtonEvent(event.button.button - 1, false);
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    input.QueueMouseMove(event.motion.x, event.motion.y);
                    break;
                case SDL_EVENT_MOUSE_WHEEL:
                    input.QueueMouseScroll(event.wheel.x, event.wheel.y);
                    break;
                }
            }

            // Signal render thread to render this frame
            {
                std::lock_guard<std::mutex> lock(renderMutex);
                renderReady = true;
            }
            renderCondition.notify_one();
        }

        // Disconnect from server
        networkManager.Disconnect();

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
        if (networkThread.joinable())
        {
            networkThread.join();
        }
        
        physics.Shutdown();

        // Clean up SDL resources
        SDL_DestroyRenderer(renderer.GetRenderer());
        SDL_DestroyWindow(window.GetNativeWindow());
        TTF_Quit();
        SDL_Quit();
    }

    void ServerSignalHandler(int signal)
    {
        if (signal == SIGINT && g_running)
        {
            g_running->store(false);
        }
    }
}
