#pragma once

#include <iostream>

namespace CubeCore {

struct InputEvent {
    enum class Type { Key, MouseButton, MouseMove, MouseScroll };
    Type type;
    int code;
    bool pressed;
    float x, y;
};

}
