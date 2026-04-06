# Roadmap — Audio Polish
> Áudio ambiente, música dinâmica, variação de SFX, áudio posicional simples, footsteps.

---

## O que já existe

- `AudioSystem` — SDL3 streams, WAV-only, um stream por `SoundId`
- `play_sfx(SoundId, volume)` — envia buffer WAV para stream
- `play_music(path, volume)` + `fade_music(ms)` + `tick_music()`
- `SoundId`: Hit, PlayerAttack, EnemyDeath, PlayerDeath, Dash, RangedAttack,
  SpellBolt, SpellNova, Parry, UiConfirm, UiDelete

---

## O que não existe

- Sons ambiente (dungeon, cidade)
- Estados de música dinâmica (calmo, combate, boss, cidade)
- Variação de SFX (mesmo hit toda a vez)
- Áudio posicional (atenuação por distância)
- Footsteps do player/inimigos
- SFX para novas skills (do roadmap skill tree)

---

## Ficheiros a criar / tocar

| Ficheiro | Tipo |
|---|---|
| `src/core/audio.hpp` | +AmbientId, +MusicState, novos métodos |
| `src/core/audio.cpp` | implementar novos métodos |
| `src/scenes/dungeon_scene.hpp` | trigger music states + footsteps + ambient |
| `src/scenes/town_scene.hpp` | ambient cidade + música calma |
| `src/systems/movement_system.hpp` | acumulador de footstep |
| `assets/audio/` | novos ficheiros WAV (listados no fim) |

---

## 1. `src/core/audio.hpp` — extensão

### Novos `SoundId` (acrescentar antes de COUNT):
```cpp
// Skills novas (roadmap skill tree)
SpellFrost,
SpellChain,
SkillCleave,
SkillMultiShot,
SkillPoisonArrow,
SkillBattleCry,
// Footsteps
FootstepPlayer,
FootstepEnemy,
// Ambiente (disparados como SFX em loop manual, ou via AmbientSystem)
// Não entram em SoundId — são geridos separadamente
COUNT
```

### Novo `AmbientId`:
```cpp
enum class AmbientId : int {
    DungeonDrips,   // goteiras periódicas (dungeon)
    DungeonTorch,   // crepitar de tocha (dungeon)
    TownWind,       // vento suave (cidade)
    TownBirds,      // pássaros distantes (cidade)
    AMBIENT_COUNT
};
```

### Novo `MusicState`:
```cpp
enum class MusicState { None, Town, DungeonCalm, DungeonCombat, DungeonBoss, Victory };
```

### Novos métodos em `AudioSystem`:

```cpp
// Ambient — loop independente do stream de música
void play_ambient(AmbientId id, float volume = 0.3f);
void stop_ambient(AmbientId id);
void stop_all_ambient();

// Música dinâmica com cross-fade
void set_music_state(MusicState state);
MusicState current_music_state() const { return _music_state; }

// Áudio posicional — atenua por distância
// cam_x/cam_y = centro da câmara no mundo
void play_sfx_at(SoundId id, float world_x, float world_y,
                 float cam_x, float cam_y,
                 float max_dist = 600.0f, float base_volume = 1.0f);

// Variação: toca aleatoriamente um de N SoundIds contíguos a partir de base_id
// Ex.: play_sfx_variant(SoundId::Hit, 3) toca Hit_0, Hit_1 ou Hit_2 aleatoriamente
// (requer SoundIds adicionais Hit_1, Hit_2 no enum — ou uma abordagem diferente)
// Alternativa mais simples usada aqui:
void play_sfx_pitched(SoundId id, float volume = 1.0f);
// Aplica pitch shift leve aleatório (±5%) via resampling simples no buffer WAV
// Dá impressão de variação sem precisar de ficheiros extra
```

