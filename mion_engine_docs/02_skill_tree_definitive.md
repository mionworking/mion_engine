# 02 — Skill Tree (Definitive)

All skill nodes for the 3 trees (Warrior / Mage / Archer), Y-shaped.
Each tree has a common trunk (pre-Y) that branches into two
specializations after the bifurcation point.

> Companion to `01_implementation_plan.md` § Sprint K2.
> This doc is the **content** (every node defined). K2 implements the
> structure that loads this content from INI files.

---

## Structure overview

Each tree:
- **Pre-Y trunk:** 5 shared nodes (the foundation of the class)
- **Bifurcation:** at node 5, the player chooses branch A or B
- **Branch A:** 7 nodes (one specialization)
- **Branch B:** 7 nodes (the other specialization)
- **Total per tree:** 19 nodes
- **Total across 3 trees:** 57 nodes
- **Synergies:** 12 cross-tree pairs that grant passive bonuses when both nodes are unlocked

### Karma cost curve

Costs scale within each tree:

| Position in tree | Cost range |
|------------------|------------|
| Pre-Y nodes 0-4  | 50, 75, 100, 150, 200 |
| Branch nodes 0-2 | 250, 300, 400 |
| Branch nodes 3-5 | 500, 650, 800 |
| Branch capstone (node 6) | 1200 |

Total to fully complete one branch (all pre-Y + one branch): **5,275 karma**.
Total to complete both branches of one tree: **10,150 karma**.
Total all 3 trees fully maxed: ~30,500 karma. Player won't reach this in
Act 1 — that's intentional. Forces specialization.

### Effect DSL reference

Effects in INI use a simple text DSL parsed by the engine:

| DSL          | Meaning                                           |
|--------------|---------------------------------------------------|
| `hp_max +N`        | flat HP increase                            |
| `hp_max_pct +N`    | percentage HP increase                      |
| `damage +N`        | flat damage increase to all weapons         |
| `damage_pct +N`    | percentage damage increase                  |
| `damage_pct_<cat> +N` | damage % for specific weapon category    |
| `armor +N`         | flat armor                                  |
| `crit_chance +N`   | crit chance percent                         |
| `crit_damage +N`   | crit damage multiplier percent              |
| `move_speed_pct +N`| movement speed                              |
| `attack_speed_pct +N` | attack speed                             |
| `cooldown_pct -N`  | cooldown reduction                          |
| `mana_max +N`      | flat mana                                   |
| `mana_regen +N`    | mana regeneration per second                |
| `stamina_max +N`   | flat stamina                                |
| `lifesteal +N`     | percent damage returned as HP               |
| `dodge +N`         | dodge chance percent                        |
| `resist_<element> +N` | elemental resistance                     |
| `grant_skill <id>` | unlocks an active skill                     |
| `grant_passive <id>` | unlocks a passive effect                  |
| `damage_pct_low_hp +N` | conditional: bonus damage when HP < 50% |
| `damage_pct_full_hp +N` | conditional: bonus damage when HP > 90% |
| `vs_<enemy_type> +N` | bonus damage vs specific enemy types      |

Multiple effects per node separated by `|` in INI:
```
effect = hp_max_pct +20 | armor +5
```

---

# Warrior Tree

**Identity:** physical damage, durability, melee. The class of weight
and impact. Karma materialized as muscle, scars, and steel.

**Pre-Y identity:** raw martial competence. Damage, HP, basic skills.

**Branch A — Berserker:** offense focus. Lifesteal, low-HP bonuses,
aggressive abilities. The warrior who feeds on the fight.

**Branch B — Guardian:** defense focus. Armor, control, allies. The
warrior who anchors the line.

---

## Warrior — Pre-Y trunk

### `warrior:0` — Vigor Bruto / Raw Vigor
- **Cost:** 50 karma
- **Prereqs:** none (entry node)
- **Branch:** pre_y
- **Description (PT):** Aumenta HP máximo em 20%.
- **Description (EN):** +20% maximum HP.
- **Effect:** `hp_max_pct +20`

### `warrior:1` — Investida / Charge
- **Cost:** 75 karma
- **Prereqs:** `warrior:0`
- **Branch:** pre_y
- **Description (PT):** Avança 5m em linha reta causando dano e atordoando o primeiro alvo. Cooldown 8s.
- **Description (EN):** Dash 5m forward, dealing damage and stunning first target. 8s cooldown.
- **Effect:** `grant_skill charge`

