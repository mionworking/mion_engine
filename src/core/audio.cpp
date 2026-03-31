#include "audio.hpp"
#include <SDL3/SDL.h>
#include <cmath>
#include <cstring>
#include <random>
#include <vector>

namespace mion {

namespace {

float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

float rand_pitch_factor(std::mt19937& rng) {
    // ±5% em torno de 1.0
    float u = (float)(std::uniform_int_distribution<int>(0, 10000)(rng)) / 10000.0f;
    return 0.95f + u * 0.1f;
}

} // namespace

const char* AudioSystem::_music_paths[(int)MusicState::MUSIC_STATE_COUNT] = {
    nullptr,
    "assets/audio/music_town.wav",
    "assets/audio/music_dungeon_calm.wav",
    "assets/audio/music_dungeon_combat.wav",
    "assets/audio/music_boss.wav",
    "assets/audio/music_victory.wav",
};

const char* AudioSystem::_ambient_paths[(int)AmbientId::AMBIENT_COUNT] = {
    "assets/audio/ambient_dungeon_drips.wav",
    "assets/audio/ambient_dungeon_torch.wav",
    "assets/audio/ambient_town_wind.wav",
    "assets/audio/ambient_town_birds.wav",
};

const char* AudioSystem::_sfx_paths[(int)SoundId::COUNT] = {
    "assets/audio/hit.wav",
    "assets/audio/player_attack.wav",
    "assets/audio/enemy_death.wav",
    "assets/audio/player_death.wav",
    "assets/audio/dash.wav",
    "assets/audio/ranged_attack.wav",
    "assets/audio/spell_bolt.wav",
    "assets/audio/spell_nova.wav",
    "assets/audio/sfx_spell_frost.wav",
    "assets/audio/sfx_spell_chain.wav",
    "assets/audio/sfx_skill_cleave.wav",
    "assets/audio/sfx_skill_multishot.wav",
    "assets/audio/sfx_skill_poison_arrow.wav",
    "assets/audio/sfx_skill_battle_cry.wav",
    "assets/audio/parry.wav",
    "assets/audio/ui_confirm.wav",
    "assets/audio/ui_delete.wav",
    "assets/audio/sfx_footstep_player.wav",
    "assets/audio/sfx_footstep_enemy.wav",
};

void AudioSystem::_apply_music_state_after_fade(MusicState state) {
    _music_state = state;
    stop_music();
    const char* path = _music_paths[(int)state];
    if (!path || !path[0]) return;
    float v = 0.52f;
    if (state == MusicState::Town) v = 0.42f;
    else if (state == MusicState::Victory) v = 0.48f;
    else if (state == MusicState::DungeonBoss) v = 0.58f;
    play_music(path, v);
}

bool AudioSystem::init() {
    _device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!_device) {
        SDL_Log("AudioSystem: nao abriu dispositivo de audio: %s", SDL_GetError());
        return false;
    }

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

    for (int a = 0; a < (int)AmbientId::AMBIENT_COUNT; ++a) {
        if (!_load_ambient((AmbientId)a, _ambient_paths[a])) continue;
        AmbientTrack& t = _ambient[a];
        t.stream = SDL_CreateAudioStream(&t.wav.spec, nullptr);
        if (!t.stream) {
            SDL_Log("AudioSystem: stream ambiente %d falhou: %s", a, SDL_GetError());
            continue;
        }
        if (!SDL_BindAudioStream(_device, t.stream)) {
            SDL_Log("AudioSystem: bind ambiente %d falhou: %s", a, SDL_GetError());
        }
    }

    return true;
}

void AudioSystem::set_master_volume(float volume) {
    _master_volume = clamp01(volume);
    if (_music_stream) SDL_SetAudioStreamGain(_music_stream, effective_gain(_music_volume));
}

float AudioSystem::effective_gain(float requested_volume, float fade_scale) const {
    return clamp01(requested_volume) * _master_volume * clamp01(fade_scale);
}

