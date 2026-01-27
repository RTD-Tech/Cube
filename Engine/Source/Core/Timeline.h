#ifndef TIMELINE_H
#define TIMELINE_H

#include <atomic>

namespace SquareCore {

class Timeline {
public:
    Timeline() = default;

    // Core time control
    void SetTimeScale(float scale);
    float GetTimeScale() const;
    void SetPaused(bool isPaused);
    bool IsPaused() const;

    // Convenience methods for discrete steps
    void IncreaseTimeScale();  // 0.5x → 1.0x → 2.0x
    void DecreaseTimeScale();  // 2.0x → 1.0x → 0.5x

    // Calculate effective time with pause and scaling applied
    float CalculateEffectiveTime(float rawTime) const;

    // Update total elapsed time
    void Update(float deltaTime);

    // Get the current game time
    float GetCurrentTime() const;

private:
    std::atomic<bool> paused{false};
    std::atomic<float> timeScale{1.0f};
    std::atomic<float> totalElapsedTime{0.0f};
};

}

#endif
