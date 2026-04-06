# Controller Map — mion_engine_cpp

> Atualizado em 2026-04-06. **Cena em runtime:** `WorldScene` (`world`). Os ficheiros `dungeon_scene.hpp` e `town_scene.hpp` **já não existem** no repositório; as secções 3–4 descrevem o fluxo **histórico equivalente** (lógica absorvida por `WorldScene`). Validar build com `ctest -L official`.

---

## 1. Controllers e orquestração ativos (`WorldScene`)

Além da tabela clássica de controllers, o open world concentra lógica em:

| Peça | Arquivo | Notas |
|---|---|---|
| `WorldScene` | `scenes/world_scene.hpp` | Orquestra `fixed_update` / `render`; `ZoneManager`; `_actors` (player + inimigos por área); combate / AI / projéteis só com `in_dungeon()`; cidade com `TownNpcWander` + interação; `NpcActorFactory` em `_rebuild_all_actors`. |
| `WorldSaveController` | `systems/world_save_controller.hpp` | Save v6, posição global, bitmask de áreas visitadas; `persist` / `apply_world_save` / `snapshot_last_run`. |
| `AreaEntrySystem` | `systems/area_entry_system.hpp` | Primeira entrada por zona: diálogo opcional + `EnemySpawner` com offsets globais da área. |
| `WorldAudioSystem` | `systems/world_audio_system.hpp` | Ambient/música por `ZoneId` (incl. corredor `Transition` / `TransitionWind`). |
| `WorldContext` | `world/world_context.hpp` | Contexto unificado para callbacks (`world_map`, `zone_mgr`, `player`, `actors`, diálogo, boss, loja, etc.). |
| `DungeonConfigLoader` / `TownConfigLoader` | `systems/dungeon_config_loader.hpp`, `systems/town_config_loader.hpp` | `enter()`: dados estáticos da dungeon, progression/town; diálogos da cidade em `_dialogue`. |
| `WorldMapBuilder` | `world/world_map_builder.hpp` | Constrói `WorldMap` a partir de `rooms.ini`, NPCs, loja, player. |

### Coordenadas (evitar regressões)

- **Globais:** posições de `Actor`, projéteis, drops, broadphase de obstáculos (`get_obstacles_near` soma `offset_*` aos AABBs).
- **Locais à sala:** `RoomDefinition`, tilemap, `Pathfinder` / `NavGrid` — a IA recebe `nav_ox/oy` e converte para `find_path`; waypoints voltam a mundo ao steer.
- **Render dungeon:** `DungeonWorldRenderInputs::camera` (local, `offset_cam`) vs `world_camera` para tiles vs entidades dinâmicas; player desenhado no fim com câmera global.

---

## 1b. Controllers e módulos reutilizados (nomeados)

| Controller / módulo | Arquivo | Tipo | Responsabilidade |
|---|---|---|---|
| `CombatFxController` | `systems/combat_fx_controller.hpp` | namespace | Feedback de combate: `apply_combat_feedback`, `apply_projectile_hit_fx`, `apply_melee_hit_fx`, `update_death_flow`. |
| `EnemyDeathController` | `systems/enemy_death_controller.hpp` | namespace | Mortes: partículas, drops, XP, run_stats, quest Grimjaw, áudio — retorna `DeathResult`. |
| `BossState` | `systems/boss_state_controller.hpp` | struct | Intro do boss + phase2 (shake, diálogo, screen flash). |
| `PauseMenuController` | `systems/pause_menu_controller.hpp` | class | Estado, input e render do pause menu. |
| `SkillTreeController` | `systems/skill_tree_controller.hpp` | class | Overlay de talentos: abrir/fechar, navegar, gastar pontos, sincronizar `spell_book`. |
| `AttributeLevelUpController` | `systems/attribute_levelup_controller.hpp` | class | Overlay de level-up de atributos. |
| `EquipmentScreenController` | `systems/equipment_screen_controller.hpp` | class | Inventário/equipamento (abre com edge `inventory` no overlay); `EquipmentScreenUI` por baixo. |
| `ShopInputController` | `systems/shop_input_controller.hpp` | namespace | Input da loja (move Y, confirm, cancel) com hysteresis; chama `ShopSystem::try_buy`. |
| `TownNpcInteractionController` | `systems/town_npc_interaction.hpp` | namespace | NPC mais próximo, confirmação, abre loja ou diálogo; recompensas de quest. |
| `OverlayInputTracker` | `systems/overlay_input.hpp` | class | Captura edges de input quando há menus / pause. |

