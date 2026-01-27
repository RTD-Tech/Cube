#pragma once

#include <cstdint>

namespace SquareCore
{
    struct RGB
    {
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;

        RGB(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0) { this->r = r; this->g = g; this->b = b; }
    };
    struct RGBA
    {
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 255;
        
        RGBA(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255) { this->r = r; this->g = g; this->b = b; this->a = a; }
    };
}