### `warrior:2` — Pele de Couro / Hardened Hide
- **Cost:** 100 karma
- **Prereqs:** `warrior:0`
- **Branch:** pre_y
- **Description (PT):** +10 de armadura. Reduz dano físico recebido.
- **Description (EN):** +10 armor. Reduces physical damage taken.
- **Effect:** `armor +10`

### `warrior:3` — Pancada Pesada / Heavy Strike
- **Cost:** 150 karma
- **Prereqs:** `warrior:1`
- **Branch:** pre_y
- **Description (PT):** Próximo ataque após investida causa +50% de dano.
- **Description (EN):** Next attack after Charge deals +50% damage.
- **Effect:** `grant_passive heavy_strike_after_charge`

### `warrior:4` — Espinha de Aço / Iron Spine (bifurcation node)
- **Cost:** 200 karma
- **Prereqs:** `warrior:2` AND `warrior:3`
- **Branch:** pre_y
- **Description (PT):** +15% de HP máximo e +5% de dano. Desbloqueia o caminho do Berserker ou do Guardian.
- **Description (EN):** +15% max HP and +5% damage. Unlocks Berserker or Guardian path.
- **Effect:** `hp_max_pct +15 | damage_pct +5`

> After this node, the player chooses A (Berserker) or B (Guardian).
> Both branches are visible but locked until at least one is invested.

---

## Warrior — Branch A: Berserker

### `warrior:5a` — Sangue Quente / Hot Blood
- **Cost:** 250 karma
- **Prereqs:** `warrior:4`
- **Branch:** a
- **Description (PT):** +10% velocidade de ataque. Quando abaixo de 50% de HP, +20% de dano.
- **Description (EN):** +10% attack speed. When HP < 50%, +20% damage.
- **Effect:** `attack_speed_pct +10 | damage_pct_low_hp +20`

### `warrior:6a` — Sede de Sangue / Bloodlust
- **Cost:** 300 karma
- **Prereqs:** `warrior:5a`
- **Branch:** a
- **Description (PT):** Cada inimigo morto restaura 5% do HP máximo.
- **Description (EN):** Each enemy killed restores 5% max HP.
- **Effect:** `grant_passive heal_on_kill_5pct`

### `warrior:7a` — Ferocidade / Ferocity
- **Cost:** 400 karma
- **Prereqs:** `warrior:5a`
- **Branch:** a
- **Description (PT):** +15% chance de crítico em ataques corpo a corpo.
- **Description (EN):** +15% melee crit chance.
- **Effect:** `crit_chance +15`

### `warrior:8a` — Frenesi / Frenzy
- **Cost:** 500 karma
- **Prereqs:** `warrior:6a` OR `warrior:7a`
- **Branch:** a
- **Description (PT):** Acertos críticos reduzem cooldowns em 1s.
- **Description (EN):** Critical hits reduce cooldowns by 1s.
- **Effect:** `grant_passive crit_reduces_cd`

### `warrior:9a` — Sem Recuo / No Retreat
- **Cost:** 650 karma
- **Prereqs:** `warrior:8a`
- **Branch:** a
- **Description (PT):** Investida não tem cooldown enquanto abaixo de 30% de HP.
- **Description (EN):** Charge has no cooldown while HP < 30%.
- **Effect:** `grant_passive no_charge_cd_below_30pct`

### `warrior:10a` — Roubo de Vida / Lifesteal
- **Cost:** 800 karma
- **Prereqs:** `warrior:9a`
- **Branch:** a
- **Description (PT):** 10% do dano causado retorna como HP.
- **Description (EN):** 10% of damage dealt returns as HP.
- **Effect:** `lifesteal +10`

### `warrior:11a` — Avatar da Fúria / Avatar of Wrath (capstone)
- **Cost:** 1200 karma
- **Prereqs:** `warrior:10a`
- **Branch:** a
- **Description (PT):** Quando matar um inimigo enquanto abaixo de 50% de HP, recebe 5s de invulnerabilidade e +50% de dano.
- **Description (EN):** Killing an enemy while HP < 50% grants 5s invulnerability and +50% damage.
- **Effect:** `grant_skill avatar_wrath`

