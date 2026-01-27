#include "Map.h"

#include "PlayerKeybinds.h"
#include "UserInterface.h"

void Map::OnStart()
{
    main_menu_music = AddAudioClip("Resources/Audio/29th.wav");
    level_1_music = AddAudioClip("Resources/Audio/12.mp3");
    level_2_music = AddAudioClip("Resources/Audio/03.mp3");
    level_3_music = AddAudioClip("Resources/Audio/11.mp3");
    final_boss_music = AddAudioClip("Resources/Audio/14.wav");
    SetAudioVolume(main_menu_music, 0.0f);
    SetAudioVolume(level_1_music, 0.1f);
    SetAudioVolume(level_2_music, 0.1f);
    SetAudioVolume(level_3_music, 0.1f);
    SetAudioLooping(main_menu_music, true);
    SetAudioLooping(level_1_music, true);
    SetAudioLooping(level_2_music, true);
    SetAudioLooping(level_3_music, true);
    SetAudioLooping(final_boss_music, true);
    SetAudioMasterVolume(1.0f);
    
    SetCameraZoom(0.85f);
    //SetCameraZoom(0.3f);
    EnableCameraBounds(true);
    SetGravity(-1500.0f);
    
    LoadMap(0, SquareCore::Vec2(0.0f, 0.0f));
}


void Map::OnUpdate(float deltaTime)
{
    wormhole_rotation += deltaTime * wormhole_speed;
    for (int i = 0 ; i < wormholes.size(); i++)
    {
        SetRotation(wormholes[i], wormhole_rotation * (1.0f + (i*0.4f)));
    }
    for (int i = 0 ; i < wormholes_negative.size(); i++)
    {
        SetRotation(wormholes_negative[i], -wormhole_rotation * (1.0f + (i*0.1f)));
    }

    ability_icon_rotation += deltaTime * ability_icon_speed;
    for (uint32_t icon : ability_icons)
    {
        if (!EntityExists(icon)) continue;
        SetRotation(icon, ability_icon_rotation);
    }

    /*if (GetKeyPressed(debug_hot_reload))
    {
        uint32_t player_id = GetFirstEntityWithTag("Player");
        SquareCore::Vec2 player_pos = GetPosition(player_id);
        LoadMap(current_map, {player_pos.x, player_pos.y});
    }*/
    
    if (EntityExists(ball_entity))
    {
        auto collisions = GetEntityCollisions(ball_entity);
        for (auto collision : collisions)
        {
            if (!EntityExists(collision.first)) continue;
            
            if (EntityHasTag(collision.first, "CrushedEnemyShield"))
            {
                RemoveEntity(crushed_enemy);
                RemoveEntity(collision.first);
                player_script->GetPlayerData().the_wall_dead = true;
                continue;
            }
            
            if (EntityHasTag(collision.first, "Ground"))
            {
                uint32_t ball_rubble = GetFirstEntityWithTag("BallRubble");
                
                SetEntityColor(ball_rubble, SquareCore::RGBA(255, 255, 255, 255));
                
                SetRotation(level_1_boss_door, 70.0f);
                SetPosition(level_1_boss_door, -7900.0f, -100.0f);
                
                RemoveEntity(ball_entity);
                
                enemy_manager->SpawnJumpBoss({-11000.0f, 500.0f});
                enemy_manager->boss_1_active = true;
            }
        }
    }
}

