#include "Player.h"
#include "GameStateManager.h"
#include "PlayerKeybinds.h"
#include "UserInterface.h"
#include "Map.h"

void Player::OnStart()
{
    player_data = GameStateManager::LoadPlayerData("Saves/S_001.square");
    player = AddEntity("Resources/Sprites/player.png", player_data.x_pos, player_data.y_pos, 0.0f, 0.05f, 0.05f, true);
    AddTagToEntity(player, "Player");
    FlipSprite(player, true, false);
    SetEntityPersistent(player, true);
    AddPropertyToEntity(player, new Character(player_data.max_health, player_data.health, player_data.damage));
    SetZIndex(player, 1000);
    
    slash_fps = 7.0f / slash_length;
    slash = AddAnimatedEntity("Resources/Sprites/slash-sheet-2.png", 7, slash_fps, player_data.x_pos, player_data.y_pos, 0.0f, 0.25f, 0.1f, false);
    SetColliderType(slash, SquareCore::ColliderType::TRIGGER);
    AddTagToEntity(slash, "PlayerSlash");
    SetEntityVisible(slash, false);
    SetEntityPersistent(slash, true);
    SetZIndex(slash, 1000);

    dash_fps = 6.0f / dash_length;
    dash = AddAnimatedEntity("Resources/Sprites/dash-sheet.png", 6, dash_fps, player_data.x_pos, player_data.y_pos, 0.0f, 1.0f, 0.5f, false);
    SetColliderType(dash, SquareCore::ColliderType::NONE);
    AddTagToEntity(dash, "PlayerDash");
    SetEntityVisible(dash, false);
    SetEntityPersistent(dash, true);
    SetEntityColor(dash, SquareCore::RGBA(200, 200, 200, 255));
    SetZIndex(dash, 1000);
    
    projectile_fps = 100.0f;
    for (int i = 0; i < 5; i++)
    {
        int slot = projectile_pool.Alloc();
        ProjectileEntity* projectile = static_cast<ProjectileEntity*>(projectile_pool.GetPointer(slot));
        projectile->id = AddAnimatedEntity("Resources/Sprites/projectile-sheet.png", 4, projectile_fps, player_data.x_pos, player_data.y_pos, 0.0f, 0.1f, 0.1f, false);
        projectile->active = false;
        projectile->timer = 0.0f;
        projectile->direction = Direction::LEFT;

        SetColliderType(projectile->id, SquareCore::ColliderType::TRIGGER);
        AddTagToEntity(projectile->id, "PlayerProjectile");
        SetEntityVisible(projectile->id, false);
        SetEntityPersistent(projectile->id, true);
        SetColliderBox(projectile->id, 50.0f, 25.0f);
        SetZIndex(projectile->id, -1);
    }

    for (int i = 0; i < slash_audio.size(); i++)
    {
        slash_audio[i] = AddAudioClip("Resources/Audio/SFX/swoosh.mp3");
        SetAudioLooping(slash_audio[i], false);
    }
    for (int i = 0; i < jump_audio.size(); i++)
    {
        jump_audio[i] = AddAudioClip("Resources/Audio/SFX/jump.wav");
        SetAudioLooping(jump_audio[i], false);
    }
    for (int i = 0; i < heal_audio.size(); i++)
    {
        heal_audio[i] = AddAudioClip("Resources/Audio/SFX/heal.wav");
        SetAudioLooping(heal_audio[i], false);
    }
    for (int i = 0; i < projectile_audio.size(); i++)
    {
        projectile_audio[i] = AddAudioClip("Resources/Audio/SFX/projectile.wav");
        SetAudioLooping(projectile_audio[i], false);
    }
    for (int i = 0; i < dash_audio.size(); i++)
    {
        dash_audio[i] = AddAudioClip("Resources/Audio/SFX/enemy_dash.wav");
        SetAudioLooping(dash_audio[i], false);
    }
    for (int i = 0; i < take_damage_audio.size(); i++)
    {
        take_damage_audio[i] = AddAudioClip("Resources/Audio/SFX/damage.mp3");
        SetAudioLooping(take_damage_audio[i], false);
    }
    for (int i = 0; i < bounce_audio.size(); i++)
    {
        bounce_audio[i] = AddAudioClip("Resources/Audio/SFX/bounce.wav");
        SetAudioLooping(bounce_audio[i], false);
    }
    for (int i = 0; i < final_boss_laser_audio.size(); i++)
    {
        final_boss_laser_audio[i] = AddAudioClip("Resources/Audio/SFX/boss_lazar.wav");
        SetAudioLooping(final_boss_laser_audio[i], false);
    }
    for (int i = 0; i < final_boss_sword_audio.size(); i++)
    {
        final_boss_sword_audio[i] = AddAudioClip("Resources/Audio/SFX/sword_slash.wav");
        SetAudioLooping(final_boss_sword_audio[i], false);
    }
    for (int i = 0; i < ability_pickup_audio.size(); i++)
    {
        ability_pickup_audio[i] = AddAudioClip("Resources/Audio/SFX/ability.wav");
        SetAudioLooping(ability_pickup_audio[i], false);
    }
    for (int i = 0; i < door_slam_audio.size(); i++)
    {
        door_slam_audio[i] = AddAudioClip("Resources/Audio/SFX/metal_slam.wav");
        SetAudioLooping(door_slam_audio[i], false);
    }
    for (int i = 0; i < enemy_dash_audio.size(); i++)
    {
        enemy_dash_audio[i] = AddAudioClip("Resources/Audio/SFX/enemy_dash.wav");
        SetAudioLooping(enemy_dash_audio[i], false);
    }
    UpdateAudioVolumes();

    player_data.heals = player_data.max_heals;
    
    if (uint32_t key = GetFirstEntityWithTag("GainKey") && has_key)
        SetEntityVisible(key, false);
}

