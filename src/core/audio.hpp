#pragma once
#include <cmath>
#include <random>
#include <SDL3/SDL.h>

namespace mion {

enum class SoundId : int {
    Hit,              // any hit landed
    PlayerAttack,     // player melee attack
    EnemyDeath,       // enemy dies
    PlayerDeath,      // player dies
    Dash,             // player dash
    RangedAttack,     // ranged attack
    SpellBolt,        // Strafe volley (spell slot 3 when Chain unavailable)
    SpellNova,        // Nova spell (E)
    SpellFrost,       // FrostBolt (Q)
    SpellChain,       // Chain Lightning (R)
    SkillCleave,
    SkillMultiShot,
    SkillPoisonArrow,
    SkillBattleCry,
    Parry,            // successful parry
    UiConfirm,        // advance dialogue / UI
    UiDelete,         // erase save (title screen)
    FootstepPlayer,
    FootstepEnemy,
    COUNT
};

enum class AmbientId : int {
    DungeonDrips,
    DungeonTorch,
    TownWind,
    TownBirds,
    TransitionWind,
    AMBIENT_COUNT
};

enum class MusicState : int {
    None = 0,
    Town,
    DungeonCalm,
    DungeonCombat,
    DungeonBoss,
    Victory,
    MUSIC_STATE_COUNT
};

// World-space distance attenuation: 0 beyond max_dist, quadratic falloff inside.
inline float sfx_distance_attenuation(float dist_sq, float max_dist) {
    if (max_dist <= 0.0f) return 0.0f;
    if (dist_sq >= max_dist * max_dist) return 0.0f;
    float dist = std::sqrt(dist_sq);
    float a    = 1.0f - (dist / max_dist);
    return a * a;
}

// Audio system using native SDL3 (no SDL_mixer).
// SFX: one persistent stream per SoundId, WAVs kept in memory.
// Music: single stream with looping via per-frame refill.
// All files are WAV (no external decoder dependency).
class AudioSystem {
public:
    bool init();
    void shutdown();
    void set_master_volume(float volume);
    float master_volume() const { return _master_volume; }

    // Plays SFX by pushing the WAV buffer into the corresponding stream.
    // Multiple quick calls queue audio in the stream (fine for short sounds).
    void play_sfx(SoundId id, float volume = 1.0f);

    // Slight pitch variation: ±5% with linear resample (mono S16); falls back to play_sfx.
    void play_sfx_pitched(SoundId id, float volume = 1.0f);

    // SFX with distance attenuation from the camera center (cam_x/cam_y = world camera center).
    void play_sfx_at(SoundId id, float world_x, float world_y, float cam_x, float cam_y,
                     float max_dist = 600.0f, float base_volume = 1.0f);

    // Loads and starts background music. Call with a WAV path.
    // Returns false silently if the file does not exist.
    bool play_music(const char* path, float volume = 0.6f);

    void stop_music();

    // Gradual music fade-out. Call before stop_music() or scene transitions.
    // tick_music() applies decay; stop_music() is called automatically at the end,
    // unless the fade is part of set_music_state (which loads the next track).
    void fade_music(int duration_ms);

    // Switch track by state (cross-fade). None = just stop at the end of the fade.
    void set_music_state(MusicState state);
    MusicState current_music_state() const { return _music_state; }

    void play_ambient(AmbientId id, float volume = 0.3f);
    void stop_ambient(AmbientId id);
    void stop_all_ambient();

    // Must be called once per frame for music/ambient looping and fades.
    void tick_music();

    bool is_initialized() const { return _device != 0; }
    float effective_gain(float requested_volume, float fade_scale = 1.0f) const;

    void set_rng(std::mt19937* r) { _rng = r; }

private:
    struct WavBuffer {
        Uint8*        data = nullptr;
        Uint32        len  = 0;
        SDL_AudioSpec spec = {};
    };

    struct AmbientTrack {
        WavBuffer        wav{};
        SDL_AudioStream* stream = nullptr;
        bool             active = false;
        float            volume = 0.3f;
    };

    std::mt19937* _rng = nullptr;
    SDL_AudioDeviceID _device = 0;

    // SFX
    WavBuffer        _sfx_wavs[(int)SoundId::COUNT]    = {};
    SDL_AudioStream* _sfx_streams[(int)SoundId::COUNT] = {};

    // Music
    WavBuffer        _music_wav    = {};
    SDL_AudioStream* _music_stream = nullptr;
    float            _music_volume = 0.6f;
    float            _master_volume = 1.0f;

    MusicState _music_state           = MusicState::None;
    MusicState _pending_music_state   = MusicState::None;
    bool       _fade_then_change_state = false;

    AmbientTrack _ambient[(int)AmbientId::AMBIENT_COUNT]{};

    // Fade
    bool   _fading            = false;
    Uint32 _fade_start_ticks  = 0;
    Uint32 _fade_duration_ms  = 0;

    static constexpr float kMusicTransitionSecs = 1.5f;

    void _apply_music_state_after_fade(MusicState state);
    bool _load_ambient(AmbientId id, const char* path);
    bool _load_sfx(SoundId id, const char* path);

};

} // namespace mion