---

## Warrior — Branch B: Guardian

### `warrior:5b` — Postura Defensiva / Defensive Stance
- **Cost:** 250 karma
- **Prereqs:** `warrior:4`
- **Branch:** b
- **Description (PT):** +20 de armadura. -10% velocidade de movimento.
- **Description (EN):** +20 armor. -10% movement speed.
- **Effect:** `armor +20 | move_speed_pct -10`

### `warrior:6b` — Provocar / Taunt
- **Cost:** 300 karma
- **Prereqs:** `warrior:5b`
- **Branch:** b
- **Description (PT):** Força inimigos próximos a te atacar por 4s. Cooldown 12s.
- **Description (EN):** Forces nearby enemies to target you for 4s. 12s cooldown.
- **Effect:** `grant_skill taunt`

### `warrior:7b` — Bloqueio / Block
- **Cost:** 400 karma
- **Prereqs:** `warrior:5b`
- **Branch:** b
- **Description (PT):** 15% de chance de bloquear ataques físicos completamente.
- **Description (EN):** 15% chance to fully block physical attacks.
- **Effect:** `grant_passive block_15pct`

### `warrior:8b` — Vingança / Vengeance
- **Cost:** 500 karma
- **Prereqs:** `warrior:6b` OR `warrior:7b`
- **Branch:** b
- **Description (PT):** Receber dano gera retaliação de 30% do dano causado em volta.
- **Description (EN):** Taking damage retaliates with 30% of damage taken to nearby enemies.
- **Effect:** `grant_passive damage_retaliation_30`

### `warrior:9b` — Resistência / Resilience
- **Cost:** 650 karma
- **Prereqs:** `warrior:8b`
- **Branch:** b
- **Description (PT):** +25% de resistência a stun e knockback.
- **Description (EN):** +25% stun and knockback resistance.
- **Effect:** `resist_stun +25 | resist_knockback +25`

### `warrior:10b` — Escudo Vivo / Living Shield
- **Cost:** 800 karma
- **Prereqs:** `warrior:9b`
- **Branch:** b
- **Description (PT):** Aliados próximos recebem 30% menos dano.
- **Description (EN):** Nearby allies take 30% less damage.
- **Effect:** `grant_passive ally_damage_reduction_30`

### `warrior:11b` — Bastião / Bastion (capstone)
- **Cost:** 1200 karma
- **Prereqs:** `warrior:10b`
- **Branch:** b
- **Description (PT):** Ao receber um golpe que mataria, sobrevive com 1 de HP e fica imune por 3s. Cooldown 90s.
- **Description (EN):** A killing blow leaves you at 1 HP and grants 3s invulnerability. 90s cooldown.
- **Effect:** `grant_skill bastion_revive`

---

# Mage Tree

**Identity:** elemental damage, control, distance. Karma channeled
through intent. The class that bends energy.

**Pre-Y identity:** basic spellcasting, mana, elemental fundamentals.

**Branch A — Elementalist:** burst and AoE. Fire, lightning, ice. The
mage who erases enemies in waves.

**Branch B — Warlock:** DoT and debuff. Curses, poisons, life drain.
The mage who watches enemies wither.

---

## Mage — Pre-Y trunk

### `mage:0` — Despertar / Awakening
- **Cost:** 50 karma
- **Prereqs:** none
- **Branch:** pre_y
- **Description (PT):** +30 de mana máxima e +1 mana por segundo.
- **Description (EN):** +30 max mana and +1 mana per second.
- **Effect:** `mana_max +30 | mana_regen +1`

### `mage:1` — Centelha / Spark
- **Cost:** 75 karma
- **Prereqs:** `mage:0`
- **Branch:** pre_y
- **Description (PT):** Spell básico de dano elétrico em alcance médio. Custo 10 mana.
- **Description (EN):** Basic ranged lightning spell. 10 mana cost.
- **Effect:** `grant_skill spark`

### `mage:2` — Foco Mental / Mental Focus
- **Cost:** 100 karma
- **Prereqs:** `mage:0`
- **Branch:** pre_y
- **Description (PT):** -10% de cooldown em todos os spells.
- **Description (EN):** -10% cooldown on all spells.
- **Effect:** `cooldown_pct -10`

