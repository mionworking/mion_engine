# Roadmap — Distribution / Packaging
> Rebinding de teclas, suporte a gamepad, build Windows, empacotamento de assets, créditos, localização.

---

## O que já existe

- Build Linux + `ctest` CI em `.github/workflows/cmake.yml`
- `make release` via `CMakeLists.txt`
- `SDL_GetBasePath()` para resolução de assets
- `config.ini` com `[window]`, `[audio]`, `[ui]`
- `IInputSource` / `KeyboardInputSource` — interface abstrata já permite trocar a fonte de input
- `InputState` com todos os botões do jogo

---

## O que não existe

- Rebinding de teclas
- Suporte a gamepad (SDL3 `SDL_Gamepad` API)
- Build Windows no CI
- Empacotamento real de assets (`cmake --install`)
- Ecrã de créditos
- Localização (strings em código duro em inglês/pt)

---

## Ficheiros a criar / tocar

| Ficheiro | Tipo |
|---|---|
| `src/core/keybind_config.hpp` | NOVO — carrega keybinds de config.ini |
| `src/core/input.hpp` | +GamepadInputSource |
| `src/core/input.cpp` | +GamepadInputSource::read_state() |
| `src/core/locale.hpp` | NOVO — sistema de localização |
| `data/locale_en.ini` | NOVO |
| `data/locale_ptbr.ini` | NOVO |
| `src/scenes/credits_scene.hpp` | NOVO |
| `src/core/register_scenes.cpp` | +registo "credits" |
| `src/scenes/title_scene.hpp` | +opção "CREDITS" no menu |
| `CMakeLists.txt` | +regras de install + opção Windows |
| `.github/workflows/cmake.yml` | +job windows |
| `config.ini` | +secção [keybinds] |
| `src/main.cpp` | gamepad auto-detect + locale init |

---

## 1. `src/core/keybind_config.hpp` — NOVO

```cpp
#pragma once
#include <SDL3/SDL.h>
#include "ini_loader.hpp"

namespace mion {

// Mapeamento de ações → scancodes (carregado do config.ini [keybinds])
struct KeybindConfig {
    SDL_Scancode attack      = SDL_SCANCODE_Z;
    SDL_Scancode attack_alt  = SDL_SCANCODE_SPACE;
    SDL_Scancode ranged      = SDL_SCANCODE_X;
    SDL_Scancode dash        = SDL_SCANCODE_LSHIFT;
    SDL_Scancode parry       = SDL_SCANCODE_C;
    SDL_Scancode spell_1     = SDL_SCANCODE_Q;
    SDL_Scancode spell_2     = SDL_SCANCODE_E;
    SDL_Scancode spell_3     = SDL_SCANCODE_R;
    SDL_Scancode spell_4     = SDL_SCANCODE_F;
    SDL_Scancode confirm     = SDL_SCANCODE_RETURN;
    SDL_Scancode pause       = SDL_SCANCODE_ESCAPE;
    SDL_Scancode skill_tree  = SDL_SCANCODE_TAB;
    SDL_Scancode erase_save  = SDL_SCANCODE_N;
    SDL_Scancode upgrade_1   = SDL_SCANCODE_1;
    SDL_Scancode upgrade_2   = SDL_SCANCODE_2;
    SDL_Scancode upgrade_3   = SDL_SCANCODE_3;
    SDL_Scancode talent_1    = SDL_SCANCODE_4;
    SDL_Scancode talent_2    = SDL_SCANCODE_5;
    SDL_Scancode talent_3    = SDL_SCANCODE_6;
};

// Converte string de nome de tecla para scancode via SDL3
inline SDL_Scancode scancode_from_name(const std::string& name) {
    if (name.empty()) return SDL_SCANCODE_UNKNOWN;
    SDL_Scancode sc = SDL_GetScancodeFromName(name.c_str());
    return (sc != SDL_SCANCODE_UNKNOWN) ? sc : SDL_SCANCODE_UNKNOWN;
}

// Carrega [keybinds] do IniData; fallback para defaults se chave ausente
inline KeybindConfig load_keybinds(const IniData& d) {
    KeybindConfig kb{};
    auto get = [&](const std::string& key, SDL_Scancode def) -> SDL_Scancode {
        auto si = d.sections.find("keybinds");
        if (si == d.sections.end()) return def;
        auto ki = si->second.find(key);
        if (ki == si->second.end()) return def;
        SDL_Scancode sc = scancode_from_name(ki->second);
        return (sc != SDL_SCANCODE_UNKNOWN) ? sc : def;
    };
    kb.attack     = get("attack",     kb.attack);
    kb.attack_alt = get("attack_alt", kb.attack_alt);
    kb.ranged     = get("ranged",     kb.ranged);
    kb.dash       = get("dash",       kb.dash);
    kb.parry      = get("parry",      kb.parry);
    kb.spell_1    = get("spell_1",    kb.spell_1);
    kb.spell_2    = get("spell_2",    kb.spell_2);
    kb.spell_3    = get("spell_3",    kb.spell_3);
    kb.spell_4    = get("spell_4",    kb.spell_4);
    kb.confirm    = get("confirm",    kb.confirm);
    kb.pause      = get("pause",      kb.pause);
    kb.skill_tree = get("skill_tree", kb.skill_tree);
    return kb;
}

} // namespace mion
```

