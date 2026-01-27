#ifndef CAMERA_H
#define CAMERA_H

#include "Math/Math.h"

namespace SquareCore {

class Camera {
public:
    Camera(int viewportWidth, int viewportHeight);

    // Position control
    void SetPosition(const Vec2& pos);
    Vec2 GetPosition() const;
    void Move(const Vec2& delta);
    void SnapToPosition(const Vec2& pos);

    // Zoom control
    void SetZoom(float zoom);
    float GetZoom() const;
    void SetZoomLimits(float min, float max);

    // Viewport management
    void SetViewportSize(int width, int height);
    int GetViewportWidth() const;
    int GetViewportHeight() const;

    // Converts world position to camera-relative position
    // This should be called before scaling mode conversion
    Vec2 ApplyCameraTransform(const Vec2& worldPos) const;

    // Follow/tracking
    void FollowTarget(const Vec2& targetPos, float smoothing, float deltaTime);
    void SetFollowDeadZone(float width, float height);
    bool IsInDeadZone(const Vec2& targetPos) const;

    // Bounds constraints
    void SetBounds(const Vec2& min, const Vec2& max);
    void EnableBounds(bool enable);
    bool IsBoundsEnabled() const;
    Vec2 ClampToBounds(const Vec2& pos) const;

    // Viewport culling helpers
    Vec2 GetVisibleWorldMin() const;
    Vec2 GetVisibleWorldMax() const;
    bool IsVisible(const Vec2& worldPos, const Vec2& size) const;

private:
    // Core transform
    Vec2 position = Vec2::zero();
    float zoom = 1.0f;
    float zoomMin = 0.0f;
    float zoomMax = 10.0f;

    // Viewport
    int viewportWidth = 1920;
    int viewportHeight = 1080;

    // Bounds
    bool boundsEnabled = false;
    Vec2 boundsMin = Vec2::zero();
    Vec2 boundsMax = Vec2::zero();

    // Dead zone
    Vec2 deadZoneSize = Vec2::zero();

    // Internal helper to clamp zoom
    void ClampZoom();
};

}

#endif
