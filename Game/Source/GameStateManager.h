#pragma once

#include "json/json.hpp"
#include "Math/Math.h"

struct PlayerData
{
    float x_pos = 0;
    float y_pos = 0;
    int max_health = 5;
    int health = 5;
    int level = 1;
    int damage = 1;
    int heals = 3;
    int max_heals = 3;

    bool has_double_dash = false;
    bool has_double_jump = false;
    bool has_projectile = false;
    bool has_faster_slash = false;

    bool first_boss_dead = false;
    bool second_boss_dead = false;
    bool third_boss_dead = false;
    bool the_wall_dead = false;
    
    float sfx_volume = 0.5f;
    float music_volume = 0.5f;

    std::vector<SquareCore::Vec2> spawn_points = { {-100.0f, 200.0f}, {-250.0f, 200.0f}, {8300.0f, 2000.0f}};
};

class GameStateManager
{
public:
    static PlayerData LoadPlayerData(const std::string& save_path);
    static void SavePlayerData(const std::string& save_path, PlayerData data);
};