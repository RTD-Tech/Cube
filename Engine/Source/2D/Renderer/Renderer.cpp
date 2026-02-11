#include "Renderer.h"
#include <algorithm>
#include <cmath>

namespace CubeCore
{
    Renderer::Renderer()
        : camera(1920, 1080)
    {
    }

    Renderer::~Renderer()
    {
        
    }

    void Renderer::Init(SDL_Window* window)
    {
    }

    void Renderer::BeginFrame(float deltaTime, EntityManager& entityManager)
    {

    }

    void Renderer::EndFrame()
    {
        
    }

    void Renderer::RenderUI()
    {
        
    }

    void Renderer::RenderEntity(const Entity& entity, float globalScaleX, float globalScaleY) const
    {
    }

    void Renderer::ToggleScalingMode()
    {
        ScalingMode newMode = (scalingMode == ScalingMode::PixelBased)
                                  ? ScalingMode::Proportional
                                  : ScalingMode::PixelBased;
        scalingMode = newMode;
    }

    void Renderer::CalculateScalingFactors(float& scaleX, float& scaleY) const
    {
        switch (scalingMode)
        {
        case ScalingMode::PixelBased:
            // Constant size - no scaling based on window size
            scaleX = 1.0f;
            scaleY = 1.0f;
            break;
        case ScalingMode::Proportional:
            // Proportional scaling based on window size change
            scaleX = static_cast<float>(windowWidth) / baseWindowWidth;
            scaleY = static_cast<float>(windowHeight) / baseWindowHeight;
            break;
        }
    }

    void Renderer::ToggleDebugCollisions()
    {
        debugCollisions = !debugCollisions;
    }

    void Renderer::Resize(int width, int height)
    {
        windowWidth = width;
        windowHeight = height;
        camera.SetViewportSize(width, height);
    }

    Camera& Renderer::GetCamera()
    {
        return camera;
    }

    Vec2 Renderer::ScreenToWorld(const Vec2& screenPos) const
    {
        float globalScaleX, globalScaleY;
        CalculateScalingFactors(globalScaleX, globalScaleY);

        float scaledX = screenPos.x - (static_cast<float>(windowWidth) / 2.0f);
        float scaledY = (static_cast<float>(windowHeight) / 2.0f) - screenPos.y;

        float cameraRelativeX = scaledX / globalScaleX;
        float cameraRelativeY = scaledY / globalScaleY;

        float zoom = camera.GetZoom();
        Vec2 camPos = camera.GetPosition();

        return Vec2(
            (cameraRelativeX / zoom) + camPos.x,
            (cameraRelativeY / zoom) + camPos.y
        );
    }

    void Renderer::SetUIManager(UIManager* uiManager)
    {
        uiManagerRef = uiManager;
        uiManagerRef->OnWindowResize(windowWidth, windowHeight, baseWindowWidth, baseWindowHeight);
    }
    
    void Renderer::DrawDebugCollider(const Entity& entity, float globalScaleX, float globalScaleY) const
    {
        if (entity.collider.type == ColliderType::NONE || !entity.collider.enabled)
            return;

        RGBA color = GetDebugColor(entity);
        Vec2 screenCenter = WorldToScreen(entity.position + entity.collider.offset, globalScaleX, globalScaleY);

        float effectiveScaleX = (scalingMode == ScalingMode::Proportional) ? globalScaleX : 1.0f;
        float effectiveScaleY = (scalingMode == ScalingMode::Proportional) ? globalScaleY : 1.0f;
        
        float zoom = camera.GetZoom();
        effectiveScaleX *= zoom;
        effectiveScaleY *= zoom;
        
        float screenRotation = -entity.rotation;

        switch (entity.shapeData.shape)
        {
        case ColliderShape::CIRCLE:
            DrawDebugCircle(screenCenter, entity.shapeData.circle.radius,
                            effectiveScaleX, effectiveScaleY, color);
            break;

        case ColliderShape::CAPSULE:
            DrawDebugCapsule(screenCenter,
                             entity.shapeData.capsule.center1,
                             entity.shapeData.capsule.center2,
                             entity.shapeData.capsule.radius,
                             screenRotation, effectiveScaleX, effectiveScaleY, color);
            break;

        case ColliderShape::POLYGON:
            DrawDebugPolygon(screenCenter, entity.shapeData.polygon.vertices,
                             screenRotation, effectiveScaleX, effectiveScaleY, color);
            break;

        case ColliderShape::BOX:
        default:
            {
                Vec2 halfExtents = entity.shapeData.box.halfExtents;
                if (halfExtents.x <= 0 || halfExtents.y <= 0)
                {
                    if (entity.isSpriteless)
                    {
                        halfExtents.x = (entity.spritelessWidth * std::abs(entity.scale.x)) / 2.0f;
                        halfExtents.y = (entity.spritelessHeight * std::abs(entity.scale.y)) / 2.0f;
                    }
                    else
                    {
                        float frameWidth = entity.totalFrames > 1 ?
                                               (entity.spriteWidth / static_cast<float>(entity.totalFrames)) : entity.spriteWidth;
                        halfExtents.x = (frameWidth * std::abs(entity.scale.x)) / 2.0f;
                        halfExtents.y = (entity.spriteHeight * std::abs(entity.scale.y)) / 2.0f;
                    }
                }
                DrawDebugBox(screenCenter, halfExtents, screenRotation,
                             effectiveScaleX, effectiveScaleY, color);
            }
            break;
        }
    }

