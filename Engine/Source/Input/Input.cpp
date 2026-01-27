#include "Input.h"

namespace SquareCore {

Input::Input() {
    std::memset(keyStates, 0, sizeof(keyStates));
    std::memset(mouseButtonStates, 0, sizeof(mouseButtonStates));
}

void Input::QueueKeyEvent(SDL_Scancode key, bool pressed) {
    std::lock_guard<std::mutex> lock(queueMutex);
    eventQueue.push_back({InputEvent::Type::Key, static_cast<int>(key), pressed, 0, 0});
}

void Input::QueueMouseButtonEvent(int button, bool pressed) {
    std::lock_guard<std::mutex> lock(queueMutex);
    eventQueue.push_back({InputEvent::Type::MouseButton, button, pressed, 0, 0});
}

void Input::QueueMouseMove(float x, float y) {
    std::lock_guard<std::mutex> lock(queueMutex);
    eventQueue.push_back({InputEvent::Type::MouseMove, 0, false, x, y});
}

void Input::QueueMouseScroll(float xOffset, float yOffset) {
    std::lock_guard<std::mutex> lock(queueMutex);
    eventQueue.push_back({InputEvent::Type::MouseScroll, 0, false, xOffset, yOffset});
}

void Input::ProcessEvents() {
    std::vector<InputEvent> events;
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        events = std::move(eventQueue);
        eventQueue.clear();
    }

    mouseDelta = {0.0f, 0.0f};
    mouseScroll = {0.0f, 0.0f};

    for (const auto& e : events) {
        switch (e.type) {
        case InputEvent::Type::Key:
            if (e.code >= 0 && e.code < SDL_SCANCODE_COUNT) {
                if (e.pressed) {
                    if (keyStates[e.code] == KeyState::NONE || keyStates[e.code] == KeyState::RELEASED)
                        keyStates[e.code] = KeyState::PRESSED;
                } else {
                    keyStates[e.code] = KeyState::RELEASED;
                }
            }
            break;

        case InputEvent::Type::MouseButton:
            if (e.code >= 0 && e.code < MAX_MOUSE_BUTTONS) {
                if (e.pressed) {
                    if (mouseButtonStates[e.code] == KeyState::NONE || mouseButtonStates[e.code] == KeyState::RELEASED)
                        mouseButtonStates[e.code] = KeyState::PRESSED;
                } else {
                    mouseButtonStates[e.code] = KeyState::RELEASED;
                }
            }
            break;

        case InputEvent::Type::MouseMove:
            previousMousePosition = mousePosition;
            mousePosition = {e.x, e.y};
            if (firstMouse) {
                firstMouse = false;
            } else {
                mouseDelta.x += mousePosition.x - previousMousePosition.x;
                mouseDelta.y += mousePosition.y - previousMousePosition.y;
            }
            break;

        case InputEvent::Type::MouseScroll:
            mouseScroll.x += e.x;
            mouseScroll.y += e.y;
            break;
        }
    }
}

void Input::EndFrame() {
    for (int i = 0; i < SDL_SCANCODE_COUNT; i++) {
        if (keyStates[i] == KeyState::PRESSED)
            keyStates[i] = KeyState::HELD;
        else if (keyStates[i] == KeyState::RELEASED)
            keyStates[i] = KeyState::NONE;
    }
    for (int i = 0; i < MAX_MOUSE_BUTTONS; i++) {
        if (mouseButtonStates[i] == KeyState::PRESSED)
            mouseButtonStates[i] = KeyState::HELD;
        else if (mouseButtonStates[i] == KeyState::RELEASED)
            mouseButtonStates[i] = KeyState::NONE;
    }
}

bool Input::GetKeyPressed(SDL_Scancode key) {
    if (key >= SDL_SCANCODE_COUNT) return false;
    return keyStates[key] == KeyState::PRESSED;
}

bool Input::GetKeyHeld(SDL_Scancode key) {
    if (key >= SDL_SCANCODE_COUNT) return false;
    return keyStates[key] == KeyState::PRESSED || keyStates[key] == KeyState::HELD;
}

bool Input::GetKeyReleased(SDL_Scancode key) {
    if (key >= SDL_SCANCODE_COUNT) return false;
    return keyStates[key] == KeyState::RELEASED;
}

bool Input::GetMouseButtonPressed(int button) {
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) return false;
    return mouseButtonStates[button] == KeyState::PRESSED;
}

bool Input::GetMouseButtonHeld(int button) {
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) return false;
    return mouseButtonStates[button] == KeyState::PRESSED || mouseButtonStates[button] == KeyState::HELD;
}

bool Input::GetMouseButtonReleased(int button) {
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) return false;
    return mouseButtonStates[button] == KeyState::RELEASED;
}

}
