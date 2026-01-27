#include "UserInterface.h"
#include "PlayerKeybinds.h"
#include "Properties.h"

void UserInterface::OnStart()
{
    main_menu_background = AddUIRect(0.0f, 0.0f, 1920, 1080,
                             SquareCore::RGBA(70, 70, 70, 255), "", {SquareCore::RGBA(0, 0, 0, 0)});
    SetElementSprite(main_menu_background, "Resources/Sprites/main-menu.png");
    
    main_menu_title = AddUIText(1920.0f / 2.0f, 150.0f, 64, SquareCore::RGBA(255, 255, 255, 255), "Resources/Fonts/Helvetica.ttf", "SQUARE SOULS");
    SquareCore::Vec2 text_size = GetTextSize(main_menu_title);
    SetUIElementPosition(main_menu_title, 1920.0f / 2.0f - text_size.x / 2.0f, 150.0f);
    
    level = player_script->GetPlayerData().level;
    if (level <= 0) level = 1;
    spawn_point = player_script->GetPlayerData().spawn_points[level-1];
    main_menu_play_button = AddUIButton(1920.0f / 2.0f - 150.0f, 1080.0f / 2.0f + 200.0f, 300, 75,
                                        SquareCore::RGBA(50, 50, 50, 255), "PLAY", {SquareCore::RGBA(0, 0, 0, 0)},
                                        [this] { map->LoadMap(level, spawn_point); OnPlay(); }, "Resources/Fonts/Helvetica.ttf", 32,
                                        SquareCore::RGBA(255, 255, 255, 255));
    main_menu_credits_button = AddUIButton(1920.0f / 2.0f - 150.0f, 1080.0f / 2.0f + 300.0f, 300, 75,
                                      SquareCore::RGBA(50, 50, 50, 255), "CREDITS", {SquareCore::RGBA(0, 0, 0, 0)},
                                      [this] { ShowCredits(true); }, "Resources/Fonts/Helvetica.ttf", 32,
                                      SquareCore::RGBA(255, 255, 255, 255));
    main_menu_quit_button = AddUIButton(1920.0f / 2.0f - 150.0f, 1080.0f / 2.0f + 400.0f, 300, 75,
                                      SquareCore::RGBA(50, 50, 50, 255), "QUIT", {SquareCore::RGBA(0, 0, 0, 0)},
                                      [this] { Quit(); }, "Resources/Fonts/Helvetica.ttf", 32,
                                      SquareCore::RGBA(255, 255, 255, 255));
    main_menu_reset_data_button = AddUIButton(1920.0f / 2.0f - 70.0f, 1030.0f, 140, 25,
                                      SquareCore::RGBA(50, 0, 0, 255), "RESET DATA", {SquareCore::RGBA(0, 0, 0, 0)},
                                      [this] { player_script->ResetPlayerData(); }, "Resources/Fonts/Helvetica.ttf", 12,
                                      SquareCore::RGBA(255, 255, 255, 255));

    SetUIElementPersistent(main_menu_title, true);
    SetUIElementPersistent(main_menu_background, true);
    SetUIElementPersistent(main_menu_play_button, true);
    SetUIElementPersistent(main_menu_credits_button, true);
    SetUIElementPersistent(main_menu_quit_button, true);
    
    credits_title = AddUIText(1920.0f / 2.0f, 150.0f, 64, SquareCore::RGBA(255, 255, 255, 255), "Resources/Fonts/Helvetica.ttf", "CREDITS");
    text_size = GetTextSize(credits_title);
    SetUIElementPosition(credits_title, 1920.0f / 2.0f - text_size.x / 2.0f, 150.0f);
    credits_info_od = AddUIText(1920.0f / 2.0f, 300.0f, 24, SquareCore::RGBA(255, 255, 255, 255), "Resources/Fonts/Helvetica.ttf",
    "Ohm Desai - Lead Engine Programmer, Gameplay Programmer, Story, Art, Music, Level Design");
    text_size = GetTextSize(credits_info_od);
    SetUIElementPosition(credits_info_od, 1920.0f / 2.0f - text_size.x / 2.0f, 300.0f);
    credits_info_ck = AddUIText(1920.0f / 2.0f, 300.0f, 24, SquareCore::RGBA(255, 255, 255, 255), "Resources/Fonts/Helvetica.ttf",
    "Caleb Kronstad - Lead Gameplay Programmer, Engine Programmer, Story, Art, Music, Level Design");
    text_size = GetTextSize(credits_info_ck);
    SetUIElementPosition(credits_info_ck, 1920.0f / 2.0f - text_size.x / 2.0f, 350.0f);
    
    credits_back_button = AddUIButton(1920.0f / 2.0f - 150.0f, 900.0f, 300, 75,
                                      SquareCore::RGBA(50, 50, 50, 255), "BACK", {SquareCore::RGBA(0, 0, 0, 0)},
                                      [this] { ShowCredits(false); }, "Resources/Fonts/Helvetica.ttf", 32,
                                      SquareCore::RGBA(255, 255, 255, 255));
    
    SetUIElementPersistent(credits_title, true);
    SetUIElementPersistent(credits_info_od, true);
    SetUIElementPersistent(credits_info_ck, true);
    SetUIElementPersistent(credits_back_button, true);
    ShowCredits(false);


    dialogBox = AddUIRect(50, 800, 1820, 250, SquareCore::RGBA(20, 20, 20, 220), "",
                          {SquareCore::RGBA(255, 255, 255, 255), 2.0f, 5.0f});
    speakerText = AddUIText(100, 850, 32, SquareCore::RGBA(255, 200, 100, 255), "Resources/Fonts/Helvetica.ttf", "");
    dialogText = AddUIText(100, 900, 24, SquareCore::RGBA(255, 255, 255, 255), "Resources/Fonts/Helvetica.ttf", "");

    SetUIElementVisible(dialogBox, false);
    SetUIElementVisible(speakerText, false);
    SetUIElementVisible(dialogText, false);

    SetUIElementPersistent(dialogBox, true);
    SetUIElementPersistent(speakerText, true);
    SetUIElementPersistent(dialogText, true);

    dialogTestTrigger = GetFirstEntityWithTag("DialogTrigger");
    dialogManager.Load("Resources/Data/dialog.json");

    float boxWidth = 500.0f;
    float boxHeight = 800.0f;
    pauseMenuBox = AddUIRect(1920.0f / 2.0f - boxWidth / 2.0f, 1080.0f / 2.0f - boxHeight / 2.0f, boxWidth, boxHeight,
                             SquareCore::RGBA(20, 20, 20, 230), "", {SquareCore::RGBA(0, 0, 0, 0)});
    pauseMenuResumeButton = AddUIButton(1920.0f / 2.0f - 150.0f, 1080.0f / 2.0f - 325.0f, 300, 75,
                                        SquareCore::RGBA(50, 50, 50, 255), "RESUME", {SquareCore::RGBA(0, 0, 0, 0)},
                                        [this] { Pause(); }, "Resources/Fonts/Helvetica.ttf", 32,
                                        SquareCore::RGBA(255, 255, 255, 255));
    pauseMenuQuitButton = AddUIButton(1920.0f / 2.0f - 150.0f, 1080.0f / 2.0f - 175.0f, 300, 75,
                                      SquareCore::RGBA(50, 50, 50, 255), "QUIT", {SquareCore::RGBA(0, 0, 0, 0)},
                                      [this] { Quit(); }, "Resources/Fonts/Helvetica.ttf", 32,
                                      SquareCore::RGBA(255, 255, 255, 255));

    pauseMenuMusicLabel = AddUIText(1920.0f / 2.0f - 200.0f, 1080.0f / 2.0f, 24,
                                     SquareCore::RGBA(255, 255, 255, 255), "Resources/Fonts/Helvetica.ttf", "Music:");
    pauseMenuMusicMinusButton = AddUIButton(1920.0f / 2.0f - 70.0f, 1080.0f / 2.0f - 10.0f, 50, 50,
                                            SquareCore::RGBA(50, 50, 50, 255), "-", {SquareCore::RGBA(0, 0, 0, 0)},
                                            [this] {
                                                float& vol = player_script->GetPlayerData().music_volume;
                                                vol = std::max(0.0f, vol - 0.1f);
                                                map->UpdateMusicVolumes();
                                            }, "Resources/Fonts/Helvetica.ttf", 32,
                                            SquareCore::RGBA(255, 255, 255, 255));
    pauseMenuMusicVolumeText = AddUIText(1920.0f / 2.0f - 10.0f, 1080.0f / 2.0f, 24,
                                         SquareCore::RGBA(255, 255, 255, 255), "Resources/Fonts/Helvetica.ttf", "1.0");
    pauseMenuMusicPlusButton = AddUIButton(1920.0f / 2.0f + 30.0f, 1080.0f / 2.0f - 10.0f, 50, 50,
                                           SquareCore::RGBA(50, 50, 50, 255), "+", {SquareCore::RGBA(0, 0, 0, 0)},
                                           [this] {
                                               float& vol = player_script->GetPlayerData().music_volume;
                                               vol = std::min(1.0f, vol + 0.1f);
                                                map->UpdateMusicVolumes();
                                           }, "Resources/Fonts/Helvetica.ttf", 32,
                                           SquareCore::RGBA(255, 255, 255, 255));

    pauseMenuSfxLabel = AddUIText(1920.0f / 2.0f - 200.0f, 1080.0f / 2.0f + 80.0f, 24,
                                   SquareCore::RGBA(255, 255, 255, 255), "Resources/Fonts/Helvetica.ttf", "SFX:");
    pauseMenuSfxMinusButton = AddUIButton(1920.0f / 2.0f - 70.0f, 1080.0f / 2.0f + 70.0f, 50, 50,
                                          SquareCore::RGBA(50, 50, 50, 255), "-", {SquareCore::RGBA(0, 0, 0, 0)},
                                          [this] {
                                              float& vol = player_script->GetPlayerData().sfx_volume;
                                              vol = std::max(0.0f, vol - 0.1f);
                                              player_script->UpdateAudioVolumes();
                                          }, "Resources/Fonts/Helvetica.ttf", 32,
                                          SquareCore::RGBA(255, 255, 255, 255));
    pauseMenuSfxVolumeText = AddUIText(1920.0f / 2.0f - 10.0f, 1080.0f / 2.0f + 80.0f, 24,
                                       SquareCore::RGBA(255, 255, 255, 255), "Resources/Fonts/Helvetica.ttf", "1.0");
    pauseMenuSfxPlusButton = AddUIButton(1920.0f / 2.0f + 30.0f, 1080.0f / 2.0f + 70.0f, 50, 50,
                                         SquareCore::RGBA(50, 50, 50, 255), "+", {SquareCore::RGBA(0, 0, 0, 0)},
                                         [this] {
                                             float& vol = player_script->GetPlayerData().sfx_volume;
                                             vol = std::min(1.0f, vol + 0.1f);
                                              player_script->UpdateAudioVolumes();
                                         }, "Resources/Fonts/Helvetica.ttf", 32,
                                         SquareCore::RGBA(255, 255, 255, 255));
    pauseMenuControlsText = AddUIText(1920.0f / 2.0f - 200.0f, 1080.0f / 2.0f + 250.0f, 16,
                                       SquareCore::RGBA(255, 255, 255, 255), "Resources/Fonts/Helvetica.ttf", 
                                       "CONTROLS:\nMove/Look - WASD\nJump - SPACE\nDash - SHIFT\nSlash - LMB\nShoot - RMB\nHeal - E"
                                       );

    SetUIElementVisible(pauseMenuControlsText, false);
    SetUIElementVisible(pauseMenuSfxVolumeText, false);
    SetUIElementVisible(pauseMenuBox, false);
    SetUIElementVisible(pauseMenuResumeButton, false);
    SetUIElementVisible(pauseMenuQuitButton, false);
    SetUIElementVisible(pauseMenuMusicLabel, false);
    SetUIElementVisible(pauseMenuMusicMinusButton, false);
    SetUIElementVisible(pauseMenuMusicVolumeText, false);
    SetUIElementVisible(pauseMenuMusicPlusButton, false);
    SetUIElementVisible(pauseMenuSfxLabel, false);
    SetUIElementVisible(pauseMenuSfxMinusButton, false);
    SetUIElementVisible(pauseMenuSfxVolumeText, false);
    SetUIElementVisible(pauseMenuSfxPlusButton, false);

    SetUIElementPersistent(pauseMenuBox, true);
    SetUIElementPersistent(pauseMenuResumeButton, true);
    SetUIElementPersistent(pauseMenuQuitButton, true);
    SetUIElementPersistent(pauseMenuMusicLabel, true);
    SetUIElementPersistent(pauseMenuMusicMinusButton, true);
    SetUIElementPersistent(pauseMenuMusicVolumeText, true);
    SetUIElementPersistent(pauseMenuMusicPlusButton, true);
    SetUIElementPersistent(pauseMenuSfxLabel, true);
    SetUIElementPersistent(pauseMenuSfxMinusButton, true);
    SetUIElementPersistent(pauseMenuSfxVolumeText, true);
    SetUIElementPersistent(pauseMenuSfxPlusButton, true);
    SetUIElementPersistent(pauseMenuControlsText, true);

    SetButtonSprites(pauseMenuResumeButton, "Resources/Sprites/albinauric_fatty.png", "Resources/Sprites/fish.png",
                     "Resources/Sprites/albinauric_fatty.png");
    SetButtonSprites(pauseMenuQuitButton, "Resources/Sprites/fish.png", "Resources/Sprites/albinauric_fatty.png",
                     "Resources/Sprites/albinauric_fatty.png");

    player = GetFirstEntityWithTag("Player");

    for (int i = 0; i < player_script->GetPlayerData().max_health; i++)
    {
        uint32_t healthSquare = AddUIRect(50.0f + i * 40.0f, 50.0f, 30.0f, 30.0f,
                                          SquareCore::RGBA(150, 0, 0, 255), "",
                                          {SquareCore::RGBA(0, 0, 0, 0)});
        SetUIElementPersistent(healthSquare, true);
        SetUIElementVisible(healthSquare, false);
        healthSquares.push_back(healthSquare);
    }
    for (int i = 0; i < player_script->GetPlayerData().max_heals; i++)
    {
        uint32_t healSquare = AddUIRect(50.0f + i * 40.0f, 100.0f, 30.0f, 30.0f,
                                          SquareCore::RGBA(200, 80, 65, 255), "",
                                          {SquareCore::RGBA(0, 0, 0, 0)});
        SetUIElementPersistent(healSquare, true);
        SetUIElementVisible(healSquare, false);
        healSquares.push_back(healSquare);
    }

    UpdateHealthBar();
    UpdateHeals();
}

