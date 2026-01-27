#ifndef INPUT_H
#define INPUT_H

#include "Math/Math.h"
#include "KeyState.h"
#include "InputEvent.h"
#include <SDL3/SDL_scancode.h>
#include <mutex>
#include <vector>

namespace SquareCore {

class Input {
public:
    Input();

    void QueueKeyEvent(SDL_Scancode key, bool pressed);
    void QueueMouseButtonEvent(int button, bool pressed);
    void QueueMouseMove(float x, float y);
    void QueueMouseScroll(float xOffset, float yOffset);

    void ProcessEvents();
    void EndFrame();

    bool GetKeyPressed(SDL_Scancode key);
    bool GetKeyHeld(SDL_Scancode key);
    bool GetKeyReleased(SDL_Scancode key);

    bool GetMouseButtonPressed(int button);
    bool GetMouseButtonHeld(int button);
    bool GetMouseButtonReleased(int button);

    Vec2 GetMousePosition() const { return mousePosition; }
    Vec2 GetMouseDelta() const { return mouseDelta; }
    Vec2 GetMouseScroll() const { return mouseScroll; }

private:
    std::mutex queueMutex;
    std::vector<InputEvent> eventQueue;
    
    KeyState keyStates[SDL_SCANCODE_COUNT] = {};

    static constexpr int MAX_MOUSE_BUTTONS = 8;
    KeyState mouseButtonStates[MAX_MOUSE_BUTTONS] = {};
    
    Vec2 mousePosition = {0.0f, 0.0f};
    Vec2 previousMousePosition = {0.0f, 0.0f};
    Vec2 mouseDelta = {0.0f, 0.0f};
    Vec2 mouseScroll = {0.0f, 0.0f};
    bool firstMouse = true;
};

}

#endif