### `mage:3` — Toque Arcano / Arcane Touch
- **Cost:** 150 karma
- **Prereqs:** `mage:1`
- **Branch:** pre_y
- **Description (PT):** Acertar com spell devolve 5% da mana gasta.
- **Description (EN):** Spell hits return 5% of mana spent.
- **Effect:** `grant_passive mana_refund_on_hit`

### `mage:4` — Vontade Forjada / Forged Will (bifurcation)
- **Cost:** 200 karma
- **Prereqs:** `mage:2` AND `mage:3`
- **Branch:** pre_y
- **Description (PT):** +20% de dano mágico. Desbloqueia o caminho do Elementalista ou do Warlock.
- **Description (EN):** +20% magic damage. Unlocks Elementalist or Warlock path.
- **Effect:** `damage_pct_magic +20`

---

## Mage — Branch A: Elementalist

### `mage:5a` — Fogo Vivo / Living Flame
- **Cost:** 250 karma
- **Prereqs:** `mage:4`
- **Branch:** a
- **Description (PT):** Spells de fogo causam +30% de dano. Inimigos atingidos queimam por 3s.
- **Description (EN):** Fire spells deal +30% damage. Hit enemies burn for 3s.
- **Effect:** `damage_pct_fire +30 | grant_passive burn_on_fire_hit`

### `mage:6a` — Tempestade / Storm
- **Cost:** 300 karma
- **Prereqs:** `mage:5a`
- **Branch:** a
- **Description (PT):** Centelha agora pula para até 3 inimigos próximos.
- **Description (EN):** Spark now chains to up to 3 nearby enemies.
- **Effect:** `grant_passive spark_chain_3`

### `mage:7a` — Frio Cortante / Biting Cold
- **Cost:** 400 karma
- **Prereqs:** `mage:5a`
- **Branch:** a
- **Description (PT):** Spells de gelo lentificam alvos em 30% por 4s.
- **Description (EN):** Ice spells slow targets by 30% for 4s.
- **Effect:** `grant_passive ice_slow_30`

### `mage:8a` — Conjuração Veloz / Swift Casting
- **Cost:** 500 karma
- **Prereqs:** `mage:6a` OR `mage:7a`
- **Branch:** a
- **Description (PT):** -20% no tempo de conjuração de todos os spells.
- **Description (EN):** -20% spell casting time.
- **Effect:** `cast_speed_pct +20`

### `mage:9a` — Explosão / Detonation
- **Cost:** 650 karma
- **Prereqs:** `mage:8a`
- **Branch:** a
- **Description (PT):** Inimigos mortos por dano elemental explodem causando 50% do dano em volta.
- **Description (EN):** Enemies killed by elemental damage explode for 50% damage to nearby foes.
- **Effect:** `grant_passive elemental_kill_explosion`

### `mage:10a` — Maestria Elemental / Elemental Mastery
- **Cost:** 800 karma
- **Prereqs:** `mage:9a`
- **Branch:** a
- **Description (PT):** +40% de dano mágico contra alvos com efeito elemental ativo (queimando, congelado, eletrocutado).
- **Description (EN):** +40% magic damage against targets with active elemental effect.
- **Effect:** `damage_pct_magic_vs_elemental +40`

### `mage:11a` — Avatar dos Elementos / Avatar of Elements (capstone)
- **Cost:** 1200 karma
- **Prereqs:** `mage:10a`
- **Branch:** a
- **Description (PT):** Habilidade ativa: por 8s, todos os spells custam metade de mana e têm cooldown reduzido em 50%. Cooldown 60s.
- **Description (EN):** Active: for 8s, all spells cost half mana and have 50% reduced cooldown. 60s cooldown.
- **Effect:** `grant_skill elemental_overflow`

---

## Mage — Branch B: Warlock

### `mage:5b` — Maldição / Curse
- **Cost:** 250 karma
- **Prereqs:** `mage:4`
- **Branch:** b
- **Description (PT):** Spell que enfraquece o alvo: -25% de dano e -20% de defesa por 8s.
- **Description (EN):** Curse target: -25% damage and -20% defense for 8s.
- **Effect:** `grant_skill curse`

