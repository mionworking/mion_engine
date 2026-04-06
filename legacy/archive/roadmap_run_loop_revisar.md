# Roadmap — Run Loop / Win Condition

> **Arquivado (2026-04-04):** rascunho histórico. Grande parte disto **já está no código** (`run_stats.hpp`, `game_over_scene`, `victory_scene`, dificuldade no título, `SaveData::last_run_stats`). Ver [ROADMAP.md](../../ROADMAP.md) para o estado actual.

> Game over com estatísticas, ecrã de vitória, seletor de dificuldade, tracking de run.

---

## O que já existe

- Player death detectado em `DungeonScene` (renderiza "GAME OVER", Escape reseta)
- Quest system (do roadmap cidade) marca Grimjaw como Completed
- `TitleScene` — menu simples
- `SaveSystem` — save/load em texto
- `_room_index` e `_stress_mode` em `DungeonScene`

---

## O que não existe

- Ecrã próprio de Game Over (está inline na dungeon)
- Ecrã de Vitória
- Tracking de estatísticas do run (tempo, inimigos mortos, dano tomado, etc.)
- Seletor de dificuldade
- New Game+ ou indicador de vitória prévia no menu

---

## Ficheiros a criar / tocar

| Ficheiro | Tipo |
|---|---|
| `src/core/run_stats.hpp` | NOVO — RunStats struct |
| `src/scenes/game_over_scene.hpp` | NOVO |
| `src/scenes/victory_scene.hpp` | NOVO |
| `src/core/scene_context.hpp` | +RunStats* ptr, +DifficultyLevel |
| `src/core/register_scenes.cpp` | +registo "game_over", "victory" |
| `src/scenes/dungeon_scene.hpp` | tracking stats + transição para "game_over" / "victory" |
| `src/scenes/title_scene.hpp` | seletor de dificuldade + indicador New Game+ |
| `src/core/save_system.hpp` | +victory_reached, +difficulty, +run_stats |
| `src/main.cpp` | passa RunStats para SceneCreateContext |

---

## 1. `src/core/run_stats.hpp` — NOVO

```cpp
#pragma once
namespace mion {

struct RunStats {
    int   rooms_cleared    = 0;
    int   enemies_killed   = 0;
    int   gold_collected   = 0;
    int   damage_taken     = 0;
    int   spells_cast      = 0;
    int   potions_used     = 0;
    float time_seconds     = 0.0f;
    int   max_level_reached= 1;
    bool  boss_defeated    = false;

    void reset() { *this = RunStats{}; }
};

} // namespace mion
```

Partilhado via `SceneCreateContext::run_stats` (ponteiro não-owner).
Instanciado em `main.cpp`, passado para todas as cenas.

---

## 2. `src/core/scene_context.hpp` — extensão

```cpp
// Acrescentar:
enum class DifficultyLevel { Easy = 0, Normal = 1, Hard = 2 };

struct SceneCreateContext {
    // ... campos existentes ...
    RunStats*       run_stats  = nullptr;  // instância em main.cpp; cenas escrevem aqui
    DifficultyLevel difficulty = DifficultyLevel::Normal;
};
```

---

## 3. `src/main.cpp` — instanciar RunStats

```cpp
mion::RunStats run_stats;
// ...
ctx.run_stats  = &run_stats;
ctx.difficulty = /* lido do save ou default Normal */;
```

---

## 4. `src/scenes/dungeon_scene.hpp` — tracking + transições

### Tracking (escrever em `_run_stats` via ponteiro):
```cpp
RunStats* _run_stats = nullptr;   // recebe via set_run_stats(RunStats*)

// Em _load_data_files() ou enter():
if (_run_stats) _run_stats->time_seconds = 0.0f;  // reset ao entrar

// Em fixed_update(), acumulador de tempo:
if (_run_stats) _run_stats->time_seconds += dt;

// Quando inimigo morre:
if (_run_stats) { _run_stats->enemies_killed++; }

// Quando player toma dano (em MeleeCombatSystem callback ou inline):
if (_run_stats) _run_stats->damage_taken += dmg;

// Quando spell é lançada:
if (_run_stats) _run_stats->spells_cast++;

// Quando sala avança:
if (_run_stats) _run_stats->rooms_cleared++;

// Quando gold é apanhado:
if (_run_stats) _run_stats->gold_collected += amount;
```

### Transição para "game_over" (substituir lógica inline actual):
```cpp
// No bloco de player morto (actualmente imprime "GAME OVER" e espera Escape):
if (!_player.is_alive && _player.was_alive) {
    if (_run_stats) _run_stats->boss_defeated = false;
    _next = "game_over";
}
```

### Transição para "victory":
```cpp
// Quando quest DefeatGrimjaw == Rewarded (verificado no enter() da TownScene)
// ou: quando DungeonScene deteta Grimjaw morto + porta de saída usada + quest ativa
// O mais limpo: TownScene::enter() verifica quest e muda _next = "victory" se for o momento
// (ver secção TownScene no roadmap cidade)
```