void Player::TeleportPlayer(const SquareCore::Vec2& position)
{
    SetPosition(player, position.x, position.y);
}

void Player::OnExit()
{
    for (auto property : GetAllEntityProperties(player))
    {
        if (Character* cprop = dynamic_cast<Character*>(property))
        {
            player_data.health = cprop->health;
        }
    }
    GameStateManager::SavePlayerData("Saves/S_001.square", player_data);
}

void Player::OnUpdate(float delta_time)
{
    Move(delta_time);
    Jump(delta_time);
    Dash(delta_time);
    Heal(delta_time);
    Slash(delta_time);
    Projectile(delta_time);
    OnCollision(delta_time);
    UpdateBounceEntities(delta_time);
    
    if (in_octagon_dialogue)
    {
        if (dialog_manager->HasBeenSeen(5) && !has_ability)
        {
            has_ability = true;
            PlayPickupSound();
            player_data.has_faster_slash = true;
            user_interface->AbilityGained("Faster Slash", "idk maybe slash faster or something you can figure it out");
        }
    }
    
    if (in_final_boss_dialogue)
    {
        if (dialog_manager->HasBeenSeen(6))
        {
            in_final_boss_dialogue = false;
            map->PlayBossMusic();
            enemy_manager->boss_3_active = true;
            user_interface->AreaTitle("Reginald Bartholomew Pemberton III", "Now Playing:\nRBP3\nCaleb Kronstad");
            
            uint32_t final_boss = GetFirstEntityWithTag("FinalBoss");
            for (auto& prop : GetAllEntityProperties(final_boss))
            {
                if (FinalBoss* fb = dynamic_cast<FinalBoss*>(prop))
                {
                    fb->state = FinalBossState::IDLE;
                }
            }
        }
    }

    if (!can_take_damage)
    {
        can_take_damage_timer += delta_time;
        if (can_take_damage_timer >= can_take_damage_cooldown)
        {
            can_take_damage = true;
            can_take_damage_timer = 0.0f;
        }
    }

    // DEBUG KEYS
    /*if (GetKeyPressed(debug_collision))
        ToggleDebugCollisions();*/
    //

    if (!enemies_to_remove.empty())
    {
        for (uint32_t enemy : enemies_to_remove)
        {
            RemoveEntity(enemy);
        }
        enemies_to_remove.clear();
    }

    if (!SquareCore::CompareFloats(bounds[1], target_bounds_y_min))
    {
        float new_y_min = SquareCore::Lerp(bounds[1], target_bounds_y_min, bounds_lerp_speed * delta_time);
        SetCameraBounds(bounds[0], new_y_min, bounds[2], bounds[3]);
        bounds[1] = new_y_min;
    }
    
    FollowCameraTarget(GetPosition(player), 10.0f, delta_time);
}

void Player::HealMaxHealth()
{
    for (auto& player_property : GetAllEntityProperties(player))
    {
        if (Character* player_character = dynamic_cast<Character*>(player_property))
        {
            player_character->health = player_data.max_health;
        }
    }
    player_data.heals = player_data.max_heals;
}

void Player::CancelVelocity()
{
    SetVelocity(player, 0.0f, 0.0f);
}

void Player::UpdateCameraBounds(float min_x, float min_y, float max_x, float max_y)
{
    bounds[0] = min_x;
    bounds[1] = min_y;
    bounds[2] = max_x;
    bounds[3] = max_y;
    target_bounds_y_min = min_y;
}