### `mage:6b` — Dreno Vital / Life Drain
- **Cost:** 300 karma
- **Prereqs:** `mage:5b`
- **Branch:** b
- **Description (PT):** Canalização que drena HP do alvo (15 por segundo) e te cura na mesma quantidade.
- **Description (EN):** Channeled drain: 15 HP/sec from target, heals you for same.
- **Effect:** `grant_skill life_drain`

### `mage:7b` — Veneno / Poison
- **Cost:** 400 karma
- **Prereqs:** `mage:5b`
- **Branch:** b
- **Description (PT):** Ataques mágicos têm 20% de chance de envenenar (10 dano/s por 5s).
- **Description (EN):** Magic attacks have 20% chance to poison (10 dmg/s for 5s).
- **Effect:** `grant_passive poison_on_hit_20pct`

### `mage:8b` — Aflição Crescente / Mounting Affliction
- **Cost:** 500 karma
- **Prereqs:** `mage:6b` OR `mage:7b`
- **Branch:** b
- **Description (PT):** Efeitos de DoT (queimadura, veneno, dreno) duram 50% mais.
- **Description (EN):** DoT effects last 50% longer.
- **Effect:** `dot_duration_pct +50`

### `mage:9b` — Marca da Morte / Mark of Death
- **Cost:** 650 karma
- **Prereqs:** `mage:8b`
- **Branch:** b
- **Description (PT):** Inimigos amaldiçoados recebem 25% mais dano de todas as fontes.
- **Description (EN):** Cursed enemies take 25% more damage from all sources.
- **Effect:** `grant_passive cursed_take_more_damage`

### `mage:10b` — Colheita / Harvest
- **Cost:** 800 karma
- **Prereqs:** `mage:9b`
- **Branch:** b
- **Description (PT):** Inimigos mortos enquanto sob DoT regeneram 10% da sua mana.
- **Description (EN):** Enemies killed while under DoT restore 10% of your mana.
- **Effect:** `grant_passive mana_on_dot_kill`

### `mage:11b` — Avatar da Praga / Avatar of Plague (capstone)
- **Cost:** 1200 karma
- **Prereqs:** `mage:10b`
- **Branch:** b
- **Description (PT):** DoTs aplicados se espalham para inimigos próximos quando o alvo morre.
- **Description (EN):** DoTs spread to nearby enemies when the afflicted target dies.
- **Effect:** `grant_passive dot_spread_on_death`

---

# Archer Tree

**Identity:** ranged precision, mobility, traps. Karma as motion and
distance. The class that controls space.

**Pre-Y identity:** archery fundamentals, dodge, basic ranged skills.

**Branch A — Sharpshooter:** single-target, crit, precision. The archer
who ends fights with one shot.

**Branch B — Trapper:** AoE, traps, terrain control. The archer who
makes the battlefield itself dangerous.

---

## Archer — Pre-Y trunk

### `archer:0` — Olhar Atento / Keen Eye
- **Cost:** 50 karma
- **Prereqs:** none
- **Branch:** pre_y
- **Description (PT):** +5% chance de crítico. +10% de dano à distância.
- **Description (EN):** +5% crit chance. +10% ranged damage.
- **Effect:** `crit_chance +5 | damage_pct_bow +10`

### `archer:1` — Tiro Rápido / Quick Shot
- **Cost:** 75 karma
- **Prereqs:** `archer:0`
- **Branch:** pre_y
- **Description (PT):** Habilidade ativa: dispara 3 flechas em sequência rápida. Cooldown 6s.
- **Description (EN):** Active: 3 quick shots in sequence. 6s cooldown.
- **Effect:** `grant_skill quick_shot`

### `archer:2` — Pés Leves / Light Feet
- **Cost:** 100 karma
- **Prereqs:** `archer:0`
- **Branch:** pre_y
- **Description (PT):** +15% velocidade de movimento.
- **Description (EN):** +15% movement speed.
- **Effect:** `move_speed_pct +15`

### `archer:3` — Esquiva / Dodge
- **Cost:** 150 karma
- **Prereqs:** `archer:2`
- **Branch:** pre_y
- **Description (PT):** Dash curto que esquiva projéteis. Cooldown 5s.
- **Description (EN):** Short dash that evades projectiles. 5s cooldown.
- **Effect:** `grant_skill dodge_roll`

