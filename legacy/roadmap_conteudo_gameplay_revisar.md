# Roadmap — Conteúdo / Gameplay

> **Actualizado 2026-04-04.** O quadro **Estado no código** resume o repo actual. Tudo a partir de **«Ordem de implementação»** e dos temas 1–2 em diante é **especificação histórica** (útil como arquivo de desenho, não como lista de tarefas).

## Estado no código (última actualização: 2026-04-04)

| Área | Estado | Notas |
|------|--------|--------|
| **Fase A INI (stats)** | Feito | `player.ini`, `progression.ini`, `talents.ini`, `spells.ini`, `enemies.ini`, `items.ini` + `apply_*` / `g_*` mutáveis. |
| **Tema 1 — salas** | Feito | `dungeon_rules::room_bounds` / `room_template` / `room_template_id_name()`; `room.hpp` (obstáculos compostos); `DungeonScene` `_build_room` + `_layout_*`; portas relativas aos bounds. |
| **Tema 2 — skill tree + magias** | Feito | 17 talentos (`TalentState::levels[]`); 8 `SpellId`; `talent_data` / `talent_loader` / `talent_tree`; `spell_book::sync_from_talents` / `spell_damage_rank`; save com talentos por nível (ver **Save v5** abaixo); Q/E/R/F + cleave / multishot+veneno / chain+strafe / battle cry; `spell_effects`; projétil `is_frost`/`frost_rank`, `is_poison`/`poison_rank`; SFX dedicados em `audio` (WAV em `assets/audio/`). |
| **Testes (oficial V2)** | Feito | Suíte modular em `tests/` por domínio (`tests/core/`, `tests/systems/`, `tests/components/`, …). Categorias `spell` / `ini` / `dungeon_rules` / `save` cobrem grande parte do Tema 2; suíte legada em `tests/legacy/test_legacy.cpp` + `tests_legacy/` no CMake. |
| **Tema 3 Fase A1** | Feito (doc + código) | `enemy_type.hpp` / `apply_enemy_ini_section`: `sprite_sheet_path`, `sprite_scale`, `frame_fps`, `dir_row`. `data/enemies.ini` documenta campos; valores de sprite podem ficar comentados até à arte final. |
| **Tema 3 Fase B0–B2 + refactor** | Feito | `IniData::sections_with_prefix`; `data/rooms.ini` com templates e `ini_obstacles=0` por defeito; `room_loader.hpp`; `_load_data_files()` carrega `rooms.ini`; `_build_room` tenta INI se `ini_obstacles=1`, senão `_layout_*`. |
| **Save / migração** | Feito | `SaveData` em `save_data.hpp` — **v5**: atributos base, `attr_points_available`, `scene_flags`; cadeia `save_migration.hpp` (v1→…→v5). Ver também `last_run_stats` para ecrãs de fim de run. |
| **Cena dungeon — modularização** | Feito | Fluxo, save, áudio, FX, morte, boss e diálogo extraídos para headers em `src/systems/` (`dungeon_flow_controller`, `dungeon_save_controller`, `dungeon_audio_system`, `combat_fx_controller`, `enemy_death_controller`, `boss_state_controller`, `dungeon_config_loader`, `footstep_audio_system`, `animation_driver`, etc.). Mapa: [`map_controllers_revisar.md`](map_controllers_revisar.md). |
| **Cena town — modularização** | Feito | `town_builder`, `town_config_loader`, `town_save_controller`, `town_world_renderer`, `town_npc_wander`, `town_npc_interaction`, `shop_input_controller`, etc. |

### Detalhes já registados no código (Tema 2)

- **`spell_damage_rank`**: inclui `SpellId::BattleCry` (`max(BattleCry, IronBody)`) para coerência de CD/calling code; o efeito da magia continua a ser buff, não dano directo.
- **`audio.cpp`**: `SpellChain` → `assets/audio/sfx_spell_chain.wav`; `SkillBattleCry` → `assets/audio/sfx_skill_battle_cry.wav` (quando os ficheiros existirem, o loader encaixa; até lá SDL regista falha silenciosa como nos outros SFX).
- **Projétil vs. rascunho antigo do plano**: campos oficiais `is_frost`, `frost_rank`, `is_poison`, `poison_rank` em `projectile.hpp`.
- **FrostBolt + cooldown**: `g_spell_defs` pode ter `cooldown_per_level=0` para FrostBolt; o **rank** sobe com `SpellPower` (teste `Spell.RankFrostSpellPow`). O **CD por rank** com `cooldown_per_level>0` está coberto por teste em **Chain Lightning** (`Spell.ChainCDScales`).