void Player::Move(float delta_time)
{
    SquareCore::Vec2 player_velocity = GetVelocity(player);
    float target_x = 0.0f;

    if (GetKeyHeld(move_left_bind))
    {
        player_direction = Direction::LEFT;
        FlipSprite(player, true, false);
        target_x = SquareCore::Lerp(player_velocity.x, -move_speed, acceleration * delta_time);
    }
    else if (GetKeyHeld(move_right_bind))
    {
        player_direction = Direction::RIGHT;
        FlipSprite(player, false, false);
        target_x = SquareCore::Lerp(player_velocity.x, move_speed, acceleration * delta_time);
    }

    bool up_held = GetKeyHeld(look_up_bind);
    bool down_held = GetKeyHeld(look_down_bind);
    
    if (up_held && down_held)
    {
        is_looking_up = last_vertical_look_was_up;
        is_looking_down = !last_vertical_look_was_up;
    }
    else if (up_held)
    {
        is_looking_up = true;
        is_looking_down = false;
        last_vertical_look_was_up = true;
    }
    else if (down_held)
    {
        is_looking_up = false;
        is_looking_down = true;
        last_vertical_look_was_up = false;
    }
    else
    {
        is_looking_up = false;
        is_looking_down = false;
    }

    if (!dialog_manager->IsActive())
        SetVelocity(player, target_x, player_velocity.y);
}

void Player::Jump(float delta_time)
{
    SquareCore::Vec2 player_velocity = GetVelocity(player);

    bool grounded = IsGrounded(player);

    if (grounded)
        can_double_jump = true;
    
    if (GetKeyPressed(jump_bind) && (grounded || (player_data.has_double_jump && can_double_jump)) && !dialog_manager->IsActive())
    {
        PlayJumpSound();
        
        if (!grounded)
        {
            can_double_jump = false;
            SetVelocity(player, player_velocity.x, jump_velocity / 1.2f);
        }
        else
        {
            SetVelocity(player, player_velocity.x, jump_velocity);
        }
    }
    
    if (!grounded && player_velocity.y < 0.0f)
    {
        player_velocity = GetVelocity(player);
        SetVelocity(player, player_velocity.x, player_velocity.y - (1500.0f * delta_time));
    }
}

void Player::Dash(float delta_time)
{
    SquareCore::Vec2 player_velocity = GetVelocity(player);
    SquareCore::Vec2 player_position = GetPosition(player);

    if (GetKeyPressed(dash_bind) && !is_dashing && player_velocity.x != 0.0f && !in_cooldown && !dialog_manager->IsActive())
    {
        PlayAudioClip(dash_audio[dash_audio_index]);
        dash_audio_index = (dash_audio_index + 1) % 5;
        
        is_dashing = true;
        dashes_used++;
        time_since_last_dash = 0.0f;

        int max_dashes = player_data.has_double_dash ? 2 : 1;
        if (dashes_used >= max_dashes)
        {
            in_cooldown = true;
            dash_cooldown_elapsed = 0.0f;
        }
        
        player_velocity = GetVelocity(player);
        SetVelocity(player, (player_direction == Direction::LEFT ? -dash_velocity : dash_velocity) + player_velocity.x, 0.0f);

        SquareCore::Vec2 offset_position = {0.0f, 0.0f};
        if (player_direction == Direction::RIGHT)
            offset_position.x = -25.0f;
        else if (player_direction == Direction::LEFT)
            offset_position.x = 25.0f;

        ResetAnimation(dash);
        SetPosition(dash, player_position.x + offset_position.x, player_position.y + offset_position.y);
        FlipSprite(dash, !GetFlipX(player), false);
        SetEntityVisible(dash, true);
        
        normal_player_scale_y = GetScale(player).y;
    }

    if (is_dashing)
    {
        dash_duration += delta_time;
        
        SquareCore::Vec2 player_scale = GetScale(player);
        float squished_scale_y = normal_player_scale_y / 1.5f;
        
        float squish_start = dash_length * 0.1f;
        float squish_end = dash_length * 0.9f;
        
        if (dash_duration <= squish_start)
        {
            float t = dash_duration / squish_start;
            float new_scale_y = SquareCore::Lerp(normal_player_scale_y, squished_scale_y, t);
            SetScale(player, {player_scale.x, new_scale_y});
        }
        else if (dash_duration >= squish_end)
        {
            float t = (dash_duration - squish_end) / (dash_length - squish_end);
            float new_scale_y = SquareCore::Lerp(squished_scale_y, normal_player_scale_y, t);
            SetScale(player, {player_scale.x, new_scale_y});
        }

        if (dash_duration >= dash_length)
        {
            SetEntityVisible(dash, false);
            dash_duration = 0.0f;
            is_dashing = false;
            
            SetScale(player, {player_scale.x, normal_player_scale_y});
        }
    }

    int max_dashes = player_data.has_double_dash ? 2 : 1;
    if (!in_cooldown && dashes_used > 0 && dashes_used < max_dashes)
    {
        time_since_last_dash += delta_time;
        if (time_since_last_dash > double_dash_window)
        {
            in_cooldown = true;
            dash_cooldown_elapsed = 0.0f;
        }
    }

    if (in_cooldown)
    {
        dash_cooldown_elapsed += delta_time;
        if (dash_cooldown_elapsed >= dash_cooldown)
        {
            in_cooldown = false;
            dashes_used = 0;
            time_since_last_dash = 0.0f;
            dash_cooldown_elapsed = 0.0f;
        }
    }
}