void AudioSystem::shutdown() {
    stop_music();
    stop_all_ambient();

    for (int a = 0; a < (int)AmbientId::AMBIENT_COUNT; ++a) {
        if (_ambient[a].stream) {
            SDL_DestroyAudioStream(_ambient[a].stream);
            _ambient[a].stream = nullptr;
        }
        if (_ambient[a].wav.data) {
            SDL_free(_ambient[a].wav.data);
            _ambient[a].wav.data = nullptr;
        }
    }

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
    SDL_SetAudioStreamGain(_sfx_streams[idx], effective_gain(volume));
    SDL_PutAudioStreamData(_sfx_streams[idx],
                           _sfx_wavs[idx].data,
                           (int)_sfx_wavs[idx].len);
}

void AudioSystem::play_sfx_pitched(SoundId id, float volume) {
    int idx = (int)id;
    if (!_sfx_streams[idx] || !_sfx_wavs[idx].data) return;
    const SDL_AudioSpec& sp = _sfx_wavs[idx].spec;
    if (sp.format != SDL_AUDIO_S16 || sp.channels != 1) {
        play_sfx(id, volume);
        return;
    }

    const float pitch = _rng ? rand_pitch_factor(*_rng) : 1.0f;
    const Sint16*     in  = reinterpret_cast<const Sint16*>(_sfx_wavs[idx].data);
    const int         n_in = (int)_sfx_wavs[idx].len / 2;
    if (n_in < 2) {
        play_sfx(id, volume);
        return;
    }

    const int n_out = (int)std::floor((double)n_in / (double)pitch);
    if (n_out < 2) {
        play_sfx(id, volume);
        return;
    }

    std::vector<Sint16> out((size_t)n_out);
    for (int j = 0; j < n_out; ++j) {
        float    src  = (float)j * pitch;
        int      i0   = (int)src;
        int      i1   = i0 + 1;
        float    frac = src - (float)i0;
        if (i1 >= n_in) i1 = n_in - 1;
        Sint16   s0   = in[i0];
        Sint16   s1   = in[i1];
        float    m    = (1.0f - frac) * (float)s0 + frac * (float)s1;
        int      ir   = (int)std::lrintf(m);
        if (ir < -32768) ir = -32768;
        if (ir > 32767) ir = 32767;
        out[j] = (Sint16)ir;
    }

    SDL_SetAudioStreamGain(_sfx_streams[idx], effective_gain(volume));
    SDL_PutAudioStreamData(_sfx_streams[idx], out.data(), (int)(out.size() * sizeof(Sint16)));
}

void AudioSystem::play_sfx_at(SoundId id, float world_x, float world_y, float cam_x, float cam_y,
                              float max_dist, float base_volume) {
    float dx = world_x - cam_x;
    float dy = world_y - cam_y;
    float att = sfx_distance_attenuation(dx * dx + dy * dy, max_dist);
    if (att <= 0.0f) return;
    play_sfx_pitched(id, base_volume * att);
}

bool AudioSystem::play_music(const char* path, float volume) {
    _fading                 = false;
    _fade_then_change_state = false;
    stop_music();

    if (!path || !path[0]) return false;
    if (!SDL_LoadWAV(path, &_music_wav.spec, &_music_wav.data, &_music_wav.len)) {
        SDL_Log("AudioSystem: musica nao carregou '%s': %s", path, SDL_GetError());
        return false;
    }

    _music_volume = clamp01(volume);
    _music_stream = SDL_CreateAudioStream(&_music_wav.spec, nullptr);
    if (!_music_stream) {
        SDL_Log("AudioSystem: falha ao criar stream de musica: %s", SDL_GetError());
        SDL_free(_music_wav.data);
        _music_wav.data = nullptr;
        return false;
    }

    SDL_SetAudioStreamGain(_music_stream, effective_gain(_music_volume));
    SDL_BindAudioStream(_device, _music_stream);

    SDL_PutAudioStreamData(_music_stream, _music_wav.data, (int)_music_wav.len);
    return true;
}

