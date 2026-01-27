#pragma once

#include "Renderer/EntityManager.h"
#include "UI/UIManager.h"
#include "Physics/Physics.h"
#include <string>

#include "Renderer/Renderer.h"

namespace SquareCore {

class SceneManager {
public:
    bool SaveScene(const std::string& filepath);
    bool LoadScene(const std::string& filepath);
    
    void SetEntityManager(EntityManager* entityManager) {entityManagerRef = entityManager; }
    void SetUIManager(UIManager* uiManager) { uiManagerRef = uiManager; }
    void SetPhysics(Physics* physics) { physicsRef = physics; }
    void SetRenderer(Renderer* renderer) { rendererRef = renderer; }
    
private:
    EntityManager* entityManagerRef = nullptr;
    UIManager* uiManagerRef = nullptr;
    Physics* physicsRef = nullptr;
    Renderer* rendererRef = nullptr;
};

}
