#include "UIManager.h"

namespace SquareCore
{
    uint32_t UIManager::AddRect(float x_pos, float y_pos, float width, float height, RGBA color, std::string textStr, Border border, const std::string& fontPath, float fontSize, RGBA textColor)
    {
        std::lock_guard<std::mutex> lock(uiMutex);

        UIRect* rect = new UIRect();
        rect->ID = nextElementID++;
        rect->x = x_pos;
        rect->y = y_pos;
        rect->width = width;
        rect->height = height;
        rect->color = color;
        rect->border = border;

        if (!textStr.empty() && !fontPath.empty() && textEngineRef)
        {
            rect->text.fontPath = fontPath;
            rect->text.fontSize = fontSize;
            
            float scale = std::min(currentScaleX, currentScaleY);
            float scaledFontSize = fontSize * scale;
            
            rect->text.font = TTF_OpenFont(fontPath.c_str(), scaledFontSize);
            rect->text.text = textStr;
            if (rect->text.font)
            {
                TTF_SetFontSize(rect->text.font, scaledFontSize);
                rect->text.textObject = TTF_CreateText(textEngineRef, rect->text.font, textStr.c_str(), textStr.length());
                if (rect->text.textObject)
                {
                    TTF_SetTextColor(rect->text.textObject, textColor.r, textColor.g, textColor.b, textColor.a);
                }
            }
        }

        elements[rect->ID] = rect;
        return rect->ID;
    }

    uint32_t UIManager::AddButton(float x_pos, float y_pos, float width, float height, RGBA color, std::string textStr, Border border, std::function<void()> onPress, const std::string& fontPath, float fontSize, RGBA textColor)
    {
        std::lock_guard<std::mutex> lock(uiMutex);

        UIButton* button = new UIButton();
        button->ID = nextElementID++;
        button->x = x_pos;
        button->y = y_pos;
        button->width = width;
        button->height = height;
        button->color = color;
        button->border = border;
        button->onPress = onPress;

        if (!textStr.empty() && !fontPath.empty() && textEngineRef)
        {
            button->text.fontPath = fontPath;
            button->text.fontSize = fontSize;
            
            float scale = std::min(currentScaleX, currentScaleY);
            float scaledFontSize = fontSize * scale;
            
            button->text.font = TTF_OpenFont(fontPath.c_str(), scaledFontSize);
            button->text.text = textStr;
            if (button->text.font)
            {
                TTF_SetFontSize(button->text.font, scaledFontSize);
                button->text.textObject = TTF_CreateText(textEngineRef, button->text.font, textStr.c_str(), textStr.length());
                if (button->text.textObject)
                {
                    TTF_SetTextColor(button->text.textObject, textColor.r, textColor.g, textColor.b, textColor.a);
                }
            }
        }

        elements[button->ID] = button;
        return button->ID;
    }

    uint32_t UIManager::AddText(float x_pos, float y_pos, float fontSize, RGBA color, const std::string& fontPath, const std::string& textStr)
    {
        std::lock_guard<std::mutex> lock(uiMutex);

        UIText* ui_text = new UIText();
        ui_text->ID = nextElementID++;
        ui_text->x = x_pos;
        ui_text->y = y_pos;
        ui_text->color = color;
        ui_text->text.fontPath = fontPath;
        ui_text->text.fontSize = fontSize;
        
        float scale = std::min(currentScaleX, currentScaleY);
        float scaledFontSize = fontSize * scale;
        
        TTF_Font* font = TTF_OpenFont(fontPath.c_str(), scaledFontSize);
        ui_text->text.font = font;
        ui_text->text.text = textStr;
        
        if (font && textEngineRef)
        {
            TTF_SetFontSize(font, scaledFontSize);
            ui_text->text.textObject = TTF_CreateText(textEngineRef, font, textStr.c_str(), textStr.length());
            if (ui_text->text.textObject)
            {
                TTF_SetTextColor(ui_text->text.textObject, color.r, color.g, color.b, color.a);
            }
        }

        elements[ui_text->ID] = ui_text;
        return ui_text->ID;
    }

