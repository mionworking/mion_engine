# Roadmap — Enemy Variety + Boss Phases
> Novos comportamentos de IA, inimigos ranged/patrulha/elite, boss Grimjaw com 2 fases.

---

## O que já existe

- `EnemyAISystem` — aggro + separação + chase + melee attack
- `EnemyType` enum: Skeleton, Orc, Ghost (3 tipos)
- `EnemyDef` — stats estáticos por tipo em `get_enemy_def()`
- `Actor` — `aggro_range`, `stop_range_ai`, `attack_range_ai`, `nav_path`
- `StatusEffectType` — Poison, Slow, Stun
- Grimjaw = Orc nomeado na sala 3, HP×1.8, dano+4
- Pathfinder A* já integrado no AI chase

---

## O que não existe

- Inimigos que disparam projéteis (ranged AI)
- Inimigos que patrulham waypoints e alertam vizinhos
- Variantes elite (stats maiores, loot melhor)
- Boss com 2 fases, ataque especial de carga e entrada dramatica
- Campo de comportamento de IA por actor (todos usam o mesmo loop)

---

## Ficheiros a criar / tocar

| Ficheiro | Tipo |
|---|---|
| `src/entities/actor.hpp` | +campos AI: behavior, patrol, ranged_cd, is_elite, boss_phase |
| `src/entities/enemy_type.hpp` | +Archer, PatrolGuard, EliteSkeleton, BossGrimjaw; +AiBehavior em EnemyDef |
| `src/systems/enemy_ai.hpp` | +bloco ranged, +state machine patrulha, +dispatch por behavior |
| `src/scenes/dungeon_scene.hpp` | Grimjaw spawn usa BossGrimjaw; boss phase trigger + intro dramatica |
| `data/enemies.ini` | +secções archer, patrol_guard, elite_skeleton, boss_grimjaw |

---

## 1. `src/entities/enemy_type.hpp` — extensão

### Enum `AiBehavior`:
```cpp
enum class AiBehavior {
    Melee,        // atual: chase + ataque melee direto
    Ranged,       // mantém distância + dispara projétil periodicamente
    Patrol,       // patrulha waypoints; agro ao ver player; alerta vizinhos
    Elite,        // igual a Melee mas stats aumentados + loot melhor
    BossPhased,   // Melee fase 1; fase 2 (<50% HP): mais rápido + carga
};
```

### `EnemyType` — acrescentar:
```cpp
enum class EnemyType { Skeleton, Orc, Ghost, Archer, PatrolGuard, EliteSkeleton, BossGrimjaw };
```

### `EnemyDef` — acrescentar campos:
```cpp
AiBehavior  ai_behavior       = AiBehavior::Melee;
float       ranged_fire_rate  = 1.5f;  // segundos entre disparos (Ranged/Boss)
float       ranged_keep_dist  = 200.0f; // distância mínima que mantém do player
int         ranged_proj_damage= 6;     // dano do projétil disparado
int         gold_drop_min     = 1;
int         gold_drop_max     = 4;
bool        is_elite_base     = false; // true = tipo always-elite
```

### `get_enemy_def()` — novos registos:
```cpp
{ // Archer: frágil, mantém distância, dispara a cada 1.5s
  EnemyType::Archer, 40, 70.0f, 450.0f, 200.0f, 160.0f, 0, 0.75f,
  {14,14},{12,12},{0,0,0}, 0.0f,0.0f,0.0f,
  AiBehavior::Ranged, 1.5f, 200.0f, 10,  2, 5,
  "assets/Puny-Characters/Puny-Characters/Mage-Purple.png", 2.0f, 8.0f, 0
},
{ // PatrolGuard: médio, patrulha; ao aggro alerta inimigos no raio 200
  EnemyType::PatrolGuard, 80, 65.0f, 280.0f, 50.0f, 32.0f, 12, 0.70f,
  {18,18},{16,16},{22,12,24}, 0.12f,0.18f,0.30f,
  AiBehavior::Patrol, 0.0f, 0.0f, 0,  2, 6,
  "assets/Puny-Characters/Puny-Characters/Soldier-Blue.png", 2.0f, 8.0f, 0
},
{ // EliteSkeleton: Skeleton × 2 HP, × 1.5 dano, × 1.2 speed; loot melhor
  EnemyType::EliteSkeleton, 120, 96.0f, 420.0f, 45.0f, 30.0f, 12, 0.72f,
  {16,16},{14,14},{20,12,22}, 0.10f,0.15f,0.25f,
  AiBehavior::Elite, 0.0f, 0.0f, 0,  5, 10,
  "assets/Puny-Characters/Puny-Characters/Soldier-Red.png", 2.4f, 8.0f, 0
},
{ // BossGrimjaw: fase 1 melee; fase 2 carga + mais rápido
  EnemyType::BossGrimjaw, 0/*overridden*/,70.0f,600.0f,52.0f,36.0f,22,0.65f,
  {22,20},{20,18},{28,14,30}, 0.15f,0.20f,0.35f,
  AiBehavior::BossPhased, 0.0f,0.0f,0,  25,35,
  "assets/Puny-Characters/Puny-Characters/Orc-Grunt.png", 2.8f, 8.0f, 0
},
```

