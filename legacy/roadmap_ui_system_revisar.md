# Roadmap — UI System
> Pause menu, Skill Tree screen, HUD melhorado, navegação de menu.

---

## O que já existe

- `BitmapFont` + `draw_text` / `text_width` — fonte 5×7 pixels, escalável
- `DialogueSystem` — barra de diálogo com wrap, speaker, hint piscante
- `InputState` — `confirm_pressed` (Enter), `upgrade_1/2/3`, `talent_1/2/3`
- Escape atualmente **fecha o jogo** diretamente em `main.cpp`

---

## O que não existe

- Nenhum ecrã de pausa
- Nenhum ecrã de skill tree
- Nenhum painel / botão reutilizável
- HUD sem gold, sem slots de skill ativos, sem barra de XP
- Escape não pausa — mata o processo

---

## Ficheiros a criar / tocar

| Ficheiro | Tipo |
|---|---|
| `src/core/ui.hpp` | NOVO — primitivas de UI |
| `src/core/input.hpp` | +pause_pressed, ui_up/down, skill_tree_pressed |
| `src/core/input.cpp` | mapear novas teclas |
| `src/main.cpp` | Escape → pausa (não quit); Alt+F4 → quit |
| `src/scenes/dungeon_scene.hpp` | +_paused, overlay de pausa, overlay de skill tree |
| `src/scenes/town_scene.hpp` | +_paused, overlay de pausa (mesmo sistema) |

---

## 1. `src/core/ui.hpp` — NOVO

Primitivas reutilizáveis em todas as cenas. Sem estado global — cada cena instancia o que precisa.

```cpp
namespace mion::ui {

// Painel retangular com fundo e borda opcionais
struct Panel {
    SDL_FRect   rect    = {};
    SDL_Color   bg      = {10, 8, 20, 220};
    SDL_Color   border  = {180, 160, 100, 255};
    int         border_px = 2;

    void render(SDL_Renderer* r) const;
};

// Texto posicionado com escala e cor
struct Label {
    float       x = 0.0f, y = 0.0f;
    std::string text;
    int         scale = 2;
    SDL_Color   color = {230, 230, 210, 255};

    void render(SDL_Renderer* r) const;
};

// Lista navegável de strings (up/down + highlight na selecionada)
struct List {
    std::vector<std::string> items;
    int   selected      = 0;
    int   item_h_px     = 28;   // altura em pixels por item (inclui padding)
    int   text_scale    = 2;
    SDL_Color color_normal   = {200, 200, 190, 255};
    SDL_Color color_selected = {255, 220,  60, 255};
    SDL_Color color_disabled = {100,  95,  85, 255};
    std::vector<bool> disabled;  // por índice; disabled[i]=true → não selecionável

    void nav_up();    // decrementa selected, salta disabled, wraps
    void nav_down();  // incrementa selected, salta disabled, wraps
    void render(SDL_Renderer* r, float x, float y) const;
};

// Barra de progresso horizontal (HP, XP, mana, stamina)
struct ProgressBar {
    SDL_FRect   rect      = {};
    float       value     = 1.0f;   // 0.0–1.0
    SDL_Color   color_bg  = { 40,  30,  30, 200};
    SDL_Color   color_fill= {200,  60,  60, 255};
    SDL_Color   color_border={120, 100,  80, 180};

    void render(SDL_Renderer* r) const;
};

} // namespace mion::ui
```

---

## 2. `src/core/input.hpp` — extensão

Acrescentar ao `InputState`:

```cpp
// Navegação de menu (edge-triggered na cena, não aqui)
bool pause_pressed      = false;  // Escape
bool ui_up_pressed      = false;  // seta cima (separado de move_y)
bool ui_down_pressed    = false;  // seta baixo
bool skill_tree_pressed = false;  // Tab
bool cancel_pressed     = false;  // Escape em contexto de menu (fechar overlay)
```

`pause_pressed` e `cancel_pressed` mapeiam ambos para Escape — interpretados
diferentemente consoante `_paused`:
- não pausado → `pause_pressed = true`
- pausado / overlay aberto → `cancel_pressed = true`

Acrescentar ao `KeyboardInputSource::read_state()` em `src/core/input.cpp`:
```
SDL_SCANCODE_ESCAPE      → pause_pressed = true (sempre)
SDL_SCANCODE_UP          → ui_up_pressed
SDL_SCANCODE_DOWN        → ui_down_pressed
SDL_SCANCODE_TAB         → skill_tree_pressed
```

---

## 3. `src/main.cpp` — alterar gestão de Escape

```cpp
// Remover:
if (event.type == SDL_EVENT_KEY_DOWN
    && event.key.scancode == SDL_SCANCODE_ESCAPE) running = false;

// Substituir por:
if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) running = false;
// Alt+F4 já gera SDL_EVENT_WINDOW_CLOSE_REQUESTED no SDL3.
// Escape agora vai para InputState::pause_pressed → tratado pela cena.
```

---

## 4. Pause Menu — `DungeonScene` e `TownScene`

### Campos privados a acrescentar (ambas as cenas):
```cpp
bool          _paused          = false;
ui::List      _pause_list;     // items: "RESUME", "SKILL TREE", "SETTINGS", "QUIT TO MENU"
bool          _prev_pause      = false;
bool          _prev_ui_up      = false;
bool          _prev_ui_down    = false;
```

