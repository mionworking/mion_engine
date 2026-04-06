# Roadmap — Cidade, NPCs e Quest

> **Arquivado (2026-04-04):** blueprint pré-implementação. Cidade, loja, NPCs, `town_dialogues.ini`, quest Grimjaw **já no código** (`town_scene`, `shop_system`, `town_npc_*`).

> Documento de referência antes da implementação.

---

## Fluxo de jogo alvo

```
TitleScene
    ↓ PLAY
TownScene  ←─────────────────────────────────────┐
    │ (porta de entrada da dungeon)               │
    ↓                                             │
DungeonScene (salas 0–3)                          │
    │ Grimjaw morto → quest marcada Completed     │
    │ porta esquerda → "town"                     │
    └─────────────────────────────────────────────┘
```

---

## Ficheiros a criar / tocar

| Ficheiro | Tipo |
|---|---|
| `src/core/quest_state.hpp` | NOVO |
| `src/entities/npc.hpp` | NOVO |
| `src/entities/shop.hpp` | NOVO |
| `src/systems/shop_system.hpp` | NOVO |
| `src/scenes/town_scene.hpp` | NOVO |
| `data/town_dialogues.ini` | NOVO |
| `src/core/register_scenes.cpp` | +registo "town" |
| `src/scenes/title_scene.hpp` | "dungeon" → "town" como cena inicial |
| `src/scenes/dungeon_scene.hpp` | door target + quest completion + gold drop |
| `src/core/save_system.hpp` | +gold, +quest_status[] |
| `src/entities/actor.hpp` | +gold |
| `src/entities/enemy_type.hpp` | +gold_drop_min/max |
| `src/entities/ground_item.hpp` | +GroundItemType::Gold + gold_value |
| `src/systems/drop_system.hpp` | rola gold separado do item |

---

## 1. `src/core/quest_state.hpp` — NOVO

```cpp
enum class QuestId { DefeatGrimjaw, Count };

enum class QuestStatus { NotStarted, InProgress, Completed, Rewarded };

struct QuestState {
    QuestStatus status[static_cast<int>(QuestId::Count)] = {};

    QuestStatus get(QuestId id) const;
    void        set(QuestId id, QuestStatus s);
    bool        is(QuestId id, QuestStatus s) const;
};
```

Integrado no `SaveData`. Serializado como `quest_N=0/1/2/3`.

---

## 2. `src/entities/npc.hpp` — NOVO

```cpp
enum class NpcType { QuestGiver, Merchant, Generic };

struct NpcEntity {
    float       x, y;
    std::string name;
    NpcType     type            = NpcType::Generic;
    float       interact_radius = 48.0f;
    SDL_Color   portrait_color  = {180, 200, 160, 255};

    // IDs de diálogo por estado (vazio = não usa)
    std::string dialogue_default;       // sem quest activa / quest rewarded
    std::string dialogue_quest_active;  // quest InProgress
    std::string dialogue_quest_done;    // quest Completed (antes do reward)
};
```

---

## 3. `src/entities/shop.hpp` — NOVO

```cpp
enum class ShopItemType { HpPotion, StaminaPotion, AttackUpgrade, ManaUpgrade };

struct ShopItem {
    std::string  display_name;
    ShopItemType type;
    int          gold_cost;
    int          value;  // quantidade curada / bónus permanente
};

struct ShopInventory {
    std::vector<ShopItem> items;
    int                   selected_index = 0;
};
```

Inventário inicial do Forge (Ferreiro):
```
HP Potion        20g  → cura 50 hp
Stamina Potion   15g  → restaura 80 stamina
Sharpening Stone 30g  → +3 attack_damage permanente
Mana Crystal     25g  → +10 mana_max permanente
```

---

## 4. `src/systems/shop_system.hpp` — NOVO

```cpp
struct ShopSystem {
    static bool try_buy(Actor& player, ShopInventory& shop, int item_index);
    //   verifica player.gold >= cost → aplica efeito → deduz gold → true se ok

    static void render_shop_ui(SDL_Renderer* r, const ShopInventory& shop,
                                int player_gold, int viewport_w, int viewport_h);
    //   painel lateral: lista items, custo, gold actual, hint compra/fechar
};
```

---

## 5. `src/scenes/town_scene.hpp` — NOVO

Estrutura análoga à DungeonScene mas sem combate:

```
Sistemas reutilizados (sem alteração):
  MovementSystem, RoomCollisionSystem, RoomFlowSystem,
  DialogueSystem, Camera2D

Estado interno:
  Actor                  _player
  RoomDefinition         _room
  std::vector<NpcEntity> _npcs
  ShopInventory          _shop_forge
  QuestState             _quest_state
  DialogueSystem         _dialogue
  bool                   _shop_open       = false
  int                    _interacting_npc = -1

enter():
  → carrega save (player + quest_state + gold)
  → ini_load("town_dialogues.ini") → register_sequence por cada secção
  → _build_town()

fixed_update():
  → movimento + colisão do player
  → verifica proximidade de NPCs → hint "ENTER - falar"
  → confirm_pressed perto de NPC:
      QuestGiver → inicia diálogo contextual (consoante quest_state)
      Merchant   → abre _shop_open = true
  → se _shop_open: navega com up/down, confirm compra, cancel fecha
  → RoomFlowSystem → porta para "dungeon"

render():
  → cidade (obstáculos/edifícios como AABB coloridas)
  → NPCs (retângulos com nome acima)
  → hint "ENTER - falar" quando perto
  → HUD: gold no canto
  → DialogueSystem::render
  → ShopSystem::render_shop_ui se _shop_open

exit():
  → grava save com quest_state + gold actualizados
```

