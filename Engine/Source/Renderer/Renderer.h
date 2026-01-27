#ifndef RENDERER_H
#define RENDERER_H

#include "Entity.h"
#include "EntityManager.h"
#include "Camera.h"
#include <SDL3/SDL.h>

#include "UI/UIManager.h"

namespace SquareCore {

// Enum for different scaling modes
enum class ScalingMode {
    PixelBased,    // Constant size (pixel-based)
    Proportional   // Proportional scaling (percentage-based)
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    // Returns a pointer to the underlying SDL renderer
    SDL_Renderer* GetRenderer() const { return rendererRef; }

    // Initializes renderer
    void Init(SDL_Window* window);
    // Begins the render pass for the current frame
    void BeginFrame(float deltaTime, EntityManager& entityManager);
    // Ends the render pass for the current frame
    void EndFrame();

    void RenderUI();

    // Function to toggle the scaling mode
    void ToggleScalingMode();
    // Function to toggle the collision debug boxes
    void ToggleDebugCollisions();

    // Resize the renderer and update camera viewport
    void Resize(int width, int height);

    // Get a reference to the camera
    Camera& GetCamera();
    
    TTF_TextEngine* GetTextEngine() const { return textEngineRef; }
    
    Vec2 ScreenToWorld(const Vec2& screenPos) const;
    
    void SetUIManager(UIManager* uiManager);

    void SetBackgroundColor(RGBA color) { background_color = color; }
    RGBA GetBackgroundColor() { return background_color; }

private:
    RGBA background_color = RGBA(0, 0, 31, 255);
    
    // Internal pointer to the underlying SDL renderer
    SDL_Renderer* rendererRef = nullptr;
    TTF_TextEngine* textEngineRef = nullptr;
    UIManager* uiManagerRef = nullptr;
    // Stores the width of the application window
    int windowWidth;
    // Stores the height of the application window
    int windowHeight;

    // Scaling mode
    ScalingMode scalingMode = ScalingMode::Proportional;
    static constexpr float baseWindowWidth = 1920.0f;
    static constexpr float baseWindowHeight = 1080.0f;
    // Function to calculate scaling factors for scaling modes
    void CalculateScalingFactors(float& scaleX, float& scaleY) const;

    // Boolean flag to toggle collision debug boxes
    bool debugCollisions = false;

    // Camera for viewport transforms
    Camera camera;

    // Function to render an entity
    void RenderEntity(const Entity& entity, float globalScaleX, float globalScaleY) const;
    
    SDL_Texture* LoadUITexture(const std::string & path);
    
    void DrawDebugCollider(const Entity& entity, float globalScaleX, float globalScaleY) const;
    void DrawDebugBox(const Vec2& screenCenter, const Vec2& halfExtents, float rotationDegrees,
                      float scaleX, float scaleY, RGBA color) const;
    void DrawDebugCircle(const Vec2& screenCenter, float radius, float scaleX, float scaleY, RGBA color) const;
    void DrawDebugCapsule(const Vec2& screenCenter, const Vec2& c1, const Vec2& c2, float radius,
                          float rotationDegrees, float scaleX, float scaleY, RGBA color) const;
    void DrawDebugPolygon(const Vec2& screenCenter, const std::vector<Vec2>& vertices,
                          float rotationDegrees, float scaleX, float scaleY, RGBA color) const;
    Vec2 WorldToScreen(const Vec2& worldPos, float globalScaleX, float globalScaleY) const;
    Vec2 RotatePoint(const Vec2& point, float angleDegrees) const;
    RGBA GetDebugColor(const Entity& entity) const;
};

}

#endif