### Aplicar dificuldade ao spawn de inimigos:
```cpp
// _spawn_enemy(): após copiar stats do EnemyDef, aplicar multiplicadores:
float diff_hp_mult = (_difficulty == DifficultyLevel::Easy)   ? 0.75f :
                     (_difficulty == DifficultyLevel::Hard)   ? 1.35f : 1.0f;
float diff_dmg_mult= (_difficulty == DifficultyLevel::Easy)   ? 0.75f :
                     (_difficulty == DifficultyLevel::Hard)   ? 1.25f : 1.0f;
enemy.health.max_hp      = (int)(base_hp * diff_hp_mult);
enemy.health.current_hp  = enemy.health.max_hp;
enemy.attack_damage      = (int)(base_dmg * diff_dmg_mult);

// Hard: +1 inimigo extra no budget de spawn
// Easy: budget de spawn -1 (mínimo 1)
```

---

## 5. `src/scenes/game_over_scene.hpp` — NOVO

```cpp
class GameOverScene : public IScene {
public:
    int        viewport_w = 1280, viewport_h = 720;
    RunStats   run_stats  = {};     // cópia no enter()

    void set_audio(AudioSystem* a)   { _audio = a; }
    void set_run_stats(const RunStats& rs) { run_stats = rs; }

    void enter() override {
        _selected = 0;
        _prev_confirm = false;
        _prev_up = _prev_down = false;
        _elapsed = 0.0f;
        _list.items = {"RETRY (NEW RUN)", "MAIN MENU", "QUIT"};
    }

    void fixed_update(float dt, const InputState& input) override {
        _elapsed += dt;
        // edge nav
        if (input.ui_up_pressed   && !_prev_up)   _list.nav_up();
        if (input.ui_down_pressed && !_prev_down)  _list.nav_down();
        _prev_up   = input.ui_up_pressed;
        _prev_down = input.ui_down_pressed;

        if (input.confirm_pressed && !_prev_confirm && _elapsed > 0.8f) {
            switch (_list.selected) {
                case 0: SaveSystem::remove_default_saves(); _next = "title"; break;
                case 1: _next = "title"; break;
                case 2: _quit = true; break;
            }
        }
        _prev_confirm = input.confirm_pressed;
    }

    void render(SDL_Renderer* r) override {
        // Fundo vermelho escuro
        SDL_SetRenderDrawColor(r, 30, 5, 5, 255);
        SDL_RenderClear(r);

        // Título
        draw_text(r, cx - tw("GAME OVER",4)*0.5f, viewport_h*0.12f,
                  "GAME OVER", 4, 200, 40, 40, 255);

        // Estatísticas do run
        float sy = viewport_h * 0.30f;
        float sx = viewport_w * 0.30f;
        char buf[64];
        SDL_snprintf(buf,sizeof(buf),"Rooms cleared:   %d", run_stats.rooms_cleared);
        draw_text(r, sx, sy,      buf, 2, 200,200,190,255); sy += 28;
        SDL_snprintf(buf,sizeof(buf),"Enemies killed:  %d", run_stats.enemies_killed);
        draw_text(r, sx, sy,      buf, 2, 200,200,190,255); sy += 28;
        SDL_snprintf(buf,sizeof(buf),"Gold collected:  %d", run_stats.gold_collected);
        draw_text(r, sx, sy,      buf, 2, 200,200,190,255); sy += 28;
        SDL_snprintf(buf,sizeof(buf),"Damage taken:    %d", run_stats.damage_taken);
        draw_text(r, sx, sy,      buf, 2, 200,200,190,255); sy += 28;
        SDL_snprintf(buf,sizeof(buf),"Time:  %dm %02ds",
                     (int)(run_stats.time_seconds/60),
                     (int)run_stats.time_seconds % 60);
        draw_text(r, sx, sy,      buf, 2, 200,200,190,255); sy += 28;

        // Lista de opções
        _list.render(r, viewport_w*0.35f, viewport_h*0.70f);
    }

    const char* next_scene() const override { return _next.c_str(); }
    bool wants_quit() const { return _quit; }

private:
    std::string _next;
    bool  _quit = false, _prev_confirm=false, _prev_up=false, _prev_down=false;
    float _elapsed = 0.0f;
    ui::List _list;
    AudioSystem* _audio = nullptr;
};
```

---

## 6. `src/scenes/victory_scene.hpp` — NOVO

