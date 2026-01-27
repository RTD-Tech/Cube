#pragma once

namespace SquareCore {

struct InputEvent {
    enum class Type { Key, MouseButton, MouseMove, MouseScroll };
    Type type;
    int code;
    bool pressed;
    float x, y;
};

}