void Player::Heal(float delta_time)
{
    if (GetKeyPressed(heal_bind) && player_data.heals > 0)
    {
        for (auto& player_property : GetAllEntityProperties(player))
        {
            if (Character* player_character = dynamic_cast<Character*>(player_property))
            {
                if (player_character->health < player_data.max_health)
                {
                    PlayHealSound();
                    player_character->health = player_data.max_health;
                    player_data.heals--;
                }
            }
        }
    }
}

void Player::OnCollision(float delta_time)
{
    bool bouncing = false;
    bool onlyCollidingWithTop = true;
    uint32_t bounce_entity = 0;
    std::vector<std::pair<uint32_t, int>> collisions = GetEntityCollisions(player);
    std::vector<uint32_t> enemies_hit_this_frame;
    for (const auto& collision : collisions)
    {
        if (!EntityExists(collision.first)) continue;

        if (EntityHasTag(collision.first, "WormholeTrigger"))
        {
            if (is_transitioning) continue;
    
            if (EntityHasTag(collision.first, "1"))
            {
                is_transitioning = true;
                GameStateManager::SavePlayerData("Saves/S_001.square", player_data);
                player_data.level = 1;
                RemoveEntity(collision.first);
                map->LoadMap(1, player_data.spawn_points[0]);
                is_transitioning = false;
                return;
            }
            if (EntityHasTag(collision.first, "2"))
            {
                is_transitioning = true;
                GameStateManager::SavePlayerData("Saves/S_001.square", player_data);
                player_data.level = 2;
                RemoveEntity(collision.first);
                map->LoadMap(2, player_data.spawn_points[1]);
                is_transitioning = false;
                return;
            }
            if (EntityHasTag(collision.first, "3"))
            {
                is_transitioning = true;
                GameStateManager::SavePlayerData("Saves/S_001.square", player_data);
                player_data.level = 3;
                RemoveEntity(collision.first);
                map->LoadMap(3, player_data.spawn_points[2]);
                is_transitioning = false;
                return;
            }
            if (EntityHasTag(collision.first, "TeleportPlayer"))
            {
                TeleportPlayer({-12700.0f, -250.0f});
            }
        }
        
        if (EntityHasTag(collision.first, "Level2BoundsTrigger1") && !SquareCore::CompareFloats(target_bounds_y_min, -400.0f))
        {
            target_bounds_y_min = -400.0f;
            continue;
        }
        if (EntityHasTag(collision.first, "Level2BoundsTrigger2") && !SquareCore::CompareFloats(target_bounds_y_min, 500.0f))
        {
            target_bounds_y_min = 500.0f;
            continue;
        }
        if (EntityHasTag(collision.first, "Level2BoundsTrigger3") && !SquareCore::CompareFloats(target_bounds_y_min, -4800.0f))
        {
            target_bounds_y_min = -4800.0f;
            continue;
        }
        
        if (EntityHasTag(collision.first, "Level3BoundsTrigger1") && !SquareCore::CompareFloats(target_bounds_y_min, -400.0f))
        {
            target_bounds_y_min = -400.0f;
            continue;
        }
        if (EntityHasTag(collision.first, "Level3BoundsTrigger2") && !SquareCore::CompareFloats(target_bounds_y_min, 0.0f))
        {
            target_bounds_y_min = -1750.0f;
            continue;
        }
        if (EntityHasTag(collision.first, "Level3BoundsTrigger3") && !SquareCore::CompareFloats(target_bounds_y_min, 0.0f))
        {
            target_bounds_y_min = -3000.0f;
            continue;
        }
        if (EntityHasTag(collision.first, "Level3BoundsTrigger4") && !SquareCore::CompareFloats(target_bounds_y_min, 0.0f))
        {
            target_bounds_y_min = 5000.0f;
            continue;
        }
        
        if (EntityHasTag(collision.first, "FinalBossDoorLockTrigger") && !enemy_manager->boss_3_active && has_key)
        {
            uint32_t fbdo = GetFirstEntityWithTag("FinalBossDoorOpen");
            uint32_t fbdc = GetFirstEntityWithTag("FinalBossDoorClosed");
            uint32_t fbdl = GetFirstEntityWithTag("FinalBossDoorLock");
            SetEntityColor(fbdo, SquareCore::RGBA(255, 255, 255, 255));
            SetEntityVisible(fbdl, false);
            SetEntityVisible(fbdc, false);
        }
        
        if (EntityHasTag(collision.first, "DialogTriggerLastOcta"))
        {
            in_octagon_dialogue = true;
        }
        
        if (EntityHasTag(collision.first, "DialogTriggerReginald") && !enemy_manager->boss_3_active)
        {
            map->MuteAllMusic();
            in_final_boss_dialogue = true;
            
            uint32_t fbdo = GetFirstEntityWithTag("FinalBossDoorOpen");
            uint32_t fbdc = GetFirstEntityWithTag("FinalBossDoorClosed");
            uint32_t fbdl = GetFirstEntityWithTag("FinalBossDoorLock");
            SetEntityColor(fbdo, SquareCore::RGBA(255, 255, 255, 0));
            SetEntityVisible(fbdl, true);
            SetEntityVisible(fbdc, true);
        }

        if (EntityHasTag(collision.first, "Boss2Activate") && !enemy_manager->boss_2_active && !player_data.second_boss_dead)
        {
            enemy_manager->SpawnSecondBoss(SquareCore::Vec2(-5000.0f, -3700.0f));
            enemy_manager->boss_2_active = true;
            continue;
        }

        if (EntityHasTag(collision.first, "GainDoubleDash") && !player_data.has_double_dash)
        {
            PlayPickupSound();
            player_data.has_double_dash = true;
            user_interface->AbilityGained("Double Dash", "Press SHIFT in quick succession to dash twice");
            RemoveEntity(collision.first);
            continue;
        }
        if (EntityHasTag(collision.first, "GainDoubleJump") && !player_data.has_double_jump)
        {
            PlayPickupSound();
            player_data.has_double_jump = true;
            user_interface->AbilityGained("Double Jump", "Press SPACE while in the air to jump a second time");
            RemoveEntity(collision.first);
            continue;
        }
        if (EntityHasTag(collision.first, "GainProjectile") && !player_data.has_projectile)
        {
            PlayPickupSound();
            player_data.has_projectile = true;
            user_interface->AbilityGained("Projectile", "Press RMB to fire a ranged projectile");
            RemoveEntity(collision.first);
            continue;
        }
        if (EntityHasTag(collision.first, "GainFasterSlash") && !player_data.has_faster_slash)
        {
            PlayPickupSound();
            player_data.has_faster_slash = true;
            user_interface->AbilityGained("Faster Slash", "idk maybe slash faster or something you can figure it out");
            RemoveEntity(collision.first);
            continue;
        }
        if (EntityHasTag(collision.first, "GainKey") && !has_key)
        {
            PlayPickupSound();
            has_key = true;
            user_interface->AbilityGained("Mysterious Golden Key", "I wonder what this unlocks?");
            RemoveEntity(collision.first);
            continue;
        }
        
        // player collides with an enemy
        if (EntityHasTag(collision.first, "Enemy"))
        {
            bool already_processed = false;
            for (uint32_t hit_enemy : enemies_hit_this_frame)
            {
                if (hit_enemy == collision.first)
                {
                    already_processed = true;
                    break;
                }
            }
        
            if (already_processed) continue;
            enemies_hit_this_frame.push_back(collision.first);

            if (!EntityHasTag(collision.first, "SecondBoss"))
            {
                float knockback_x = (player_direction == Direction::RIGHT ? -1.0f : 1.0f) * 500.0f;
                float knockback_y = collision.second != 0 ? 200.0f : 0.0f;
                collision.second == 0 ? knockback_x *= 10.0f : knockback_x *= 1.0f;

                SetVelocity(player, knockback_x, knockback_y);
            }

            bool can_hit = true;
            JumpEnemy* jump_enemy = nullptr;
            ChargeEnemy* charge_enemy = nullptr;
            JumpBoss* jump_boss = nullptr;
            SecondBoss* second_boss = nullptr;

            if (EntityHasTag(collision.first, "EnemyProjectile"))
            {
                can_hit = true;
            }
    
            for (auto& enemy_property : GetAllEntityProperties(collision.first))
            {
                if (JumpEnemy* je = dynamic_cast<JumpEnemy*>(enemy_property))
                {
                    jump_enemy = je;
                    if (jump_enemy->hit_player_this_attack)
                        can_hit = false;
                }
                if (ChargeEnemy* ce = dynamic_cast<ChargeEnemy*>(enemy_property))
                {
                    charge_enemy = ce;
                    if (charge_enemy->hit_player_this_attack)
                        can_hit = false;
                }
                if (JumpBoss* jb = dynamic_cast<JumpBoss*>(enemy_property))
                {
                    jump_boss = jb;
                    if (jump_boss->hit_player_this_attack)
                        can_hit = false;
                }
                if (SecondBoss* sb = dynamic_cast<SecondBoss*>(enemy_property))
                {
                    second_boss = sb;
                    if (sb->hit_player_this_attack)
                        can_hit = false;
                }
            }
    
            if (can_hit)
            {
                SDL_Log("Dealing damage - can_take_damage before: %d", can_take_damage);
                for (auto& enemy_property : GetAllEntityProperties(collision.first))
                {
                    if (Character* enemy_character = dynamic_cast<Character*>(enemy_property))
                    {
                        for (auto& player_property : GetAllEntityProperties(player))
                        {
                            if (Character* player_character = dynamic_cast<Character*>(player_property))
                            {
                                if (jump_enemy)
                                    jump_enemy->hit_player_this_attack = true;
                                if (charge_enemy)
                                    charge_enemy->hit_player_this_attack = true;
                                if (jump_boss)
                                    jump_boss->hit_player_this_attack = true;
                                if (second_boss)
                                    second_boss->hit_player_this_attack = true;
                        
                                TakeDamage(player_character, enemy_character->damage);
                                if (player_character->health <= 0)
                                    return;
                            }
                        }
                    }
                }
            }
        }
        
        if (EntityHasTag(collision.first, "TrapWallTrigger"))
        {
            trap_walls = GetAllEntitiesWithTag("TrapWall");
            
            for (auto& trap_wall : trap_walls)
            {
                SetPosition(trap_wall, GetPosition(trap_wall).x, 1570.0f);
            }
            
            spawned_enemy_ids.clear();
            
            for (int i = 0; i < 3; i++)
            {
                PlayDoorSound();
                uint32_t charge_enemy = enemy_manager->SpawnChargeEnemy({800 + i * 100.0f, 1950.0f});
                uint32_t jump_enemy = enemy_manager->SpawnJumpEnemy({2100 - i * 100.0f, 1950.0f});
                FlipSprite(jump_enemy, false, false);
                spawned_enemy_ids.push_back(charge_enemy);
                spawned_enemy_ids.push_back(jump_enemy);
            }
            
            RemoveEntity(collision.first);
            continue;
        }
        
        if (collision.second == 2 && EntityHasTag(collision.first, "ValidGround"))
        {
            SquareCore::Vec2 player_pos = GetPosition(player);
            SquareCore::Vec2 ground_pos = GetPosition(collision.first);
            SquareCore::Vec2 ground_scale = GetScale(collision.first);
            
            float platform_half_width = (ground_scale.x * 500.0f) / 2.0f;
            float horizontal_distance = abs(player_pos.x - ground_pos.x);
            float safe_zone = platform_half_width * 0.9f;

            if (horizontal_distance < safe_zone)
            {
                last_grounded_position = player_pos;
                last_valid_ground_entity = collision.first;
            }
        }

        // player collides with a bouncy object
        if (EntityHasTag(collision.first, "Bounce"))
        {
            bouncing = true;
            bounce_entity = collision.first;
            if (collision.second != 2)
                onlyCollidingWithTop = false;
        }

        if (EntityHasTag(collision.first, "Spike"))
        {
            for (auto& player_property : GetAllEntityProperties(player))
            {
                if (Character* player_character = dynamic_cast<Character*>(player_property))
                {
                    TakeDamage(player_character, 1);
                    if (player_character->health <= 0)
                        return;
                }
            }
            SetVelocity(player, 0.0f, 0.0f);
            SetPosition(player, last_grounded_position.x, last_grounded_position.y);
            break;
        }
    }

    if (bouncing && onlyCollidingWithTop && bounce_entity)
    {
        PlayAudioClip(bounce_audio[bounce_audio_index]);
        bounce_audio_index = (bounce_audio_index + 1) % 5;
        HandleBounceEntities(delta_time, bounce_entity);
    }
}

