#include "SceneManager.h"
#include "UI/UIElement.h"
#include <SDL3/SDL_log.h>
#include <json/json.hpp>
#include <filesystem>
#include <fstream>

namespace SquareCore
{
    bool SceneManager::SaveScene(const std::string& filepath) {
        nlohmann::json sceneJson;

        if (rendererRef)
        {
            RGBA bgc = rendererRef->GetBackgroundColor();
            sceneJson["background-color"] = {bgc.r, bgc.g, bgc.b, bgc.a};
        }
        
        sceneJson["entities"] = nlohmann::json::array();
        auto entities = entityManagerRef->GetEntitiesCopy();

        for (const auto& entity : entities) {
            if (entity.persistent) continue;

            nlohmann::json entityJson;
            
            entityJson["isSpriteless"] = entity.isSpriteless;
            if (entity.isSpriteless) {
                entityJson["spritelessWidth"] = entity.spritelessWidth;
                entityJson["spritelessHeight"] = entity.spritelessHeight;
                entityJson["spritelessColor"] = {
                    entity.spritelessColor.r,
                    entity.spritelessColor.g,
                    entity.spritelessColor.b,
                    entity.spritelessColor.a
                };
            } else {
                entityJson["spritePath"] = entity.spritePath;
                if (entity.totalFrames != 1) entityJson["totalFrames"] = entity.totalFrames;
                if (entity.fps != 0.0f) entityJson["fps"] = entity.fps;
            }
            
            entityJson["position"] = { entity.position.x, entity.position.y };
            if (entity.rotation != 0.0f) entityJson["rotation"] = entity.rotation;
            if (entity.scale.x != 1.0f || entity.scale.y != 1.0f) {
                entityJson["scale"] = { entity.scale.x, entity.scale.y };
            }
            if (entity.flipX) entityJson["flipX"] = true;
            if (entity.flipY) entityJson["flipY"] = true;
            if (!entity.visible) entityJson["visible"] = false;
            
            if (entity.color.r != 255 || entity.color.g != 255 || entity.color.b != 255 || entity.color.a != 255) {
                entityJson["color"] = { entity.color.r, entity.color.g, entity.color.b, entity.color.a };
            }

            entityJson["zIndex"] = entity.zIndex;
            
            if (entity.physApplied) entityJson["physApplied"] = true;
            if (entity.mass != 1.0f) entityJson["mass"] = entity.mass;
            if (entity.drag != 0.0f) entityJson["drag"] = entity.drag;
            
            nlohmann::json colliderJson;
            if (entity.collider.type != ColliderType::SOLID) {
                colliderJson["type"] = static_cast<int>(entity.collider.type);
            }
            if (!entity.collider.enabled) colliderJson["enabled"] = false;
            if (entity.collider.offset.x != 0.0f || entity.collider.offset.y != 0.0f) {
                colliderJson["offset"] = { entity.collider.offset.x, entity.collider.offset.y };
            }
            if (entity.collider.size.x != 0.0f || entity.collider.size.y != 0.0f) {
                colliderJson["size"] = { entity.collider.size.x, entity.collider.size.y };
            }
            if (!colliderJson.empty()) entityJson["collider"] = colliderJson;
            
            if (!entity.tags.empty()) entityJson["tags"] = entity.tags;

            sceneJson["entities"].push_back(entityJson);
        }
        
        sceneJson["ui"] = nlohmann::json::array();
        auto& uiElements = uiManagerRef->GetElements();

        for (const auto& [id, element] : uiElements) {
            if (element->persistent || element->type == UIElementType::BUTTON) continue;

            nlohmann::json uiJson;

            uiJson["type"] = static_cast<int>(element->type);
            uiJson["x"] = element->x;
            uiJson["y"] = element->y;
            
            if (element->type != UIElementType::TEXT) {
                uiJson["width"] = element->width;
                uiJson["height"] = element->height;
            }

            if (!element->visible) uiJson["visible"] = false;
            if (element->zIndex != 0) uiJson["zIndex"] = element->zIndex;
            if (!element->tags.empty()) uiJson["tags"] = element->tags;
            
            if (element->color.r != 255 || element->color.g != 255 || element->color.b != 255 || element->color.a != 255) {
                uiJson["color"] = { element->color.r, element->color.g, element->color.b, element->color.a };
            }
            
            nlohmann::json borderJson;
            if (element->border.color.r != 0 || element->border.color.g != 0 ||
                element->border.color.b != 0 || element->border.color.a != 255) {
                borderJson["color"] = {
                    element->border.color.r, element->border.color.g,
                    element->border.color.b, element->border.color.a
                };
            }
            if (element->border.thickness != 1.0f) borderJson["thickness"] = element->border.thickness;
            if (element->border.radius != 0.0f) borderJson["radius"] = element->border.radius;
            if (!borderJson.empty()) uiJson["border"] = borderJson;
            
            nlohmann::json textJson;
            if (!element->text.text.empty()) textJson["content"] = element->text.text;
            if (!element->text.fontPath.empty()) textJson["fontPath"] = element->text.fontPath;
            if (element->text.fontSize != 16.0f) textJson["fontSize"] = element->text.fontSize;
            if (element->text.color.r != 0 || element->text.color.g != 0 ||
                element->text.color.b != 0 || element->text.color.a != 255) {
                textJson["color"] = {
                    element->text.color.r, element->text.color.g,
                    element->text.color.b, element->text.color.a
                };
            }
            if (!textJson.empty()) uiJson["text"] = textJson;

            sceneJson["ui"].push_back(uiJson);
        }
        
        std::filesystem::path path(filepath);
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }

