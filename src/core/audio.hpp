#pragma once
#include <cmath>
#include <SDL3/SDL.h>

namespace mion {

enum class SoundId : int {
    Hit,           // qualquer hit acertado
    PlayerAttack,  // ataque melee do player
    EnemyDeath,    // inimigo morre
    PlayerDeath,   // player morre
    Dash,          // dash do player
    RangedAttack,  // ataque à distância
    SpellBolt,     // legado / fallback
    SpellNova,     // magia Nova (E)
    SpellFrost,    // FrostBolt (Q)
    SpellChain,    // Chain Lightning (R)
    SkillCleave,
    SkillMultiShot,
    SkillPoisonArrow,
    SkillBattleCry,
    Parry,         // parry bem-sucedido
    UiConfirm,     // avançar diálogo / UI
    UiDelete,      // apagar save (title)
    FootstepPlayer,
    FootstepEnemy,
    COUNT
};

enum class AmbientId : int {
    DungeonDrips,
    DungeonTorch,
    TownWind,
    TownBirds,
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

// Atenuação por distância (mundo): 0 fora de max_dist, quadrático no interior.
inline float sfx_distance_attenuation(float dist_sq, float max_dist) {
    if (max_dist <= 0.0f) return 0.0f;
    if (dist_sq >= max_dist * max_dist) return 0.0f;
    float dist = std::sqrt(dist_sq);
    float a    = 1.0f - (dist / max_dist);
    return a * a;
}

// Sistema de áudio usando SDL3 nativo (sem SDL_mixer).
// SFX: um stream persistente por SoundId, WAVs mantidos em memória.
// Música: stream único com looping via refill a cada frame.
// Todos os arquivos são WAV (sem dependência de decodificador externo).
class AudioSystem {
public:
    bool init();
    void shutdown();
    void set_master_volume(float volume);
    float master_volume() const { return _master_volume; }

    // Toca SFX enviando o buffer WAV para o stream correspondente.
    // Múltiplas chamadas rápidas concatenam o áudio no stream (ok para sons curtos).
    void play_sfx(SoundId id, float volume = 1.0f);

    /// Variação leve: pitch ±5% com resample linear (mono S16); fallback = play_sfx.
    void play_sfx_pitched(SoundId id, float volume = 1.0f);

    /// SFX com atenuação pelo centro da vista (cam_x/cam_y = centro mundo da câmara).
    void play_sfx_at(SoundId id, float world_x, float world_y, float cam_x, float cam_y,
                     float max_dist = 600.0f, float base_volume = 1.0f);

    // Carrega e inicia a música de fundo. Chame com caminho WAV.
    // Retorna false silenciosamente se o arquivo não existir.
    bool play_music(const char* path, float volume = 0.6f);

    void stop_music();

    // Fade-out gradual da música. Chame antes de stop_music() ou transição de cena.
    // tick_music() aplica o decay; stop_music() é chamado automaticamente ao fim,
    // salvo se o fade for parte de set_music_state (aí carrega a próxima faixa).
    void fade_music(int duration_ms);

    /// Troca faixa por estado (cross-fade). None = só parar ao fim do fade.
    void set_music_state(MusicState state);
    MusicState current_music_state() const { return _music_state; }

    void play_ambient(AmbientId id, float volume = 0.3f);
    void stop_ambient(AmbientId id);
    void stop_all_ambient();

    // Deve ser chamado uma vez por frame para looping música/ambiente e fades
    void tick_music();

    bool is_initialized() const { return _device != 0; }
    float effective_gain(float requested_volume, float fade_scale = 1.0f) const;

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

    SDL_AudioDeviceID _device = 0;

    // SFX
    WavBuffer        _sfx_wavs[(int)SoundId::COUNT]    = {};
    SDL_AudioStream* _sfx_streams[(int)SoundId::COUNT] = {};

    // Música
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

    static const char* _sfx_paths[(int)SoundId::COUNT];
    static const char* _ambient_paths[(int)AmbientId::AMBIENT_COUNT];
    static const char* _music_paths[(int)MusicState::MUSIC_STATE_COUNT];
};

} // namespace mion
