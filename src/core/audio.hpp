#pragma once
#include <SDL3/SDL.h>

namespace mion {

enum class SoundId : int {
    Hit,           // qualquer hit acertado
    PlayerAttack,  // player inicia ataque
    EnemyDeath,    // inimigo morre
    PlayerDeath,   // player morre
    COUNT
};

// Sistema de áudio usando SDL3 nativo (sem SDL_mixer).
// SFX: um stream persistente por SoundId, WAVs mantidos em memória.
// Música: stream único com looping via refill a cada frame.
// Todos os arquivos são WAV (sem dependência de decodificador externo).
class AudioSystem {
public:
    bool init();
    void shutdown();

    // Toca SFX enviando o buffer WAV para o stream correspondente.
    // Múltiplas chamadas rápidas concatenam o áudio no stream (ok para sons curtos).
    void play_sfx(SoundId id, float volume = 1.0f);

    // Carrega e inicia a música de fundo. Chame com caminho WAV.
    // Retorna false silenciosamente se o arquivo não existir.
    bool play_music(const char* path, float volume = 0.6f);

    void stop_music();

    // Deve ser chamado uma vez por frame para fazer looping da música
    void tick_music();

    bool is_initialized() const { return _device != 0; }

private:
    struct WavBuffer {
        Uint8*        data = nullptr;
        Uint32        len  = 0;
        SDL_AudioSpec spec = {};
    };

    SDL_AudioDeviceID _device = 0;

    // SFX
    WavBuffer        _sfx_wavs[(int)SoundId::COUNT]    = {};
    SDL_AudioStream* _sfx_streams[(int)SoundId::COUNT] = {};

    // Música
    WavBuffer        _music_wav    = {};
    SDL_AudioStream* _music_stream = nullptr;
    float            _music_volume = 0.6f;

    bool _load_sfx(SoundId id, const char* path);

    static const char* _sfx_paths[(int)SoundId::COUNT];
};

} // namespace mion