        std::ofstream file(filepath);
        if (!file.is_open()) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to save scene to %s", filepath.c_str());
            return false;
        }

        file << sceneJson.dump(4);
        return true;
    }

    bool SceneManager::LoadScene(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open scene file %s", filepath.c_str());
            return false;
        }
        
        nlohmann::json sceneJson;
        try {
            sceneJson = nlohmann::json::parse(file, nullptr, true, true);
        } catch (const nlohmann::json::exception& e) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to parse scene JSON: %s", e.what());
            return false;
        }
        
        if (physicsRef)
        {
            physicsRef->ClearBodies();
        }
        if (entityManagerRef)
        {
            entityManagerRef->ClearEntities();
        }
        if (uiManagerRef)
        {
            uiManagerRef->ClearElements();
        }
        if (sceneJson.contains("background-color"))
        {
            if (rendererRef)
            {
                std::vector<uint8_t> background_color = sceneJson["background-color"].get<std::vector<uint8_t>>();
                rendererRef->SetBackgroundColor(RGBA(background_color[0], background_color[1], background_color[2], background_color[3]));
            }
        }
        if (sceneJson.contains("entities")) {
            for (const auto& entityJson : sceneJson["entities"]) {
                uint32_t id;

                bool isSpriteless = entityJson.value("isSpriteless", false);
                float posX = 0.0f;
                float posY = 0.0f;
                if (entityJson.contains("position"))
                {
                    posX = entityJson["position"][0];
                    posY = entityJson["position"][1];
                }
                float rotation = entityJson.value("rotation", 0.0f);
                float scaleX = 1.0f;
                float scaleY = 1.0f;
                if (entityJson.contains("scale")) {
                    scaleX = entityJson["scale"][0];
                    scaleY = entityJson["scale"][1];
                }
                int zIndex = entityJson["zIndex"];

                std::vector<std::string> tags;
                if (entityJson.contains("tags")) {
                    tags = entityJson["tags"].get<std::vector<std::string>>();
                }

                if (isSpriteless) {
                    float width = entityJson.value("spritelessWidth", 10.0f);
                    float height = entityJson.value("spritelessHeight", 10.0f);
                    RGBA color(
                        entityJson["spritelessColor"][0],
                        entityJson["spritelessColor"][1],
                        entityJson["spritelessColor"][2],
                        entityJson["spritelessColor"][3]
                    );
                    id = entityManagerRef->AddSpritelessEntity(width, height, color, posX, posY, rotation, scaleX, scaleY);
                } else {
                    std::string spritePath = entityJson.value("spritePath", "");
                    int totalFrames = entityJson.value("totalFrames", 1);
                    float fps = entityJson.value("fps", 0.0f);

                    if (totalFrames > 1 || fps > 0) {
                        id = entityManagerRef->AddAnimatedEntity(spritePath.c_str(), totalFrames, fps, posX, posY, rotation, scaleX,
                                                                 scaleY, false, tags);
                    } else {
                        id = entityManagerRef->AddEntity(spritePath.c_str(), posX, posY, rotation, scaleX, scaleY, false, tags);
                    }
                }

                Entity* entity = entityManagerRef->GetEntityByID(id);
                if (entity) {
                    entity->flipX = entityJson.value("flipX", false);
                    entity->flipY = entityJson.value("flipY", false);
                    entity->visible = entityJson.value("visible", true);
                    entity->mass = entityJson.value("mass", 1.0f);
                    entity->drag = entityJson.value("drag", 0.0f);
                    entity->tags = tags;
                    entity->zIndex = zIndex;
                    
                    if (entityJson.contains("physApplied"))
                    {
                        entityManagerRef->SetPhysicsEnabled(id, entityJson["physApplied"].get<bool>());
                    }

                    if (entityJson.contains("color")) {
                        entity->color = RGBA(
                            entityJson["color"][0],
                            entityJson["color"][1],
                            entityJson["color"][2],
                            entityJson["color"][3]
                        );
                    }

                    if (entityJson.contains("collider")) {
                        if (entityJson["collider"].contains("type")) {
                            entityManagerRef->SetColliderType(id, static_cast<ColliderType>(entityJson["collider"]["type"].get<int>()));
                        }
                        if (entityJson["collider"].contains("enabled")) {
                            entity->collider.enabled = entityJson["collider"]["enabled"].get<bool>();
                        }
                        if (entityJson["collider"].contains("offset")) {
                            entity->collider.offset = Vec2(entityJson["collider"]["offset"][0],
                                                           entityJson["collider"]["offset"][1]);
                        }
                        if (entityJson["collider"].contains("size")) {
                            entity->collider.size = Vec2(entityJson["collider"]["size"][0],
                                                         entityJson["collider"]["size"][1]);
                        }
                    }
                }
            }
        }
        
        if (sceneJson.contains("ui")) {
            for (const auto& uiJson : sceneJson["ui"]) {
                UIElementType type = static_cast<UIElementType>(uiJson.value("type", 0));
                
                if (type == UIElementType::BUTTON) continue;

                uint32_t id;
                float x = uiJson.value("x", 0.0f);
                float y = uiJson.value("y", 0.0f);
                float width = uiJson.value("width", 100.0f);
                float height = uiJson.value("height", 50.0f);

                RGBA color(255, 255, 255, 255);
                if (uiJson.contains("color")) {
                    color = RGBA(uiJson["color"][0], uiJson["color"][1], uiJson["color"][2], uiJson["color"][3]);
                }

                Border border;
                if (uiJson.contains("border")) {
                    if (uiJson["border"].contains("color")) {
                        border.color = RGBA(
                            uiJson["border"]["color"][0],
                            uiJson["border"]["color"][1],
                            uiJson["border"]["color"][2],
                            uiJson["border"]["color"][3]
                        );
                    }
                    border.thickness = uiJson["border"].value("thickness", 1.0f);
                    border.radius = uiJson["border"].value("radius", 0.0f);
                }

                std::string textContent = "";
                std::string fontPath = "";
                float fontSize = 16.0f;
                RGBA textColor(0, 0, 0, 255);

                if (uiJson.contains("text")) {
                    textContent = uiJson["text"].value("content", "");
                    fontPath = uiJson["text"].value("fontPath", "");
                    fontSize = uiJson["text"].value("fontSize", 16.0f);
                    if (uiJson["text"].contains("color")) {
                        textColor = RGBA(
                            uiJson["text"]["color"][0],
                            uiJson["text"]["color"][1],
                            uiJson["text"]["color"][2],
                            uiJson["text"]["color"][3]
                        );
                    }
                }

                if (type == UIElementType::TEXT) {
                    id = uiManagerRef->AddText(x, y, fontSize, textColor, fontPath, textContent);
                } else {
                    id = uiManagerRef->AddRect(x, y, width, height, color, textContent, border, fontPath, fontSize, textColor);
                }

                UIElement* element = uiManagerRef->GetElementByID(id);
                if (element) {
                    element->visible = uiJson.value("visible", true);
                    element->zIndex = uiJson.value("zIndex", 0);

                    if (uiJson.contains("tags")) {
                        element->tags = uiJson["tags"].get<std::vector<std::string>>();
                    }
                }
            }
        }

        return true;
    }
}