void UserInterface::ShowCredits(bool show)
{
    SetUIElementVisible(main_menu_background, true);
    if (show)
    {   
        SetUIElementVisible(credits_title, true);
        SetUIElementVisible(credits_info_od, true);
        SetUIElementVisible(credits_info_ck, true);
        SetUIElementVisible(credits_back_button, true);

        SetUIElementVisible(main_menu_title, false);
        SetUIElementVisible(main_menu_play_button, false);
        SetUIElementVisible(main_menu_credits_button, false);
        SetUIElementVisible(main_menu_quit_button, false);
        return;
    }
    SetUIElementVisible(credits_title, false);
    SetUIElementVisible(credits_info_od, false);
    SetUIElementVisible(credits_info_ck, false);
    SetUIElementVisible(credits_back_button, false);
    SetUIElementVisible(main_menu_title, true);
    SetUIElementVisible(main_menu_play_button, true);
    SetUIElementVisible(main_menu_credits_button, true);
    SetUIElementVisible(main_menu_quit_button, true);
}

void UserInterface::AreaTitle(std::string title, std::string info)
{
    if (top_of_screen_text != 0)
        RemoveUIElement(top_of_screen_text);
    if (bottom_left_of_screen_text != 0)
        RemoveUIElement(bottom_left_of_screen_text);
    
    top_of_screen_text = AddUIText(1920.0f / 2.0f, 150.0f, 64, SquareCore::RGBA(255, 255, 255, 0), "Resources/Fonts/Helvetica.ttf", title);
    SquareCore::Vec2 text_size = GetTextSize(top_of_screen_text);
    SetUIElementPosition(top_of_screen_text, 1920.0f / 2.0f - text_size.x / 2.0f, 150.0f);
    SetUIElementVisible(top_of_screen_text, true);
    SetUIElementPersistent(top_of_screen_text, true);
    
    bottom_left_of_screen_text = AddUIText(0.0f, 150.0f, 24, SquareCore::RGBA(255, 255, 255, 0), "Resources/Fonts/Helvetica.ttf", info);
    text_size = GetTextSize(bottom_left_of_screen_text);
    SetUIElementPosition(bottom_left_of_screen_text, text_size.x / 2.0f, 1080.0f - 150.0f);
    SetUIElementVisible(bottom_left_of_screen_text, true);
    SetUIElementPersistent(bottom_left_of_screen_text, true);
    
    area_title_alpha = 0.0f;
    area_title_state = FADE_IN;
    area_title_wait_timer = 0.0f;
}