    void Renderer::DrawDebugBox(const Vec2& screenCenter, const Vec2& halfExtents, float rotationDegrees,
                                float scaleX, float scaleY, RGBA color) const
    {
        Vec2 corners[4] = {
            Vec2(-halfExtents.x, -halfExtents.y),
            Vec2( halfExtents.x, -halfExtents.y),
            Vec2( halfExtents.x,  halfExtents.y),
            Vec2(-halfExtents.x,  halfExtents.y)
        };
        
        SDL_FPoint screenCorners[5];
        for (int i = 0; i < 4; ++i)
        {
            Vec2 rotated = RotatePoint(corners[i], rotationDegrees);
            screenCorners[i].x = screenCenter.x + rotated.x * scaleX;
            screenCorners[i].y = screenCenter.y - rotated.y * scaleY;
        }
        screenCorners[4] = screenCorners[0];

        SDL_SetRenderDrawColor(rendererRef, color.r, color.g, color.b, color.a);
        SDL_RenderLines(rendererRef, screenCorners, 5);
    }

    void Renderer::DrawDebugCircle(const Vec2& screenCenter, float radius,
                                   float scaleX, float scaleY, RGBA color) const
    {
        const int segments = 32;
        SDL_FPoint points[segments + 1];

        for (int i = 0; i <= segments; ++i)
        {
            float angle = (static_cast<float>(i) / segments) * 2.0f * MATH_PI;
            float x = std::cos(angle) * radius;
            float y = std::sin(angle) * radius;
            points[i].x = screenCenter.x + x * scaleX;
            points[i].y = screenCenter.y - y * scaleY;
        }

        SDL_SetRenderDrawColor(rendererRef, color.r, color.g, color.b, color.a);
        SDL_RenderLines(rendererRef, points, segments + 1);
    }

    void Renderer::DrawDebugCapsule(const Vec2& screenCenter, const Vec2& c1, const Vec2& c2, float radius,
                                    float rotationDegrees, float scaleX, float scaleY, RGBA color) const
    {
        Vec2 rc1 = RotatePoint(c1, rotationDegrees);
        Vec2 rc2 = RotatePoint(c2, rotationDegrees);
        
        Vec2 dir = Vec2(rc2.x - rc1.x, rc2.y - rc1.y);
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len < 0.001f) len = 0.001f;
        Vec2 perp(-dir.y / len, dir.x / len);
        
        float axisAngle = std::atan2(dir.y, dir.x);

        const int arcSegments = 16;
        
        SDL_FPoint arc1[arcSegments + 1];
        for (int i = 0; i <= arcSegments; ++i)
        {
            float angle = axisAngle + MATH_PI / 2.0f + (static_cast<float>(i) / arcSegments) * MATH_PI;
            float x = rc1.x + std::cos(angle) * radius;
            float y = rc1.y + std::sin(angle) * radius;
            arc1[i].x = screenCenter.x + x * scaleX;
            arc1[i].y = screenCenter.y - y * scaleY;
        }
        