void Map::LoadMap(int level, SquareCore::Vec2 player_position)
{
    current_map = level;
    std::string scene_path;
    switch (level)
    {
    case 0:
        scene_path = "Resources/Scenes/test.square";
        break;
    case 1:
        scene_path = "Resources/Scenes/level1.square";
        break;
    case 2:
        scene_path = "Resources/Scenes/level2.square";
        break;
    case 3:
        scene_path = "Resources/Scenes/level3.square";
        break;
    default:
        scene_path = "Resources/Scenes/test.square";
        break;
    }
    
    LoadScene(scene_path);
    if (enemy_manager) enemy_manager->LoadEnemies();
    if (player_script) player_script->TeleportPlayer({player_position.x, player_position.y});
    player_script->UpdateAudioVolumes();
    UpdateMusicVolumes();
    
    ability_icons = GetAllEntitiesWithTag("Ability");
    wormholes = GetAllEntitiesWithTag("Wormhole");
    wormholes_negative = GetAllEntitiesWithTag("WormholeNegative");
    for (uint32_t wormhole : wormholes)
    {
        SetColliderType(wormhole, SquareCore::ColliderType::NONE);
    }
    for (uint32_t wormhole : wormholes_negative)
    {
        SetColliderType(wormhole, SquareCore::ColliderType::NONE);
    }
    
    if (uint32_t level_1_wormhole_shield = GetFirstEntityWithTag("Level1WormholeShield"))
    {
        if (player_script->GetPlayerData().first_boss_dead)
        {
            RemoveEntity(level_1_wormhole_shield);
        }
    }

    if (uint32_t boss_2_exit = GetFirstEntityWithTag("Boss2Exit"))
    {
        if (player_script->GetPlayerData().second_boss_dead)
        {
            RemoveEntity(boss_2_exit);
        }
    }

    player_script->HealMaxHealth();
    
    if (player_script->GetPlayerData().the_wall_dead)
    {
        if (level == 1)
        {
            uint32_t rope = GetFirstEntityWithTag("Breakable");
            RemoveEntity(rope);
        
            uint32_t ball_rubble = GetFirstEntityWithTag("BallRubble");
            SetEntityColor(ball_rubble, SquareCore::RGBA(255, 255, 255, 255));
            
            uint32_t crushed_enemy_shield = GetFirstEntityWithTag("CrushedEnemyShield");
            RemoveEntity(crushed_enemy_shield);
            
            ball_entity = GetFirstEntityWithTag("Ball");
            RemoveEntity(ball_entity);
            
            crushed_enemy = GetFirstEntityWithTag("CrushedEnemy");
            RemoveEntity(crushed_enemy);
            
            level_1_boss_door = GetFirstEntityWithTag("Level1BossDoor");
            SetRotation(level_1_boss_door, 70.0f);
            SetPosition(level_1_boss_door, -7900.0f, -100.0f);
        }
    }
    else
    {
        ball_entity = GetFirstEntityWithTag("Ball");
        crushed_enemy = GetFirstEntityWithTag("CrushedEnemy");
        level_1_boss_door = GetFirstEntityWithTag("Level1BossDoor");
    }
    
    uint32_t ability = GetFirstEntityWithTag("GainDoubleDash");
    if (ability && player_script->GetPlayerData().has_double_dash)
        RemoveEntity(ability);
    ability = GetFirstEntityWithTag("GainDoubleJump") ;
    if (ability && player_script->GetPlayerData().has_double_jump)
        RemoveEntity(ability);
    ability = GetFirstEntityWithTag("GainProjectile");
    if (ability  && player_script->GetPlayerData().has_projectile)
        RemoveEntity(ability);
    ability = GetFirstEntityWithTag("GainFasterSlash");
    if (ability  && player_script->GetPlayerData().has_faster_slash)
        RemoveEntity(ability);
    
    switch (level)
    {
    case 0:
        {
            StopAudioClip(final_boss_music);
            StopAudioClip(level_1_music);
            StopAudioClip(level_2_music);
            StopAudioClip(level_3_music);
            //PlayAudioClip(main_menu_music);
            player_script->UpdateCameraBounds(-10000.0f, -400.0f, 9500.0f, 10000.0f);
            SetCameraPosition(player_script->GetPlayerData().spawn_points[0]);
            break;
        }
    case 1:
        {
            StopAudioClip(final_boss_music);
            StopAudioClip(main_menu_music);
            StopAudioClip(level_2_music);
            StopAudioClip(level_3_music);
            PlayAudioClip(level_1_music);
            SetCameraBounds(-14000.0f, -400.0f, 6000.0f, 10000.0f);
            player_script->UpdateCameraBounds(-14000.0f, -400.0f, 6000.0f, 10000.0f);
            SetCameraPosition(player_script->GetPlayerData().spawn_points[0]);
            if (ui) ui->AreaTitle("The Cage", "Now Playing:\nThe Cage\nCaleb Kronstad and Ohm Desai");
            break;
        }
    case 2:
        {
            StopAudioClip(final_boss_music);
            StopAudioClip(level_1_music);
            StopAudioClip(main_menu_music);
            StopAudioClip(level_3_music);
            PlayAudioClip(level_2_music);
            SetCameraBounds(-10000.0f, -400.0f, 6000.0f, 10000.0f);
            player_script->UpdateCameraBounds(-10000.0f, -400.0f, 6000.0f, 10000.0f);
            SetCameraPosition(player_script->GetPlayerData().spawn_points[1]);
            if (ui) ui->AreaTitle("The Slums", "Now Playing:\nThe Slums\nCaleb Kronstad");
            //player_script->TeleportPlayer(SquareCore::Vec2(-5500.0, -4550.0f));
            break;
        }
    case 3:
        {
            StopAudioClip(final_boss_music);
            StopAudioClip(level_1_music);
            StopAudioClip(level_2_music);
            StopAudioClip(main_menu_music);
            PlayAudioClip(level_3_music);
            SetCameraBounds(-14000.0f, -400.0f, 9500.0f, 14000.0f);
            player_script->UpdateCameraBounds(-14000.0f, -400.0f, 9500.0f, 14000.0f);
            SetCameraPosition(player_script->GetPlayerData().spawn_points[2]);
            if (ui) ui->AreaTitle("The Palace", "Now Playing:\nThe Palace\nCaleb Kronstad");
            if (!player_script->GetPlayerData().third_boss_dead)
            {
                enemy_manager->SpawnFinalBoss({-9200.0f, 5200.0f});
                enemy_manager->boss_3_active = false;
            }
            break;
        }
    default:
        {
            StopAudioClip(final_boss_music);
            StopAudioClip(level_1_music);
            StopAudioClip(level_2_music);
            StopAudioClip(level_3_music);
            StopAudioClip(main_menu_music);
            break;
        }
    }
    
}

void Map::UpdateMusicVolumes()
{
    SetAudioVolume(main_menu_music, player_script->GetPlayerData().music_volume);
    SetAudioVolume(level_1_music, player_script->GetPlayerData().music_volume);
    SetAudioVolume(level_2_music, player_script->GetPlayerData().music_volume);
    SetAudioVolume(level_3_music, player_script->GetPlayerData().music_volume);
    SetAudioVolume(final_boss_music, player_script->GetPlayerData().music_volume);
}

void Map::MuteAllMusic()
{
    StopAudioClip(final_boss_music);
    StopAudioClip(level_1_music);
    StopAudioClip(level_2_music);
    StopAudioClip(level_3_music);
    StopAudioClip(main_menu_music);
}

void Map::PlayBossMusic()
{
    StopAudioClip(level_1_music);
    StopAudioClip(level_2_music);
    StopAudioClip(level_3_music);
    StopAudioClip(main_menu_music);
    PlayAudioClip(final_boss_music);
}