### `archer:4` — Postura do Caçador / Hunter's Stance (bifurcation)
- **Cost:** 200 karma
- **Prereqs:** `archer:1` AND `archer:3`
- **Branch:** pre_y
- **Description (PT):** +10% de dano e +5% chance de crítico. Desbloqueia o caminho do Sharpshooter ou do Trapper.
- **Description (EN):** +10% damage and +5% crit chance. Unlocks Sharpshooter or Trapper path.
- **Effect:** `damage_pct +10 | crit_chance +5`

---

## Archer — Branch A: Sharpshooter

### `archer:5a` — Mira Letal / Deadly Aim
- **Cost:** 250 karma
- **Prereqs:** `archer:4`
- **Branch:** a
- **Description (PT):** +30% de dano crítico.
- **Description (EN):** +30% crit damage.
- **Effect:** `crit_damage +30`

### `archer:6a` — Disparo Perfurante / Piercing Shot
- **Cost:** 300 karma
- **Prereqs:** `archer:5a`
- **Branch:** a
- **Description (PT):** Flechas atravessam até 2 inimigos.
- **Description (EN):** Arrows pierce up to 2 enemies.
- **Effect:** `grant_passive arrow_pierce_2`

### `archer:7a` — Distância Cruel / Cruel Distance
- **Cost:** 400 karma
- **Prereqs:** `archer:5a`
- **Branch:** a
- **Description (PT):** +25% de dano contra alvos a mais de 8m.
- **Description (EN):** +25% damage to targets >8m away.
- **Effect:** `damage_pct_far +25`

### `archer:8a` — Concentração / Concentration
- **Cost:** 500 karma
- **Prereqs:** `archer:6a` OR `archer:7a`
- **Branch:** a
- **Description (PT):** Parado por 2s, próximo tiro tem 100% de chance de crítico.
- **Description (EN):** Standing still 2s, next shot has 100% crit chance.
- **Effect:** `grant_passive guaranteed_crit_after_still`

### `archer:9a` — Disparo Carregado / Charged Shot
- **Cost:** 650 karma
- **Prereqs:** `archer:8a`
- **Branch:** a
- **Description (PT):** Habilidade ativa: carrega por 2s e dispara um tiro de alto dano que perfura todos os alvos em linha. Cooldown 15s.
- **Description (EN):** Active: charge 2s, fire piercing line shot. 15s cooldown.
- **Effect:** `grant_skill charged_shot`

### `archer:10a` — Cabeça Fria / Cool Head
- **Cost:** 800 karma
- **Prereqs:** `archer:9a`
- **Branch:** a
- **Description (PT):** Críticos restauram 5 mana e reduzem cooldowns em 0.5s.
- **Description (EN):** Crits restore 5 mana and reduce cooldowns by 0.5s.
- **Effect:** `grant_passive crit_restores_resources`

### `archer:11a` — Avatar da Caça / Avatar of the Hunt (capstone)
- **Cost:** 1200 karma
- **Prereqs:** `archer:10a`
- **Branch:** a
- **Description (PT):** O primeiro acerto em qualquer inimigo é sempre um crítico. Reseta ao trocar de alvo.
- **Description (EN):** First hit on any enemy is always a critical. Resets on target change.
- **Effect:** `grant_passive first_hit_crit`

---

## Archer — Branch B: Trapper

### `archer:5b` — Armadilha de Espinhos / Spike Trap
- **Cost:** 250 karma
- **Prereqs:** `archer:4`
- **Branch:** b
- **Description (PT):** Habilidade ativa: coloca uma armadilha que causa dano e lentifica. 3 cargas, recupera 1 a cada 10s.
- **Description (EN):** Active: place spike trap dealing damage and slowing. 3 charges, recovers 1 every 10s.
- **Effect:** `grant_skill spike_trap`

### `archer:6b` — Flecha Explosiva / Explosive Arrow
- **Cost:** 300 karma
- **Prereqs:** `archer:5b`
- **Branch:** b
- **Description (PT):** A cada 5 disparos, uma flecha explode causando dano em área.
- **Description (EN):** Every 5th shot explodes for area damage.
- **Effect:** `grant_passive explosive_every_5`

