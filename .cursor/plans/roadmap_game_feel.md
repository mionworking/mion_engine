# Roadmap — Game Feel / Juice
> Transições de cena, dano flutuante, tint de status effects, hit flash, level-up FX, screen flash, boss intro.

---

## O que já existe

- `Camera2D::trigger_shake()` + `step_shake()` — shake com decay linear
- `SimpleParticleSystem` — burst de partículas em impacto/morte
- `CombatState` — hitstop (freeze frames partilhado)
- `Actor::was_alive` — detetar transição de morte
- `BitmapFont` — `draw_text` com cor por canal
- `StatusEffectState` — Poison, Slow, Stun presentes no Actor

---

## O que não existe

- Fade in/out entre cenas
- Números de dano / cura flutuantes
- Tint de cor nos actores consoante status effect activo
- Flash branco no actor ao receber hit
- Texto "+LEVEL UP!" ou "+3 ATK" flutuante
- Flash de ecrã vermelho ao player ser atingido
- Boss intro (title card)
- Sequência visual de morte do player (fade vermelho)

---

## Ficheiros a criar / tocar

| Ficheiro | Tipo |
|---|---|
| `src/core/scene_fader.hpp` | NOVO — fade in/out entre cenas |
| `src/systems/floating_text.hpp` | NOVO — FloatingText + FloatingTextSystem |
| `src/main.cpp` | integrar SceneFader nas transições |
| `src/scenes/dungeon_scene.hpp` | hit flash, tint de status, screen flash, boss intro, floating texts |
| `src/scenes/town_scene.hpp` | fade in/out (mesmo SceneFader) |

---

## 1. `src/core/scene_fader.hpp` — NOVO

```cpp
#pragma once
#include <SDL3/SDL.h>

namespace mion {

// Fade simples sobre o viewport inteiro.
// Renderizado POR ÚLTIMO em cada frame (sobre tudo).
struct SceneFader {
    enum class State { Idle, FadingOut, FadingIn };

    State  state    = State::Idle;
    float  duration = 0.35f;   // segundos por fase
    float  elapsed  = 0.0f;
    Uint8  alpha    = 0;       // 0=transparente 255=preto total

    // Chamar para iniciar fade-out (escurece). Quando termina, chamar on_complete.
    void start_fade_out(float dur = 0.35f) {
        state = State::FadingOut; duration = dur; elapsed = 0.0f; alpha = 0;
    }

    void start_fade_in(float dur = 0.35f) {
        state = State::FadingIn; duration = dur; elapsed = 0.0f; alpha = 255;
    }

    bool is_complete() const { return state == State::Idle; }
    bool is_fading_out() const { return state == State::FadingOut; }

    void tick(float dt) {
        if (state == State::Idle) return;
        elapsed += dt;
        float t = (duration > 0.0f) ? (elapsed / duration) : 1.0f;
        if (t >= 1.0f) { t = 1.0f; state = State::Idle; }
        alpha = (state == State::FadingOut || state == State::Idle)
              ? (Uint8)(t * 255.0f)
              : (Uint8)((1.0f - t) * 255.0f);
    }

    void render(SDL_Renderer* r, int viewport_w, int viewport_h) const {
        if (alpha == 0) return;
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0, 0, 0, alpha);
        SDL_FRect full = {0,0,(float)viewport_w,(float)viewport_h};
        SDL_RenderFillRect(r, &full);
    }
};

} // namespace mion
```

---

## 2. `src/main.cpp` — integrar SceneFader

```cpp
mion::SceneFader fader;
// ...

while (running) {
    // ... event loop ...

    // fixed_update loop:
    while (accumulator >= fixed_dt) {
        accumulator -= fixed_dt;
        fader.tick(fixed_dt);

        mion::InputState input = keyboard.read_state();
        const char* requested_next = scene_mgr.fixed_update(fixed_dt, input);

        std::string next_id = (requested_next && requested_next[0]) ? requested_next : "";
        if (!next_id.empty() && fader.is_complete()) {
            // Iniciar fade-out; ao completar, trocar de cena e fade-in
            fader.start_fade_out();
            _pending_scene = next_id;
            scene_mgr.clear_pending_transition();
        }
        if (fader.is_complete() && !_pending_scene.empty()) {
            auto next_scene = registry.create(_pending_scene, ctx);
            if (next_scene) scene_mgr.set(std::move(next_scene));
            _pending_scene.clear();
            fader.start_fade_in();
        }
    }

    scene_mgr.render(renderer);
    fader.render(renderer, logic_w, logic_h);  // por cima de tudo
    SDL_RenderPresent(renderer);
    // ...
}

// Campos locais a acrescentar em main():
std::string _pending_scene;
```

---

## 3. `src/systems/floating_text.hpp` — NOVO

