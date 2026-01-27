#pragma once

#include "Script.h"
#include "DialogManager.h"
#include "Map.h"

class UserInterface : public SquareCore::Script
{
public:
    void OnStart() override;
    void OnUpdate(float deltaTime) override;

    void OnPlay();

    DialogManager* GetDialogManager() { return &dialogManager; }
    void SetPlayerScript(Player* player_s) { this->player_script = player_s; }
    void SetMap(Map* m) { this->map = m; }
    void AreaTitle(std::string title, std::string info);
    void AbilityGained(std::string title, std::string info);
    void ShowCredits(bool show);

private:
    void DialogUpdate();
    void Pause();
    void UpdateHealthBar();
    void UpdateHeals();
    void UpdateAreaTitle(float deltaTime);
    void UpdateAbilityGained(float deltaTime);
    void UpdateVolumeDisplays();
    
private:
    Map* map = nullptr;
    DialogManager dialogManager;
    uint32_t dialogBox = 0;
    uint32_t speakerText = 0;
    uint32_t dialogText = 0;
    
    uint32_t dialogTestTrigger = 0;

    uint32_t pauseMenuBox = 0;
    uint32_t pauseMenuResumeButton = 0;
    uint32_t pauseMenuQuitButton = 0;
    uint32_t pauseMenuMusicLabel = 0;
    uint32_t pauseMenuMusicMinusButton = 0;
    uint32_t pauseMenuMusicVolumeText = 0;
    uint32_t pauseMenuMusicPlusButton = 0;
    uint32_t pauseMenuSfxLabel = 0;
    uint32_t pauseMenuSfxMinusButton = 0;
    uint32_t pauseMenuSfxVolumeText = 0;
    uint32_t pauseMenuSfxPlusButton = 0;
    uint32_t pauseMenuControlsText = 0;
    bool paused = false;

    uint32_t player = 0;
    Player* player_script = nullptr;
    std::vector<uint32_t> healthSquares;
    std::vector<uint32_t> healSquares;

    enum TextState { FADE_IN, WAITING, FADE_OUT, DONE };
    
    uint32_t top_of_screen_text = 0;
    uint32_t bottom_left_of_screen_text = 0;
    TextState area_title_state = DONE;
    float area_title_alpha = 0.0f;
    float area_title_wait_timer = 0.0f;
    float area_title_fade_speed = 150.0f;

    uint32_t ability_gained_text = 0;
    uint32_t ability_explained_text = 0;
    TextState ability_title_state = DONE;
    float ability_title_alpha = 0.0f;
    float ability_title_wait_timer = 0.0f;
    float ability_title_fade_speed = 150.0f;

    uint32_t main_menu_title = 0;
    uint32_t main_menu_play_button = 0;
    uint32_t main_menu_credits_button = 0;
    uint32_t main_menu_quit_button = 0;
    uint32_t main_menu_reset_data_button = 0;
    uint32_t main_menu_background = 0;

    uint32_t credits_title = 0;
    uint32_t credits_info_od = 0;
    uint32_t credits_info_ck = 0;
    uint32_t credits_back_button = 0;
    
    int level = 0;
    SquareCore::Vec2 spawn_point = SquareCore::Vec2::zero();
};