### Salas por INI (`rooms.ini`)

- Obstáculos: secções `[tpl.obstacle.N]` com `nx1..ny2` normalizados; `name`, `sprite`, `sw`, `sh`, `anchor_x`, `anchor_y` opcionais.
- Activação: só quando `[tpl].ini_obstacles=1` e existe pelo menos um obstáculo válido; caso contrário mantém-se o layout em código.

---

## Ordem de implementação recomendada *(histórico — já concluída)*

```
1. INI Fase A (stats)   — sem dependências, desbloqueia balanceamento imediato
2. Salas / Geometria    — depende de dungeon_rules (já existe) e room.hpp
3. Skill Tree + Magias  — maior cascata de ficheiros; fazer por sub-etapas
4. INI Fase B (rooms)   — depende da Skill Tree estar estável
```

---

## Tema 1 — Geometria e Obstáculos por room_index *(especificação de arquivo)*

### Ficheiros a tocar

| Ficheiro | O que muda |
|---|---|
| `src/world/dungeon_rules.hpp` | +2 funções: `room_bounds()`, `room_template()` |
| `src/world/room.hpp` | +4 helpers de obstáculos compostos |
| `src/scenes/dungeon_scene.hpp` | `_build_room()` refatorado + 6 `_layout_*()` privadas |

---

### 1.1 `src/world/dungeon_rules.hpp`

Acrescentar ao namespace `mion::dungeon_rules`:

```cpp
// Tamanho da sala cresce com room_index
WorldBounds room_bounds(int room_index);
//   0–2  → 1200×900
//   3–5  → 1400×1050
//   6–9  → 1600×1200
//   10+  → 1800×1350

// Seleciona template (0–5) para o room_index dado
int room_template(int room_index);
//   sala 0         → 5  (intro / vazia)
//   múltiplos de 3 → 4  (boss_arena / respiro)
//   restantes      → room_index % 4  (roda os 4 layouts normais)
```

---

### 1.2 `src/world/room.hpp`

Acrescentar à struct `RoomDefinition` (chamam `add_obstacle` internamente):

```cpp
// 3–4 barris pequenos (24×24) em disposição irregular à volta de (cx, cy)
void add_barrel_cluster(float cx, float cy);

// 2 segmentos em L; orientation: 0=NW 1=NE 2=SW 3=SE
void add_wall_L(float corner_x, float corner_y, int orientation);

// 1 obstáculo central 80×80 (altares, relíquias)
void add_altar(float cx, float cy);

// 2 pilares a 60 px de distância; axis: 0=horizontal 1=vertical
void add_pillar_pair(float cx, float cy, int axis);
```

Assets de fallback (retângulos coloridos enquanto não há sprites):
- barrel  → `assets/props/barrel.png`
- wall_L  → `assets/props/dungeon_wall_mid.png`  (já existe)
- altar   → `assets/props/altar.png`
- pillar  → `assets/props/dungeon_pillar.png`  (já existe)

---

### 1.3 `src/scenes/dungeon_scene.hpp`

**`_build_room()` novo fluxo:**
```
1. bounds  = dungeon_rules::room_bounds(_room_index)
2. t       = dungeon_rules::room_template(_room_index)
3. Inicializar tilemap com bounds dinâmico
4. Adicionar portas calculadas sobre bounds (não hardcoded 1600/1200)
5. Dispatch → _layout_*(bounds)
```

**6 funções privadas novas** (todas recebem `const WorldBounds&`):

| Função | Obstáculos | Conceito |
|---|---|---|
| `_layout_arena(b)` | 4 pilares centrais + 2 barrel_cluster nos cantos | Combate aberto |
| `_layout_corredor(b)` | 2 wall_L (NW+SE) + 2 pillar_pair no centro | Estreito, forçado |
| `_layout_cruzamento(b)` | wall_mid vertical + wall_mid horizontal | Divide sala em 4 |
| `_layout_labirinto(b)` | 2 wall_L (SW+NE) + 1 altar central | Cobertura táctica |
| `_layout_boss_arena(b)` | 4 pilares recuados nas bordas, centro livre | Boss fight |
| `_layout_intro(b)` | Nenhum ou 1 altar de lore | Sala 0 / respiro |