```cpp
#pragma once
#include <string>
#include <vector>
#include <SDL3/SDL.h>
#include "../core/bitmap_font.hpp"

namespace mion {

struct FloatingText {
    float       x, y;
    float       vy          = -60.0f;  // sobe a 60 px/s
    float       lifetime    = 1.0f;
    float       elapsed     = 0.0f;
    std::string text;
    int         scale       = 2;
    SDL_Color   color       = {255, 255, 255, 255};

    bool is_dead() const { return elapsed >= lifetime; }
};

struct FloatingTextSystem {
    std::vector<FloatingText> texts;

    void spawn(float x, float y, const char* text,
               SDL_Color color = {255,255,255,255}, int scale = 2,
               float lifetime = 0.9f, float vy = -55.0f) {
        texts.push_back({x, y, vy, lifetime, 0.0f, text, scale, color});
    }

    void spawn_damage(float x, float y, int damage) {
        char buf[16];
        SDL_snprintf(buf, sizeof(buf), "-%d", damage);
        spawn(x, y - 20.0f, buf, {255, 80, 60, 255}, 2);
    }

    void spawn_heal(float x, float y, int amount) {
        char buf[16];
        SDL_snprintf(buf, sizeof(buf), "+%d", amount);
        spawn(x, y - 20.0f, buf, {80, 220, 100, 255}, 2);
    }

    void spawn_level_up(float x, float y) {
        spawn(x, y - 32.0f, "LEVEL UP!", {255, 215, 0, 255}, 3, 1.4f, -45.0f);
    }

    void spawn_upgrade(float x, float y, const char* text) {
        spawn(x, y - 48.0f, text, {180, 255, 180, 255}, 2, 1.2f, -40.0f);
    }

    void tick(float dt) {
        for (auto& ft : texts) {
            ft.elapsed += dt;
            ft.y += ft.vy * dt;
        }
        texts.erase(
            std::remove_if(texts.begin(), texts.end(),
                           [](const FloatingText& f){ return f.is_dead(); }),
            texts.end());
    }

    // world_to_screen: passar lambdas ou Camera2D diretamente
    void render(SDL_Renderer* r,
                const std::function<float(float)>& to_sx,
                const std::function<float(float)>& to_sy) const {
        for (const auto& ft : texts) {
            float alpha = 1.0f - (ft.elapsed / ft.lifetime);
            Uint8 a = (Uint8)(alpha * 255.0f);
            draw_text(r,
                      to_sx(ft.x) - text_width(ft.text.c_str(), ft.scale) * 0.5f,
                      to_sy(ft.y),
                      ft.text.c_str(), ft.scale,
                      ft.color.r, ft.color.g, ft.color.b, a);
        }
    }
};

} // namespace mion
```

---

## 4. `src/scenes/dungeon_scene.hpp` — todas as alterações de game feel

### 4.1 Campos privados a acrescentar:

```cpp
FloatingTextSystem  _floating_texts;
SceneFader          _fader;

// Screen flash (vermelho ao tomar hit, branco no boss phase 2)
float     _screen_flash_timer  = 0.0f;
SDL_Color _screen_flash_color  = {255, 0, 0, 0};

// Hit flash por actor: acrescentar ao Actor (ou mapa actor→timer aqui)
// Mais limpo: campo hit_flash_timer no Actor
```

### 4.2 No `Actor` (`src/entities/actor.hpp`) — acrescentar:
```cpp
float hit_flash_timer = 0.0f;  // segundos restantes de flash branco
```

### 4.3 Quando actor recebe dano (em `MeleeCombatSystem` ou inline):
```cpp
// Após aplicar dano:
target.hit_flash_timer = 0.15f;
_floating_texts.spawn_damage(target.transform.x, target.transform.y, damage_applied);
```

### 4.4 Quando player recebe dano:
```cpp
// Screen flash vermelho
_screen_flash_timer = 0.25f;
_screen_flash_color = {180, 20, 20, 120};
```

### 4.5 Quando player level-up:
```cpp
_floating_texts.spawn_level_up(_player.transform.x, _player.transform.y);
if (_audio) _audio->play_sfx(SoundId::UiConfirm, 0.8f);
```

### 4.6 Quando upgrade escolhido:
```cpp
// pick_upgrade_damage():
_floating_texts.spawn_upgrade(_player.transform.x, _player.transform.y, "+3 ATK");
// pick_upgrade_hp():
_floating_texts.spawn_upgrade(_player.transform.x, _player.transform.y, "+15 HP");
```

### 4.7 Em `fixed_update()` — tick de timers:
```cpp
// Hit flash timers
_player.hit_flash_timer = std::max(0.0f, _player.hit_flash_timer - dt);
for (auto& e : _enemies)
    e.hit_flash_timer = std::max(0.0f, e.hit_flash_timer - dt);

// Screen flash decay
if (_screen_flash_timer > 0.0f) {
    _screen_flash_timer -= dt;
    if (_screen_flash_timer < 0.0f) _screen_flash_timer = 0.0f;
}

// FloatingTextSystem tick
_floating_texts.tick(dt);
```