### `archer:7b` — Fumaça / Smokescreen
- **Cost:** 400 karma
- **Prereqs:** `archer:5b`
- **Branch:** b
- **Description (PT):** Habilidade ativa: lança fumaça que cega inimigos numa área por 4s. Cooldown 18s.
- **Description (EN):** Active: smokescreen blinds enemies in area for 4s. 18s cooldown.
- **Effect:** `grant_skill smokescreen`

### `archer:8b` — Caçador de Pistas / Pathfinder
- **Cost:** 500 karma
- **Prereqs:** `archer:6b` OR `archer:7b`
- **Branch:** b
- **Description (PT):** Inimigos pegos em armadilha recebem 30% mais dano de todas as fontes por 5s.
- **Description (EN):** Trapped enemies take 30% more damage from all sources for 5s.
- **Effect:** `grant_passive trapped_take_more_damage`

### `archer:9b` — Chuva de Flechas / Arrow Rain
- **Cost:** 650 karma
- **Prereqs:** `archer:8b`
- **Branch:** b
- **Description (PT):** Habilidade ativa: chove flechas numa área grande por 4s. Cooldown 25s.
- **Description (EN):** Active: arrow rain over large area for 4s. 25s cooldown.
- **Effect:** `grant_skill arrow_rain`

### `archer:10b` — Terreno Hostil / Hostile Terrain
- **Cost:** 800 karma
- **Prereqs:** `archer:9b`
- **Branch:** b
- **Description (PT):** Áreas onde caíram explosões ou armadilhas continuam causando dano por 6s.
- **Description (EN):** Areas hit by traps/explosions deal damage for 6s.
- **Effect:** `grant_passive lingering_ground_damage`

### `archer:11b` — Avatar do Cerco / Avatar of Siege (capstone)
- **Cost:** 1200 karma
- **Prereqs:** `archer:10b`
- **Branch:** b
- **Description (PT):** Armadilhas e explosões causam dano dobrado. Cargas de armadilha recuperam 50% mais rápido.
- **Description (EN):** Traps and explosions deal double damage. Trap charges recover 50% faster.
- **Effect:** `grant_passive trap_master`

---

# Synergies (cross-tree)

Synergies activate automatically when both nodes of a pair are unlocked.
Cost no extra karma. Encourage hybrid builds.

| ID | Node A | Node B | Name (PT/EN) | Effect |
|----|--------|--------|--------------|--------|
| 1 | `warrior:1` (Investida) | `mage:5a` (Fogo Vivo) | Investida Ardente / Burning Charge | Investida deixa rastro de fogo por 3s |
| 2 | `warrior:7a` (Ferocidade) | `archer:0` (Olhar Atento) | Olhar do Predador / Predator's Gaze | +10% chance de crítico extra em ataques corpo a corpo |
| 3 | `warrior:5b` (Postura Defensiva) | `mage:0` (Despertar) | Disciplina / Discipline | Postura defensiva regenera 2 mana/s |
| 4 | `mage:1` (Centelha) | `archer:0` (Olhar Atento) | Tiro Energizado / Charged Shot | Tiros normais têm 15% de chance de eletrocutar |
| 5 | `warrior:2` (Pele de Couro) | `archer:3` (Esquiva) | Reflexos Treinados / Trained Reflexes | +5% chance de evasão de projéteis |
| 6 | `mage:5b` (Maldição) | `archer:7b` (Fumaça) | Sombra Maldita / Cursed Shadow | Inimigos na fumaça também ficam amaldiçoados |
| 7 | `warrior:6a` (Sede de Sangue) | `mage:6b` (Dreno Vital) | Vampiro / Vampire | Drenos curam 50% mais HP |
| 8 | `archer:5b` (Armadilha) | `mage:7a` (Frio Cortante) | Armadilha Gélida / Frost Trap | Armadilhas também congelam por 1s |
| 9 | `warrior:11a` (Avatar da Fúria) | `archer:11a` (Avatar da Caça) | Berserker Preciso / Precise Berserker | Críticos em invulnerabilidade causam dano triplo |
| 10 | `warrior:11b` (Bastião) | `mage:11b` (Avatar da Praga) | Reverso Vingador / Avenger's Reverse | Bastião revive aplicando DoT a todos os inimigos próximos |
| 11 | `mage:11a` (Avatar dos Elementos) | `archer:11b` (Avatar do Cerco) | Tempestade Total / Total Storm | Chuva de Flechas gasta meia mana e dá dano elemental |
| 12 | `warrior:9b` (Resistência) | `mage:9b` (Marca da Morte) | Vingança Implacável / Relentless Vengeance | Receber dano amaldiçoa o atacante |

