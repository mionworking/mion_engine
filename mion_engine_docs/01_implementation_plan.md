# 01 — Implementation Plan

Detailed technical execution per sprint. Companion to `00_master.md`
(roadmap) and `karma_system_design.md` (mechanical design).

This document tells **where to edit, what to add, what to test, and what
DONE looks like** for every sprint K1-K8 and C1-C9.

> Read order: `CLAUDE.md` → `00_master.md` → this doc § for current sprint.

---

## How to read each sprint

Every sprint section follows this structure:

1. **Goal** — one sentence
2. **Files read** — context needed before coding
3. **Files written** — what gets created or modified
4. **Sub-sprints** — autonomous work units, each compiles + tests
5. **DONE criteria** — checkable list

Sub-sprints are ordered. Each compiles and passes tests. The game is
playable between sub-sprints — never broken between them.

---

## Conventions used

### File paths

All paths relative to project root. Examples:
- `src/components/karma_data.hpp` — component header
- `src/systems/karma_drop_system.hpp` — system header
- `data/karma.ini` — game data INI

### Code style

Per CLAUDE.md:
- `mion` namespace for everything new
- Plain structs + free functions
- Header-heavy: `.hpp` with `inline`, `.cpp` only when needed
- No exceptions in hot path
- Comments in pt-BR

### **VERIFY** markers

When the plan assumes a function/file exists with specific signature
based on CLAUDE.md but hasn't been confirmed by reading the actual file,
the marker `**VERIFY:** <what to verify>` is used. The implementer reads
the actual file first and adjusts the plan if needed.

---

# Phase A — Karma core systems

---

## Sprint K1 — Karma foundation

**Goal:** Karma exists as a resource, replaces XP, persists in save.
Existing gameplay continues working; karma is additive, not destructive.

### Files read

- `src/components/progression.hpp` — current XP/level structure
- `src/components/health.hpp` — to understand component pattern
- `src/systems/enemy_death_controller.hpp` — to know where karma drops
- `src/core/save_data.hpp` — for save schema
- `src/core/save_migration.hpp` — for migration pattern v6→v7 as template
- `src/systems/dungeon_hud.hpp` (or wherever HUD lives) — to add karma display
- `src/systems/attribute_level_up_controller.hpp` — to adapt to focus mode
- `data/attributes.ini` — for attribute config
- `data/enemies.ini` — to add karma drop field

### Files written

- `src/components/karma_data.hpp` (new)
- `src/systems/karma_drop_system.hpp` (new)
- `src/components/progression.hpp` (modified — adds karma helpers, keeps XP for migration)
- `src/systems/enemy_death_controller.hpp` (modified — emits karma)
- `src/core/save_data.hpp` (modified — adds karma fields)
- `src/core/save_migration.hpp` (modified — adds v7→v8)
- `src/systems/attribute_level_up_controller.hpp` (modified — focus mode)
- `src/systems/dungeon_hud.hpp` (modified — karma display)
- `data/enemies.ini` (modified — add `karma_drop` field per enemy)
- `data/progression.ini` (new or modified — karma level thresholds)
- `data/attributes.ini` (modified — add `focus_bonus` per attribute)
- `tests/test_karma_data.cpp` (new)
- `tests/test_karma_drop_system.cpp` (new)
- `tests/test_save_migration_v8.cpp` (new)
- `tests/test_attribute_focus.cpp` (new)

### K1.1 — KarmaData component

Create `src/components/karma_data.hpp`:

```cpp
#pragma once
#include <cstdint>

namespace mion {

// Componente que armazena o karma do jogador.
// `total` representa o karma acumulado na vida (nunca diminui — funciona
// como "level"). `available` é o saldo disponível para gastar em
// skills, armas, traje ou serviços.
struct KarmaData {
    int64_t total = 0;       // karma acumulado total (nunca decresce)
    int64_t available = 0;   // karma disponível para gasto

    // Invariante: total >= already_invested (= total - available)
};

// Adiciona karma ao jogador (drop de inimigo, recompensa, etc.).
// Sobe tanto `total` quanto `available`.
inline void karma_add(KarmaData& k, int64_t amount) {
    if (amount <= 0) return;
    k.total += amount;
    k.available += amount;
}

// Tenta gastar karma. Retorna true se houve karma suficiente.
// `total` permanece inalterado (nunca decresce).
inline bool karma_spend(KarmaData& k, int64_t amount) {
    if (amount <= 0) return true;
    if (k.available < amount) return false;
    k.available -= amount;
    return true;
}

// Devolve karma gasto ao pool disponível (usado em respec).
// `total` permanece inalterado.
inline void karma_refund(KarmaData& k, int64_t amount) {
    if (amount <= 0) return;
    k.available += amount;
    // Não pode passar do total (sanity check)
    if (k.available > k.total) k.available = k.total;
}

} // namespace mion
```

**VERIFY:** that `PlayerData` (per Sprint 5 actor split) accepts new
component. Add `KarmaData karma;` field to `PlayerData`.

**Tests** (`tests/test_karma_data.cpp`):
- `karma_add` increases total and available equally
- `karma_add` with negative or zero is no-op
- `karma_spend` succeeds when enough available
- `karma_spend` fails (returns false) when insufficient
- `karma_spend` does not modify total
- `karma_refund` returns to available, capped at total
- Round-trip: spend X, refund X → state equals before spend

### K1.2 — Karma drop on enemy death

Modify `src/systems/enemy_death_controller.hpp`:

**VERIFY:** the function that handles enemy death. Per CLAUDE.md it's
`process_deaths()` returning `DeathResult`. Look for where XP is currently
awarded.

