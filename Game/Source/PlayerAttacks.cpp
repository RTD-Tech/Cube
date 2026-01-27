#include "Player.h"
#include "GameStateManager.h"

void Player::Slash(float delta_time)
{
    SquareCore::Vec2 player_velocity = GetVelocity(player);
    SquareCore::Vec2 player_position = GetPosition(player);
    
    if (GetMouseButtonPressed(0) && !dialog_manager->IsActive() && !is_slashing && !slash_in_cooldown && !(player_velocity.x > 600.0f || player_velocity.x < -600.0f))
    {
        PlaySlashSound();
        
        is_slashing = true;
        slash_in_cooldown = true;
        slash_cooldown_elapsed = 0.0f;
        
        if (player_data.has_faster_slash)
            slash_cooldown = 0.1f;
        else
            slash_cooldown = 0.3f;
        
        if (is_looking_up)
        {
            slash_direction = Direction::UP;
            SquareCore::Vec2 offset_position = {0.0f, 75.0f};
            
            ResetAnimation(slash);
            SetPosition(slash, player_position.x + offset_position.x + (player_velocity.x / 10.0f), player_position.y + offset_position.y);
            SetRotation(slash, 90.0f);
            SetScale(slash, SquareCore::Vec2(0.2f, 0.15f));
            FlipSprite(slash, false, true);
            SetEntityVisible(slash, true);
        }
        else if (is_looking_down && !IsGrounded(player))
        {
            slash_direction = Direction::DOWN;
            SquareCore::Vec2 offset_position = {0.0f, -75.0f};
            
            ResetAnimation(slash);
            SetPosition(slash, player_position.x + offset_position.x + (player_velocity.x / 10.0f), player_position.y + offset_position.y);
            SetRotation(slash, -90.0f);
            SetScale(slash, SquareCore::Vec2(0.2f, 0.15f));
            FlipSprite(slash, false, false);
            SetEntityVisible(slash, true);
        }
        else
        {
            slash_direction = player_direction;
            SquareCore::Vec2 offset_position = {0.0f, 10.0f};
            if (slash_direction == Direction::RIGHT)
                offset_position.x = 100.0f;
            else if (slash_direction == Direction::LEFT)
                offset_position.x = -100.0f;

            ResetAnimation(slash);
            SetPosition(slash, player_position.x + offset_position.x + (player_velocity.x / 10.0f), player_position.y + offset_position.y);
            SetRotation(slash, 0.0f);
            SetScale(slash, SquareCore::Vec2(0.25f, 0.1f));
            FlipSprite(slash, !GetFlipX(player), false);
            SetEntityVisible(slash, true);
        }
    }

    if (is_slashing)
    {
        slash_duration += delta_time;

        std::vector<std::pair<uint32_t, int>> collisions = GetEntityCollisions(slash);
        for (std::pair<uint32_t, int>& collision : collisions)
        {
            if (!EntityExists(collision.first)) continue;

            if (EntityHasTag(collision.first, "Lever2Back"))
            {
                FlipSprite(collision.first, true, false);
                if (uint32_t lever_2_block = GetFirstEntityWithTag("Lever2BackBlock"))
                {
                    RemoveEntity(lever_2_block);
                }
                continue;
            }

            if (EntityHasTag(collision.first, "Lever2Boss"))
            {
                FlipSprite(collision.first, true, false);
                std::vector<uint32_t> lever_2_boss_blocks = GetAllEntitiesWithTag("Boss2Gate");
                for (uint32_t block : lever_2_boss_blocks)
                {
                    if (EntityExists(block))
                    {
                        RemoveEntity(block);
                    }
                }
                continue;
            }

            if (EntityHasTag(collision.first, "Pogo"))
            {
                if (slash_direction == Direction::DOWN)
                {
                    player_velocity = GetVelocity(player);
                    float pogo_bounce = 800.0f;
                    SetVelocity(player, player_velocity.x, pogo_bounce);
                }
            }
        
            if (EntityHasTag(collision.first, "Enemy") && !EnemyHitByCurrentSlash(collision.first))
            {
                damaged_by_slash_enemies.push_back(collision.first);
                
                for (auto& property : GetAllEntityProperties(collision.first))
                {
                    if (ChargeEnemy* charge_enemy = dynamic_cast<ChargeEnemy*>(property))
                    {
                        charge_enemy->aware_of_player = true;
                    }
                    if (JumpEnemy* jump_enemy = dynamic_cast<JumpEnemy*>(property))
                    {
                        jump_enemy->aggro_on_player = true;
                    }
                }
                
                SquareCore::Vec2 enemy_velocity = GetVelocity(collision.first);
                SquareCore::Vec2 enemy_position = GetPosition(collision.first);

                float direction_x = enemy_position.x - player_position.x;
                float knockback_x = (direction_x > 0 ? 1.0f : -1.0f) * slash_knockback;
                float knockback_y;
            
                if (slash_direction == Direction::UP)
                {
                    knockback_y = slash_knockback;
                }
                else if (slash_direction == Direction::DOWN)
                {
                    knockback_y = -slash_knockback;
                }
                else
                {
                    knockback_y = 200.0f;
                }

                if (!EntityHasTag(collision.first, "EnemyProjectile"))
                {
                    SetVelocity(collision.first, enemy_velocity.x + knockback_x, enemy_velocity.y + knockback_y);
                }
            
                for (auto& property : GetAllEntityProperties(collision.first))
                {
                    if (Character* health_property = dynamic_cast<Character*>(property))
                    {
                        DealDamage(health_property, collision.first, slash_damage);
                    }
                }
            }
        }

        if (slash_duration >= slash_length)
        {
            SetEntityVisible(slash, false);
            damaged_by_slash_enemies.clear();
            slash_duration = 0.0f;
            is_slashing = false;
        }
    }

    if (slash_in_cooldown)
    {
        slash_cooldown_elapsed += delta_time;
        if (slash_cooldown_elapsed >= slash_cooldown)
        {
            slash_in_cooldown = false;
            slash_cooldown_elapsed = 0.0f;
        }
    }
}