void Player::TakeDamage(Character* player_character, int damage)
{
    if (!can_take_damage) return;
    can_take_damage = false;
    can_take_damage_timer = 0.0f;
    
    PlayDamageSound();
    player_character->health -= damage;
    SDL_Log(("Player health: " + std::to_string(player_character->health)).c_str());

    if (player_character->health <= 0)
    {
        bool was_boss_3_active = enemy_manager->boss_3_active;
    
        int spawn_index = map->current_map > 0 ? map->current_map - 1 : 0;
        last_grounded_position = player_data.spawn_points[spawn_index];
        map->LoadMap(map->current_map, player_data.spawn_points[spawn_index]);
    
        if (was_boss_3_active)
        {
            dialog_manager->ClearSeenEntry(6);
            TeleportPlayer({-11880.0, 5375.0});
            SetCameraPosition(GetPosition(player));
        }
    
        SDL_Log("Player died");
    }
}

void Player::DealDamage(Character* enemy_character, uint32_t enemy_id, int damage)
{
    if (!EntityExists(enemy_id)) return;

    if (EntityHasTag(enemy_id, "EnemyProjectile")) return;
    
    enemy_character->health -= damage;
    SDL_Log(("Enemy : " + std::to_string(enemy_id) + " now has " + std::to_string(enemy_character->health) + " health").c_str());

    if (enemy_character->health <= 0)
    {
        for (int i = 0; i < spawned_enemy_ids.size(); i++)
        {
            if (spawned_enemy_ids[i] == enemy_id)
            {
                spawned_enemy_ids.erase(spawned_enemy_ids.begin() + i);
                if (spawned_enemy_ids.empty())
                {
                    trap_walls = GetAllEntitiesWithTag("TrapWall");
            
                    for (auto& trap_wall : trap_walls)
                    {
                        SetPosition(trap_wall, GetPosition(trap_wall).x, 1870.0f);
                    }
                }
                break;
            }
        }
        
        if (EntityHasTag(enemy_id, "Level2Boss"))
        {
            if (uint32_t level_2_exit = GetFirstEntityWithTag("Level2Exit"))
            {
                RemoveEntity(level_2_exit);
                player_data.second_boss_dead = true;
                enemy_manager->boss_2_active = false;
            }
        }
        
        if (EntityHasTag(enemy_id, "JumpBoss"))
        {
            if (uint32_t level_1_wormhole_shield = GetFirstEntityWithTag("Level1WormholeShield"))
            {
                RemoveEntity(level_1_wormhole_shield);
                player_data.first_boss_dead = true;
                enemy_manager->boss_1_active = false;
            }
        }
        
        if (EntityHasTag(enemy_id, "FinalBoss"))
        {
            player_data.third_boss_dead = true;
            enemy_manager->boss_3_active = false;
            user_interface->ShowCredits(true);
            HealMaxHealth();
            SetMouseVisible(true);
            player_data.level = 1;
            map->LoadMap(0, SquareCore::Vec2(0.0f, 0.0f));
        }
        
        SDL_Log(("Enemy : " + std::to_string(enemy_id) + " died").c_str());
        enemies_to_remove.push_back(enemy_id);
    }
}

