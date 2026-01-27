#include "GameStateManager.h"
#include <SDL3/SDL_log.h>
#include <filesystem>
#include <fstream>

PlayerData GameStateManager::LoadPlayerData(const std::string& save_path)
{
    std::ifstream file(save_path);
    if (!file.is_open())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open file at path %s\n ... Proceeding with default data", save_path.c_str());
        return PlayerData();
    }
    
    nlohmann::json json;
    try
    {
        file >> json;
    }
    catch (const nlohmann::json::exception& e)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to parse JSON at path %s ... %s\n ... Proceeding with default data", save_path.c_str(), e.what());
        return PlayerData();
    }

    PlayerData data = PlayerData();
    if (json.contains("x_pos"))
        data.x_pos = json["x_pos"];
    if (json.contains("y_pos"))
        data.y_pos = json["y_pos"];
    if (json.contains("health"))
        data.health = json["health"];
    if (json.contains("level"))
        data.level = json["level"];
    if (json.contains("heals"))
        data.heals = json["heals"];
    if (json.contains("max_heals"))
        data.max_heals = json["max_heals"];
    if (json.contains("has_double_dash"))
        data.has_double_dash = json["has_double_dash"];
    if (json.contains("has_double_jump"))
        data.has_double_jump = json["has_double_jump"];
    if (json.contains("has_projectile"))
        data.has_projectile = json["has_projectile"];
    if (json.contains("has_faster_slash"))
        data.has_faster_slash = json["has_faster_slash"];
    if (json.contains("the_wall_dead"))
        data.the_wall_dead = json["the_wall_dead"];
    if (json.contains("max_health"))
        data.max_health = json["max_health"];
    if (json.contains("damage"))
        data.damage = json["damage"];
    if (json.contains("first_boss_dead"))
        data.first_boss_dead = json["first_boss_dead"];
    if (json.contains("second_boss_dead"))
        data.second_boss_dead = json["second_boss_dead"];
    if (json.contains("third_boss_dead"))
        data.third_boss_dead = json["third_boss_dead"];
    if (json.contains("music_volume"))
        data.music_volume = json["music_volume"];
    if (json.contains("sfx_volume"))
        data.sfx_volume = json["sfx_volume"];

    return data;
}

void GameStateManager::SavePlayerData(const std::string& save_path, PlayerData data)
{
    nlohmann::json json;
    json["x_pos"] = data.x_pos;
    json["y_pos"] = data.y_pos;
    json["health"] = data.health;
    json["level"] = data.level;
    json["heals"] = data.heals;
    json["max_heals"] = data.max_heals;
    json["has_double_dash"] = data.has_double_dash;
    json["has_double_jump"] = data.has_double_jump;
    json["has_projectile"] = data.has_projectile;
    json["has_faster_slash"] = data.has_faster_slash;
    json["max_health"] = data.max_health;
    json["damage"] = data.damage;
    json["first_boss_dead"] = data.first_boss_dead;
    json["second_boss_dead"] = data.second_boss_dead;
    json["third_boss_dead"] = data.third_boss_dead;
    json["the_wall_dead"] = data.the_wall_dead;
    json["music_volume"] = data.music_volume;
    json["sfx_volume"] = data.sfx_volume;

    std::filesystem::path path(save_path);
    if (path.has_parent_path())
    {
        std::filesystem::create_directories(path.parent_path());
    }
    
    std::ofstream file(save_path);
    if (!file.is_open())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open file at path %s", save_path.c_str());
        return;
    }

    try
    {
        file << json.dump(4);
    }
    catch (const nlohmann::json::exception& e)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to write to file %s ... %s", save_path.c_str(), e.what());
    }
}