---

# INI structure (per tree file)

Each tree lives in its own INI file. Format used by Sprint K2 parser:

```ini
# data/skills/warrior_tree.ini

[meta]
tree_id = warrior
display_name_pt = Guerreiro
display_name_en = Warrior

[node:0]
name_pt = Vigor Bruto
name_en = Raw Vigor
branch = pre_y
karma_cost = 50
prereqs =
desc_pt = Aumenta HP máximo em 20%.
desc_en = +20% maximum HP.
effect = hp_max_pct +20

[node:1]
name_pt = Investida
name_en = Charge
branch = pre_y
karma_cost = 75
prereqs = 0
desc_pt = Avança 5m em linha reta causando dano e atordoando o primeiro alvo. Cooldown 8s.
desc_en = Dash 5m forward, dealing damage and stunning first target. 8s cooldown.
effect = grant_skill charge

# ... e assim por diante para todos os 19 nós
```

Multiple prereqs separated by commas. Branch values: `pre_y`, `a`, `b`.

Synergies in separate file:

```ini
# data/skills/synergies.ini

[synergy:0]
node_a = warrior:1
node_b = mage:5a
name_pt = Investida Ardente
name_en = Burning Charge
effect = grant_passive burning_charge
```

---

# Build viability (informational)

The 3 trees × 2 branches = 6 specializations. Plus traje (3 lines) =
**18 base archetypes** before considering hybrid investment.

Some examples of viable Act 1 builds:

| Build | Skill investment | Traje | Identity |
|-------|------------------|-------|----------|
| Pure Berserker | warrior:0-11a | Warrior | Glass cannon melee, lifesteal carries you |
| Pure Guardian | warrior:0-11b | Warrior | Tank with allies, anchors fights |
| Spell Storm | mage:0-11a | Mage | AoE wipes, kite enemies |
| Plague Lord | mage:0-11b | Mage | DoT pressure, slow death |
| Sniper | archer:0-11a | Archer | High burst, single target |
| Battlefield General | archer:0-11b | Archer | Traps + AoE, terrain control |
| Spell Blade | warrior pre-Y + mage 5a-9a | Warrior | Tank with elemental finishers |
| Battle Mage | mage pre-Y + warrior 5b-8b | Mage | Caster who can take a hit |
| Death Archer | archer pre-Y + mage 5b-8b | Archer | Ranged DoT applicator |
| Hunter Knight | warrior pre-Y + archer 5a-8a | Warrior | Slow, durable, strikes hard |

These archetypes don't need to be designed individually — they emerge
from skill combinations. Synergies reward hybrids; capstones reward focus.

---

# Tuning notes for sprint K2 implementation

When K2 implements this tree:

1. **Don't rebalance during structural sprint.** Costs and effects
   listed are provisional. Tuning happens during playtest, not during
   implementation.

2. **Effect DSL parser fails gracefully.** Unknown effects log a
   warning and don't crash. This lets the tree expand later without
   blocking core systems.

3. **Capstones are not prerequisites for branch completion.** Player
   doesn't need capstone to "finish" a branch — it's the reward, not
   the gate.

4. **Synergies are runtime-checked, not unlock-gated.** When both
   nodes of a synergy are unlocked, the synergy passive turns on. No
   separate unlock action needed.

5. **Respec returns total invested karma.** Skill tree is one of the
   sources counted in `respec_full()` from K5.

6. **Bifurcation enforcement.** After unlocking `tree:4` (the
   bifurcation node), both branches A and B become visible. Player
   can invest in either or both — but each node still costs full
   karma. Investing in both branches is allowed but expensive.

---

# Open decisions

- [ ] Specific damage/HP/multiplier values per node — provisional, tune
      during playtest
- [ ] Cooldowns of granted skills — provisional, tune during combat
      sprint integration
- [ ] Visual effects for granted skills — deferred to post-Act-1 polish
- [ ] Whether some passives stack with similar passives from other trees
      (e.g., crit_chance from `archer:0` + `warrior:7a`) — default: yes,
      additive stacking. Multiplicative requires explicit `_mult` suffix
      in DSL.