void Player::HandleBounceEntities(float delta_time, uint32_t current_bounce_entity)
{
    SquareCore::Vec2 player_velocity = GetVelocity(player);
    float bounce = 1200.0f;
    SetVelocity(player, player_velocity.x, bounce);
    
    bool already_bouncing = false;
    for (const auto& bounce : recently_bounced_on)
    {
        if (bounce.id == current_bounce_entity)
        {
            already_bouncing = true;
            break;
        }
    }
    
    if (!already_bouncing)
    {
        BounceEntity new_bounce;
        new_bounce.id = current_bounce_entity;
        new_bounce.original_scale_y = GetScale(current_bounce_entity).y;
        new_bounce.original_position_y = GetPosition(current_bounce_entity).y;
        recently_bounced_on.push_back(new_bounce);
        
        SquareCore::Vec2 bounce_position = GetPosition(current_bounce_entity);
        SetScale(current_bounce_entity, SquareCore::Vec2(0.15f, 0.075f));
        SetPosition(current_bounce_entity, bounce_position.x, bounce_position.y - 20.0f);
    }
}

void Player::UpdateBounceEntities(float delta_time)
{
    float lerp_speed = 5.0f * delta_time;
    
    for (int i = recently_bounced_on.size() - 1; i >= 0; i--)
    {
        BounceEntity& squish = recently_bounced_on[i];
        
        if (!EntityExists(squish.id))
        {
            recently_bounced_on.erase(recently_bounced_on.begin() + i);
            continue;
        }
        
        SquareCore::Vec2 current_scale = GetScale(squish.id);
        SquareCore::Vec2 current_position = GetPosition(squish.id);
        
        float new_scale_y = SquareCore::Lerp(current_scale.y, squish.original_scale_y, lerp_speed);
        float new_position_y = SquareCore::Lerp(current_position.y, squish.original_position_y, lerp_speed);
        
        SetScale(squish.id, {current_scale.x, new_scale_y});
        SetPosition(squish.id, current_position.x, new_position_y);
        
        if (abs(new_scale_y - squish.original_scale_y) < 0.001f && abs(new_position_y - squish.original_position_y) < 0.5f)
        {
            SetScale(squish.id, {current_scale.x, squish.original_scale_y});
            SetPosition(squish.id, current_position.x, squish.original_position_y);
            recently_bounced_on.erase(recently_bounced_on.begin() + i);
        }
    }
}

