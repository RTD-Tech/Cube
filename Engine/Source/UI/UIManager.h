#pragma once

#include "UIElement.h"
#include "Input/Input.h"
#include <unordered_map>
#include <mutex>
#include <functional>

namespace SquareCore
{
    class UIManager
    {
    public:
        uint32_t AddRect(float x_pos, float y_pos, float width, float height, RGBA color, std::string text, Border border, const std::string& fontPath = "", float fontSize = 16.0f, RGBA textColor = RGBA(0, 0, 0, 255));
        uint32_t AddButton(float x_pos, float y_pos, float width, float height, RGBA color, std::string text, Border border, std::function<void()> onPress, const std::string& fontPath = "", float fontSize = 16.0f, RGBA textColor = RGBA(0, 0, 0, 255));
        uint32_t AddText(float x_pos, float y_pos, float fontSize, RGBA color, const std::string& fontPath, const std::string& text);
        void DeleteElement(uint32_t id);
        void ClearElements();

        UIElement* GetElementByID(uint32_t id);
        std::unordered_map<uint32_t, UIElement*>& GetElements() { return elements; }
        bool ElementExists(uint32_t id) const;

        void Update();
        
        void SetButtonCallback(uint32_t buttonID, std::function<void()> onPress);
        void SetElementVisible(uint32_t elementID, bool visible);
        void SetElementPersistent(uint32_t elementID, bool persistent);
        void SetElementPosition(uint32_t elementID, float x, float y);
        void SetElementColor(uint32_t elementID, RGBA color);
        void SetElementZIndex(uint32_t elementID, int zIndex);
        void SetTextColor(uint32_t elementID, RGBA color);
        void SetUIText(uint32_t elementID, const std::string& newText);
        void SetElementSprite(uint32_t elementID, std::string spritePath);
        void SetButtonSprites(uint32_t elementID, std::string spritePath, std::string hoveredSpritePath, std::string pressedSpritePath);

        Vec2 GetTextSize(uint32_t elementID);
        
        void AddTagToUIElement(uint32_t elementID, const std::string& tag);
        void RemoveTagFromUIElement(uint32_t elementID, const std::string& tag);
        std::vector<uint32_t> GetAllUIElementsWithTag(std::string tag);
        uint32_t GetFirstUIElementWithTag(std::string tag);
        bool UIElementHasTag(uint32_t elementID, std::string tag);
        
        void SetTextEngine(TTF_TextEngine* textEngine) { textEngineRef = textEngine; }
        void SetInput(Input* input) { inputRef = input; }
        
        void OnWindowResize(int windowWidth, int windowHeight, float baseWidth = 1920.0f, float baseHeight = 1080.0f);

    private:
        mutable std::mutex uiMutex;
        std::unordered_map<uint32_t, UIElement*> elements;
        uint32_t nextElementID = 1;
        
        TTF_TextEngine* textEngineRef = nullptr;
        Input* inputRef = nullptr;
        
        float currentScaleX = 1.0f;
        float currentScaleY = 1.0f;
    
        bool PointInRect(float px, float py, float rx, float ry, float rw, float rh) const;
    };
}