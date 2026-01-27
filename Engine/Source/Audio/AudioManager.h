#pragma once

#include "AudioClip.h"
#include <miniaudio/miniaudio.h>
#include <string>
#include <cstdint>
#include <unordered_map>

namespace SquareCore {

class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    
    uint32_t AddAudioClip(const std::string& path);
    
    void PlayAudioClip(uint32_t id);
    void PauseAudioClip(uint32_t id);
    void StopAudioClip(uint32_t id);
    
    void SetAudioVolume(uint32_t id, float volume);
    float GetAudioVolume(uint32_t id);
    void SetAudioMasterVolume(float volume);
    float GetAudioMasterVolume();
    void SetAudioLooping(uint32_t id, bool loop);
    bool GetAudioLooping(uint32_t id);
    void SetAudioPlayOnAwake(uint32_t id, bool playOnAwake);
    bool GetAudioPlayOnAwake(uint32_t id);
    void SetAudioSpatialization(uint32_t id, bool spatialization);
    
    bool IsAudioPlaying(uint32_t id);
    bool IsAudioPaused(uint32_t id);
    bool IsAudioStopped(uint32_t id);

private:
    std::unordered_map<uint32_t, AudioClip*> audioClips;
    uint32_t nextAudioId = 1;
    ma_engine engine;
    ma_engine_config engine_config;

};

}