**Portas ajustadas ao bounds dinâmico:**
```cpp
float midy = (b.min_y + b.max_y) * 0.5f;
// porta direita:  (b.max_x-72, midy-56) → (b.max_x-20, midy+56)
// porta esquerda: (b.min_x+20, midy-56) → (b.min_x+72, midy+56)
```

---

## Tema 2 — Skill Tree (D2-style) + Magias + Melee + Ranged *(especificação de arquivo)*

### Árvores

```
MELEE (6 nós)                RANGED/BOW (5 nós)           MAGIC (6 nós)
────────────────             ──────────────────            ──────────────
MeleeForce   [P][T1]         SharpArrow   [P][T1]          ArcaneReservoir [P][T1] ← existe
IronBody     [P][T1]               │                              │
     │                     ┌───────┴───────┐                ┌────┴─────────┐
CrushingBlow [P][T2]    MultiShot [A][T2]  PoisonArrow[A]  ManaFlow [P]  SpellPower [P] ← existem
Cleave       [A][T2]          │                                            │
     │                  PiercingShot[P][T3]                        FrostBolt [A][T3]
Whirlwind    [A][T3]    Strafe    [A][T3]              Nova [A][T3]
BattleCry    [A][T3]                                   ChainLightning [A][T4]
```
P = Passivo | A = Activo | T1–T4 = tier (pré-requisito)
Cada nó: nível 1–3, custo 1 ponto por nível.

---

### Ficheiros a tocar — por ordem de dependência

| # | Ficheiro | O que muda |
|---|---|---|
| 1 | `src/components/talent_tree.hpp` | Redesign completo |
| 2 | `src/components/talent_state.hpp` | `bool unlocked[]` → `int levels[]` |
| 3 | `src/components/spell_defs.hpp` | +7 SpellIds + campos de scaling por nível |
| 4 | `src/components/spell_book.hpp` | `sync_from_talents` usa `level_of()` |
| 5 | `src/core/input.hpp` | +`spell_3_pressed` (R) e `spell_4_pressed` (F) |
| 6 | `src/systems/spell_effects.hpp` | +4 novas funções de efeito |
| 7 | `src/systems/player_action.hpp` | +7 handlers de skills activas |
| 8 | `src/core/save_system.hpp` | `talent_X=` grava int em vez de bool |
| 9 | `src/core/audio.hpp` | +6 SoundIds |
| 10 | `data/spells.ini` | +7 entradas + campos `damage_per_level` / `cooldown_per_level` |

---

### 2.1 `src/components/talent_tree.hpp` — REDESIGN COMPLETO

```cpp
enum class Discipline { Melee, Ranged, Magic };
enum class SkillType  { Passive, Active };

enum class TalentId {
    // Melee
    MeleeForce, IronBody, CrushingBlow, Cleave, Whirlwind, BattleCry,
    // Ranged
    SharpArrow, MultiShot, PoisonArrow, PiercingShot, Strafe,
    // Magic
    ArcaneReservoir, ManaFlow, SpellPower, FrostBolt, Nova, ChainLightning,
    Count
};

struct TalentNode {
    TalentId    id;
    Discipline  discipline;
    SkillType   skill_type;
    const char* display_name;   // carregado de talents.ini na Fase A3
    int         max_level;      // default 3
    int         cost_per_level; // sempre 1
    bool        has_parent;
    TalentId    parent;
    int         parent_min_level;
};
```

---

### 2.2 `src/components/talent_state.hpp` — REDESIGN

```
int  levels[kTalentCount] = {}          // substitui bool unlocked[]
int  level_of(TalentId) const
bool is_active(TalentId) const          // level_of > 0  (alias de is_unlocked para compatibilidade)
bool is_unlocked(TalentId) const        // alias de is_active — mantém usos existentes
bool can_spend(TalentId) const          // verifica parent_min_level + pending_points + não no max
bool try_spend(TalentId)                // incrementa levels[id]++
bool has_spendable_options() const      // substitui has_unlockable_options
```

---

### 2.3 `src/components/spell_defs.hpp` — EXTENSÃO

Novos SpellIds:
```
FrostBolt, ChainLightning          ← magic activas
MultiShot, PoisonArrow, Strafe     ← ranged activas
Cleave, BattleCry                  ← melee activas
```