### `config.ini` — acrescentar secção [keybinds]:
```ini
[keybinds]
# Nomes de teclas conforme SDL_GetScancodeFromName (inglês, case-insensitive).
# Descomente para customizar.
# attack=Z
# attack_alt=Space
# ranged=X
# dash=Left Shift
# parry=C
# spell_1=Q
# spell_2=E
# spell_3=R
# spell_4=F
# confirm=Return
# pause=Escape
# skill_tree=Tab
```

### `src/core/input.hpp` — `KeyboardInputSource` lê `KeybindConfig`:

```cpp
class KeyboardInputSource : public IInputSource {
public:
    explicit KeyboardInputSource(const KeybindConfig& kb = {}) : _kb(kb) {}
    InputState read_state() const override;
private:
    KeybindConfig _kb;
};
```

`read_state()` em `src/core/input.cpp` passa a usar `_kb.attack`, `_kb.ranged`, etc.
em vez dos scancodes hardcoded.

---

## 2. Gamepad — `src/core/input.hpp` + `src/core/input.cpp`

### `src/core/input.hpp` — nova classe:

```cpp
class GamepadInputSource : public IInputSource {
public:
    GamepadInputSource();
    ~GamepadInputSource();

    // Retorna true se um gamepad estiver conectado e operacional
    bool is_connected() const { return _gamepad != nullptr; }

    // Tenta conectar ao primeiro gamepad disponível
    void try_connect();

    InputState read_state() const override;

private:
    SDL_Gamepad* _gamepad = nullptr;
};
```

### Mapeamento de botões (SDL3 `SDL_GamepadButton`):
```
Move     → SDL_GAMEPAD_AXIS_LEFTX / LEFTY (analógico, threshold 0.25)
Attack   → SDL_GAMEPAD_BUTTON_SOUTH (A/Cruz)
Ranged   → SDL_GAMEPAD_BUTTON_WEST  (X/Quadrado)
Dash     → SDL_GAMEPAD_BUTTON_EAST  (B/Círculo)
Parry    → SDL_GAMEPAD_BUTTON_LEFT_SHOULDER (LB/L1)
Spell_1  → SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER (RB/R1)
Spell_2  → SDL_GAMEPAD_AXIS_RIGHTTRIGGER (threshold 0.5)
Spell_3  → DPAD_LEFT
Spell_4  → DPAD_UP
Confirm  → SDL_GAMEPAD_BUTTON_SOUTH
Pause    → SDL_GAMEPAD_BUTTON_START
UI up    → SDL_GAMEPAD_BUTTON_DPAD_UP
UI down  → SDL_GAMEPAD_BUTTON_DPAD_DOWN
```

### `src/main.cpp` — auto-detect gamepad:
```cpp
// Após SDL_Init:
mion::KeyboardInputSource keyboard(keybinds);
mion::GamepadInputSource  gamepad;
gamepad.try_connect();

// No event loop:
if (event.type == SDL_EVENT_GAMEPAD_ADDED)
    gamepad.try_connect();
if (event.type == SDL_EVENT_GAMEPAD_REMOVED && !gamepad.is_connected())
    SDL_Log("Gamepad desconectado — voltando ao teclado");

// Na leitura de input:
mion::InputState input = gamepad.is_connected()
                       ? gamepad.read_state()
                       : keyboard.read_state();
```

---

## 3. `src/core/locale.hpp` — NOVO