### Campos privados novos:
```cpp
// Ambient
struct AmbientTrack {
    WavBuffer        wav    = {};
    SDL_AudioStream* stream = nullptr;
    bool             active = false;
    float            volume = 0.3f;
};
AmbientTrack _ambient[(int)AmbientId::AMBIENT_COUNT] = {};

// Música dinâmica
MusicState   _music_state      = MusicState::None;
MusicState   _pending_state    = MusicState::None;
bool         _transitioning    = false;
float        _transition_timer = 0.0f;
static constexpr float kMusicTransitionSecs = 1.5f;

// Caminhos de música por estado
static const char* _music_paths[(int)MusicState::Victory + 1];
```

---

## 2. `src/core/audio.cpp` — implementar novos métodos

### `play_ambient()`:
```cpp
void AudioSystem::play_ambient(AmbientId id, float volume) {
    if (!is_initialized()) return;
    auto& track = _ambient[(int)id];
    if (!track.stream) return; // não carregado
    track.active = true;
    track.volume = volume;
    // Repor o stream para o início e iniciar loop
    SDL_ClearAudioStream(track.stream);
    SDL_PutAudioStreamData(track.stream, track.wav.data, track.wav.len);
    SDL_FlushAudioStream(track.stream);
}
```

### `tick_music()` — estender para estados dinâmicos:
```cpp
// Após o loop de música existente, acrescentar:
if (_transitioning) {
    _transition_timer += 1.0f / 60.0f; // aprox — melhor usar dt passado via parâmetro
    float t = _transition_timer / kMusicTransitionSecs;
    if (t >= 1.0f) {
        // Fade out completou; carregar próxima faixa
        stop_music();
        const char* path = _music_paths[(int)_pending_state];
        if (path && path[0]) play_music(path, 0.6f);
        _music_state   = _pending_state;
        _transitioning = false;
    }
}
```

### `set_music_state()`:
```cpp
void AudioSystem::set_music_state(MusicState state) {
    if (state == _music_state && !_transitioning) return;
    _pending_state    = state;
    _transitioning    = true;
    _transition_timer = 0.0f;
    fade_music((int)(kMusicTransitionSecs * 1000));
}
```

### `play_sfx_at()`:
```cpp
void AudioSystem::play_sfx_at(SoundId id, float wx, float wy,
                               float cam_x, float cam_y, float max_dist, float base_vol) {
    float dx   = wx - cam_x;
    float dy   = wy - cam_y;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist >= max_dist) return;
    float attenuation = 1.0f - (dist / max_dist);
    attenuation = attenuation * attenuation; // falloff quadrático
    play_sfx(id, base_vol * attenuation);
}
```

### Caminhos de música (definir como constante no .cpp):
```cpp
const char* AudioSystem::_music_paths[] = {
    "",                                     // None
    "assets/audio/music_town.wav",          // Town
    "assets/audio/music_dungeon_calm.wav",  // DungeonCalm
    "assets/audio/music_dungeon_combat.wav",// DungeonCombat
    "assets/audio/music_boss.wav",          // DungeonBoss
    "assets/audio/music_victory.wav",       // Victory
};
```

---

## 3. `src/scenes/dungeon_scene.hpp` — integração

### `enter()`:
```cpp
if (_audio) {
    _audio->play_ambient(AmbientId::DungeonDrips, 0.25f);
    _audio->play_ambient(AmbientId::DungeonTorch, 0.15f);
    _audio->set_music_state(MusicState::DungeonCalm);
}
```

### `exit()`:
```cpp
if (_audio) {
    _audio->stop_all_ambient();
    // fade_music já é chamado antes de voltar ao menu/cidade
}
```

### Mudar estado de música dinamicamente em `fixed_update()`:

```cpp
// Verificar se há inimigos em aggro para mudar música
if (_audio && !_stress_mode) {
    bool any_aggro = false;
    for (const auto& e : _enemies) {
        if (!e.is_alive) continue;
        float dx = _player.transform.x - e.transform.x;
        float dy = _player.transform.y - e.transform.y;
        if (dx*dx + dy*dy < e.aggro_range * e.aggro_range) { any_aggro = true; break; }
    }

    MusicState target = any_aggro ? MusicState::DungeonCombat : MusicState::DungeonCalm;
    if (_room_index == 3 && !_enemies.empty()) target = MusicState::DungeonBoss;
    if (target != _audio->current_music_state())
        _audio->set_music_state(target);
}
```

