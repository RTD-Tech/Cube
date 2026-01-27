#pragma once

#include "Script.h"

enum class Direction : uint8_t
{
    UP = 0,
    RIGHT = 1,
    DOWN = 2,
    LEFT = 3,
};

enum class JumpEnemyState : uint8_t
{
    IDLE,
    WINDING_UP,
    JUMPING,
    RECOVERING
};

enum class ChargeEnemyState : uint8_t
{
    PATROLLING,
    NOTICING,
    PREPARING,
    CHARGING,
    STUNNED
};

enum class SecondBossState : uint8_t
{
    IDLE,
    TELEGRAPHING,
    ATTACKING,
    RETURNING
};

enum class SecondBossAttackType : uint8_t
{
    LEFT,
    RIGHT,
    ABOVE
};

enum class FinalBossState : uint8_t
{
    TALKING,
    ATTACKING,
    IDLE
};

enum class FinalBossAttackType : uint8_t
{
    NONE,
    SLASH,
    SHOOT,
    SLAM
};

struct Character : SquareCore::Property
{
    int max_health = 10;
    int health = 10;
    int damage = 1;

    Character(int max_health = 10, int health = 10, int damage = 1) { this->max_health = max_health; this->health = health; this->damage = damage; }
};

struct JumpEnemy : SquareCore::Property
{
    JumpEnemyState state = JumpEnemyState::IDLE;
    
    SquareCore::Vec2 base_scale = {1.0f, 1.0f};
    
    float detection_range = 800.0f;
    float jump_cooldown = 2.0f;
    float cooldown_timer = 0.0f;
    
    float windup_duration = 0.4f;
    float windup_timer = 0.0f;
    
    float recovery_duration = 0.4f;
    float recovery_timer = 0.0f;
    
    SquareCore::Vec2 base_jump_force = {600.0f, 1200.0f};
    SquareCore::Vec2 min_jump_force = {600.0f, 1200.0f};
    SquareCore::Vec2 max_jump_force = {1200.0f, 1800.0f};
    
    float min_distance_for_scaling = 100.0f;
    float max_distance_for_scaling = 800.0f;
    
    bool chasing = false;
    bool aggro_on_player = false;
    bool hit_player_this_attack = false;
    bool was_grounded = true;

    JumpEnemy(float detection_range, float jump_cooldown, SquareCore::Vec2 jump_force)
    {
        this->detection_range = detection_range;
        this->jump_cooldown = jump_cooldown;
        this->max_jump_force = jump_force;
        this->base_jump_force = jump_force;
    }
};

struct ChargeEnemy : SquareCore::Property
{
    ChargeEnemyState state = ChargeEnemyState::PATROLLING;
    Direction facing_direction = Direction::RIGHT;
    bool aware_of_player = false;
    bool aggro_on_player = false;
    
    SquareCore::Vec2 base_scale = {1.0f, 1.0f};
    
    float patrol_point_a_x;
    float patrol_point_b_x;
    float patrol_speed = 100.0f;
    
    float notice_duration = 0.3f;
    float notice_timer = 0.0f;
    
    float prepare_duration = 0.5f;
    float prepare_timer = 0.0f;
    
    float charge_duration = 1.0f;
    float charge_elapsed = 0.0f;
    float base_charge_speed = 300.0f;
    float min_charge_speed = 300.0f;
    float max_charge_speed = 600.0f;
    
    float stun_duration = 3.0f;
    float stun_elapsed = 0.0f;
    
    bool hit_player_this_attack = false;
    
    ChargeEnemy(float spawn_x, float patrol_range = 400.0f)
    {
        this->patrol_point_a_x = spawn_x;
        this->patrol_point_b_x = spawn_x + patrol_range;
    }
};

struct JumpBoss : SquareCore::Property
{
    float jump_cooldown = 3.0f;
    float jump_cooldown_timer = 0.0f;
    SquareCore::Vec2 jump_force = {5.0f, 3500.0f};
    
    float charge_force = 6000.0f;
    float charge_windup_time = 0.5f;
    float charge_windup_timer = 0.0f;
    bool is_winding_up = false;

    float close_range = 500.0f;
    float far_range = 2000.0f;
    
    bool hit_player_this_attack = false;
};

struct SecondBoss : SquareCore::Property
{
    uint32_t id = 0;
    SquareCore::Vec2 spawn_position;
    SecondBossState state = SecondBossState::IDLE;
    SecondBossAttackType attack_type = SecondBossAttackType::LEFT;
    
    float telegraph_duration = 0.5f;
    float telegraph_timer = 0.0f;
    float attack_velocity = 1000.0f;

    bool attacking = false;
    bool hit_player_this_attack = false;
    
    bool is_dead = false;
    
    SecondBoss(SquareCore::Vec2 spawn_pos)
    {
        this->spawn_position = spawn_pos;
    }
};

struct FinalBoss : SquareCore::Property
{
    FinalBossState state = FinalBossState::TALKING;
    FinalBossAttackType attack_type = FinalBossAttackType::NONE;

    FinalBossAttackType last_attack_type = FinalBossAttackType::NONE;

    SquareCore::Vec2 spawn_position;
    
    float time_between_attacks = 1.0f;
    float time_elapsed_between_attacks = 0.0f;

    int num_shots = 8;
    int shots_fired = 0;
    std::vector<SquareCore::Vec2> shot_positions = { {-9000.0f, 6000.0f}, {-9300.0f, 6000.0f}, {-9600.0f, 6000.0f}, {-9900.0f, 6000.0f}, {-10200.0f, 6000.0f}, {-10500.0f, 6000.0f}, {-10800.0f, 6000.0f}, {-11100, 6000.0f} };
    float time_between_shots = 0.5f;
    float time_elapsed_after_firing = 0.0f;
    
    uint32_t sword_entity = 0;
    uint32_t gun_entity = 0;
    
    bool gravity_enabled_for_fall = false;

    bool slashed = false;
    float slash_range = 800.0f;
    float slash_length = 2.0f;
    float slash_length_elapsed = 0.0f;
    float slash_force = 5000.0f;

    bool slammed = false;
    float slam_range = 100.0f;
    float slam_length = 1.0f;
    float slam_length_elapsed = 0.0f;
    float slam_speed = 2000.0f;
};