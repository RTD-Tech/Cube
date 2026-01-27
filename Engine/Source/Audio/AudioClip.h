#pragma once

#include <miniaudio/miniaudio.h>
#include <string>

namespace SquareCore {

struct AudioClip {
    uint32_t id = 0;
    ma_sound sound;
    bool play_on_awake = false;
};

}