void AudioSystem::stop_music() {
    _fading = false;
    _fade_then_change_state = false;
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

void AudioSystem::fade_music(int duration_ms) {
    if (!_music_stream) return;
    _fade_duration_ms         = (Uint32)(duration_ms > 0 ? duration_ms : 1);
    _fade_start_ticks         = SDL_GetTicks();
    _fading                   = true;
    _fade_then_change_state   = false;
}

void AudioSystem::set_music_state(MusicState state) {
    if (!is_initialized()) return;
    if (state == MusicState::None) {
        _fade_then_change_state = false;
        _fading                 = false;
        stop_music();
        _music_state = MusicState::None;
        return;
    }
    if (state == _music_state && !_fading) return;

    _pending_music_state = state;
    if (!_music_stream) {
        _apply_music_state_after_fade(state);
        _fade_then_change_state = false;
        _fading                 = false;
        return;
    }

    _fade_then_change_state = true;
    const int ms            = (int)(kMusicTransitionSecs * 1000.0f);
    _fade_duration_ms       = (Uint32)(ms > 0 ? ms : 1);
    _fade_start_ticks       = SDL_GetTicks();
    _fading                 = true;
}

void AudioSystem::play_ambient(AmbientId id, float volume) {
    if (!is_initialized()) return;
    auto& track = _ambient[(int)id];
    if (!track.stream || !track.wav.data) return;
    track.active  = true;
    track.volume  = volume;
    SDL_ClearAudioStream(track.stream);
    SDL_SetAudioStreamGain(track.stream, effective_gain(track.volume));
    SDL_PutAudioStreamData(track.stream, track.wav.data, (int)track.wav.len);
}

void AudioSystem::stop_ambient(AmbientId id) {
    auto& track = _ambient[(int)id];
    track.active = false;
    if (track.stream) SDL_ClearAudioStream(track.stream);
}

void AudioSystem::stop_all_ambient() {
    for (int a = 0; a < (int)AmbientId::AMBIENT_COUNT; ++a)
        stop_ambient((AmbientId)a);
}

void AudioSystem::tick_music() {
    // Fade da música
    if (_music_stream && _music_wav.data && _fading) {
        Uint32 elapsed = SDL_GetTicks() - _fade_start_ticks;
        if (elapsed >= _fade_duration_ms) {
            const bool change = _fade_then_change_state;
            _fading                   = false;
            _fade_then_change_state   = false;
            MusicState pending        = _pending_music_state;
            stop_music();
            if (change)
                _apply_music_state_after_fade(pending);
        } else {
            float t =
                1.0f - (float)elapsed / (float)_fade_duration_ms;
            SDL_SetAudioStreamGain(_music_stream, effective_gain(_music_volume, t));
        }
    } else if (_music_stream && _music_wav.data) {
        int available     = SDL_GetAudioStreamAvailable(_music_stream);
        int bytes_per_sec = _music_wav.spec.freq * _music_wav.spec.channels
                          * (SDL_AUDIO_BITSIZE(_music_wav.spec.format) / 8);
        if (available < bytes_per_sec / 2)
            SDL_PutAudioStreamData(_music_stream, _music_wav.data, (int)_music_wav.len);
    }

    // Loops de ambiente
    for (int a = 0; a < (int)AmbientId::AMBIENT_COUNT; ++a) {
        AmbientTrack& tr = _ambient[a];
        if (!tr.active || !tr.stream || !tr.wav.data) continue;
        SDL_SetAudioStreamGain(tr.stream, effective_gain(tr.volume));
        int available = SDL_GetAudioStreamAvailable(tr.stream);
        int bps       = tr.wav.spec.freq * tr.wav.spec.channels
                  * (SDL_AUDIO_BITSIZE(tr.wav.spec.format) / 8);
        if (available < bps / 2)
            SDL_PutAudioStreamData(tr.stream, tr.wav.data, (int)tr.wav.len);
    }
}

bool AudioSystem::_load_ambient(AmbientId id, const char* path) {
    int idx = (int)id;
    if (!SDL_LoadWAV(path, &_ambient[idx].wav.spec,
                     &_ambient[idx].wav.data, &_ambient[idx].wav.len)) {
        SDL_Log("AudioSystem: ambiente nao carregou '%s': %s (ignorado)", path, SDL_GetError());
        return false;
    }
    return true;
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