### Structs de estado associadas

| Struct | Arquivo | Usado por |
|---|---|---|
| `ScreenFlashState` | `systems/screen_fx.hpp` | `CombatFxController`, `BossState`, `WorldScene::render` |

### Removidos / nunca integrados no tree atual

Os nomes `DungeonFlowController`, `DungeonSaveController` e `TownSaveController` aparecem em planos legados; **não há** `dungeon_flow_controller.hpp` nem `dungeon_save_controller.hpp` / `town_save_controller.hpp` em `src/`. Fluxo de sala e persistência unificados passam por `WorldScene` + `WorldSaveController`.

---

## 2. Fluxo de chamadas — `WorldScene` (cena principal)

```
WorldScene::fixed_update(dt, input)
│
├─ OverlayInputTracker::capture; autosave flash tick
├─ ScreenFlashState::tick; PlayerPotionQuickslot::update
├─ ZoneManager::update(player pos, world_map) + _handle_zone_transition (áudio por zona, reset boss se área boss vazia)
│
├─ [gate] hitstop → return
├─ [gate] DialogueSystem::fixed_update (ativo) → return
├─ [gate] PauseMenuController::update → return se mundo pausado
├─ [gate] AttributeLevelUpController::update → save se necessário → return se overlay
├─ [gate] SkillTreeController::update → save se necessário → return se overlay
├─ [gate] EquipmentScreenController::update → save se necessário → return se overlay
│
├─ [shop] se loja aberta em town: _update_shop (ShopInputController), ResourceSystem, câmera, música → return
│
├─ uso de poção (edge pressed)
├─ MovementSystem::fixed_update(_actors)
├─ [boss intro] se Boss e intro pendente: zera input de combate até timer acabar
├─ PlayerActionSystem::fixed_update → SpellSystem, dash, melee, projéteis…
├─ ResourceSystem + StatusEffectSystem; combat.advance_time por ator
│
├─ [dungeon only] se ZoneManager::in_dungeon():
│   ├─ cur_area = world_map.area_at(player)
│   ├─ EnemyAISystem::fixed_update(..., pathfinder, nav_ox/oy)
│   ├─ BossState::update(_all_enemies(), …)
│   ├─ ProjectileSystem::fixed_update(..., cur_area->room, offset_x, offset_y)
│   ├─ CombatFxController::apply_projectile_hit_fx
│   ├─ MeleeCombatSystem::fixed_update
│   ├─ CombatFxController::apply_combat_feedback + apply_melee_hit_fx
│   └─ EnemyDeathController::process_deaths → post_mortem id, save se quest/boss
│
├─ was_alive ← is_alive por ator (marcador pós-frame)
├─ [dungeon] DropSystem::pickup_near_player → diálogo "dungeon_rare_relic" se aplicável
│
├─ [town] TownNpcWanderSystem + TownNpcInteractionController (hint + confirm edge)
│
├─ [collision] get_obstacles_near(AABB expandido em torno do player)
│              → RoomCollisionSystem::resolve_all + resolve_actors
│
├─ AreaEntrySystem::update (spawn + diálogos 1ª visita via callback)
├─ se zona mudou → _rebuild_all_actors()
├─ diálogo pós-morte → ao terminar: pedido de transição para "victory"
│
├─ WorldAudioSystem::update (não stress)
├─ run stats (dano, ouro, tempo, spells_cast)
├─ CombatFxController::update_death_flow → snapshot last run
├─ partículas, AnimationDriver (player dungeon/town + inimigos), FloatingTextSystem
├─ câmera follow + shake; tick música; FootstepAudioSystem
├─ hit_flash timers; _scene_exit_pending → _pending_next_scene
└─ flush overlay input
```

### `WorldScene::render` (resumo)