Campos novos em `SpellDef`:
```cpp
int   damage_per_level    = 0;   // bónus de dano por nível do talento
float cooldown_per_level  = 0.0f; // redução de cooldown por nível

int   effective_damage(int talent_level) const;
float effective_cooldown(int talent_level) const;
```

---

### 2.4 `src/components/spell_book.hpp` — sync_from_talents

Mapa talento → SpellId (unlocked quando `level_of(talento) >= 1`):
```
SpellPower L1+        → FrostBolt
ArcaneReservoir L2+   → Nova
SpellPower L2+        → ChainLightning
SharpArrow L1+        → MultiShot, PoisonArrow
MeleeForce L1+        → Cleave
IronBody L2+          → BattleCry
MultiShot L2+         → Strafe
```

---

### 2.5 `src/core/input.hpp`

```cpp
// Acrescentar ao InputState:
bool spell_3_pressed = false;   // tecla R
bool spell_4_pressed = false;   // tecla F

// KeyboardInputSource::read_state():
spell_3_pressed = keys[SDL_SCANCODE_R];
spell_4_pressed = keys[SDL_SCANCODE_F];
```

---

### 2.6 `src/systems/spell_effects.hpp` — novos efeitos

```cpp
void apply_cleave(const Actor& player, std::vector<Actor*>& actors,
                  float radius, int damage);

void apply_chain_lightning(const Actor& player, std::vector<Actor*>& actors,
                           int bounces, int damage);

void apply_frost_slow(Actor& target, float duration, float factor);
// → StatusEffect::Slow com factor vindo do nível do talento FrostBolt

void apply_battle_cry(Actor& player, int talent_level);
// → buff temporário de dano (StatusEffect::Empowered ou via CombatState)
```

---

### 2.7 `src/systems/player_action.hpp` — novos handlers

Acrescentar blocos no `fixed_update`, após os existentes:

```
FrostBolt     (spell_1 se talento activo, ou slot Q)
ChainLightning (spell_2 ou slot E)
Cleave        (attack_pressed + Cleave activo → AoE em vez de melee simples)
MultiShot     (ranged_pressed + MultiShot activo → 3 projéteis em leque ±15°)
PoisonArrow   (ranged_pressed + PoisonArrow activo → projétil com poison on-hit)
Strafe        (spell_3_pressed + Strafe activo → 5 projéteis em arco 60°)
BattleCry     (spell_4_pressed + BattleCry activo → buff temporário)
```

Sinergias (inline, sem estrutura extra):
```
SharpArrow.level_of → +1 dano por nível a MultiShot e PoisonArrow
SpellPower.level_of → +2 dano por nível a FrostBolt e ChainLightning
MeleeForce.level_of → +50% dano de Cleave por nível
```

---

### 2.8 `src/core/save_system.hpp`

```
Serialização:  talent_X= grava int (0/1/2/3)
Deserialização: lê como int (retrocompatível — saves antigos com 0/1 ainda funcionam)
Remover:        spell_unlocked[] do SaveData (reconstruído via sync_from_talents ao carregar)
```

---

### 2.9 `src/core/audio.hpp`

Acrescentar ao enum `SoundId`:
```
SpellFrost, SpellChain, SkillCleave, SkillMultiShot, SkillPoisonArrow, SkillBattleCry
```

---

## Tema 3 — INI Externo

### Fase A — Stats (extensão do padrão actual)

Usa `IniLoader` existente. Sem alterações à infra.

| Sub-etapa | Ficheiro data | Ficheiro código | O que muda |
|---|---|---|---|
| A1 | `data/enemies.ini` | `src/entities/enemy_type.hpp` | +sprite_sheet_path, sprite_scale, frame_fps, dir_row por secção |
| A2 | `data/spells.ini` | `src/components/spell_defs.hpp` | +7 secções + damage_per_level, cooldown_per_level |
| A3 | `data/talents.ini` | `src/components/talent_tree.hpp` | display_name, max_level, cost_per_level, parent, parent_min_level |
| A4 | `data/player.ini` | `src/entities/actor.hpp` ou init do player em dungeon_scene | stats base do player |
| A5 | `data/progression.ini` | `src/components/progression.hpp` | xp_base, xp_scale, bonuses de level-up |

