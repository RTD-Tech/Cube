#pragma once

#include <array>

#include "Script.h"
#include "Properties.h"

class Player;

struct EnemyProjectileEntity
{
    uint32_t id;
    bool active = false;
    float timer = 0.0f;
    Direction direction = Direction::LEFT;
};

class EnemyManager : public SquareCore::Script
{
public:
    EnemyManager() {}
    void OnStart() override;
    void OnUpdate(float deltaTime) override;

    void LoadEnemies();
    void SetPlayerScript(Player* player_s) { this->player_script = player_s; }
    uint32_t SpawnChargeEnemy(const SquareCore::Vec2& position);
    uint32_t SpawnJumpEnemy(const SquareCore::Vec2& position);
    uint32_t SpawnJumpBoss(const SquareCore::Vec2& position = SquareCore::Vec2(-6000.0f, 50.0f));
    void SpawnSecondBoss(const SquareCore::Vec2& position = SquareCore::Vec2(0.0f, 0.0f));
    uint32_t SpawnFinalBoss(const SquareCore::Vec2& position);

    bool boss_1_active = false;
    bool boss_2_active = false;
    bool boss_3_active = false;

    bool final_boss_slam_active = false;
    bool final_boss_slash_active = false;

private:
    void AlterChargeEnemy(uint32_t enemy_id);
    void AlterJumpEnemy(uint32_t enemy_id);
    void DetermineSecondBossAttack(uint32_t boss_id);

private:
    uint32_t player = 0;
    Player* player_script = nullptr;
    
    std::vector<uint32_t> enemies;
    
    std::vector<SquareCore::Vec2> enemy_collider_vertices={
        SquareCore::Vec2(-41.25f, -41.25f),
        SquareCore::Vec2(0.0f, 41.25f),
        SquareCore::Vec2(41.25f, -41.25f)
    };
    std::vector<SquareCore::Vec2> boss_collider_vertices={
        SquareCore::Vec2(-275.5f, -275.5f),
        SquareCore::Vec2(0.0f, 275.5f),
        SquareCore::Vec2(275.5f, -275.5f)
    };
    std::vector<SquareCore::Vec2> final_boss_collider_vertices={
        SquareCore::Vec2(-275.5f * 0.5f, -275.5f * 0.5f),
        SquareCore::Vec2(0.0f, 275.5f * 0.5f),
        SquareCore::Vec2(275.5f * 0.5f, -275.5f * 0.5f)
    };
    std::vector<SquareCore::Vec2> boss_2_collider_vertices={
        SquareCore::Vec2(-275.5f * 0.1f, -275.5f *0.1f),
        SquareCore::Vec2(0.0f, 275.5f * 0.1f),
        SquareCore::Vec2(275.5f * 0.1f, -275.5f * 0.1f)
    };

    uint32_t jump_boss = 0;

    std::array<uint32_t, 3> second_bosses = {};
    std::array<SecondBoss*, 3> second_boss_properties = {};
    std::array<Character*, 3> second_boss_ch_properties = {};
    int active_boss = 0;
    SquareCore::Vec2 left_attack_pos = SquareCore::Vec2::zero();
    SquareCore::Vec2 right_attack_pos = SquareCore::Vec2::zero();
    float time_between_attacks = 3.0f;
    float time_elapsed_between_attacks = 0.0f;
    bool active_boss_has_attacked = false;
    float wind_up_time = 3.0f;
    float wind_up_timer = 0.0f;
    bool is_winding_up = false;
    int current_attack_type = -1;

    float intro_countdown = 3.0f;
    
    std::vector<EnemyProjectileEntity> boss_projectiles = {};
    uint32_t final_boss = 0;
    float fb_direction = 0.0f;
};