void UserInterface::AbilityGained(std::string title, std::string info)
{
    if (ability_gained_text)
        RemoveUIElement(ability_gained_text);
    if (ability_explained_text)
        RemoveUIElement(ability_explained_text);
    
    ability_gained_text = AddUIText(1920.0f / 2.0f, 150.0f, 64, SquareCore::RGBA(255, 255, 255, 0), "Resources/Fonts/Helvetica.ttf", title);
    SquareCore::Vec2 text_size = GetTextSize(ability_gained_text);
    SetUIElementPosition(ability_gained_text, 1920.0f / 2.0f - text_size.x / 2.0f, 150.0f);
    SetUIElementVisible(ability_gained_text, true);
    SetUIElementPersistent(ability_gained_text, true);

    ability_explained_text = AddUIText(1920.0f / 2.0f, 300.0f, 24, SquareCore::RGBA(255, 255, 255, 0), "Resources/Fonts/Helvetica.ttf", info);
    text_size = GetTextSize(ability_explained_text);
    SetUIElementPosition(ability_explained_text, 1920.0f / 2.0f - text_size.x / 2.0f, 300.0f);
    SetUIElementVisible(ability_explained_text, true);
    SetUIElementPersistent(ability_explained_text, true);

    ability_title_alpha = 0.0f;
    ability_title_state = FADE_IN;
    ability_title_wait_timer = 0.0f;
}

