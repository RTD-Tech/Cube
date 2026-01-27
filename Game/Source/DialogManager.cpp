#include "DialogManager.h"
#include <SDL3/SDL_log.h>
#include <json/json.hpp>
#include <fstream>

#include "Player.h"

bool DialogManager::Load(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open dialog file: %s", path.c_str());
        return false;
    }

    nlohmann::json json;
    try
    {
        json = nlohmann::json::parse(file, nullptr, true, true);
    }
    catch (const nlohmann::json::exception& e)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to parse dialog JSON: %s", e.what());
        return false;
    }

    for (auto& entry : json)
    {
        int id = entry["id"];
        std::vector<DialogLine*> dialogLines;

        for (auto& line : entry["lines"])
        {
            DialogLine* dl = new DialogLine();
            dl->speaker = line.value("speaker", "");
            dl->text = line.value("text", "");
            dialogLines.push_back(dl);
        }

        entries[id] = dialogLines;
    }

    return true;
}

void DialogManager::Start(int entryId)
{
    if (entries.find(entryId) == entries.end())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Dialog entry not found: %d", entryId);
        return;
    }
    
    if (seenEntries.count(entryId) > 0)
        return;

    player_script->CancelVelocity();
    currentEntry = entryId;
    currentLine = 0;
    active = true;
}

void DialogManager::Advance()
{
    if (!active) return;

    currentLine++;
    if (currentLine >= entries[currentEntry].size())
    {
        Close();
    }
}

void DialogManager::Close()
{
    if (currentEntry >= 0)
        seenEntries.insert(currentEntry);
    
    active = false;
    currentLine = 0;
    currentEntry = -1;
}

DialogLine* DialogManager::GetCurrentLine() const
{
    if (!active || entries.find(currentEntry) == entries.end())
        return nullptr;

    return entries.at(currentEntry)[currentLine];
}
