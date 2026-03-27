#include "audio.hpp"
#include <SDL3/SDL.h>
#include <algorithm>

namespace mion {

const char* AudioSystem::_sfx_paths[(int)SoundId::COUNT] = {
    "assets/audio/hit.wav",
    "assets/audio/player_attack.wav",
    "assets/audio/enemy_death.wav",
    "assets/audio/player_death.wav",
};

bool AudioSystem::init() {
    _device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!_device) {
        SDL_Log("AudioSystem: nao abriu dispositivo de audio: %s", SDL_GetError());
        return false;
    }

    // Carrega e configura streams de SFX
    for (int i = 0; i < (int)SoundId::COUNT; ++i) {
        if (!_load_sfx((SoundId)i, _sfx_paths[i])) continue;

        _sfx_streams[i] = SDL_CreateAudioStream(&_sfx_wavs[i].spec, nullptr);
        if (!_sfx_streams[i]) {
            SDL_Log("AudioSystem: falha ao criar stream SFX %d: %s", i, SDL_GetError());
            continue;
        }
        if (!SDL_BindAudioStream(_device, _sfx_streams[i])) {
            SDL_Log("AudioSystem: falha ao bind stream SFX %d: %s", i, SDL_GetError());
        }
    }

    return true;
}

void AudioSystem::shutdown() {
    stop_music();

    for (int i = 0; i < (int)SoundId::COUNT; ++i) {
        if (_sfx_streams[i]) {
            SDL_DestroyAudioStream(_sfx_streams[i]);
            _sfx_streams[i] = nullptr;
        }
        if (_sfx_wavs[i].data) {
            SDL_free(_sfx_wavs[i].data);
            _sfx_wavs[i].data = nullptr;
        }
    }

    if (_device) {
        SDL_CloseAudioDevice(_device);
        _device = 0;
    }
}

void AudioSystem::play_sfx(SoundId id, float volume) {
    int idx = (int)id;
    if (!_sfx_streams[idx] || !_sfx_wavs[idx].data) return;
    SDL_SetAudioStreamGain(_sfx_streams[idx], volume);
    SDL_PutAudioStreamData(_sfx_streams[idx],
                           _sfx_wavs[idx].data,
                           (int)_sfx_wavs[idx].len);
}

bool AudioSystem::play_music(const char* path, float volume) {
    stop_music();

    if (!SDL_LoadWAV(path, &_music_wav.spec, &_music_wav.data, &_music_wav.len)) {
        SDL_Log("AudioSystem: musica nao carregou '%s': %s", path, SDL_GetError());
        return false;
    }

    _music_volume = volume;
    _music_stream = SDL_CreateAudioStream(&_music_wav.spec, nullptr);
    if (!_music_stream) {
        SDL_Log("AudioSystem: falha ao criar stream de musica: %s", SDL_GetError());
        SDL_free(_music_wav.data);
        _music_wav.data = nullptr;
        return false;
    }

    SDL_SetAudioStreamGain(_music_stream, volume);
    SDL_BindAudioStream(_device, _music_stream);

    // Push inicial — tick_music() reenche conforme necessário
    SDL_PutAudioStreamData(_music_stream, _music_wav.data, (int)_music_wav.len);
    return true;
}

void AudioSystem::stop_music() {
    if (_music_stream) {
        SDL_DestroyAudioStream(_music_stream);
        _music_stream = nullptr;
    }
    if (_music_wav.data) {
        SDL_free(_music_wav.data);
        _music_wav.data = nullptr;
        _music_wav.len  = 0;
    }
}

void AudioSystem::tick_music() {
    if (!_music_stream || !_music_wav.data) return;
    // Reenche se o stream ficou abaixo de ~0.5s de áudio
    int available = SDL_GetAudioStreamAvailable(_music_stream);
    int bytes_per_sec = _music_wav.spec.freq
                      * _music_wav.spec.channels
                      * (SDL_AUDIO_BITSIZE(_music_wav.spec.format) / 8);
    if (available < bytes_per_sec / 2) {
        SDL_PutAudioStreamData(_music_stream, _music_wav.data, (int)_music_wav.len);
    }
}

bool AudioSystem::_load_sfx(SoundId id, const char* path) {
    int idx = (int)id;
    if (!SDL_LoadWAV(path, &_sfx_wavs[idx].spec,
                     &_sfx_wavs[idx].data, &_sfx_wavs[idx].len)) {
        SDL_Log("AudioSystem: SFX nao carregou '%s': %s (ignorado)", path, SDL_GetError());
        return false;
    }
    return true;
}

} // namespace mion