    void UIManager::DeleteElement(uint32_t id)
    {
        std::lock_guard<std::mutex> lock(uiMutex);

        auto it = elements.find(id);
        if (it != elements.end())
        {
            UIElement* elem = it->second;
            if (elem->text.textObject) TTF_DestroyText(elem->text.textObject);
            if (elem->text.font) TTF_CloseFont(elem->text.font);
            
            if (elem->type == UIElementType::RECT)
            {
                UIRect* rect = static_cast<UIRect*>(elem);
                if (rect->sprite) SDL_DestroyTexture(rect->sprite);
            }
            else if (elem->type == UIElementType::BUTTON)
            {
                UIButton* button = static_cast<UIButton*>(elem);
                if (button->sprite) SDL_DestroyTexture(button->sprite);
                if (button->hoverSprite) SDL_DestroyTexture(button->hoverSprite);
                if (button->pressedSprite) SDL_DestroyTexture(button->pressedSprite);
            }
            
            delete elem;
            elements.erase(it);
        }
    }

    void UIManager::ClearElements()
    {
        std::lock_guard<std::mutex> lock(uiMutex);

        for (auto it = elements.begin(); it != elements.end(); ) {
            if (!it->second->persistent) {
                delete it->second;
                it = elements.erase(it);
            } else {
                ++it;
            }
        }
    }

    void UIManager::Update()
    {
        if (!inputRef) return;

        std::function<void()> callbackExecute = nullptr;

        {
            std::lock_guard<std::mutex> lock(uiMutex);

            Vec2 mousePos = inputRef->GetMousePosition();
            bool leftButtonPressed = inputRef->GetMouseButtonPressed(0);
            bool leftButtonReleased = inputRef->GetMouseButtonReleased(0);

            for (auto& pair : elements)
            {
                UIElement* elem = pair.second;
                if (!elem->visible || elem->type != UIElementType::BUTTON) continue;

                UIButton* button = static_cast<UIButton*>(elem);

                float scaledX = button->x * currentScaleX;
                float scaledY = button->y * currentScaleY;
                float scaledW = button->width * currentScaleX;
                float scaledH = button->height * currentScaleY;

                button->isHovered = PointInRect(mousePos.x, mousePos.y, scaledX, scaledY, scaledW, scaledH);
                button->isPressed = button->isHovered && leftButtonPressed;

                if (button->isHovered && leftButtonReleased)
                {
                    if (button->onPress)
                    {
                        callbackExecute = button->onPress;
                        break;
                    }
                }
            }
        }

        if (callbackExecute)
        {
            callbackExecute();
        }
    }

    UIElement* UIManager::GetElementByID(uint32_t id)
    {
        std::lock_guard<std::mutex> lock(uiMutex);

        auto it = elements.find(id);
        return (it != elements.end()) ? it->second : nullptr;
    }

    bool UIManager::ElementExists(uint32_t id) const
    {
        std::lock_guard<std::mutex> lock(uiMutex);
        return elements.find(id) != elements.end();
    }

    void UIManager::SetButtonCallback(uint32_t buttonID, std::function<void()> onPress)
    {
        std::lock_guard<std::mutex> lock(uiMutex);

        auto it = elements.find(buttonID);
        if (it != elements.end() && it->second->type == UIElementType::BUTTON)
        {
            static_cast<UIButton*>(it->second)->onPress = onPress;
        }
    }

    void UIManager::SetElementVisible(uint32_t elementID, bool visible)
    {
        std::lock_guard<std::mutex> lock(uiMutex);

        auto it = elements.find(elementID);
        if (it != elements.end())
        {
            it->second->visible = visible;
        }
    }