```cpp
class VictoryScene : public IScene {
public:
    int      viewport_w = 1280, viewport_h = 720;
    RunStats run_stats  = {};

    void enter() override {
        _selected = 0; _elapsed = 0.0f;
        _list.items = {"NEW GAME+", "MAIN MENU", "QUIT"};
        if (_audio) _audio->play_sfx(SoundId::UiConfirm, 1.0f);
    }

    void fixed_update(float dt, const InputState& input) override {
        _elapsed += dt;
        if (input.ui_up_pressed   && !_prev_up)   _list.nav_up();
        if (input.ui_down_pressed && !_prev_down)  _list.nav_down();
        _prev_up = input.ui_up_pressed; _prev_down = input.ui_down_pressed;

        if (input.confirm_pressed && !_prev_confirm && _elapsed > 1.0f) {
            switch (_list.selected) {
                case 0: /* new game+ flag no save */ _next = "title"; break;
                case 1: _next = "title"; break;
                case 2: _quit = true; break;
            }
        }
        _prev_confirm = input.confirm_pressed;
    }

    void render(SDL_Renderer* r) override {
        SDL_SetRenderDrawColor(r, 5, 20, 10, 255);
        SDL_RenderClear(r);

        draw_text(r, cx - tw("VICTORY",4)*0.5f, viewport_h*0.10f,
                  "VICTORY", 4, 80, 220, 100, 255);
        draw_text(r, cx - tw("Grimjaw has fallen. The depths are silent.",2)*0.5f,
                  viewport_h*0.24f,
                  "Grimjaw has fallen. The depths are silent.", 2, 190,230,190,255);

        // Estatísticas (mesmo layout do GameOverScene)
        // ...

        _list.render(r, viewport_w*0.35f, viewport_h*0.72f);
    }

    const char* next_scene() const override { return _next.c_str(); }
    bool wants_quit() const { return _quit; }

private:
    std::string _next;
    bool  _quit=false, _prev_confirm=false, _prev_up=false, _prev_down=false;
    float _elapsed=0.0f;
    ui::List _list;
    AudioSystem* _audio = nullptr;
};
```

---

## 7. `src/scenes/title_scene.hpp` — seletor de dificuldade

### Novo estado:
```cpp
enum class TitleState { Main, DifficultySelect };
TitleState _state = TitleState::Main;
DifficultyLevel _selected_diff = DifficultyLevel::Normal;
ui::List _diff_list;
bool _new_game_plus = false;  // lido do save
```

### `enter()`:
```cpp
_diff_list.items = {"EASY", "NORMAL", "HARD"};
_diff_list.selected = 1; // Normal default
_new_game_plus = SaveSystem::load_default_if_exists().victory_reached;
```

### `fixed_update()`:
```cpp
if (_state == TitleState::Main) {
    if (attack_edge) {
        _state = TitleState::DifficultySelect;
    }
} else if (_state == TitleState::DifficultySelect) {
    // nav up/down na lista de dificuldade
    // confirm → _selected_diff = (DifficultyLevel)_diff_list.selected; _next = "town"
    // cancel → voltar a Main
}
```

### `render()` — indicador New Game+:
```cpp
if (_new_game_plus)
    draw_text(r, cx - tw("★ NEW GAME+ AVAILABLE ★",2)*0.5f,
              viewport_h*0.80f, "★ NEW GAME+ AVAILABLE ★", 2, 255,215,0,255);
```

---

## 8. `src/core/save_system.hpp` — extensão

```cpp
// Em SaveData, acrescentar:
bool           victory_reached = false;
int            difficulty      = 1;  // 0=easy 1=normal 2=hard
RunStats       last_run_stats  = {};

// Serializar:
victory_reached=0/1
difficulty=N
last_enemies_killed=N
last_rooms_cleared=N
last_time_seconds=N.N
```

---

## 9. `src/core/register_scenes.cpp` — registo das novas cenas

```cpp
registry.register_scene("game_over", [](const SceneCreateContext& ctx) {
    auto s = std::make_unique<GameOverScene>();
    s->viewport_w = ctx.viewport_w;
    s->viewport_h = ctx.viewport_h;
    s->set_audio(ctx.audio);
    if (ctx.run_stats) s->set_run_stats(*ctx.run_stats);
    return s;
});

registry.register_scene("victory", [](const SceneCreateContext& ctx) {
    auto s = std::make_unique<VictoryScene>();
    s->viewport_w = ctx.viewport_w;
    s->viewport_h = ctx.viewport_h;
    s->set_audio(ctx.audio);
    if (ctx.run_stats) s->set_run_stats(*ctx.run_stats);
    return s;
});
```

---

## Resumo de impacto

```
NOVOS ficheiros (3):
  src/core/run_stats.hpp
  src/scenes/game_over_scene.hpp
  src/scenes/victory_scene.hpp

FICHEIROS EXISTENTES alterados (6):
  src/core/scene_context.hpp    +RunStats*, +DifficultyLevel
  src/core/register_scenes.cpp  +registo "game_over" e "victory"
  src/scenes/dungeon_scene.hpp  tracking stats + transição correta + dificuldade
  src/scenes/title_scene.hpp    seletor dificuldade + New Game+ indicator
  src/core/save_system.hpp      +victory_reached, +difficulty, +last_run_stats
  src/main.cpp                  instanciar RunStats + passar para ctx
```
