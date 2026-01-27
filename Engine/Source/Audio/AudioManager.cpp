#include "AudioManager.h"
#include <SDL3/SDL_log.h>
#include <algorithm>

namespace SquareCore {

AudioManager::AudioManager() {
    engine_config = ma_engine_config_init();
    ma_result result = ma_engine_init(&engine_config, &engine);
    if (result != MA_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error initializing miniaudio engine");
    }
}

AudioManager::~AudioManager() {
    for (auto& it : audioClips) {
        ma_sound_uninit(&it.second->sound);
        delete it.second;
    }
    audioClips.clear();
    
    ma_engine_uninit(&engine);
}

uint32_t AudioManager::AddAudioClip(const std::string& path) {
    AudioClip* audioClip = new AudioClip();
    audioClip->id = nextAudioId++;
    
    ma_result result = ma_sound_init_from_file(&engine, path.c_str(), MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC,
                                               nullptr, nullptr, &audioClip->sound);
    if (result != MA_SUCCESS) {
        SDL_Log("Failed to load audio clip: %s", path.c_str());
        delete audioClip;
        return 0;
    }
    
    audioClips[audioClip->id] = audioClip;
    
    return audioClip->id;
}

void AudioManager::PlayAudioClip(uint32_t id) {
    auto it = audioClips.find(id);
    if (it == audioClips.end()) {
        return;
    }
    ma_sound_start(&audioClips[id]->sound);
}

void AudioManager::PauseAudioClip(uint32_t id) {
    auto it = audioClips.find(id);
    if (it == audioClips.end()) {
        return;
    }
    ma_sound_stop(&audioClips[id]->sound);
}

void AudioManager::StopAudioClip(uint32_t id) {
    auto it = audioClips.find(id);
    if (it == audioClips.end()) {
        return;
    }
    ma_sound_seek_to_pcm_frame(&audioClips[id]->sound, 0);
    ma_sound_stop(&audioClips[id]->sound);
}

void AudioManager::SetAudioVolume(uint32_t id, float volume) {
    auto it = audioClips.find(id);
    if (it == audioClips.end()) {
        return;
    }
    ma_sound_set_volume(&audioClips[id]->sound, std::clamp(volume, 0.0f, 10.0f));
}

float AudioManager::GetAudioVolume(uint32_t id) {
    auto it = audioClips.find(id);
    if (it == audioClips.end()) {
        return 0.0f;
    }
    return ma_sound_get_volume(&audioClips[id]->sound);
}

void AudioManager::SetAudioMasterVolume(float volume) {
    ma_engine_set_volume(&engine, std::clamp(volume, 0.0f, 10.0f));
}

float AudioManager::GetAudioMasterVolume() {
    return ma_engine_get_volume(&engine);
}

void AudioManager::SetAudioLooping(uint32_t id, bool loop) {
    auto it = audioClips.find(id);
    if (it == audioClips.end()) {
        return;
    }
    ma_sound_set_looping(&audioClips[id]->sound, loop ? MA_TRUE : MA_FALSE);
}

bool AudioManager::GetAudioLooping(uint32_t id) {
    auto it = audioClips.find(id);
    if (it == audioClips.end()) {
        return false;
    }
    return ma_sound_is_looping(&audioClips[id]->sound) == MA_TRUE;
}

void AudioManager::SetAudioPlayOnAwake(uint32_t id, bool playOnAwake) {
    auto it = audioClips.find(id);
    if (it == audioClips.end()) {
        return;
    }
    audioClips[id]->play_on_awake = playOnAwake;
}

bool AudioManager::GetAudioPlayOnAwake(uint32_t id) {
    auto it = audioClips.find(id);
    if (it == audioClips.end()) {
        return false;
    }
    return audioClips[id]->play_on_awake;
}

void AudioManager::SetAudioSpatialization(uint32_t id, bool spatialization) {
    auto it = audioClips.find(id);
    if (it == audioClips.end()) {
        return;
    }
    ma_sound_set_spatialization_enabled(&audioClips[id]->sound, spatialization);
}

bool AudioManager::IsAudioPlaying(uint32_t id) {
    auto it = audioClips.find(id);
    if (it == audioClips.end()) {
        return false;
    }
    return ma_sound_is_playing(&audioClips[id]->sound) == MA_TRUE;
}

bool AudioManager::IsAudioPaused(uint32_t id) {
    return !IsAudioPlaying(id) && !IsAudioStopped(id);
}

bool AudioManager::IsAudioStopped(uint32_t id) {
    auto it = audioClips.find(id);
    if (it == audioClips.end()) {
        return false;
    }
    return ma_sound_at_end(&audioClips[id]->sound) == MA_TRUE;
}

}