- **Town:** `TilemapRenderer` global + `TownWorldRenderer::render_town_world` (NPCs, etc.).
- **Dungeon / Transition:** câmera local por área (`_camera` − offset); `DungeonWorldRenderer::render` com `DungeonWorldRenderInputs` (tiles/props com `offset_cam`, entidades com `world_camera`). Zona `Transition` usa `corridor_floor.png` no renderer de área.
- **Depois das áreas:** sprite do player em coordenadas de mundo; `LightingSystem` em dungeon ou corredor (não stress, player vivo).
- **HUD:** `render_dungeon_hud` em dungeon; town: hint "ENTER - falar" + ouro.
- **Overlays:** autosave, `AttributeLevelUpController::render`, `ScreenFx` (boss intro, death fade, flash), `SkillTreeController`, `EquipmentScreenController`, diálogo, UI da loja, `PauseMenuController`.

---

## 3. Fluxo histórico — DungeonScene (removido; equivalência)

> **Nota:** corresponde ao que antes vivia em `DungeonScene::fixed_update`; hoje está repartido por `WorldScene` (bloco `in_dungeon()`), `WorldAudioSystem`, `AreaEntrySystem` e ausência de `DungeonFlowController` dedicado.

```
DungeonScene::fixed_update (histórico)
│
├─ ScreenFlashState::tick
├─ OverlayInputTracker + PauseMenuController + AttributeLevelUpController + SkillTreeController
├─ MovementSystem, PlayerActionSystem, ResourceSystem, StatusEffectSystem, EnemyAISystem
├─ BossState::update
├─ RoomCollisionSystem, ProjectileSystem + CombatFxController (proj)
├─ MeleeCombatSystem + CombatFxController (feedback + melee)
├─ EnemyDeathController::process_deaths
├─ (antes) fluxo de sala / música dungeon dedicada / run stats / death flow / anim / footsteps / floating / câmera
```

### DungeonScene::render (histórico)

`DungeonWorldRenderer`, HUD, overlays de atributos/skills/pause, `ScreenFx`, diálogo — espelhados em `WorldScene::render` para áreas não-town.

---

## 4. Fluxo histórico — TownScene (removido; equivalência)

```
TownScene::fixed_update (histórico)
│
├─ DialogueSystem, PauseMenuController, ShopInputController
├─ MovementSystem, PlayerActionSystem, ResourceSystem
├─ RoomCollisionSystem, TownNpcWanderSystem, AnimationDriver (town)
├─ (antes) RoomFlowSystem + save town dedicado
├─ TownNpcInteractionController
└─ câmera
```

Hoje: ramos `in_town()` e loja aberta dentro de `WorldScene::fixed_update`; persistência via `WorldSaveController`.

---

## 5. O que ainda está inline (baixa prioridade)

| Bloco | Localização | Notas |
|---|---|---|
| Run stats tick | `WorldScene::fixed_update` (~após `WorldAudioSystem`) | Contagem inline de dano/ouro/tempo/spells; extrair para helper se quiser simetria com testes. |
| NPC hint + gold HUD | `WorldScene::render` (town) | Poucas linhas; pode migrar para `TownWorldRenderer` no futuro. |
| Player sprite após áreas | `WorldScene::render` | Desenho explícito do player em mundo; documentado como padrão dual-camera. |
| Callback `AreaEntrySystem` | `fixed_update` (lambdas com ids de diálogo) | IDs de zona hardcoded; candidato a tabela de dados. |

---

## 6. Diagrama de responsabilidade (estado atual)

### WorldScene (runtime)

```
WorldScene
├── ZoneManager + WorldMap + WorldMapBuilder
├── DungeonConfigLoader + TownConfigLoader + register_dungeon_dialogue
├── AreaEntrySystem + EnemySpawner (offsets globais)
├── WorldSaveController (persistência unificada)
├── WorldAudioSystem
├── CombatFxController + EnemyDeathController + BossState + ScreenFlashState
├── PauseMenuController, AttributeLevelUpController, SkillTreeController, EquipmentScreenController
├── OverlayInputTracker
├── ShopInputController + TownNpcInteractionController + TownNpcWander (town)
├── TownWorldRenderer / DungeonWorldRenderer (dual camera onde aplicável)
└── Systems: Movement, PlayerAction, AI (nav offset), Projectiles, MeleeCombat, Collision broadphase,
    DropSystem, FootstepAudioSystem, AnimationDriver, Lighting (dungeon/corredor)
```

### Referência histórica (cenas separadas)

```
DungeonScene (removido) ──► lógica em WorldScene (dungeon + transição)
TownScene (removido)     ──► lógica em WorldScene (town + loja)
```