---

## 6. Layout da cidade — `_build_town()`

Hardcoded na TownScene; migra para `rooms.ini` na Fase B do roadmap INI.

```
Bounds: 2400×1600

Obstáculos (edifícios — AABB com cor de fallback):
  building_tavern   (700,200)→(1100,500)   cor: amarelo escuro
  building_forge    (1300,200)→(1600,500)  cor: cinzento
  building_elder    (300,900)→(600,1200)   cor: roxo escuro
  fountain          (1100,700)→(1300,900)  cor: azul

NPCs:
  Mira (QuestGiver)  pos=(440, 840)    cor: dourado
  Forge (Merchant)   pos=(1450, 540)   cor: laranja
  Villager_A         pos=(900, 750)    cor: verde
  Villager_B         pos=(1600, 900)   cor: verde

Porta dungeon:
  DoorZone (2350,700)→(2390,900)  target="dungeon"  requires_clear=false
```

---

## 7. Diálogos — `data/town_dialogues.ini` — NOVO

```ini
[mira_default]
line_0_speaker=Mira
line_0_text=Safe now, thanks to you. The depths are quieter.

[mira_quest_offer]
line_0_speaker=Mira
line_0_text=Traveller. A creature named Grimjaw has made its home in the third depth. It hunts our scouts.
line_1_speaker=Mira
line_1_text=If you descend and end it, the village will reward you handsomely.

[mira_quest_active]
line_0_speaker=Mira
line_0_text=Grimjaw yet lives. Be careful in the depths.

[mira_quest_done]
line_0_speaker=Mira
line_0_text=You did it. Grimjaw is no more. Take this — you earned it.

[forge_greeting]
line_0_speaker=Forge
line_0_text=Need supplies? Gold talks.

[villager_a]
line_0_speaker=Villager
line_0_text=Don't go near the dungeon at night. Or at all, really.

[villager_b]
line_0_speaker=Old Man
line_0_text=These stones have stood for three hundred years. The monster's been here for three days.
```

Carregado em `TownScene::enter()` via `ini_load` + `register_sequence`.

---

## 8. Gold — alterações em ficheiros existentes

**`src/entities/actor.hpp`**
```cpp
int gold = 0;
```

**`src/entities/enemy_type.hpp` — `EnemyDef`**
```cpp
int gold_drop_min = 1;
int gold_drop_max = 4;
// Skeleton 1–3g  |  Orc 3–6g  |  Ghost 2–4g  |  Grimjaw 25–35g
```

**`src/entities/ground_item.hpp`**
```cpp
// GroundItemType: acrescentar Gold
// GroundItem: acrescentar int gold_value = 0;
```

**`src/systems/drop_system.hpp`**
Gold rolado separadamente do item de equipment:
```
on_enemy_died: após rolar item normal,
  rola gold dentro de [def.gold_drop_min, def.gold_drop_max]
  spawna GroundItem{Gold, gold_value=rolled}
```

**`src/core/save_system.hpp`**
```cpp
// SaveData:
int        gold = 0;
QuestState quest_state{};

// Serializar / deserializar:
gold=N
quest_0=N   quest_1=N   ...
```

---

## 9. Ligações entre cenas — alterações pontuais

**`src/scenes/title_scene.hpp`**
```
_next = "dungeon"  →  _next = "town"
```

**`src/scenes/dungeon_scene.hpp`**
```
Door esquerda target_scene_id: "title" → "town"

Quando Grimjaw morre (linha já existente ~464):
  if quest_state.is(DefeatGrimjaw, InProgress):
      quest_state.set(DefeatGrimjaw, Completed)
      save imediato (ou no exit como já acontece)
```

**`src/core/register_scenes.cpp`**
```cpp
registry.register_scene("town", [](const SceneCreateContext& ctx) {
    auto s = std::make_unique<TownScene>();
    s->viewport_w = ctx.viewport_w;
    s->viewport_h = ctx.viewport_h;
    s->set_renderer(ctx.renderer);
    s->set_audio(ctx.audio);
    return s;
});
```

---

## Resumo

### Novos ficheiros (6)
```
src/core/quest_state.hpp
src/entities/npc.hpp
src/entities/shop.hpp
src/systems/shop_system.hpp
src/scenes/town_scene.hpp
data/town_dialogues.ini
```

### Ficheiros existentes alterados (8)
```
src/core/register_scenes.cpp     +registo "town"
src/scenes/title_scene.hpp       "dungeon"→"town"
src/scenes/dungeon_scene.hpp     door target + quest completion + gold
src/core/save_system.hpp         +gold, +quest_status[]
src/entities/actor.hpp           +gold
src/entities/enemy_type.hpp      +gold_drop_min/max
src/entities/ground_item.hpp     +Gold type + gold_value
src/systems/drop_system.hpp      rola gold separado
```