```cpp
#pragma once
#include <string>
#include <unordered_map>
#include "ini_loader.hpp"

namespace mion {

struct LocaleSystem {
    std::unordered_map<std::string, std::string> strings;

    void load(const std::string& path) {
        IniData d = ini_load(path);
        auto si = d.sections.find("strings");
        if (si == d.sections.end()) return;
        for (const auto& [k, v] : si->second)
            strings[k] = v;
    }

    // Retorna a string localizada ou a própria chave se não encontrar
    const char* get(const std::string& key) const {
        auto it = strings.find(key);
        return (it != strings.end()) ? it->second.c_str() : key.c_str();
    }
};

// Instância global — inicializada em main.cpp antes de qualquer cena
inline LocaleSystem g_locale;

// Helper — substituir strings hardcoded nas cenas
inline const char* L(const std::string& key) { return g_locale.get(key); }

} // namespace mion
```

### `data/locale_en.ini`:
```ini
[strings]
menu_play=SPACE OR Z - PLAY
menu_erase_save=N - ERASE SAVE (NEW RUN)
menu_credits=CREDITS
menu_resume=RESUME
menu_skill_tree=SKILL TREE
menu_settings=SETTINGS
menu_quit_menu=QUIT TO MENU
menu_quit=QUIT
menu_retry=RETRY (NEW RUN)
menu_main_menu=MAIN MENU
menu_new_game_plus=NEW GAME+ AVAILABLE
diff_easy=EASY
diff_normal=NORMAL
diff_hard=HARD
ui_level=Lv
ui_game_over=GAME OVER
ui_victory=VICTORY
ui_paused=PAUSED
ui_enter_continue=ENTER - continue
npc_interact_hint=ENTER - talk
boss_grimjaw_title=GRIMJAW
boss_grimjaw_subtitle=Guardian of the Third Depth
stat_rooms=Rooms cleared
stat_enemies=Enemies killed
stat_gold=Gold collected
stat_damage=Damage taken
stat_time=Time
```

### `data/locale_ptbr.ini`:
```ini
[strings]
menu_play=ESPACO OU Z - JOGAR
menu_erase_save=N - APAGAR SAVE (NOVO RUN)
menu_credits=CREDITOS
menu_resume=CONTINUAR
menu_skill_tree=ARVORE DE HABILIDADES
menu_settings=CONFIGURACOES
menu_quit_menu=VOLTAR AO MENU
menu_quit=SAIR
menu_retry=TENTAR NOVAMENTE (NOVO RUN)
menu_main_menu=MENU PRINCIPAL
menu_new_game_plus=NOVA JORNADA+ DISPONIVEL
diff_easy=FACIL
diff_normal=NORMAL
diff_hard=DIFICIL
ui_level=Nv
ui_game_over=FIM DE JOGO
ui_victory=VITORIA
ui_paused=PAUSADO
ui_enter_continue=ENTER - continuar
npc_interact_hint=ENTER - falar
boss_grimjaw_title=GRIMJAW
boss_grimjaw_subtitle=Guardiao do Terceiro Nivel
stat_rooms=Salas completadas
stat_enemies=Inimigos eliminados
stat_gold=Ouro coletado
stat_damage=Dano recebido
stat_time=Tempo
```

### `config.ini` — acrescentar à secção [ui]:
```ini
[ui]
# autosave_indicator=0
# language=en   ; en | ptbr
```

### `src/main.cpp` — carregar locale:
```cpp
// Após load_config():
std::string lang = /* d.get_str("ui","language","en") */ "en";
std::string locale_path = resolve_data_path("locale_" + lang + ".ini");
mion::g_locale.load(locale_path);
```

---

## 4. `src/scenes/credits_scene.hpp` — NOVO