    void UIManager::SetElementPersistent(uint32_t elementID, bool persistent)
    {
        auto it = elements.find(elementID);
        if (it != elements.end()) {
            it->second->persistent = persistent;
        }
    }

    void UIManager::SetElementPosition(uint32_t elementID, float x, float y)
    {
        std::lock_guard<std::mutex> lock(uiMutex);

        auto it = elements.find(elementID);
        if (it != elements.end())
        {
            it->second->x = x;
            it->second->y = y;
        }
    }

    void UIManager::SetElementColor(uint32_t elementID, RGBA color)
    {
        std::lock_guard<std::mutex> lock(uiMutex);

        auto it = elements.find(elementID);
        if (it != elements.end())
        {
            it->second->color = color;
        }
    }

    void UIManager::SetElementZIndex(uint32_t elementID, int zIndex)
    {
        std::lock_guard<std::mutex> lock(uiMutex);

        auto it = elements.find(elementID);
        if (it != elements.end())
        {
            it->second->zIndex = zIndex;
        }
    }

    void UIManager::SetTextColor(uint32_t elementID, RGBA color)
    {
        std::lock_guard<std::mutex> lock(uiMutex);

        auto it = elements.find(elementID);
        if (it != elements.end() && it->second->text.textObject)
        {
            TTF_SetTextColor(it->second->text.textObject, color.r, color.g, color.b, color.a);
        }
    }

    bool UIManager::PointInRect(float px, float py, float rx, float ry, float rw, float rh) const
    {
        return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
    }