---

## 2. `src/entities/actor.hpp` — novos campos

```cpp
// AI behaviour (lido do EnemyDef, copiado para o actor no spawn)
AiBehavior   ai_behavior          = AiBehavior::Melee;

// Ranged AI
float        ranged_fire_cd       = 0.0f;  // cooldown restante (conta down)
float        ranged_fire_rate     = 1.5f;  // período base (copiado do EnemyDef)
int          ranged_proj_damage   = 6;

// Patrulha
struct WayPoint { float x, y; };
std::vector<WayPoint> patrol_waypoints;
int          patrol_wp_index      = 0;
enum class PatrolState { Patrol, Alert, Chase } patrol_state = PatrolState::Patrol;
float        alert_timer          = 0.0f;  // tempo em Alert antes de Chase

// Elite
bool         is_elite             = false;

// Boss fases
int          boss_phase           = 1;    // 1 ou 2
float        boss_charge_cd       = 0.0f; // cooldown da carga (fase 2)
bool         boss_charging        = false;
float        boss_charge_remaining= 0.0f;
float        boss_charge_dir_x    = 0.0f;
float        boss_charge_dir_y    = 0.0f;
```

---

## 3. `src/systems/enemy_ai.hpp` — refactor + novos blocos

### Estrutura nova de `fixed_update()`:

```
Para cada inimigo vivo:
  1. Calcular dist ao player
  2. Dispatch por ai_behavior:
       Melee       → _update_melee(enemy, player, dist, dt, pathfinder)
       Ranged      → _update_ranged(enemy, player, dist, dt, projectiles)
       Patrol      → _update_patrol(enemy, player, dist, dt, all_actors, pathfinder)
       Elite       → _update_melee(enemy, player, dist, dt, pathfinder)  (mesmo que Melee)
       BossPhased  → _update_boss(enemy, player, dist, dt, pathfinder, projectiles)
```

### `_update_ranged()` — novo bloco:
```
Se dist > ranged_keep_dist:
    afasta-se do player (move na direcção oposta)
Senão se dist < ranged_keep_dist * 0.6:
    aproxima-se ligeiramente
Senão:
    strafe lateral (perpendicular ao vector player)

Tick ranged_fire_cd -= dt
Se ranged_fire_cd <= 0 E dist < aggro_range:
    disparar projétil em direcção ao player
    ranged_fire_cd = ranged_fire_rate
    (requer acesso ao vector de projéteis — passar como parâmetro)
```

### `_update_patrol()` — máquina de estados:
```
PatrolState::Patrol:
    Mover em direcção ao waypoint atual
    Se chegou (dist < 24px): avançar para próximo waypoint (circular)
    Se dist_ao_player < aggro_range: transição → Alert (alert_timer=1.0s)
        Alertar inimigos vizinhos num raio de 200px: set patrol_state = Chase

PatrolState::Alert:
    Parar movimento. alert_timer -= dt.
    Se alert_timer <= 0: transição → Chase

PatrolState::Chase:
    Igual ao _update_melee (chase + attack)
    Se dist_ao_player > aggro_range * 1.5: voltar a Patrol
```

### `_update_boss()` — 2 fases:
```
Fase 1 (boss_phase == 1, HP >= 50%):
    Igual a _update_melee
    Se HP < 50%: transição → fase 2
        boss_phase = 2
        move_speed *= 1.4
        attack_damage += 8
        [sinalizar para dungeon_scene: trigger boss_phase_2_event]

Fase 2 (boss_phase == 2):
    Igual a _update_melee mas com carga periódica:

    boss_charge_cd -= dt
    Se boss_charge_cd <= 0 E combat.is_attack_idle():
        boss_charging = true
        boss_charge_remaining = 0.45f
        boss_charge_dir_x/y = normalizado(player - boss)
        boss_charge_cd = 4.0f   // carga a cada 4s

    Se boss_charging:
        mover em boss_charge_dir com velocidade 480 * dt
        boss_charge_remaining -= dt
        Se boss_charge_remaining <= 0: boss_charging = false
```