void UserInterface::UpdateAreaTitle(float deltaTime)
{
    if (area_title_state == DONE) return;
    
    if (area_title_state == FADE_IN)
    {
        area_title_alpha += area_title_fade_speed * deltaTime;
        if (area_title_alpha >= 255.0f)
        {
            area_title_alpha = 255.0f;
            area_title_state = WAITING;
        }
    }
    else if (area_title_state == WAITING)
    {
        area_title_wait_timer += deltaTime;
        if (area_title_wait_timer >= 3.0f)
        {
            area_title_state = FADE_OUT;
        }
    }
    else if (area_title_state == FADE_OUT)
    {
        area_title_alpha -= area_title_fade_speed * deltaTime;
        if (area_title_alpha <= 0.0f)
        {
            area_title_alpha = 0.0f;
            area_title_state = DONE;
            SetUIElementVisible(top_of_screen_text, false);
            SetUIElementVisible(bottom_left_of_screen_text, false);
        }
    }
    
    SetUITextColor(top_of_screen_text, SquareCore::RGBA(255, 255, 255, (int)area_title_alpha));
    SetUITextColor(bottom_left_of_screen_text, SquareCore::RGBA(255, 255, 255, (int)area_title_alpha));
}

void UserInterface::UpdateAbilityGained(float deltaTime)
{
    if (ability_title_state == DONE) return;
    
    if (ability_title_state == FADE_IN)
    {
        ability_title_alpha += ability_title_fade_speed * deltaTime;
        if (ability_title_alpha >= 255.0f)
        {
            ability_title_alpha = 255.0f;
            ability_title_state = WAITING;
        }
    }
    else if (ability_title_state == WAITING)
    {
        ability_title_wait_timer += deltaTime;
        if (ability_title_wait_timer >= 3.0f)
        {
            ability_title_state = FADE_OUT;
        }
    }
    else if (ability_title_state == FADE_OUT)
    {
        ability_title_alpha -= ability_title_fade_speed * deltaTime;
        if (ability_title_alpha <= 0.0f)
        {
            ability_title_alpha = 0.0f;
            ability_title_state = DONE;
            SetUIElementVisible(ability_gained_text, false);
            SetUIElementVisible(ability_explained_text, false);
        }
    }
    
    SetUITextColor(ability_gained_text, SquareCore::RGBA(255, 255, 255, (int)ability_title_alpha));
    SetUITextColor(ability_explained_text, SquareCore::RGBA(255, 255, 255, (int)ability_title_alpha));
}