        SDL_FPoint arc2[arcSegments + 1];
        for (int i = 0; i <= arcSegments; ++i)
        {
            float angle = axisAngle - MATH_PI / 2.0f + (static_cast<float>(i) / arcSegments) * MATH_PI;
            float x = rc2.x + std::cos(angle) * radius;
            float y = rc2.y + std::sin(angle) * radius;
            arc2[i].x = screenCenter.x + x * scaleX;
            arc2[i].y = screenCenter.y - y * scaleY;
        }

        SDL_SetRenderDrawColor(rendererRef, color.r, color.g, color.b, color.a);
        SDL_RenderLines(rendererRef, arc1, arcSegments + 1);
        SDL_RenderLines(rendererRef, arc2, arcSegments + 1);
        
        Vec2 p1a = Vec2(rc1.x + perp.x * radius, rc1.y + perp.y * radius);
        Vec2 p1b = Vec2(rc2.x + perp.x * radius, rc2.y + perp.y * radius);
        Vec2 p2a = Vec2(rc1.x - perp.x * radius, rc1.y - perp.y * radius);
        Vec2 p2b = Vec2(rc2.x - perp.x * radius, rc2.y - perp.y * radius);

        SDL_RenderLine(rendererRef,
                       screenCenter.x + p1a.x * scaleX, screenCenter.y - p1a.y * scaleY,
                       screenCenter.x + p1b.x * scaleX, screenCenter.y - p1b.y * scaleY);
        SDL_RenderLine(rendererRef,
                       screenCenter.x + p2a.x * scaleX, screenCenter.y - p2a.y * scaleY,
                       screenCenter.x + p2b.x * scaleX, screenCenter.y - p2b.y * scaleY);
    }

    void Renderer::DrawDebugPolygon(const Vec2& screenCenter, const std::vector<Vec2>& vertices,
                                    float rotationDegrees, float scaleX, float scaleY, RGBA color) const
    {
        if (vertices.size() < 3) return;

        std::vector<SDL_FPoint> screenVerts(vertices.size() + 1);

        for (size_t i = 0; i < vertices.size(); ++i)
        {
            Vec2 rotated = RotatePoint(vertices[i], rotationDegrees);
            screenVerts[i].x = screenCenter.x + rotated.x * scaleX;
            screenVerts[i].y = screenCenter.y - rotated.y * scaleY;
        }
        screenVerts[vertices.size()] = screenVerts[0];

        SDL_SetRenderDrawColor(rendererRef, color.r, color.g, color.b, color.a);
        SDL_RenderLines(rendererRef, screenVerts.data(), static_cast<int>(screenVerts.size()));
    }

    Vec2 Renderer::WorldToScreen(const Vec2& worldPos, float globalScaleX, float globalScaleY) const
    {
        Vec2 cameraRelative = camera.ApplyCameraTransform(worldPos);

        float screenX, screenY;
        if (scalingMode == ScalingMode::PixelBased)
        {
            screenX = cameraRelative.x + (static_cast<float>(windowWidth) / 2.0f);
            screenY = -cameraRelative.y + (static_cast<float>(windowHeight) / 2.0f);
        }
        else
        {
            screenX = (cameraRelative.x * globalScaleX) + (static_cast<float>(windowWidth) / 2.0f);
            screenY = (-cameraRelative.y * globalScaleY) + (static_cast<float>(windowHeight) / 2.0f);
        }

        return Vec2(screenX, screenY);
    }

    Vec2 Renderer::RotatePoint(const Vec2& point, float angleDegrees) const
    {
        float rad = angleDegrees * MATH_PI / 180.0f;
        float cosA = std::cos(rad);
        float sinA = std::sin(rad);
        return Vec2(
            point.x * cosA - point.y * sinA,
            point.x * sinA + point.y * cosA
        );
    }

    RGBA Renderer::GetDebugColor(const Entity& entity) const
    {
        if (entity.collider.type == ColliderType::TRIGGER)
        {
            return entity.physApplied ? RGBA(0, 255, 0, 255) : RGBA(0, 255, 255, 200);
        }
        else
        {
            return entity.physApplied ? RGBA(255, 0, 0, 255) : RGBA(255, 255, 0, 200);
        }
    }
}