---

## 4. `src/scenes/dungeon_scene.hpp` — Grimjaw upgrade + intro

### Spawn de Grimjaw (linha ~748):
```cpp
// Substituir spawn de Orc por BossGrimjaw:
_spawn_enemy(EnemyType::BossGrimjaw, 1000.0f, 600.0f, "Grimjaw");
Actor& boss = _enemies.back();
// HP já definido no EnemyDef multiplicado por fator
boss.health.max_hp     = (int)(get_enemy_def(EnemyType::BossGrimjaw).max_hp * 1.0f);
boss.health.current_hp = boss.health.max_hp;
// Waypoints de patrulha do boss (antes do aggro):
boss.patrol_waypoints  = {{800,400},{1200,800},{800,1000},{400,600}};
boss.ai_behavior       = AiBehavior::BossPhased;
```

### Novo campo `_boss_phase2_triggered`:
```cpp
bool _boss_phase2_triggered = false;
```

### Em `fixed_update()` — detetar transição de fase:
```cpp
// Verificar se boss transitou para fase 2
for (auto& e : _enemies) {
    if (e.name == "Grimjaw" && e.boss_phase == 2 && !_boss_phase2_triggered) {
        _boss_phase2_triggered = true;
        _camera.trigger_shake(18.0f, 30);
        _dialogue.start("boss_grimjaw_phase2");
        // screen flash branco
        _screen_flash_timer  = 0.3f;
        _screen_flash_color  = {255, 200, 50, 180};
    }
}
```

### Novo diálogo de fase 2 (registar em `_setup_dialogue()`):
```cpp
{ "boss_grimjaw_phase2", {
    { "Grimjaw", "RAAAAGH! You dare…!!", {255, 80, 40, 255} },
}}
```

### Intro da sala 3 (boss entry sequence):
```cpp
// Em _enter_room() ou _build_room(), quando _room_index == 3 e não stress:
_boss_intro_pending = true;
_boss_intro_timer   = 0.0f;
// Bloquear movimento do player durante 1.5s + mostrar title card "GRIMJAW"
```

Campos novos:
```cpp
bool  _boss_intro_pending = false;
float _boss_intro_timer   = 0.0f;
```

Em `render()` — enquanto `_boss_intro_pending`:
```cpp
if (_boss_intro_pending && _boss_intro_timer < 1.5f) {
    // Escurecer fundo 50%
    SDL_SetRenderDrawColor(r, 0,0,0,130);
    SDL_FRect full = {0,0,(float)viewport_w,(float)viewport_h};
    SDL_RenderFillRect(r, &full);
    // Texto "GRIMJAW" centrado, grande
    draw_text(r, cx - text_width("GRIMJAW",4)*0.5f,
              viewport_h*0.45f, "GRIMJAW", 4, 255, 80, 40, 255);
}
```

---

## 5. `data/enemies.ini` — novas secções

```ini
[archer]
max_hp=40
damage=10
move_speed=70
aggro_range=450
ranged_fire_rate=1.5
ranged_keep_dist=200
ranged_proj_damage=10
gold_drop_min=2
gold_drop_max=5

[patrol_guard]
max_hp=80
damage=12
move_speed=65
aggro_range=280
attack_range=50
stop_range=32
gold_drop_min=2
gold_drop_max=6

[elite_skeleton]
max_hp=120
damage=12
move_speed=96
aggro_range=420
gold_drop_min=5
gold_drop_max=10

[boss_grimjaw]
max_hp=320
damage=22
move_speed=70
aggro_range=600
startup_sec=0.15
active_sec=0.20
recovery_sec=0.35
gold_drop_min=25
gold_drop_max=35
```

---

## Resumo de impacto

```
NOVOS ficheiros (0)

FICHEIROS EXISTENTES alterados (5):
  src/entities/enemy_type.hpp    +4 tipos, +AiBehavior enum, +campos EnemyDef
  src/entities/actor.hpp         +ai_behavior, +patrol, +ranged, +boss_phase fields
  src/systems/enemy_ai.hpp       dispatch por behavior + 3 novos update methods
  src/scenes/dungeon_scene.hpp   boss spawn + phase2 trigger + boss intro sequence
  data/enemies.ini               +4 secções
```