void UserInterface::UpdateHeals()
{
    int current_heals = player_script->GetPlayerData().heals;
    for (int i = 0; i < healSquares.size(); i++)
    {
        if (i < current_heals)
            SetUIElementColor(healSquares[i], SquareCore::RGBA(200, 80, 65, 255));
        else
            SetUIElementColor(healSquares[i], SquareCore::RGBA(100, 100, 100, 255));
    }
}

void UserInterface::UpdateHealthBar()
{
    int currentHealth = 0;
    for (auto& property : GetAllEntityProperties(player))
    {
        if (Character* character = dynamic_cast<Character*>(property))
        {
            currentHealth = character->health;
            break;
        }
    }

    for (int i = 0; i < healthSquares.size(); i++)
    {
        if (i < currentHealth)
            SetUIElementColor(healthSquares[i], SquareCore::RGBA(150, 0, 0, 255));
        else
            SetUIElementColor(healthSquares[i], SquareCore::RGBA(100, 100, 100, 255));
    }
}

void UserInterface::OnUpdate(float deltaTime)
{
    UpdateHealthBar();
    UpdateHeals();
    UpdateAreaTitle(deltaTime);
    UpdateAbilityGained(deltaTime);

    if (!dialogManager.IsActive() && !dialogManager.HasBeenSeen(0))
    {
        auto collisions = GetEntityCollisions(dialogTestTrigger);
        for (const auto& collision : collisions)
        {
            if (EntityHasTag(collision.first, "Player"))
            {
                dialogManager.Start(0);
                break;
            }
        }
    }
    
    if (!dialogManager.IsActive() && !dialogManager.HasBeenSeen(1))
    {
        if (uint32_t dialog_trigger = GetFirstEntityWithTag("DialogTriggerElvis"))
        {
            auto collisions = GetEntityCollisions(dialog_trigger);
            for (const auto& collision : collisions)
            {
                if (EntityHasTag(collision.first, "Player"))
                {
                    dialogManager.Start(1);
                    break;
                }
            }
        }
    }
    
    if (!dialogManager.IsActive() && !dialogManager.HasBeenSeen(2))
    {
        if (uint32_t dialog_trigger = GetFirstEntityWithTag("DialogTriggerBigTriangle"))
        {
            auto collisions = GetEntityCollisions(dialog_trigger);
            for (const auto& collision : collisions)
            {
                if (EntityHasTag(collision.first, "Player"))
                {
                    dialogManager.Start(2);
                    break;
                }
            }
        }
    }

    if (!dialogManager.IsActive() && !dialogManager.HasBeenSeen(3))
    {
        if (uint32_t dialog_trigger = GetFirstEntityWithTag("DialogTriggerPapagon"))
        {
            auto collisions = GetEntityCollisions(dialog_trigger);
            for (const auto& collision : collisions)
            {
                if (EntityHasTag(collision.first, "Player"))
                {
                    dialogManager.Start(3);
                    break;
                }
            }
        }
    }
    
    if (!dialogManager.IsActive() && !dialogManager.HasBeenSeen(4))
    {
        if (uint32_t dialog_trigger = GetFirstEntityWithTag("DialogTriggerHexHepta"))
        {
            auto collisions = GetEntityCollisions(dialog_trigger);
            for (const auto& collision : collisions)
            {
                if (EntityHasTag(collision.first, "Player"))
                {
                    dialogManager.Start(4);
                    break;
                }
            }
        }
    }
    
    if (!dialogManager.IsActive() && !dialogManager.HasBeenSeen(5))
    {
        if (uint32_t dialog_trigger = GetFirstEntityWithTag("DialogTriggerLastOcta"))
        {
            auto collisions = GetEntityCollisions(dialog_trigger);
            for (const auto& collision : collisions)
            {
                if (EntityHasTag(collision.first, "Player"))
                {
                    dialogManager.Start(5);
                    break;
                }
            }
        }
    }
    
    if (!dialogManager.IsActive() && !dialogManager.HasBeenSeen(6))
    {
        if (uint32_t dialog_trigger = GetFirstEntityWithTag("DialogTriggerReginald"))
        {
            auto collisions = GetEntityCollisions(dialog_trigger);
            for (const auto& collision : collisions)
            {
                if (EntityHasTag(collision.first, "Player"))
                {
                    dialogManager.Start(6);
                    break;
                }
            }
        }
    }

    if (dialogManager.IsActive())
    {
        DialogUpdate();
    }
    else
    {
        SetUIElementVisible(dialogBox, false);
        SetUIElementVisible(speakerText, false);
        SetUIElementVisible(dialogText, false);
    }

    if (GetKeyPressed(pause_bind))
        Pause();
    
    if (paused)
        UpdateVolumeDisplays();
}