Add karma awarding alongside (do not remove XP yet — that's K7):

```cpp
// Em DeathResult ou estrutura equivalente:
struct DeathResult {
    // ... campos existentes ...
    int64_t karma_dropped = 0;  // karma absorvido pelo player
};

// Na lógica de morte:
// VERIFY: ler `karma_drop` do EnemyType (campo em enemies.ini)
// e somar ao DeathResult. O caller chama karma_add() no PlayerData.
```

Modify `data/enemies.ini`:

Add `karma_drop = N` for each enemy type. Provisional values:
- Common enemies: 5-15 karma
- Tougher enemies: 20-40 karma
- Mini-bosses: 100-300 karma
- Boss (O Conselheiro): 1000+ karma

**Tests** (`tests/test_karma_drop_system.cpp`):
- Killing enemy with `karma_drop = 10` adds 10 to player's karma
- Multiple kills accumulate
- Killing enemy with `karma_drop = 0` does not crash, adds nothing
- Killing in sequence respects `karma_add` invariants

### K1.3 — Level by karma total

Modify `src/components/progression.hpp` (do not remove XP-based level
yet — keep both for migration):

```cpp
// Função pura: retorna o level dado o karma total.
// Tabela de thresholds em data/progression.ini.
inline int karma_level(int64_t karma_total) {
    // VERIFY: estrutura da tabela. Pode ser std::vector<int64_t> kThresholds
    // carregada na boot. Iteração linear, retorna o último threshold
    // <= karma_total.
}
```

Modify `data/progression.ini`:

Add karma level thresholds. Suggested curve (provisional):

```ini
[karma_levels]
level_1 = 0
level_2 = 100
level_3 = 250
level_4 = 500
level_5 = 1000
level_6 = 2000
level_7 = 4000
level_8 = 7000
level_9 = 11000
level_10 = 16000
# ... up to level_30 or so
```

**Tests:**
- `karma_level(0)` → 1
- `karma_level(99)` → 1
- `karma_level(100)` → 2
- `karma_level(250)` → 3
- `karma_level(very_large)` → max defined level

### K1.4 — Attribute focus mode

Modify `src/systems/attribute_level_up_controller.hpp`:

**VERIFY:** current structure. Per CLAUDE.md it's a controller that
shows the attribute screen on level up. Adapt to focus mode:

- When player levels up (karma_total crosses threshold):
  - Show 5 attributes
  - Player picks ONE
  - Picked attribute: `+focus_bonus` (from INI, e.g., +3)
  - Other 4 attributes: `+1` each
  - Screen closes

```cpp
namespace mion {

enum class AttributeId : uint8_t {
    Vigor, Forca, Destreza, Inteligencia, Endurance, Count
};

// Aplica o foco escolhido pelo jogador ao subir de level.
// `focused`: atributo escolhido. Recebe o bônus de foco.
// Os demais recebem +1.
inline void apply_attribute_focus(AttributeBlock& attrs, AttributeId focused,
                                   const AttributeConfig& cfg) {
    for (int i = 0; i < (int)AttributeId::Count; ++i) {
        AttributeId id = (AttributeId)i;
        int gain = (id == focused) ? cfg.focus_bonus : 1;
        attrs[id] += gain;
    }
}

} // namespace mion
```

Modify `data/attributes.ini`:

```ini
[focus]
focus_bonus = 3   # atributo focado ganha +3
base_gain = 1     # demais ganham +1 cada
```

**Tests** (`tests/test_attribute_focus.cpp`):
- Focused attribute gains `focus_bonus`
- Other 4 attributes gain `base_gain` (= 1) each
- Total gain per level = focus_bonus + 4 * base_gain (= 7 default)
- Multiple level-ups stack correctly

### K1.5 — HUD karma display

Modify `src/systems/dungeon_hud.hpp` (or actual HUD file):

**VERIFY:** find where XP bar/number is rendered. Either replace or
add adjacent.

Display format suggestion:
- "Karma: {available} / {total}" or
- Two bars: top = karma to next level (visual progress), bottom = available

Keep XP visible during K1-K6 (won't be removed until K7). Karma is
additive on the HUD.

**Tests:** rendering tests are integration-level. Manual verification
acceptable; existing HUD test pattern can extend.

### K1.6 — Save migration v7 → v8

Modify `src/core/save_data.hpp`:

Add karma fields:
```cpp
struct SaveData {
    // ... campos v7 existentes ...
    int64_t karma_total = 0;
    int64_t karma_available = 0;
    uint8_t attribute_focus_history[256] = {0};  // foco por level (compactado)
    // ...
    uint32_t version = 8;
};
```

Modify `src/core/save_migration.hpp`:

```cpp
namespace mion {

// Migração v7 → v8.
// XP existente é convertido em karma total (mantém progresso do jogador).
// Karma available = karma total (sem investimento prévio em sistemas
// karma — eles ainda não existem).
inline void migrate_v7_to_v8(SaveData& save) {
    // VERIFY: nome do campo de XP em SaveData v7
    save.karma_total = save.xp_total_v7;  // ou equivalente
    save.karma_available = save.karma_total;
    save.version = 8;
}

} // namespace mion
```

**Tests** (`tests/test_save_migration_v8.cpp`):
- Save v7 with XP=500 migrates to v8 with karma_total=500, available=500
- Round-trip v8 save/load preserves karma fields
- Migration is idempotent (running on v8 save does nothing)

### K1 DONE criteria

- [ ] `KarmaData` component exists with full API and tests
- [ ] Killing enemies adds karma to player
- [ ] Level computed by karma total
- [ ] Attribute level up uses focus mode
- [ ] HUD shows karma (alongside XP, both visible)
- [ ] Save v7 → v8 migration works
- [ ] All new tests pass
- [ ] Existing 867 tests still pass
- [ ] Game playable end-to-end with karma flowing

---

## Sprint K2 — 3 skill trees

**Goal:** 3 trees (warrior/mage/archer) Y-shaped, unlock by karma,
synergies between trees, free navigation.

### Files read

- `src/components/talents.hpp` (or talent component file)
- `src/systems/skill_tree_controller.hpp` (or equivalent UI controller)
- `data/talents.ini` — current single-tree structure
- Output of K1 — to know how karma is spent

### Files written

- `src/components/skill_tree_state.hpp` (new)
- `src/systems/skill_tree_controller.hpp` (modified — multi-tree nav)
- `data/skills/warrior_tree.ini` (new)
- `data/skills/mage_tree.ini` (new)
- `data/skills/archer_tree.ini` (new)
- `data/skills/synergies.ini` (new)
- `tests/test_skill_tree.cpp` (new or expanded)

> **Skill node content** (effects, costs, prerequisites) is in
> `02_skill_tree_definitive.md` — that doc will list every node.
> This sprint implements the structure; content fills in.

### K2.1 — Multi-tree data structure

Create `src/components/skill_tree_state.hpp`:

```cpp
#pragma once
#include <bitset>
#include <cstdint>

namespace mion {

enum class SkillTreeId : uint8_t {
    Warrior = 0, Mage = 1, Archer = 2, Count
};

enum class SkillBranch : uint8_t {
    PreY = 0,     // tronco comum, antes da bifurcação
    BranchA = 1,  // ramo A (ex: Berserker)
    BranchB = 2   // ramo B (ex: Guardian)
};

// Identifica univocamente um nó de skill.
struct SkillNodeId {
    SkillTreeId tree;
    uint16_t index;  // índice dentro da árvore

    bool operator==(const SkillNodeId& o) const {
        return tree == o.tree && index == o.index;
    }
};

// Estado da skill tree do jogador.
// Cada árvore tem um bitset compacto de nós desbloqueados.
struct SkillTreeState {
    static constexpr int kMaxNodesPerTree = 64;
    std::bitset<kMaxNodesPerTree> warrior_unlocked;
    std::bitset<kMaxNodesPerTree> mage_unlocked;
    std::bitset<kMaxNodesPerTree> archer_unlocked;
};

inline bool is_unlocked(const SkillTreeState& s, SkillNodeId id) {
    switch (id.tree) {
        case SkillTreeId::Warrior: return s.warrior_unlocked.test(id.index);
        case SkillTreeId::Mage:    return s.mage_unlocked.test(id.index);
        case SkillTreeId::Archer:  return s.archer_unlocked.test(id.index);
        default: return false;
    }
}

inline void set_unlocked(SkillTreeState& s, SkillNodeId id, bool v) {
    switch (id.tree) {
        case SkillTreeId::Warrior: s.warrior_unlocked.set(id.index, v); break;
        case SkillTreeId::Mage:    s.mage_unlocked.set(id.index, v); break;
        case SkillTreeId::Archer:  s.archer_unlocked.set(id.index, v); break;
        default: break;
    }
}

// Reseta tudo (usado em respec).
inline void skill_tree_reset(SkillTreeState& s) {
    s.warrior_unlocked.reset();
    s.mage_unlocked.reset();
    s.archer_unlocked.reset();
}

} // namespace mion
```

Add `SkillTreeState skill_tree;` to `PlayerData`.

### K2.2 — INI structure for trees

Each tree in its own file. Format:

```ini
# data/skills/warrior_tree.ini

[node_0]
name_pt = Vigor Bruto
name_en = Raw Vigor
branch = pre_y
karma_cost = 50
prereqs =
desc_pt = +20% HP máximo
desc_en = +20% max HP
effect = hp_max_pct +20

[node_1]
name_pt = Investida
name_en = Charge
branch = pre_y
karma_cost = 75
prereqs = node_0
desc_pt = Avança 5m em linha reta causando dano
desc_en = Dash 5m forward dealing damage
effect = grant_skill charge

# ... e assim por diante
# Bifurcação no node_5 (último do pre_y)
# node_6+ tem branch = a ou branch = b

[node_6]
name_pt = Berserker
name_en = Berserker
branch = a
karma_cost = 150
prereqs = node_5
desc_pt = +30% dano enquanto HP < 50%
desc_en = +30% damage while HP < 50%
effect = damage_pct_low_hp +30

# ...
```

Effects use a simple DSL parsed by the engine. Examples:
- `hp_max_pct +20` → multiplies max HP
- `grant_skill <id>` → adds active ability
- `damage_pct_low_hp +30` → conditional buff

> Full DSL reference and effect catalog in `02_skill_tree_definitive.md`.

### K2.3 — Unlock by karma

Modify or create skill tree controller logic:

```cpp
namespace mion {

struct UnlockResult {
    bool success;
    const char* reason;  // só usado em falha
};

inline UnlockResult try_unlock_skill(PlayerData& p, SkillNodeId id,
                                     const SkillTreeRegistry& reg) {
    if (is_unlocked(p.skill_tree, id))
        return {false, "already_unlocked"};

    const SkillNodeDef& def = reg.get(id);

    // Verifica pré-requisitos
    for (auto prereq : def.prereqs) {
        if (!is_unlocked(p.skill_tree, prereq))
            return {false, "missing_prereq"};
    }

    // Verifica karma
    if (!karma_spend(p.karma, def.karma_cost))
        return {false, "insufficient_karma"};

    set_unlocked(p.skill_tree, id, true);
    return {true, nullptr};
}

} // namespace mion
```

### K2.4 — Synergies

Synergies are passive bonuses that activate when both nodes of a synergy
pair are unlocked. They cost no extra karma.

`data/skills/synergies.ini`:

```ini
[synergy_0]
node_a = warrior:6     # warrior tree, node 6 (Berserker)
node_b = mage:4        # mage tree, node 4 (Fogo Vivo)
name_pt = Fúria Ardente
name_en = Burning Fury
desc_pt = Investida deixa rastro de fogo
desc_en = Charge leaves a fire trail
effect = grant_passive burning_charge
```

Synergy check at runtime (each frame or on unlock):
```cpp
inline bool synergy_active(const SkillTreeState& s, const SynergyDef& syn) {
    return is_unlocked(s, syn.node_a) && is_unlocked(s, syn.node_b);
}
```

### K2.5 — Save: skill tree state

Modify `SaveData`:
```cpp
struct SaveData {
    // ...
    uint64_t skill_warrior_bits = 0;  // bitset compactado
    uint64_t skill_mage_bits = 0;
    uint64_t skill_archer_bits = 0;
    // ...
};
```

Migration v8 → v9: existing single tree (if any) maps to warrior tree;
old talent points become karma_available.

### K2.6 — UI: navegação entre árvores

**VERIFY:** current `skill_tree_controller` UI structure. Adapt to:
- Tabs or arrow buttons to switch tree (warrior/mage/archer)
- Each tree renders its own layout (Y-shape)
- Synergies highlight when active

### K2 DONE criteria

- [ ] `SkillTreeState` exists with multi-tree support
- [ ] 3 INI files parsed correctly
- [ ] Unlocking spends karma; rejects without karma or prereqs
- [ ] Y-shape works (pre-Y → branchA or branchB)
- [ ] Synergies activate with paired nodes
- [ ] UI navigation between trees works
- [ ] Save/load preserves all tree state
- [ ] Tests pass

---

## Sprint K3 — Universal weapons

**Goal:** Player has access to sword/bow/magic universally. Each
evolves by tier via karma spending. Skill tree nodes can boost specific
weapon categories.

### Files read

- Output of K1, K2
- `src/systems/melee_combat.hpp`
- `src/systems/spell_system.hpp`
- `src/systems/projectile_system.hpp` (if exists for bow)
- `src/components/equipment.hpp` — to understand current weapon slot

### Files written

- `src/components/weapon_data.hpp` (new)
- `data/weapons.ini` (new)
- `src/systems/melee_combat.hpp` (modified — read damage from weapon tier)
- `src/systems/spell_system.hpp` (modified — magic weapon tier influence)
- `src/systems/projectile_system.hpp` (modified — bow tier)
- `src/systems/weapon_upgrade_controller.hpp` (new)
- `tests/test_weapon_data.cpp` (new)

### K3.1 — WeaponData component

Create `src/components/weapon_data.hpp`:

```cpp
#pragma once
#include <cstdint>

namespace mion {

enum class WeaponCategory : uint8_t {
    Sword = 0, Bow = 1, Magic = 2, Count
};

struct WeaponData {
    uint8_t tier_sword = 0;
    uint8_t tier_bow = 0;
    uint8_t tier_magic = 0;

    static constexpr uint8_t kMaxTier = 10;
};

inline uint8_t weapon_tier(const WeaponData& w, WeaponCategory cat) {
    switch (cat) {
        case WeaponCategory::Sword: return w.tier_sword;
        case WeaponCategory::Bow: return w.tier_bow;
        case WeaponCategory::Magic: return w.tier_magic;
        default: return 0;
    }
}

inline void weapon_set_tier(WeaponData& w, WeaponCategory cat, uint8_t tier) {
    if (tier > WeaponData::kMaxTier) tier = WeaponData::kMaxTier;
    switch (cat) {
        case WeaponCategory::Sword: w.tier_sword = tier; break;
        case WeaponCategory::Bow: w.tier_bow = tier; break;
        case WeaponCategory::Magic: w.tier_magic = tier; break;
        default: break;
    }
}

} // namespace mion
```

Add `WeaponData weapons;` to `PlayerData`.

### K3.2 — Weapon stats per tier

`data/weapons.ini`:

```ini
[sword]
name_pt = Espada
name_en = Sword

[sword.tier.0]
damage = 10
attack_speed = 1.0
crit_chance = 0.05
upgrade_cost = 0

[sword.tier.1]
damage = 15
attack_speed = 1.05
crit_chance = 0.07
upgrade_cost = 100

[sword.tier.2]
damage = 25
attack_speed = 1.1
crit_chance = 0.1
upgrade_cost = 250

# ...

[bow]
# similar

[magic]
# similar — afeta dano de spells
```

### K3.3 — Combat reads from weapon tier

`src/systems/melee_combat.hpp` modification:

**VERIFY:** current damage calculation. Replace or augment:

```cpp
inline int compute_melee_damage(const PlayerData& p, const WeaponDefs& defs) {
    uint8_t tier = p.weapons.tier_sword;
    int base = defs.sword_tier_damage[tier];

    // Bônus de skill tree (nós que potencializam espada)
    int skill_bonus = 0;
    if (is_unlocked(p.skill_tree, kSwordMastery1)) skill_bonus += base * 0.10;
    // ... outros nós relevantes ...

    return base + skill_bonus;
}
```

Same pattern for bow (`projectile_system`) and magic (`spell_system`).

### K3.4 — Upgrade controller

`src/systems/weapon_upgrade_controller.hpp`:

UI flow: player opens weapon menu → sees 3 weapons + their tiers + cost
to upgrade → confirms upgrade → karma deducted.

```cpp
inline UnlockResult try_upgrade_weapon(PlayerData& p, WeaponCategory cat,
                                       const WeaponDefs& defs) {
    uint8_t cur = weapon_tier(p.weapons, cat);
    if (cur >= WeaponData::kMaxTier)
        return {false, "max_tier"};

    int cost = defs.upgrade_cost(cat, cur + 1);
    if (!karma_spend(p.karma, cost))
        return {false, "insufficient_karma"};

    weapon_set_tier(p.weapons, cat, cur + 1);
    return {true, nullptr};
}
```

### K3.5 — Save weapon tiers

Add to `SaveData`:
```cpp
uint8_t weapon_tier_sword = 0;
uint8_t weapon_tier_bow = 0;
uint8_t weapon_tier_magic = 0;
```

### K3 DONE criteria

- [ ] `WeaponData` component working
- [ ] All 3 weapons usable from start
- [ ] Tier upgrade spends karma, capped at kMaxTier
- [ ] Combat damage scales with tier + skill tree
- [ ] Save/load preserves tiers
- [ ] Tests pass

---

## Sprint K4 — Traje system

**Goal:** Player chooses one of 3 traje lines. Tier evolves by karma.
Stats scale with line + tier. Synergy/trade-off with skill tree build.

### Files read

- Output of K1-K3
- `src/components/equipment.hpp` (current armor slots — to be deprecated K7)

### Files written

- `src/components/traje_data.hpp` (new)
- `data/traje.ini` (new)
- `src/systems/traje_controller.hpp` (new)
- `src/systems/player_stats_calc.hpp` (new or modified — apply traje stats)
- `tests/test_traje.cpp` (new)

### K4.1 — TrajeData

```cpp
#pragma once
#include <cstdint>

namespace mion {

enum class TrajeLine : uint8_t {
    None = 0, Warrior = 1, Mage = 2, Archer = 3
};

struct TrajeData {
    TrajeLine line = TrajeLine::None;
    uint8_t tier = 0;  // 0 = sem traje; 1+ = ativo

    static constexpr uint8_t kMaxTier = 10;
};

inline bool traje_active(const TrajeData& t) {
    return t.line != TrajeLine::None && t.tier > 0;
}

} // namespace mion
```

Add `TrajeData traje;` to `PlayerData`.

### K4.2 — Traje stats INI

`data/traje.ini`:

```ini
[warrior]
name_pt = Traje do Guerreiro
name_en = Warrior Outfit

[warrior.tier.1]
hp_bonus = 50
armor = 5
movement_speed_pct = -5
upgrade_cost = 100

[warrior.tier.2]
hp_bonus = 120
armor = 12
movement_speed_pct = -5
upgrade_cost = 250

# ... up to tier 10

[mage]
# barrier, mana regen, magic resist, slight HP penalty

[archer]
# evasion, movement speed, critical chance, lower armor
```

### K4.3 — Apply traje stats

`src/systems/player_stats_calc.hpp`:

```cpp
// Aplica stats do traje sobre os stats base do player.
// Chamado quando traje muda (escolha de linha, upgrade, respec).
inline void recalc_player_stats(PlayerData& p, const TrajeDefs& defs) {
    // 1. Reset para base
    p.stats = compute_base_stats(p);

    // 2. Aplicar atributos
    apply_attributes(p.stats, p.attrs);

    // 3. Aplicar traje
    if (traje_active(p.traje)) {
        const TrajeStats& ts = defs.line_tier(p.traje.line, p.traje.tier);
        p.stats.hp_max += ts.hp_bonus;
        p.stats.armor += ts.armor;
        p.stats.movement_speed *= (1.0f + ts.movement_speed_pct / 100.0f);
        // ...

        // 4. Sinergia com skill tree
        apply_traje_synergy(p);
    }

    // 5. Aplicar bônus de skill tree (geral)
    apply_skill_tree_passives(p);
}
```

### K4.4 — Sinergia / trade-off

When traje line matches dominant skill tree investment, bonus applied:

```cpp
inline void apply_traje_synergy(PlayerData& p) {
    int warrior_invest = count_unlocked(p.skill_tree.warrior_unlocked);
    int mage_invest = count_unlocked(p.skill_tree.mage_unlocked);
    int archer_invest = count_unlocked(p.skill_tree.archer_unlocked);

    int dominant_threshold = 5;  // pelo menos 5 nós

    bool synergy = false;
    if (p.traje.line == TrajeLine::Warrior && warrior_invest >= dominant_threshold) synergy = true;
    if (p.traje.line == TrajeLine::Mage && mage_invest >= dominant_threshold) synergy = true;
    if (p.traje.line == TrajeLine::Archer && archer_invest >= dominant_threshold) synergy = true;

    if (synergy) {
        p.stats.hp_max *= 1.10f;
        p.stats.damage_pct += 10;
        // ... bônus por sinergia
    }
}
```

Trade-off (cross-line) pode aplicar penalidade leve, ou simplesmente
ausência de sinergia (escolha de design).

### K4.5 — Controller for choice and upgrade

```cpp
inline UnlockResult try_choose_traje_line(PlayerData& p, TrajeLine line) {
    if (p.traje.line == line) return {false, "same_line"};

    // Trocar linha reseta tier (custo: o jogador volta ao tier 0)
    // Considerar isso a parte do respec se preferir
    p.traje.line = line;
    p.traje.tier = 1;  // tier inicial 1 ao escolher
    return {true, nullptr};
}

inline UnlockResult try_upgrade_traje(PlayerData& p, const TrajeDefs& defs) {
    if (!traje_active(p.traje)) return {false, "no_line_chosen"};
    if (p.traje.tier >= TrajeData::kMaxTier) return {false, "max_tier"};

    int cost = defs.upgrade_cost(p.traje.line, p.traje.tier + 1);
    if (!karma_spend(p.karma, cost)) return {false, "insufficient_karma"};

    p.traje.tier++;
    recalc_player_stats(p, defs);
    return {true, nullptr};
}
```

### K4.6 — Save

```cpp
uint8_t traje_line = 0;  // TrajeLine enum
uint8_t traje_tier = 0;
```

### K4.7 — Visual swap (procedural placeholder)

**VERIFY:** how player sprite is loaded. For this phase, use color-shift
or palette swap based on `traje.line` and `traje.tier`. No new sprites.

```cpp
inline ColorTint traje_tint(TrajeLine line, uint8_t tier) {
    float intensity = (float)tier / TrajeData::kMaxTier;
    switch (line) {
        case TrajeLine::Warrior: return {1.0f, 0.7f - intensity * 0.3f, 0.4f - intensity * 0.2f};
        case TrajeLine::Mage:    return {0.5f - intensity * 0.2f, 0.7f, 1.0f};
        case TrajeLine::Archer:  return {0.7f, 1.0f, 0.6f - intensity * 0.2f};
        default: return {1.0f, 1.0f, 1.0f};
    }
}
```

### K4 DONE criteria

- [ ] `TrajeData` component working
- [ ] 3 lines selectable; switching resets tier
- [ ] Tier up spends karma, applies stats
- [ ] Synergy with skill tree dominant build applies bonus
- [ ] Visual tint changes with line/tier
- [ ] Save preserves traje state
- [ ] Tests pass

---

## Sprint K5 — World services + respec

**Goal:** Replace traditional shop with altars and service NPCs that
trade karma for in-world services. Free respec at altar of dissolution.

### Files read

- `src/entities/shop.hpp` and `src/systems/shop_system.hpp`
- `src/entities/npc.hpp`
- `src/systems/dialogue_system.hpp`

### Files written

- `src/entities/service_npc.hpp` (new)
- `src/entities/altar.hpp` (new)
- `src/systems/service_system.hpp` (new)
- `src/systems/respec_system.hpp` (new)
- `data/services.ini` (new)
- `tests/test_services.cpp` (new)

### K5.1 — Service entity

```cpp
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace mion {

enum class ServiceKind : uint8_t {
    Heal, Buff, Potion, MapReveal, Respec
};

struct Service {
    ServiceKind kind;
    int64_t karma_cost;        // 0 = grátis (ex: respec)
    std::string param;          // ex: nome do buff, do mapa, etc.
    std::string label_pt;
    std::string label_en;
};

struct ServiceNpc {
    std::string id;             // identificador único
    std::string name_pt;
    std::string name_en;
    std::vector<Service> services;
    // posição, sprite, etc. via componentes existentes
};

struct Altar {
    std::string id;
    AltarKind kind;             // Dissolution, Healing, etc.
    Service service;            // o que o altar oferece
};

} // namespace mion
```

### K5.2 — Service system

```cpp
inline UnlockResult try_use_service(PlayerData& p, const Service& svc,
                                    GameContext& ctx) {
    if (svc.karma_cost > 0) {
        if (!karma_spend(p.karma, svc.karma_cost))
            return {false, "insufficient_karma"};
    }

    switch (svc.kind) {
        case ServiceKind::Heal:
            heal_player_full(p);
            break;
        case ServiceKind::Buff:
            apply_buff_by_id(p, svc.param);
            break;
        case ServiceKind::Potion:
            give_potion(p, svc.param);
            break;
        case ServiceKind::MapReveal:
            reveal_zone(ctx.map, svc.param);
            break;
        case ServiceKind::Respec:
            respec_full(p);
            break;
    }
    return {true, nullptr};
}
```

### K5.3 — Respec

```cpp
inline void respec_full(PlayerData& p) {
    // Calcular karma a devolver
    int64_t to_refund = 0;

    // Skill tree
    to_refund += sum_unlocked_costs(p.skill_tree);
    skill_tree_reset(p.skill_tree);

    // Armas
    to_refund += sum_weapon_upgrade_costs(p.weapons);
    p.weapons = WeaponData{};

    // Traje
    to_refund += sum_traje_upgrade_costs(p.traje);
    p.traje = TrajeData{};

    // Devolve
    karma_refund(p.karma, to_refund);

    // Recalcula stats
    recalc_player_stats(p, ...);
}
```

> Karma total inalterado. Apenas `available` aumenta.

### K5.4 — Services INI

```ini
[heal_basic]
kind = heal
cost = 50
label_pt = Curar
label_en = Heal

[buff_strength]
kind = buff
cost = 100
param = buff_strength_5min
label_pt = Bênção da Força (5 min)
label_en = Strength Blessing (5 min)

[potion_basic]
kind = potion
cost = 25
param = potion_health_minor
label_pt = Poção pequena
label_en = Minor potion

# ...

[respec]
kind = respec
cost = 0
label_pt = Dissolver karma investido
label_en = Dissolve invested karma
```

### K5.5 — Altar entity

Altars are static entities placed in zones. Different from service NPCs:
no dialogue, just interact-to-use.

```cpp
struct Altar {
    Vec2 position;
    AltarKind kind;
    bool active = true;
};

// Altar kinds:
//  Dissolution → respec (free)
//  Cura → heal
//  Bênção → buff
```

### K5.6 — Tests

- Service deduction works correctly
- Respec returns correct karma amount
- Respec resets skill tree, weapons, traje to zero
- Free service (cost = 0) succeeds without karma check
- Insufficient karma blocks service

### K5 DONE criteria

- [ ] Service NPC entity exists
- [ ] Altar entity exists
- [ ] All service kinds functional (heal, buff, potion, map, respec)
- [ ] Respec returns full karma to available
- [ ] Free respec at altar works
- [ ] Save/load preserves service-related state (e.g., revealed zones)
- [ ] Tests pass

---

## Sprint K6 — Zone scaling (floor/ceiling)

**Goal:** Each zone has a tier floor and ceiling. Enemies scale with
karma total up to the ceiling, then plateau.

### Files written

- `data/zones.ini` (new — defines floor/ceiling per zone)
- `src/systems/zone_scaling.hpp` (new)
- `src/systems/area_entry_system.hpp` (modified — apply scaling on entry)
- `tests/test_zone_scaling.cpp` (new)

### K6.1 — Zones INI

```ini
[vila_rosario]
tier_floor = 0
tier_ceiling = 0
karma_drop_multiplier = 0.0  # zona segura, sem inimigos

[centro_velho]
tier_floor = 1
tier_ceiling = 3
karma_drop_multiplier = 1.0

[parque_ibirapitanga]
tier_floor = 2
tier_ceiling = 4
karma_drop_multiplier = 1.0

[distrito_industrial]
tier_floor = 3
tier_ceiling = 6
karma_drop_multiplier = 1.2

[morro_vigilia]
tier_floor = 2
tier_ceiling = 5
karma_drop_multiplier = 1.0  # bordas escalam

[avenida_progresso]
tier_floor = 4
tier_ceiling = 7
karma_drop_multiplier = 1.3

[rio_turvao]
tier_floor = 5
tier_ceiling = 8
karma_drop_multiplier = 1.4

[torre_horizonte]
tier_floor = 6
tier_ceiling = 9
karma_drop_multiplier = 1.5
```

### K6.2 — Scaling function

```cpp
namespace mion {

struct ZoneScaling {
    int tier_floor;
    int tier_ceiling;
    float karma_drop_multiplier;
};

// Tier efetivo dos inimigos numa zona, dado o karma total do jogador.
inline int effective_tier(int64_t karma_total, const ZoneScaling& zs) {
    int player_tier = karma_to_tier(karma_total);  // f(total)
    int t = std::clamp(player_tier, zs.tier_floor, zs.tier_ceiling);
    return t;
}

// Aplica escalonamento aos stats de um inimigo.
inline void scale_enemy(EnemyType& enemy, int effective_tier) {
    float mult = 1.0f + 0.20f * (effective_tier - 1);  // +20% por tier
    enemy.hp = (int)(enemy.base_hp * mult);
    enemy.damage = (int)(enemy.base_damage * mult);
    enemy.karma_drop = (int)(enemy.base_karma_drop * mult);
}

} // namespace mion
```

### K6.3 — Apply on area entry

`src/systems/area_entry_system.hpp` modification:

When player enters zone:
1. Read zone scaling from `data/zones.ini`
2. Compute `effective_tier(player.karma.total, zone_scaling)`
3. Apply `scale_enemy` to all enemies in the zone
4. Apply `karma_drop_multiplier` to drops

### K6.4 — Tests

- Zone with floor=2, ceiling=5, player tier 1 → enemies tier 2
- Zone with floor=2, ceiling=5, player tier 4 → enemies tier 4
- Zone with floor=2, ceiling=5, player tier 8 → enemies tier 5 (capped)
- Karma drop scales with effective tier × multiplier

### K6 DONE criteria

- [ ] All 8 zones have floor/ceiling defined
- [ ] Effective tier respects floor and ceiling
- [ ] Enemy stats scale correctly
- [ ] Karma drop scales correctly
- [ ] Re-entering zone with higher karma re-scales enemies
- [ ] Tests pass

---

## Sprint K7 — Remove legacy systems

**Goal:** Remove XP, gold, equipment slots, traditional shop. All
gameplay flows now use karma exclusively.

### Files read

All output of K1-K6.

### Files written

- `src/components/progression.hpp` (XP fields removed)
- `src/core/save_data.hpp` (XP/gold removed, migration v8→v9)
- `src/core/save_migration.hpp` (added v8→v9)
- `src/components/equipment.hpp` (deprecated or repurposed)
- `src/components/item_bag.hpp` (reduced to consumables + special)
- `src/systems/shop_system.hpp` (deleted or converted)
- HUD updated to remove XP/gold display
- All references to removed fields cleaned up

### K7.1 — Remove XP

Search for `xp`, `experience`, `level_xp` references project-wide.
Remove from `PlayerData`, `SaveData`, HUD. Keep `karma_total` as level source.

### K7.2 — Remove gold

Same process. `data/items.ini` may need cleanup (drops that gave gold).

### K7.3 — Reduce inventory

`item_bag.hpp` keeps:
- Consumables (potions)
- Quest items (chave, item de respec narrativo)
- Lore documents (read but no use)

Removes:
- Armor pieces
- Weapons (since they're now universal)

`equipment.hpp` either deleted or reduced to a placeholder for future
expansion (like cosmetic accessories).

### K7.4 — Remove shop

`shop_system.hpp` deleted. Existing shop NPCs migrated to `ServiceNpc`
(K5 system). Items previously sold through shop now sold via service NPCs
or dropped naturally.

### K7.5 — Save migration v8 → v9

```cpp
inline void migrate_v8_to_v9(SaveData& save) {
    save.gold = 0;  // limpa
    // XP já não existe no struct v9; migration field leitura ignora
    save.equipment = {};  // limpa
    // Bag: filtra apenas consumíveis e quest items
    filter_bag_to_consumables(save.bag);
    save.version = 9;
}
```

### K7 DONE criteria

- [ ] No XP references in code or save
- [ ] No gold references
- [ ] Equipment slots removed or empty
- [ ] Bag only holds valid post-K7 items
- [ ] Shop system gone, replaced by services
- [ ] HUD shows only karma
- [ ] All existing tests still pass after cleanup
- [ ] Migration v8→v9 works
- [ ] No regressions in gameplay

---

## Sprint K8 — Karma faixa consequences

**Goal:** NPC dialogues vary by player's karma faixa. Faixa flag
exposed for content systems to consume (visual reactions, quest gates).

### Files written

- `src/systems/karma_faixa.hpp` (new)
- `src/systems/dialogue_system.hpp` (modified — consume faixa)
- `data/progression.ini` (modified — define faixa thresholds)
- `tests/test_faixa.cpp` (new)

### K8.1 — Faixa system

```cpp
namespace mion {

enum class KarmaFaixa : uint8_t {
    Baixa = 0,    // início do Ato 1
    Media = 1,    // meio do Ato 1
    Alta = 2      // final do Ato 1
};

inline KarmaFaixa karma_faixa(int64_t karma_total, const ProgressionConfig& cfg) {
    if (karma_total < cfg.faixa_media_threshold) return KarmaFaixa::Baixa;
    if (karma_total < cfg.faixa_alta_threshold) return KarmaFaixa::Media;
    return KarmaFaixa::Alta;
}

} // namespace mion
```

`data/progression.ini`:

```ini
[faixas]
faixa_media_threshold = 1500     # karma total para entrar em faixa média
faixa_alta_threshold = 5000      # karma total para entrar em faixa alta
```

### K8.2 — Dialogue variations by faixa

**VERIFY:** dialogue system structure. Likely uses keys to look up
strings from locale files.

Extend dialogue lookup to include faixa suffix:

```cpp
// Tenta buscar variação por faixa, com fallback para versão base.
inline std::string dialogue_for_faixa(const std::string& base_key,
                                       KarmaFaixa faixa,
                                       const Locale& loc) {
    const char* suffix;
    switch (faixa) {
        case KarmaFaixa::Baixa: suffix = "_baixa"; break;
        case KarmaFaixa::Media: suffix = "_media"; break;
        case KarmaFaixa::Alta: suffix = "_alta"; break;
    }
    std::string key = base_key + suffix;
    if (loc.has(key)) return loc.get(key);
    return loc.get(base_key);  // fallback
}
```

This way, dialogues only need faixa-specific entries when the variation
matters. Default dialogues fall back automatically.

### K8.3 — Faixa exposed for systems

Future visual systems (deferred) can query `karma_faixa(p.karma.total, cfg)`
and apply effects. K8 only exposes the value; visual application is
post-Act-1 work.

### K8 DONE criteria

- [ ] `karma_faixa` function returns correct value per threshold
- [ ] Dialogue lookup with `_faixa` suffix works
- [ ] Fallback to base key works when no variation exists
- [ ] Tests pass

---

# Phase B — Act 1 content

> Content sprints depend on output from `02_skill_tree_definitive.md`,
> `03_zones_content.md`, `04_dialogues.md`. They implement the structure
> in code; content fills in.

---

## Sprint C1 — São Chico blockout

**Goal:** 8 zones wired into the WorldMap. Zone transitions work.
Zones are empty (no enemies, no NPCs yet) but navigable.

### Files read

- `src/world/world_map.hpp`
- `src/world/world_area.hpp`
- `src/world/zone_manager.hpp`
- `src/world/tilemap.hpp`

### Files written

- `data/world/sao_chico.ini` (new — defines 8 zones)
- `src/world/world_map.hpp` (modified — load São Chico)
- 8 placeholder tilemaps via `tools/` procedural generator
- `tests/test_world_navigation.cpp` (new)

### C1.1 — World definition

```ini
# data/world/sao_chico.ini

[world]
name_pt = São Chico
name_en = São Chico

[zone.vila_rosario]
display_name_pt = Vila Rosário
display_name_en = Vila Rosário
tilemap = tilemaps/vila_rosario.json
size = 64x64
spawn_point = 32,32

[zone.centro_velho]
# ...

# 8 zonas total
```

### C1.2 — Zone transitions

Use existing `door_zone` or equivalent. Door from zone A → zone B at
specific tilemap coordinates.

### C1.3 — Procedural tilemaps

Use `tools/` to generate placeholder tilemaps:
- Vila Rosário: organic, rural-ish
- Centro Velho: grid-like, urban
- Parque: open with trees
- Industrial: blocky factories
- Morro: irregular, vertical
- Avenida: wide streets
- Rio: linear with water
- Torre: vertical multi-floor

These are functional placeholders. Final art comes in publish phase.

### C1 DONE criteria

- [ ] All 8 zones loaded
- [ ] Player can travel between all zones via doors/transitions
- [ ] No crashes navigating
- [ ] Save persists current zone
- [ ] Tests pass

---

## Sprints C2-C6 — Per-zone content

These follow the same pattern. Content (enemies, mini-boss, NPCs,
documents, action events) is detailed in `03_zones_content.md`. Below
is the structural template.

### Per-zone implementation pattern

For each zone:

1. **Spawn enemies** — list from `03_zones_content.md`, place via
   tilemap markers or spawner system
2. **Implement mini-boss** — unique behavior, possibly new
   `ai_boss_<name>.hpp` if mechanics differ from generic
3. **Place NPCs** — service NPCs (using K5 system) and dialogue NPCs
4. **Add documents** — collectible/readable lore items in bag
5. **Wire dialogues** — base dialogues (faixa variations come later)
6. **Action events** — scripted moments specific to the zone (covered in C8)

### Files read per zone sprint

- `03_zones_content.md` § for the zone(s)
- Existing systems: ai_combat, ai_patrol, ai_boss

### Files written per zone sprint

- `src/entities/<zone>_enemies.hpp` if zone has unique enemies
- `src/systems/ai_boss_<bossname>.hpp` if boss has unique mechanics
- `data/enemies.ini` (additions for zone-specific enemies)
- `data/npcs.ini` (additions for zone NPCs)
- `data/documents.ini` (additions for collectible lore)
- Tests for new bosses and enemies

---

## Sprint C7 — Allies system

**Goal:** 6 recruitable companions (Lia, Dra. Marina, Dona Marta,
Padre Inácio, Zé Boticário, Sindicalista). Follow AI, combat behavior,
swap in/out, contextual dialogue.

### Files read

- `src/entities/actor.hpp` and ally-related systems
- `src/systems/ai_combat.hpp`

### Files written

- `src/entities/ally.hpp` (new — extends Actor concept)
- `src/systems/ally_follow_ai.hpp` (new)
- `src/systems/ally_combat_ai.hpp` (new)
- `src/systems/ally_roster.hpp` (new — manages active ally)
- `data/allies.ini` (new — defines 6 allies)
- `tests/test_allies.cpp`

### C7.1 — Ally entity

```cpp
struct Ally {
    std::string id;          // "lia", "marta", etc.
    std::string name_pt;
    std::string name_en;
    AllyRole role;           // Tank, Support, DPS, Utility
    int hp_max;
    int damage;
    float move_speed;
    // ... outros stats
    AllyAi ai;               // comportamento padrão
};

enum class AllyRole {
    Tank,         // Marta
    Support,      // Lia, Inácio
    DPS,          // Sindicalista
    Utility,      // Marina, Zé
};
```

### C7.2 — Recruitment flow

Each ally has a recruitment moment in the story (defined in
`03_zones_content.md`). After recruitment, ally is available in the
roster.

```cpp
struct AllyRoster {
    std::vector<std::string> recruited;  // ids dos aliados recrutados
    std::optional<std::string> active;   // id do aliado ativo (max 1)
};

inline void recruit_ally(AllyRoster& r, const std::string& id) {
    if (std::find(r.recruited.begin(), r.recruited.end(), id) == r.recruited.end()) {
        r.recruited.push_back(id);
    }
}

inline void set_active_ally(AllyRoster& r, const std::string& id) {
    if (std::find(r.recruited.begin(), r.recruited.end(), id) != r.recruited.end()) {
        r.active = id;
    }
}
```

### C7.3 — Follow AI

Ally follows player at a distance. Maintains spacing. Pathfinds around
obstacles.

```cpp
inline void update_ally_follow(Ally& a, const PlayerData& p, World& w, float dt) {
    Vec2 to_player = p.position - a.position;
    float dist = length(to_player);

    if (dist > kFollowDistance) {
        Vec2 dir = normalize(to_player);
        a.position += dir * a.move_speed * dt;
    }
}
```

### C7.4 — Combat AI per role

- Tank: gets close, attracts aggro, takes hits
- Support: stays back, casts buffs/heals on cooldown
- DPS: engages closest enemy, attacks aggressively
- Utility: situational; throws debuffs, traps

### C7.5 — Contextual dialogue

Allies speak short lines when:
- Entering a zone
- Encountering specific enemies
- Player makes a major choice
- Ally HP critical

These are short reactions defined in `04_dialogues.md`.

### C7 DONE criteria

- [ ] Ally entity working
- [ ] Roster manages recruited / active
- [ ] Follow AI works in all zones
- [ ] Combat AI works per role
- [ ] Contextual dialogues fire
- [ ] Save preserves roster + active ally
- [ ] Tests pass

---

## Sprint C8 — Action events + choices

**Goal:** Implement scripted story moments and choice consequences:
faction in Avenida (Z5), hub defense waves (Z4), Conselheiro absorption
(Z7), and other narrative beats.

### Files written

- `src/systems/scripted_event.hpp` (new — scripted event runner)
- `src/systems/choice_consequence.hpp` (new — track choices, apply effects)
- `data/events/` (new directory — event scripts)
- `tests/test_scripted_events.cpp` (new)

### C8.1 — Scripted event system

A scripted event has triggers and steps. Triggers are conditions
(entered zone X, defeated enemy Y, talked to NPC Z). Steps are actions
(spawn enemy, play dialogue, change state, apply consequence).

```cpp
struct EventTrigger {
    enum Kind { ZoneEnter, EnemyDefeated, DialogueChoice, KarmaThreshold } kind;
    std::string param;
};

struct EventStep {
    enum Kind { SpawnEnemy, PlayDialogue, SetFlag, ApplyConsequence } kind;
    std::string param;
};

struct ScriptedEvent {
    std::string id;
    std::vector<EventTrigger> triggers;  // ALL must be true
    std::vector<EventStep> steps;
    bool fired = false;  // one-shot
};
```

### C8.2 — Choice consequences

```cpp
struct ChoiceState {
    // Faction in Avenida Progresso
    enum FactionChoice { NotMet, Confronted, Negotiated };
    FactionChoice faction = NotMet;

    // Hub defense at Morro
    bool hub_defended = false;
    int hub_morale = 100;  // 0-100, drops if defenses fail

    // Conselheiro absorption (always happens, but with variations)
    bool conselheiro_absorbed = false;
    bool conselheiro_understanding = false;  // recebeu os flashes
};
```

### C8.3 — Hub defense waves

When triggered (via story progression), waves of corrupted enemies
attack the hub barricades. Player + recruited allies defend.

```cpp
struct HubDefenseWave {
    int wave_number;
    std::vector<EnemySpawn> enemies;
    int duration_seconds;
    int reward_karma;
};
```

Failure = morale drops, some NPCs become unavailable temporarily.
Success = morale stays, ally relationships improve.

### C8.4 — Conselheiro absorption

Final boss. After HP depletes, special phase:

```cpp
inline void start_conselheiro_absorption(GameContext& ctx) {
    // 1. Combate parado, cutscene-mode
    // 2. Diálogo: O Conselheiro derrotado mas consciente
    // 3. Player escolhe: absorver (Dante puxa o karma)
    // 4. Cena de flashes (ver 04_dialogues.md)
    // 5. Conselheiro esvaziado, fim do Ato 1
    // 6. ChoiceState.conselheiro_absorbed = true
    // 7. ChoiceState.conselheiro_understanding = true
}
```

### C8 DONE criteria

- [ ] ScriptedEvent system works (triggers + steps)
- [ ] Choice consequences tracked in save
- [ ] Hub defense waves trigger and play out
- [ ] Conselheiro absorption sequence works
- [ ] Faction choice in Avenida tracked and affects later content
- [ ] Tests pass

---

## Sprint C9 — Dialogues integration

**Goal:** All dialogues from `04_dialogues.md` are loaded into the
locale system. PT-BR / EN switching works at runtime.

### Files read

- `src/core/locale.hpp`
- `src/systems/dialogue_system.hpp`
- `04_dialogues.md` (definitive content)

### Files written

- `data/locale/pt-br.json` (or .ini, depending on engine convention)
- `data/locale/en.json`
- `src/systems/dialogue_system.hpp` (modified — full integration with K8 faixa)

### C9.1 — Dialogue keys

All dialogues from `04_dialogues.md` are extracted into locale files.
Keys follow naming convention:

```
dlg_<character>_<context>_<id>
```

Example:
- `dlg_marta_intro_001`
- `dlg_inacio_question_karma_002_alta`
- `dlg_lia_combat_critical_005`

### C9.2 — Locale structure

```json
// data/locale/pt-br.json
{
  "dlg_marta_intro_001": "Olha, eu não sei o que é isso que tá tomando conta da cidade...",
  "dlg_inacio_question_karma_002": "Se essa energia é sofrimento condensado, usar ela pra se fortalecer é... o quê, exatamente?",
  "dlg_inacio_question_karma_002_alta": "Você tá usando isso há tempo demais. Não me interpreta mal — eu não tenho julgamento. Mas sente. Você sente?",
  ...
}
```

### C9.3 — Runtime switching

User in main menu picks language. Locale system loads chosen file at
boot. All dialogue lookups go through `loc.get(key)`.

```cpp
namespace mion {

struct Locale {
    std::string current_lang = "pt-br";
    std::unordered_map<std::string, std::string> entries;

    void load(const std::string& lang);
    std::string get(const std::string& key) const;
    bool has(const std::string& key) const;
};

} // namespace mion
```

### C9.4 — Faixa-aware lookup

Dialogue system, when fetching a line, calls:
```cpp
std::string final = dialogue_for_faixa(base_key, current_faixa, locale);
```

Falls back to base key if no faixa-specific variant exists.

### C9 DONE criteria

- [ ] All dialogues from `04_dialogues.md` in locale files
- [ ] PT-BR loads correctly
- [ ] EN loads correctly
- [ ] Runtime language switching works
- [ ] Faixa variations resolve correctly
- [ ] Tests pass

---

# Sprint completion checklist (universal)

Before marking any sprint DONE:

- [ ] All sub-sprint tests pass
- [ ] Existing test suite passes (no regressions)
- [ ] Manual playtest of new feature
- [ ] CLAUDE.md non-negotiable rules respected
- [ ] Documentation in sync with code (this doc updated if reality
      diverged from plan)
- [ ] Save/load round-trip works
- [ ] No new TODO/HACK comments
- [ ] Update `00_master.md` progress tracker

---

# Risk register

| Risk | Sprint | Mitigation |
|------|--------|------------|
| Actor split (Sprint 5) blocking K1 | K1 | K1 plan can adapt; KarmaData added to whatever Player struct exists |
| Save migration chain breaks | All K | Each migration tested independently + chained |
| Procedural placeholders block playtest | C1+ | Use simplest possible procedural; visual quality not the goal |
| Dialogue volume overwhelms locale system | C9 | Keys structured for batch loading; lazy load if needed |
| Allies AI making zones too easy | C7 | Tune ally damage/HP separately from player |
| Conselheiro absorption too gimmicky | C8 | Detailed in `03_zones_content.md`; iterate on feel |
| Choice consequences too subtle | C8 | Telegraph clearly via dialogue and visual feedback |