bool Player::EnemyHitByCurrentSlash(uint32_t enemy_id)
{
    for (uint32_t id : damaged_by_slash_enemies)
    {
        if (enemy_id == id)
            return true;
    }
    return false;
}

bool Player::IsGrounded(uint32_t playerId)
{
    auto collisions = GetEntityCollisions(playerId);
    for (const auto& collision : collisions)
    {
        if (collision.second == 2 && !EntityHasTag(collision.first, "PlayerSlash"))
        {
            SquareCore::Vec2 vel = GetVelocity(playerId);
            if (vel.y < 0)
            {
                SetVelocity(playerId, vel.x, vel.y);
            }
            
            return true;
        }
    }
    return false;
}



void Player::PlaySlashSound()
{
    PlayAudioClip(slash_audio[slash_audio_index]);
    slash_audio_index = (slash_audio_index + 1) % slash_audio.size();
}

void Player::PlayJumpSound()
{
    PlayAudioClip(jump_audio[jump_audio_index]);
    jump_audio_index = (jump_audio_index + 1) % 5;
}

void Player::PlayHealSound()
{
    PlayAudioClip(heal_audio[heal_audio_index]);
    heal_audio_index = (heal_audio_index + 1) % heal_audio.size();
}

void Player::PlayDashSound()
{
    PlayAudioClip(dash_audio[dash_audio_index]);
    dash_audio_index = (dash_audio_index + 1) % dash_audio.size();
}