void UserInterface::OnPlay()
{
    for (uint32_t health_square : healthSquares)
    {
        SetUIElementVisible(health_square, true);
    }
    for (uint32_t heal_square : healSquares)
    {
        SetUIElementVisible(heal_square, true);
    }

    SetMouseVisible(false);
    SetUIElementVisible(main_menu_title, false);
    SetUIElementVisible(main_menu_background, false);
    SetUIElementVisible(main_menu_play_button, false);
    SetUIElementVisible(main_menu_credits_button, false);
    SetUIElementVisible(main_menu_quit_button, false);
    SetUIElementVisible(credits_title, false);
    SetUIElementVisible(credits_info_od, false);
    SetUIElementVisible(credits_info_ck, false);
    SetUIElementVisible(credits_back_button, false);
}

void UserInterface::Pause()
{
    if (map->current_map == 0) return;
    
    if (paused)
    {
        SetMouseVisible(false);
        paused = false;
        SetUIElementVisible(pauseMenuBox, false);
        SetUIElementVisible(pauseMenuResumeButton, false);
        SetUIElementVisible(pauseMenuQuitButton, false);
        SetUIElementVisible(pauseMenuMusicLabel, false);
        SetUIElementVisible(pauseMenuMusicMinusButton, false);
        SetUIElementVisible(pauseMenuMusicVolumeText, false);
        SetUIElementVisible(pauseMenuMusicPlusButton, false);
        SetUIElementVisible(pauseMenuSfxLabel, false);
        SetUIElementVisible(pauseMenuSfxMinusButton, false);
        SetUIElementVisible(pauseMenuSfxVolumeText, false);
        SetUIElementVisible(pauseMenuSfxPlusButton, false);
        SetUIElementVisible(pauseMenuControlsText, false);
        
        SetTimeScale(1.0f);
    }
    else if (!paused)
    {
        SetMouseVisible(true);
        paused = true;
        SetUIElementVisible(pauseMenuBox, true);
        SetUIElementVisible(pauseMenuResumeButton, true);
        SetUIElementVisible(pauseMenuQuitButton, true);
        SetUIElementVisible(pauseMenuMusicLabel, true);
        SetUIElementVisible(pauseMenuMusicMinusButton, true);
        SetUIElementVisible(pauseMenuMusicVolumeText, true);
        SetUIElementVisible(pauseMenuMusicPlusButton, true);
        SetUIElementVisible(pauseMenuSfxLabel, true);
        SetUIElementVisible(pauseMenuSfxMinusButton, true);
        SetUIElementVisible(pauseMenuSfxVolumeText, true);
        SetUIElementVisible(pauseMenuSfxPlusButton, true);
        SetUIElementVisible(pauseMenuControlsText, true);
        
        float music_vol = player_script->GetPlayerData().music_volume;
        std::string music_vol_str = std::to_string(music_vol).substr(0, 3);
        SetUIText(pauseMenuMusicVolumeText, music_vol_str.c_str());
        
        float sfx_vol = player_script->GetPlayerData().sfx_volume;
        std::string sfx_vol_str = std::to_string(sfx_vol).substr(0, 3);
        SetUIText(pauseMenuSfxVolumeText, sfx_vol_str.c_str());
        
        SetTimeScale(0.0f);
    }
}

void UserInterface::DialogUpdate()
{
    DialogLine* line = dialogManager.GetCurrentLine();
    if (!line) return;

    SetUIElementVisible(dialogBox, true);
    SetUIElementVisible(speakerText, true);
    SetUIElementVisible(dialogText, true);

    SetUIText(speakerText, line->speaker);
    SetUIText(dialogText, line->text);

    if (GetMouseButtonPressed(0))
    {
        dialogManager.Advance();
    }
}

void UserInterface::UpdateVolumeDisplays()
{
    float music_vol = player_script->GetPlayerData().music_volume;
    std::string music_vol_str = std::to_string(music_vol).substr(0, 3);
    SetUIText(pauseMenuMusicVolumeText, music_vol_str.c_str());
    
    float sfx_vol = player_script->GetPlayerData().sfx_volume;
    std::string sfx_vol_str = std::to_string(sfx_vol).substr(0, 3);
    SetUIText(pauseMenuSfxVolumeText, sfx_vol_str.c_str());
}