    void UIManager::SetUIText(uint32_t elementID, const std::string& newText)
    {
        std::lock_guard<std::mutex> lock(uiMutex);

        auto it = elements.find(elementID);
        if (it != elements.end())
        {
            UIElement* elem = it->second;
            elem->text.text = newText;
            
            if (elem->text.textObject && elem->text.font && textEngineRef)
            {
                RGBA currentColor;
                TTF_GetTextColor(elem->text.textObject, &currentColor.r, &currentColor.g, &currentColor.b, &currentColor.a);
                
                TTF_DestroyText(elem->text.textObject);
                elem->text.textObject = TTF_CreateText(textEngineRef, elem->text.font, newText.c_str(), newText.length());
                if (elem->text.textObject)
                {
                    TTF_SetTextColor(elem->text.textObject, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
                }
            }
        }
    }

    void UIManager::SetElementSprite(uint32_t elementID, std::string spritePath)
    {
        auto it = elements.find(elementID);
        if (it != elements.end())
        {
            if (it->second->type == UIElementType::TEXT || it->second->type == UIElementType::BUTTON)
                return;
            
            UIRect* rect = static_cast<UIRect*>(it->second);
            
            if (rect->sprite)
            {
                SDL_DestroyTexture(rect->sprite);
                rect->sprite = nullptr;
            }
            
            rect->spritePath = spritePath;
        }
    }

    void UIManager::SetButtonSprites(uint32_t elementID, std::string spritePath,
                                     std::string hoveredSpritePath, std::string pressedSpritePath)
    {
        auto it = elements.find(elementID);
        if (it != elements.end())
        {
            if (it->second->type == UIElementType::RECT || it->second->type == UIElementType::TEXT)
                return;
            
            UIButton* button = static_cast<UIButton*>(it->second);
            
            if (button->sprite)
            {
                SDL_DestroyTexture(button->sprite);
                button->sprite = nullptr;
            }
            if (button->hoverSprite)
            {
                SDL_DestroyTexture(button->hoverSprite);
                button->hoverSprite= nullptr;
            }
            if (button->pressedSprite)
            {
                SDL_DestroyTexture(button->pressedSprite);
                button->pressedSprite = nullptr;
            }
            
            button->spritePath = spritePath;
            button->hoverSpritePath = hoveredSpritePath;
            button->pressedSpritePath = pressedSpritePath;
        }
    }

    Vec2 UIManager::GetTextSize(uint32_t elementID)
    {
        int width = 0.0f;
        int height = 0.0f;
        TTF_GetTextSize(GetElementByID(elementID)->text.textObject, &width, &height);
        return Vec2((float)width, (float)height);
    }

    void UIManager::AddTagToUIElement(uint32_t elementID, const std::string& tag)
    {
        std::lock_guard<std::mutex> lock(uiMutex);

        auto it = elements.find(elementID);
        if (it != elements.end())
        {
            it->second->tags.push_back(tag);
        }
    }

    void UIManager::RemoveTagFromUIElement(uint32_t elementID, const std::string& tag)
    {
        std::lock_guard<std::mutex> lock(uiMutex);

        auto it = elements.find(elementID);
        if (it != elements.end())
        {
            auto& tagVec = it->second->tags;
            auto tagIt = std::find(tagVec.begin(), tagVec.end(), tag);
            if (tagIt != tagVec.end())
            {
                tagVec.erase(tagIt);
            }
        }
    }

    std::vector<uint32_t> UIManager::GetAllUIElementsWithTag(std::string tag)
    {
        std::lock_guard<std::mutex> lock(uiMutex);
        std::vector<uint32_t> elementIDs;

        for (auto& pair : elements)
        {
            UIElement* element = pair.second;
            for (const auto& elementTag : element->tags)
            {
                if (elementTag == tag)
                {
                    elementIDs.push_back(element->ID);
                    break;
                }
            }
        }

        return elementIDs;
    }

    uint32_t UIManager::GetFirstUIElementWithTag(std::string tag)
    {
        std::lock_guard<std::mutex> lock(uiMutex);

        for (auto& pair : elements)
        {
            UIElement* element = pair.second;
            for (const auto& elementTag : element->tags)
            {
                if (elementTag == tag)
                {
                    return element->ID;
                }
            }
        }

        return 0;
    }

    bool UIManager::UIElementHasTag(uint32_t elementID, std::string tag)
    {
        std::lock_guard<std::mutex> lock(uiMutex);

        auto it = elements.find(elementID);
        if (it != elements.end())
        {
            return std::find(it->second->tags.begin(), it->second->tags.end(), tag) != it->second->tags.end();
        }
        
        return false;
    }

    void UIManager::OnWindowResize(int windowWidth, int windowHeight, float baseWidth, float baseHeight)
    {
        std::lock_guard<std::mutex> lock(uiMutex);

        float newScaleX = static_cast<float>(windowWidth) / baseWidth;
        float newScaleY = static_cast<float>(windowHeight) / baseHeight;
        float newScale = std::min(newScaleX, newScaleY);
        float oldScale = std::min(currentScaleX, currentScaleY);
        
        if (std::abs(newScale - oldScale) < 0.001f)
            return;

        currentScaleX = newScaleX;
        currentScaleY = newScaleY;

        for (auto& [id, element] : elements)
        {
            if (element->text.fontPath.empty() || element->text.fontSize <= 0.0f)
                continue;
            
            float scaledFontSize = element->text.fontSize * newScale;
            
            RGBA textColor = element->text.color;
            if (element->text.textObject)
            {
                TTF_GetTextColor(element->text.textObject, &textColor.r, &textColor.g, &textColor.b, &textColor.a);
            }
            
            if (element->text.font)
            {
                TTF_SetFontSize(element->text.font, scaledFontSize);
            }
            
            if (element->text.textObject)
            {
                TTF_DestroyText(element->text.textObject);
                element->text.textObject = nullptr;
            }
            
            if (element->text.font && textEngineRef && !element->text.text.empty())
            {
                element->text.textObject = TTF_CreateText(textEngineRef, element->text.font,
                                                          element->text.text.c_str(),
                                                          element->text.text.length());
                if (element->text.textObject)
                {
                    TTF_SetTextColor(element->text.textObject, textColor.r, textColor.g, textColor.b, textColor.a);
                }
            }
        }
    }
}