void Player::PlayProjectileSound()
{
    PlayAudioClip(projectile_audio[projectile_audio_index]);
    projectile_audio_index = (projectile_audio_index + 1) % projectile_audio.size();
}

void Player::PlayDamageSound()
{
    PlayAudioClip(take_damage_audio[take_damage_audio_index]);
    take_damage_audio_index = (take_damage_audio_index + 1) % take_damage_audio.size();
}

void Player::PlayLaserSound()
{
    PlayAudioClip(final_boss_laser_audio[final_boss_laser_audio_index]);
    final_boss_laser_audio_index = (final_boss_laser_audio_index + 1) % final_boss_laser_audio.size();
}

void Player::PlaySwordSound()
{
    PlayAudioClip(final_boss_sword_audio[final_boss_sword_audio_index]);
    final_boss_sword_audio_index = (final_boss_sword_audio_index + 1) % final_boss_sword_audio.size();
}

void Player::PlayBounceSound()
{
    PlayAudioClip(bounce_audio[bounce_audio_index]);
    bounce_audio_index = (bounce_audio_index + 1) % bounce_audio.size();
}

void Player::PlayPickupSound()
{
    PlayAudioClip(ability_pickup_audio[ability_pickup_audio_index]);
    ability_pickup_audio_index = (ability_pickup_audio_index + 1) % ability_pickup_audio.size();
}

void Player::PlayDoorSound()
{
    PlayAudioClip(door_slam_audio[door_slam_audio_index]);
    door_slam_audio_index = (door_slam_audio_index + 1) % door_slam_audio.size();
}

void Player::PlayEnemyDashSound()
{
    PlayAudioClip(enemy_dash_audio[enemy_dash_audio_index]);
    enemy_dash_audio_index = (enemy_dash_audio_index + 1) % enemy_dash_audio.size();
}


void Player::UpdateAudioVolumes()
{
    for (int i = 0; i < slash_audio.size(); i++)
    {
        SetAudioVolume(slash_audio[i], player_data.sfx_volume * 1.5f);
    }
    for (int i = 0; i < jump_audio.size(); i++)
    {
        SetAudioVolume(jump_audio[i], player_data.sfx_volume);
    }
    for (int i = 0; i < heal_audio.size(); i++)
    {
        SetAudioVolume(heal_audio[i], player_data.sfx_volume);
    }
    for (int i = 0; i < projectile_audio.size(); i++)
    {
        SetAudioVolume(projectile_audio[i], player_data.sfx_volume * 1.5f);
    }
    for (int i = 0; i < dash_audio.size(); i++)
    {
        SetAudioVolume(dash_audio[i], player_data.sfx_volume * 2.5f);
    }
    for (int i = 0; i < take_damage_audio.size(); i++)
    {
        SetAudioVolume(take_damage_audio[i], player_data.sfx_volume * 2.0f);
    }
    for (int i = 0; i < bounce_audio.size(); i++)
    {
        SetAudioVolume(bounce_audio[i], player_data.sfx_volume * 2.0f);
    }
    for (int i = 0; i < final_boss_laser_audio.size(); i++)
    {
        SetAudioVolume(final_boss_laser_audio[i], player_data.sfx_volume);
    }
    for (int i = 0; i < final_boss_sword_audio.size(); i++)
    {
        SetAudioVolume(final_boss_sword_audio[i], player_data.sfx_volume / 2.0f);
    }
    for (int i = 0; i < ability_pickup_audio.size(); i++)
    {
        SetAudioVolume(ability_pickup_audio[i], player_data.sfx_volume);
    }
    for (int i = 0; i < enemy_dash_audio.size(); i++)
    {
        SetAudioVolume(enemy_dash_audio[i], player_data.sfx_volume * 2.0f);
    }
}

void Player::ResetPlayerData()
{
    player_data = PlayerData();
    GameStateManager::SavePlayerData("S_001.square", player_data);
}