void Player::Projectile(float delta_time)
{
    SquareCore::Vec2 player_position = GetPosition(player);
    
    if (GetMouseButtonPressed(2) && !dialog_manager->IsActive() && !projectile_in_cooldown && player_data.has_projectile)
    {
        PlayProjectileSound();
        
        for (int i = 0; i < projectile_pool.GetTotal(); i++)
        {
            ProjectileEntity* projectile = static_cast<ProjectileEntity*>(projectile_pool.GetPointer(i));
            if (!projectile->active)
            {
                projectile->active = true;
                projectile->timer = 0.0f;
                projectile->direction = player_direction;
                projectile_in_cooldown = true;
                projectile_cooldown_elapsed = 0.0f;

        
                SquareCore::Vec2 spawn_offset = {0.0f, 10.0f};
                if (projectile->direction == Direction::RIGHT)
                    spawn_offset.x = 40.0f;
                else if (projectile->direction == Direction::LEFT)
                    spawn_offset.x = -40.0f;
        
                SetPosition(projectile->id, player_position.x + spawn_offset.x, player_position.y + spawn_offset.y);
                ResetAnimation(projectile->id);
                FlipSprite(projectile->id, projectile->direction != Direction::RIGHT, false);
                SetAnimationFPS(projectile->id, projectile_fps);
                SetAnimationFrame(projectile->id, 0);
                SetEntityVisible(projectile->id, true);

                float direction = player_direction == Direction::RIGHT ? -1.0f : 1.0f;
                SetVelocity(player, (direction * 1200.0f), 0.0f);
                break;
            }
        }
    }
    
    for (int i = 0; i < projectile_pool.GetTotal(); i++)
    {
        ProjectileEntity* projectile = static_cast<ProjectileEntity*>(projectile_pool.GetPointer(i));
        if (!projectile->active) continue;
        projectile->timer += delta_time;

        SquareCore::Vec2 current_position = GetPosition(projectile->id);
        float movement = projectile_speed * delta_time;
        if (projectile->direction == Direction::RIGHT)
            SetPosition(projectile->id, current_position.x + movement, current_position.y);
        else
            SetPosition(projectile->id, current_position.x - movement, current_position.y);
        
        if (IsAnimationComplete(projectile->id))
        {
            int final_frame = GetTotalFrames(projectile->id) - 1;
            SetAnimationFrame(projectile->id, final_frame);
            SetAnimationFPS(projectile->id, 0.0f);
        }
        
        std::vector<std::pair<uint32_t, int>> collisions = GetEntityCollisions(projectile->id);
        for (const auto& collision : collisions)
        {
            if (!EntityExists(collision.first)) continue;

            if (EntityHasTag(collision.first, "Breakable"))
            {
                uint32_t ball = GetFirstEntityWithTag("Ball");
                SetPhysicsEnabled(ball, true);
                RemoveEntity(collision.first);
                continue;
            }

            if (EntityHasTag(collision.first, "Ground"))
            {
                projectile->active = false;
                SetEntityVisible(projectile->id, false);
            }
            if (EntityHasTag(collision.first, "Enemy"))
            {
                for (auto& property : GetAllEntityProperties(collision.first))
                {
                    if (ChargeEnemy* charge_enemy = dynamic_cast<ChargeEnemy*>(property))
                    {
                        charge_enemy->aware_of_player = true;
                    }
                    if (JumpEnemy* jump_enemy = dynamic_cast<JumpEnemy*>(property))
                    {
                        jump_enemy->aggro_on_player = true;
                    }
                }
                
                float knockback_x = (projectile->direction == Direction::RIGHT ? 1.0f : -1.0f) * projectile_knockback;
                float knockback_y = 300.0f;

                if (!EntityHasTag(collision.first, "EnemyProjectile"))
                    SetVelocity(collision.first, knockback_x, knockback_y);

                for (auto& property : GetAllEntityProperties(collision.first))
                {
                    if (Character* health_property = dynamic_cast<Character*>(property))
                        DealDamage(health_property, collision.first, projectile_damage);
                }
            
                projectile->active = false;
                SetEntityVisible(projectile->id, false);
                break;
            }
        }

        if (projectile->timer >= projectile_max_life)
        {
            projectile->active = false;
            SetEntityVisible(projectile->id, false);
        }
    }
    
    if (projectile_in_cooldown)
    {
        projectile_cooldown_elapsed += delta_time;
        if (projectile_cooldown_elapsed >= projectile_cooldown)
        {
            projectile_in_cooldown = false;
            projectile_cooldown_elapsed = 0.0f;
        }
    }
}