### 4.8 Em `render()` — tint de status + hit flash por actor:

```cpp
// Ao renderizar cada actor (player + inimigos):
auto render_actor_colored = [&](const Actor& a, float sx, float sy) {
    // Prioridade de tint: hit_flash > status effect
    if (a.hit_flash_timer > 0.0f) {
        // Flash branco: render retângulo branco semi-transparente sobre o actor
        SDL_SetRenderDrawColor(r, 255, 255, 255, 180);
        // ... render rect ...
    } else if (a.status_effects.has(StatusEffectType::Poison)) {
        // Tint verde: se tem sprite → SDL_SetTextureColorMod(tex, 100, 255, 100)
        // fallback: rect verde semi-transparente
        SDL_SetRenderDrawColor(r, 80, 220, 80, 80);
    } else if (a.status_effects.has(StatusEffectType::Slow)) {
        SDL_SetRenderDrawColor(r, 80, 120, 220, 80);  // azul
    } else if (a.status_effects.has(StatusEffectType::Stun)) {
        SDL_SetRenderDrawColor(r, 240, 220, 60, 80);  // amarelo
    }
    // render overlay rect após render normal do actor
};
```

### 4.9 Em `render()` — screen flash:
```cpp
if (_screen_flash_timer > 0.0f) {
    float alpha_f = (_screen_flash_timer / 0.25f); // 0.25s = duração máx
    Uint8 a = (Uint8)(alpha_f * _screen_flash_color.a);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, _screen_flash_color.r,
                               _screen_flash_color.g,
                               _screen_flash_color.b, a);
    SDL_FRect full = {0,0,(float)viewport_w,(float)viewport_h};
    SDL_RenderFillRect(r, &full);
}
```

### 4.10 Em `render()` — floating texts (após tudo):
```cpp
_floating_texts.render(r,
    [&](float wx){ return _camera.world_to_screen_x(wx); },
    [&](float wy){ return _camera.world_to_screen_y(wy); });
```

### 4.11 Boss intro sequence — campos + lógica:
```cpp
// Campos:
bool  _boss_intro_active = false;
float _boss_intro_timer  = 0.0f;
static constexpr float kBossIntroDuration = 1.8f;

// Em _enter_room() quando _room_index == 3:
_boss_intro_active = true;
_boss_intro_timer  = 0.0f;
// Bloquear input do player durante a intro

// Em fixed_update(), antes de processar input:
if (_boss_intro_active) {
    _boss_intro_timer += dt;
    if (_boss_intro_timer >= kBossIntroDuration)
        _boss_intro_active = false;
    return; // bloqueia toda a gameplay durante a intro
}

// Em render(), quando _boss_intro_active:
if (_boss_intro_active) {
    float t = _boss_intro_timer / kBossIntroDuration;
    // Fade in do título nos primeiros 40%, estático até 80%, fade out até fim
    float alpha_f = (t < 0.4f) ? (t / 0.4f)
                  : (t < 0.8f) ? 1.0f
                  : (1.0f - (t - 0.8f) / 0.2f);
    Uint8 a = (Uint8)(alpha_f * 255.0f);

    // Overlay escuro
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0,0,0, (Uint8)(alpha_f * 160.0f));
    SDL_FRect full={0,0,(float)viewport_w,(float)viewport_h};
    SDL_RenderFillRect(r, &full);

    // Nome do boss
    const char* boss_name = "GRIMJAW";
    int sc = 5;
    float bx = viewport_w*0.5f - text_width(boss_name,sc)*0.5f;
    float by = viewport_h*0.42f;
    draw_text(r, bx, by, boss_name, sc, 220, 60, 40, a);

    // Subtítulo
    const char* sub = "Guardian of the Third Depth";
    int sc2 = 2;
    float sx2 = viewport_w*0.5f - text_width(sub,sc2)*0.5f;
    draw_text(r, sx2, by + sc*12 + 8, sub, sc2, 180, 140, 120, (Uint8)(alpha_f*200));
}
```

### 4.12 Morte do player — fade vermelho para preto:
```cpp
// Quando player morre (_player.is_alive == false && _player.was_alive == true):
_screen_flash_timer = 1.5f;
_screen_flash_color = {120, 10, 10, 200};
_fader.start_fade_out(1.5f);
// Após fader completar: _next = "game_over"
```

---

## Resumo de impacto

```
NOVOS ficheiros (2):
  src/core/scene_fader.hpp
  src/systems/floating_text.hpp

FICHEIROS EXISTENTES alterados (3):
  src/entities/actor.hpp           +hit_flash_timer
  src/main.cpp                     integrar SceneFader nas transições de cena
  src/scenes/dungeon_scene.hpp     hit flash, tint de status, screen flash,
                                   boss intro, floating texts, fader de morte
```
