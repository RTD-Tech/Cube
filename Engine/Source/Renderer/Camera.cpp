#include "Camera.h"
#include <algorithm>
#include <cmath>

namespace SquareCore {

Camera::Camera(int viewportWidth, int viewportHeight)
    : viewportWidth(viewportWidth), viewportHeight(viewportHeight)
{
    
}

void Camera::SetPosition(const Vec2& pos)
{
    position = boundsEnabled ? ClampToBounds(pos) : pos;
}

Vec2 Camera::GetPosition() const
{
    return position;
}

void Camera::Move(const Vec2& delta)
{
    SetPosition(position + delta);
}

void Camera::SnapToPosition(const Vec2& pos)
{
    SetPosition(pos);
}

void Camera::SetZoom(float newZoom)
{
    zoom = newZoom;
    ClampZoom();
}

float Camera::GetZoom() const
{
    return zoom;
}

void Camera::SetZoomLimits(float min, float max)
{
    zoomMin = min;
    zoomMax = max;
    ClampZoom();
}

void Camera::ClampZoom()
{
    zoom = std::max(zoomMin, std::min(zoom, zoomMax));
}

void Camera::SetViewportSize(int width, int height)
{
    viewportWidth = width;
    viewportHeight = height;
}

int Camera::GetViewportWidth() const
{
    return viewportWidth;
}

int Camera::GetViewportHeight() const
{
    return viewportHeight;
}

Vec2 Camera::ApplyCameraTransform(const Vec2& worldPos) const
{
    // Apply camera offset and zoom
    Vec2 cameraRelative = (worldPos - position) * zoom;
    return cameraRelative;
}

void Camera::FollowTarget(const Vec2& targetPos, float smoothing, float deltaTime)
{
    // Check if target is in dead zone
    if (IsInDeadZone(targetPos))
    {
        return; // Don't move camera
    }

    // Calculate desired camera position to keep target in view
    Vec2 desiredPos = targetPos;

    // Smooth lerp towards target
    float lerpFactor = 1.0f - std::pow(1.0f / smoothing, deltaTime);
    lerpFactor = std::max(0.0f, std::min(lerpFactor, 1.0f)); // Clamp to [0, 1]

    Vec2 newPos = position + (desiredPos - position) * lerpFactor;

    // Apply bounds if enabled
    SetPosition(newPos);
}

void Camera::SetFollowDeadZone(float width, float height)
{
    deadZoneSize = Vec2(width, height);
}

bool Camera::IsInDeadZone(const Vec2& targetPos) const
{
    // No dead zone configured
    if (deadZoneSize.x <= 0.0f && deadZoneSize.y <= 0.0f)
    {
        return false;
    }

    // Check if target is within dead zone rectangle centered on camera
    float dx = std::abs(targetPos.x - position.x);
    float dy = std::abs(targetPos.y - position.y);

    return dx <= deadZoneSize.x / 2.0f && dy <= deadZoneSize.y / 2.0f;
}

void Camera::SetBounds(const Vec2& min, const Vec2& max)
{
    boundsMin = min;
    boundsMax = max;
}

void Camera::EnableBounds(bool enable)
{
    boundsEnabled = enable;
}

bool Camera::IsBoundsEnabled() const
{
    return boundsEnabled;
}

Vec2 Camera::ClampToBounds(const Vec2& pos) const
{
    if (!boundsEnabled)
    {
        return pos;
    }

    // Calculate visible area in world space
    float halfVisibleWidth = (viewportWidth / zoom) / 2.0f;
    float halfVisibleHeight = (viewportHeight / zoom) / 2.0f;

    // Clamp camera position so visible area stays within bounds
    Vec2 clamped = pos;

    // Handle case where viewport is larger than bounds
    float boundsWidth = boundsMax.x - boundsMin.x;
    float boundsHeight = boundsMax.y - boundsMin.y;

    if (halfVisibleWidth * 2.0f >= boundsWidth)
    {
        // Viewport is wider than bounds, center on bounds
        clamped.x = boundsMin.x + boundsWidth / 2.0f;
    }
    else
    {
        // Normal clamping
        clamped.x = std::max(boundsMin.x + halfVisibleWidth,
                            std::min(pos.x, boundsMax.x - halfVisibleWidth));
    }

    if (halfVisibleHeight * 2.0f >= boundsHeight)
    {
        // Viewport is taller than bounds, center on bounds
        clamped.y = boundsMin.y + boundsHeight / 2.0f;
    }
    else
    {
        // Normal clamping
        clamped.y = std::max(boundsMin.y + halfVisibleHeight,
                            std::min(pos.y, boundsMax.y - halfVisibleHeight));
    }

    return clamped;
}

Vec2 Camera::GetVisibleWorldMin() const
{
    float halfWidth = (viewportWidth / zoom) / 2.0f;
    float halfHeight = (viewportHeight / zoom) / 2.0f;

    return Vec2(position.x - halfWidth, position.y - halfHeight);
}

Vec2 Camera::GetVisibleWorldMax() const
{
    float halfWidth = (viewportWidth / zoom) / 2.0f;
    float halfHeight = (viewportHeight / zoom) / 2.0f;

    return Vec2(position.x + halfWidth, position.y + halfHeight);
}

bool Camera::IsVisible(const Vec2& worldPos, const Vec2& size) const
{
    Vec2 min = GetVisibleWorldMin();
    Vec2 max = GetVisibleWorldMax();

    // AABB overlap test
    bool xOverlap = worldPos.x + size.x >= min.x && worldPos.x <= max.x;
    bool yOverlap = worldPos.y + size.y >= min.y && worldPos.y <= max.y;

    return xOverlap && yOverlap;
}

}