**`data/player.ini` — campos:**
```ini
[player]
base_hp=100          base_move_speed=140   base_mana=60
base_mana_regen=8    base_stamina=100      stamina_regen=40
stamina_delay=0.4    melee_damage=12       ranged_damage=8
ranged_cooldown=0.6  dash_speed=380        dash_duration=0.18
dash_cooldown=0.9    dash_iframes=0.22
```

**`data/progression.ini` — campos:**
```ini
[xp]
xp_base=100          ; xp_to_next no nível 1
xp_level_scale=100   ; cresce N por nível

[level_up]
damage_bonus=3        hp_bonus=15
speed_bonus=18        spell_mult_bonus=0.08
```

**`data/talents.ini` — exemplo:**
```ini
[melee_force]
display_name=Melee Force
max_level=3
cost_per_level=1

[cleave]
display_name=Cleave
max_level=3
cost_per_level=1
parent=melee_force
parent_min_level=1
```

---

### Fase B — Estrutural (requer extensão ao IniLoader)

#### B0. `src/core/ini_loader.hpp` — extensão

```cpp
// Acrescentar a IniData:
std::vector<std::string> sections_with_prefix(const std::string& prefix) const;
// → retorna ["arena.obstacle.0", "arena.obstacle.1", ...]
// Permite iterar múltiplos obstáculos por sala sem alterar o formato flat do ficheiro.
```

#### B1. `data/rooms.ini` — novo ficheiro

```ini
[arena]
bounds_w=1600        bounds_h=1200
door_left=1          door_right=1
obstacle_count=4

[arena.obstacle.0]
name=pillar_nw       x1=200  y1=200  x2=280  y2=280
sprite=assets/props/dungeon_pillar.png
sw=96                sh=128

[corredor]
bounds_w=1400        bounds_h=1050
obstacle_count=3
...
```

#### B2. `src/world/room_loader.hpp` — novo ficheiro

```cpp
// Carrega um template de sala do INI e retorna RoomDefinition pronto.
RoomDefinition load_room_template(const IniData& d,
                                  const std::string& template_name,
                                  const WorldBounds& bounds);
```
Chamado por `_build_room()` em `dungeon_scene` quando `rooms.ini` estiver disponível.
Fallback: se secção não existir no INI, usa os `_layout_*()` hardcoded do Tema 1.

---

### Extracção do loading em `dungeon_scene.hpp`

Actualmente o loading está inline em `enter()`. Com todas as novas fontes, deve ser extraído:

```cpp
// Método privado a criar:
void _load_data_files();
//   → enemies.ini    (já existe, move para cá)
//   → spells.ini     (já existe, move para cá)
//   → items.ini      (já existe, move para cá)
//   → talents.ini    (Fase A3)
//   → player.ini     (Fase A4)
//   → progression.ini (Fase A5)
//   → rooms.ini      (Fase B1)
```

---

## Resumo de todos os ficheiros por tema

### Tema 1 — Salas
```
src/world/dungeon_rules.hpp     +room_bounds(), +room_template()
src/world/room.hpp              +4 helpers de obstáculos
src/scenes/dungeon_scene.hpp    _build_room() + 6 _layout_*()
```

### Tema 2 — Skill Tree
```
src/components/talent_tree.hpp  redesign (17 nós, 3 disciplinas, multi-level)
src/components/talent_state.hpp bool→int + novos métodos
src/components/spell_defs.hpp   +7 SpellIds + scaling por nível
src/components/spell_book.hpp   sync_from_talents atualizado
src/core/input.hpp              +spell_3 (R) / spell_4 (F)
src/systems/spell_effects.hpp   +4 efeitos novos
src/systems/player_action.hpp   +7 handlers activos
src/core/save_system.hpp        talent int + remove spell_unlocked
src/core/audio.hpp              +6 SoundIds
data/spells.ini                 +7 entradas + campos per_level
```

### Tema 3 — INI
```
data/enemies.ini       +sprite fields (Fase A1)
data/spells.ini        +7 skills (Fase A2 — partilhado com Tema 2)
data/talents.ini       novo (Fase A3)
data/player.ini        novo (Fase A4)
data/progression.ini   novo (Fase A5)
data/rooms.ini         novo (Fase B1)
src/core/ini_loader.hpp         +sections_with_prefix (Fase B0)
src/world/room_loader.hpp       novo (Fase B1)
src/scenes/dungeon_scene.hpp    _load_data_files() extraído
```