### `enter()`:
```cpp
_pause_list.items    = {"RESUME", "SKILL TREE", "SETTINGS", "QUIT TO MENU"};
_pause_list.selected = 0;
// "SKILL TREE" desativado na TownScene se não relevante
```

### `fixed_update()` — bloco de pausa (ANTES de qualquer lógica de gameplay):
```cpp
// Edge de pause
const bool pause_edge = input.pause_pressed && !_prev_pause;
_prev_pause = input.pause_pressed;

if (pause_edge && !_paused && !_dialogue.is_active()) {
    _paused = true;
    _pause_list.selected = 0;
    return;  // não processa mais nada este frame
}

if (_paused) {
    const bool up_edge   = input.ui_up_pressed   && !_prev_ui_up;
    const bool down_edge = input.ui_down_pressed  && !_prev_ui_down;
    _prev_ui_up   = input.ui_up_pressed;
    _prev_ui_down = input.ui_down_pressed;

    if (up_edge)   _pause_list.nav_up();
    if (down_edge) _pause_list.nav_down();

    if (input.confirm_pressed && !_prev_confirm) {
        switch (_pause_list.selected) {
            case 0: _paused = false; break;                 // RESUME
            case 1: _skill_tree_open = true; break;         // SKILL TREE
            case 2: /* abrir settings overlay */ break;
            case 3: _next = "title"; _paused = false; break;// QUIT TO MENU
        }
    }
    if (input.cancel_pressed) _paused = false;
    return;  // não processa gameplay enquanto pausado
}
```

### `render()` — overlay de pausa (após render normal):
```cpp
if (_paused) {
    // Escurece o fundo
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 160);
    SDL_FRect full = {0, 0, (float)viewport_w, (float)viewport_h};
    SDL_RenderFillRect(r, &full);

    // Painel central
    ui::Panel panel;
    panel.rect = { viewport_w*0.35f, viewport_h*0.25f,
                   viewport_w*0.30f, viewport_h*0.50f };
    panel.render(r);

    // Título
    draw_text(r, panel.rect.x + 20, panel.rect.y + 16, "PAUSED", 2, 255, 220, 60, 255);

    // Lista de opções
    _pause_list.render(r, panel.rect.x + 20, panel.rect.y + 60);
}
```

---

## 5. Skill Tree Screen — overlay em `DungeonScene`

### Campos privados:
```cpp
bool     _skill_tree_open   = false;
int      _st_selected_col   = 0;   // 0=Melee 1=Ranged 2=Magic
int      _st_selected_row   = 0;   // índice dentro da coluna
```

### `fixed_update()` — bloco de skill tree (após bloco de pausa):
```cpp
const bool tab_edge = input.skill_tree_pressed && !_prev_tab;
_prev_tab = input.skill_tree_pressed;
if (tab_edge && !_paused && !_dialogue.is_active())
    _skill_tree_open = !_skill_tree_open;

if (_skill_tree_open) {
    // Navegar colunas: ui_left/right (acrescentar à InputState)
    // Navegar linhas: ui_up/down
    // confirm_pressed → try_spend(talento selecionado)
    // cancel_pressed → fechar
    return; // bloqueia gameplay enquanto skill tree aberta
}
```

### `render()` — overlay de skill tree (3 colunas):
```cpp
if (_skill_tree_open) {
    // fundo escuro
    // 3 painéis: Melee | Ranged | Magic
    // Por painel: título, lista de nós com nível/max e custo
    // Nó selecionado: borda amarela + hint "ENTER - gastar ponto"
    // Topo direito: "Pontos disponíveis: N"
    // Nó bloqueado (parent_min_level não atingido): cor cinzenta
}
```

---

## 6. HUD melhorado — `DungeonScene::render()`

Acrescentar às barras que já existem:

```cpp
// Barra de XP (rodapé, fina)
ui::ProgressBar xp_bar;
xp_bar.rect      = {0, (float)viewport_h - 6, (float)viewport_w, 6};
xp_bar.value     = (float)_player.progression.xp / _player.progression.xp_to_next;
xp_bar.color_fill= {80, 160, 255, 255};
xp_bar.render(r);

// Gold (topo direito)
char gold_buf[32];
SDL_snprintf(gold_buf, sizeof(gold_buf), "G %d", _player.gold);
draw_text(r, viewport_w - text_width(gold_buf, 2) - 12, 12, gold_buf, 2, 255, 215, 0, 255);

// Slots de skill ativos (centro inferior)
// Q / E / R / F  → mostra nome abreviado do spell bound + cooldown overlay
// (implementar após skill tree roadmap estar feito)

// Nível do player (junto ao HP)
char lvl_buf[16];
SDL_snprintf(lvl_buf, sizeof(lvl_buf), "Lv%d", _player.progression.level);
draw_text(r, 8, 48, lvl_buf, 2, 200, 200, 180, 255);
```

---

## Resumo de impacto

```
NOVOS ficheiros (1):
  src/core/ui.hpp

FICHEIROS EXISTENTES alterados (5):
  src/core/input.hpp          +5 campos InputState
  src/core/input.cpp          +mapeamento teclas novas
  src/main.cpp                Escape não fecha mais o processo
  src/scenes/dungeon_scene.hpp +pause overlay + skill tree overlay + HUD gold/XP
  src/scenes/town_scene.hpp   +pause overlay
```
