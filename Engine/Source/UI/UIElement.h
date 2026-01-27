#pragma once

#include "UI/Color.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <functional>
#include <string>

namespace SquareCore
{
    enum class UIElementType
    {
        RECT,
        BUTTON,
        TEXT
    };

    struct Border
    {
        RGBA color = RGBA(0, 0, 0, 255);
        float thickness = 1.0f;
        float radius = 0.0f;
    };

    struct Text
    {
        std::string text = "";
        std::string fontPath = "";
        float fontSize = 4.0f;
        TTF_Font* font = nullptr;
        TTF_Text* textObject = nullptr;
        RGBA color;
    };

    struct UIElement
    {
        uint32_t ID = 0;
        UIElementType type = UIElementType::RECT;
        float x = 0.0f;
        float y = 0.0f;
        float width = 100.0f;
        float height = 50.0f;
        bool visible = true;
        bool persistent = false;
        RGBA color = RGBA(255, 255, 255, 255);
        int zIndex = 0;
        Border border;
        Text text;
        std::vector<std::string> tags;
    };

    struct UIRect : UIElement
    {
        UIRect() { type = UIElementType::RECT; }
        
        std::string spritePath = "";
        SDL_Texture* sprite = nullptr;
    };

    struct UIButton : UIElement
    {
        UIButton() { type = UIElementType::BUTTON; }

        bool isHovered = false;
        bool isPressed = false;

        RGBA hoverColor = RGBA(200, 200, 200, 255);
        RGBA pressedColor = RGBA(150, 150, 150, 255);
        
        std::string spritePath = "";
        std::string hoverSpritePath = "";
        std::string pressedSpritePath = "";
        
        SDL_Texture* sprite = nullptr;
        SDL_Texture* hoverSprite = nullptr;
        SDL_Texture* pressedSprite = nullptr;

        std::function<void()> onPress;
    };
    
    struct UIText : UIElement
    {
        UIText() { type = UIElementType::TEXT; }
    };
}