```cpp
class CreditsScene : public IScene {
public:
    int viewport_w = 1280, viewport_h = 720;
    void set_audio(AudioSystem* a) { _audio = a; }

    void enter() override {
        _scroll_y = (float)viewport_h;
        _next.clear();
        _prev_cancel = false;
    }

    void fixed_update(float dt, const InputState& input) override {
        _scroll_y -= 40.0f * dt; // 40px/s de scroll
        if (input.cancel_pressed && !_prev_cancel)
            _next = "title";
        _prev_cancel = input.cancel_pressed;
        // Auto-retorno quando todo o texto passou
        float total_h = (float)(_credits_lines.size() * 28);
        if (_scroll_y + total_h < 0) _next = "title";
    }

    void render(SDL_Renderer* r) override {
        SDL_SetRenderDrawColor(r, 5, 5, 15, 255);
        SDL_RenderClear(r);

        float y = _scroll_y;
        for (const auto& line : _credits_lines) {
            if (y > -20.0f && y < viewport_h + 20.0f) {
                int sc = line.is_header ? 3 : 2;
                SDL_Color c = line.is_header
                    ? SDL_Color{255,215,0,255} : SDL_Color{200,200,190,255};
                float x = viewport_w*0.5f - text_width(line.text.c_str(), sc)*0.5f;
                draw_text(r, x, y, line.text.c_str(), sc, c.r,c.g,c.b,255);
            }
            y += line.is_header ? 48 : 28;
        }

        // Hint ESC
        draw_text(r, 12, viewport_h - 24, "ESC - back", 2, 120,120,110,255);
    }

    const char* next_scene() const override { return _next.c_str(); }

private:
    struct CreditLine { std::string text; bool is_header = false; };

    // Conteúdo — personalizar com nomes reais
    std::vector<CreditLine> _credits_lines = {
        {"MION ENGINE", true},
        {""},
        {"DESIGN & PROGRAMMING", true},
        {"[nome do dev]", false},
        {""},
        {"TOOLS", true},
        {"SDL3  |  stb_image  |  C++17", false},
        {""},
        {"PLACEHOLDER ART", true},
        {"Puny Characters (CC0)", false},
        {""},
        {"SPECIAL THANKS", true},
        {"The open source community", false},
        {""},
        {""},
        {"Made with mion_engine", false},
    };

    float        _scroll_y = 0.0f;
    std::string  _next;
    bool         _prev_cancel = false;
    AudioSystem* _audio = nullptr;
};
```

---

## 5. `CMakeLists.txt` — install + packaging

```cmake
# Acrescentar após o target mion_engine:

install(TARGETS mion_engine
        RUNTIME DESTINATION bin)

install(DIRECTORY assets/
        DESTINATION bin/assets)

install(DIRECTORY data/
        DESTINATION bin/data)

install(FILES config.ini
        DESTINATION bin)

# Opção de build de pacote (CPack)
include(CPack)
set(CPACK_PACKAGE_NAME "mion_engine")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_GENERATOR "TGZ;ZIP")  # .tar.gz para Linux, .zip para Windows
```

### Uso:
```bash
cmake --build build --target install  # instala em build/install/
cpack --config build/CPackConfig.cmake  # gera mion_engine-0.1.0-Linux.tar.gz
```

---

## 6. `.github/workflows/cmake.yml` — job Windows

```yaml
windows:
  runs-on: windows-latest
  steps:
    - uses: actions/checkout@v4

    - name: Install SDL3 (vcpkg)
      run: |
        vcpkg install sdl3:x64-windows
        vcpkg integrate install

    - name: Configure
      run: cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
                 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

    - name: Build
      run: cmake --build build --config Release

    - name: Test
      run: ctest --test-dir build --output-on-failure -C Release
```

### Ajustes de compatibilidade Windows em código:
- `engine_paths.hpp` — separadores de path: usar `SDL_GetBasePath()` que já devolve `/` no Windows via SDL3
- `save_system.hpp` — `std::getenv("USERPROFILE")` em vez de `HOME` no Windows
  → abstrair em `engine_paths.hpp` com `get_save_dir()` que usa `SDL_GetPrefPath("mion","mion_engine")`

---

## Resumo de impacto

```
NOVOS ficheiros (5):
  src/core/keybind_config.hpp
  src/core/locale.hpp
  src/scenes/credits_scene.hpp
  data/locale_en.ini
  data/locale_ptbr.ini

FICHEIROS EXISTENTES alterados (8):
  src/core/input.hpp             +GamepadInputSource; KeyboardInputSource aceita KeybindConfig
  src/core/input.cpp             GamepadInputSource::read_state() + teclado usa keybinds
  src/core/register_scenes.cpp   +registo "credits"
  src/scenes/title_scene.hpp     +opção "CREDITS" no menu principal
  src/main.cpp                   gamepad auto-detect + locale init + KeybindConfig
  CMakeLists.txt                 +install rules + CPack
  .github/workflows/cmake.yml    +job windows
  config.ini                     +[keybinds] + language em [ui]
```