### SFX posicionais para inimigos:
```cpp
// Em MeleeCombatSystem hit (ou onde play_sfx(Hit) é chamado):
// Substituir por:
if (_audio) _audio->play_sfx_at(SoundId::Hit,
    enemy.transform.x, enemy.transform.y,
    _camera.x + _camera.viewport_w*0.5f,
    _camera.y + _camera.viewport_h*0.5f);
```

### Footstep acumulador (ver secção MovementSystem):
```cpp
// Campo em DungeonScene:
float _player_footstep_acc = 0.0f;
static constexpr float kFootstepDist = 48.0f; // pixels por passo

// Em fixed_update() após mover player:
if (_player.is_moving) {
    float dx = _player.transform.x - _player_prev_x;
    float dy = _player.transform.y - _player_prev_y;
    _player_footstep_acc += sqrtf(dx*dx + dy*dy);
    if (_player_footstep_acc >= kFootstepDist) {
        _player_footstep_acc -= kFootstepDist;
        if (_audio) _audio->play_sfx(SoundId::FootstepPlayer, 0.3f);
    }
}
_player_prev_x = _player.transform.x;
_player_prev_y = _player.transform.y;
```

---

## 4. `src/scenes/town_scene.hpp` — integração

### `enter()`:
```cpp
if (_audio) {
    _audio->play_ambient(AmbientId::TownWind, 0.20f);
    _audio->play_ambient(AmbientId::TownBirds, 0.12f);
    _audio->set_music_state(MusicState::Town);
}
```

### `exit()`:
```cpp
if (_audio) {
    _audio->stop_all_ambient();
    _audio->fade_music(800);
}
```

---

## 5. `src/systems/movement_system.hpp` — footstep enemy

```cpp
// Em MovementSystem::fixed_update(), para cada inimigo que se move:
// Acumular distância percorrida → play_sfx_at(FootstepEnemy, ...) quando
// distância > 60px (inimigos pisam mais devagar que o player)
// Nota: requer acesso ao AudioSystem + Camera — passar como parâmetros opcionais
//       ou gerir no dungeon_scene directamente como para o player
```

---

## 6. Assets WAV necessários

Ficheiros a criar / obter (todos WAV mono 44100 Hz):

```
assets/audio/
├── music_town.wav               ← música ambiente tranquila
├── music_dungeon_calm.wav       ← dungeon sem inimigos próximos
├── music_dungeon_combat.wav     ← dungeon em combate (mais tensa)
├── music_boss.wav               ← boss fight
├── music_victory.wav            ← vitória
├── ambient_dungeon_drips.wav    ← goteiras (loop curto ~3s)
├── ambient_dungeon_torch.wav    ← crepitar de fogo (loop ~2s)
├── ambient_town_wind.wav        ← vento suave (loop ~4s)
├── ambient_town_birds.wav       ← pássaros distantes (loop ~5s)
├── sfx_footstep_player.wav      ← passo suave
├── sfx_footstep_enemy.wav       ← passo pesado
├── sfx_spell_frost.wav
├── sfx_spell_chain.wav
├── sfx_skill_cleave.wav
├── sfx_skill_multishot.wav
├── sfx_skill_poison_arrow.wav
└── sfx_skill_battle_cry.wav
```

---

## Resumo de impacto

```
NOVOS ficheiros (0 código; 16 assets WAV)

FICHEIROS EXISTENTES alterados (5):
  src/core/audio.hpp              +AmbientId, +MusicState, +5 métodos
  src/core/audio.cpp              implementação dos novos métodos
  src/scenes/dungeon_scene.hpp    ambient start/stop, music state dinâmico,
                                  SFX posicionais, footstep player
  src/scenes/town_scene.hpp       ambient cidade + música calma
  src/systems/movement_system.hpp footstep enemy (opcional — pode ficar na cena)
```
