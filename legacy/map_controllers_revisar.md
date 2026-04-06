# Controller Map — mion_engine_cpp

> Atualizado em 2026-04-04 após refatoração. Build: 24/24 ✓

---

## 1. Controllers ativos

| Controller | Arquivo | Tipo | Responsabilidade |
|---|---|---|---|
| `DungeonFlowController` | `systems/dungeon_flow_controller.hpp` | namespace (templates) | Transições de sala: apply_save_and_build_world, start_full_run, advance_room, update_room_flow |
| `CombatFxController` | `systems/combat_fx_controller.hpp` | namespace | Feedback de combate: apply_combat_feedback, apply_projectile_hit_fx, apply_melee_hit_fx, update_death_flow |
| `EnemyDeathController` | `systems/enemy_death_controller.hpp` | namespace | Mortes de inimigos: partículas, drops, XP, run_stats, quest Grimjaw, áudio — retorna `DeathResult` |
| `BossState` | `systems/boss_state_controller.hpp` | struct | Intro timer do boss + detecção de phase2 (shake, diálogo, screen flash) |
| `DungeonSaveController` | `systems/dungeon_save_controller.hpp` | namespace | Serialização DungeonContext ↔ SaveData; persist_dungeon_save; snapshot_last_run |
| `TownSaveController` | `systems/town_save_controller.hpp` | namespace | Serialização TownContext ↔ SaveData; make_town_save; apply_town_save |
| `PauseMenuController` | `systems/pause_menu_controller.hpp` | class | Estado, input e render do pause menu |
| `SkillTreeController` | `systems/skill_tree_controller.hpp` | class | Overlay de talentos: abrir/fechar, navegar colunas, gastar pontos, sincronizar spell_book |
| `AttributeLevelUpController` | `systems/attribute_levelup_controller.hpp` | class | Overlay de level-up de atributos: sync open/close, aplicar ponto, recalcular stats derivados |
| `ShopInputController` | `systems/shop_input_controller.hpp` | namespace | Input do shop (move Y, confirm, cancel) com hysteresis; chama ShopSystem::try_buy |
| `TownNpcInteractionController` | `systems/town_npc_interaction.hpp` | namespace | Encontra NPC mais próximo, handle interaction, abre shop ou inicia diálogo; lógica de quest |

### Structs de estado associadas

| Struct | Arquivo | Usado por |
|---|---|---|
| `ScreenFlashState` | `systems/screen_fx.hpp` | DungeonScene (`_screen_flash`); passado como ref para CombatFxController e BossState |

---

## 2. Fluxo de chamadas — DungeonScene

```
DungeonScene::fixed_update(dt, input)
│
├─ _screen_flash.tick(dt)                          ← ScreenFlashState
│
├─ [overlay] _capture_overlay_input → OverlayInputTracker
├─ [ui] _handle_pause_ui → PauseMenuController
├─ [ui] _attr_controller.update → AttributeLevelUpController
├─ [ui] _skill_tree_controller.update → SkillTreeController
│
├─ [sim] MovementSystem::fixed_update
├─ [sim] PlayerActionSystem::fixed_update → SpellSystem, DashSystem
├─ [sim] ResourceSystem::fixed_update
├─ [sim] StatusEffectSystem::fixed_update
├─ [sim] EnemyAISystem::fixed_update
│
├─ [boss] _boss_state.update(...)                  ← BossState
│         → camera shake + diálogo + screen flash na transição phase2
│
├─ [sim] RoomCollisionSystem::resolve_all / resolve_actors
├─ [sim] ProjectileSystem::fixed_update
│         └─ CombatFxController::apply_projectile_hit_fx(...)
│             → partículas, hit_flash, screen flash, áudio
│
├─ [sim] MeleeCombatSystem::fixed_update
│         └─ CombatFxController::apply_combat_feedback(...)  ← hitstop, shake, parry SFX
│         └─ CombatFxController::apply_melee_hit_fx(...)
│             → partículas, hit_flash, screen flash, áudio
│
├─ [death] EnemyDeathController::process_deaths(...)
│          → partículas morte, drops, XP, run_stats, quest, áudio
│          → retorna DeathResult (post_mortem_id, quest_completed, xp_gained)
│
├─ [flow] DungeonFlowController::update_room_flow → RoomFlowSystem, DialogueSystem
├─ [audio] DungeonAudioSystem::update_dungeon_music
├─ [stats] run stats inline (damage_taken, gold, time, spells_cast)
│
├─ [fx] CombatFxController::update_death_flow
├─ [anim] AnimationDriver
├─ [fx] FootstepAudioSystem
├─ [fx] FloatingTextSystem::tick
└─ [cam] Camera2D::follow + step_shake
```

### DungeonScene::render

```
DungeonScene::render(r, blend)
│
├─ DungeonWorldRenderer::render(...)
├─ render_dungeon_hud(...)
├─ _attr_controller.render(...)           ← AttributeLevelUpController
├─ ScreenFx::render_boss_intro(...)       ← lê _boss_state.intro_pending / intro_timer
├─ ScreenFx::render_death_fade(...)
├─ ScreenFx::render_screen_flash(_screen_flash)  ← ScreenFlashState
├─ _skill_tree_controller.render(...)     ← SkillTreeController
├─ render_dialogue_ui(...)
└─ _pause_controller.render(...)          ← PauseMenuController
```

---

## 3. Fluxo de chamadas — TownScene

```
TownScene::fixed_update(dt, input)
│
├─ [ui] DialogueSystem (bloqueia mundo)
├─ [ui] PauseMenuController::update
├─ [ui] ShopInputController::update_shop_input → ShopSystem::try_buy
│
├─ [sim] MovementSystem::fixed_update
├─ [sim] PlayerActionSystem::fixed_update
├─ [sim] ResourceSystem::fixed_update
├─ [sim] RoomCollisionSystem::resolve_all / resolve_actors
├─ [sim] TownNpcWanderSystem::update_town_npcs
├─ [sim] AnimationDriver::update_player_town_anim
├─ [sim] RoomFlowSystem::fixed_update → TownSaveController::make_town_save
│
├─ [npc] TownNpcInteractionController::find_nearest_npc
│         └─ TownNpcInteractionController::handle_npc_interaction (confirm edge)
│
└─ [cam] Camera2D::follow + step_shake
```

---

## 4. O que ainda está inline (baixa prioridade)

| Bloco | Localização | Tamanho | Veredicto |
|---|---|---|---|
| Run stats tick | `DungeonScene::fixed_update` ~linhas 430–440 | 4 linhas | `RunStats::tick_frame(dt, hp_delta, gold_delta, spells)` seria suficiente |
| NPC hint distance check | `TownScene::fixed_update` | 5 linhas | Trivial, manter |
| Player sprite render | `TownScene::render()` | ~15 linhas | Cosmético, poderia ir para `TownWorldRenderer::render_player()` |

---

## 5. Diagrama de responsabilidade (estado atual)

```
DungeonScene
├── DungeonFlowController       → transições sala/cena, save on advance
├── CombatFxController          → hitstop, parry SFX, hit FX (proj + melee), death flow
├── EnemyDeathController        → drops + XP + quest + áudio on death
├── BossState                   → intro timer + phase2 trigger + screen flash
├── ScreenFlashState            → estado de flash de tela (trigger/tick/render)
├── DungeonSaveController       → persist + snapshot last_run
├── PauseMenuController         → pause menu UI
├── SkillTreeController         → talent overlay UI
└── AttributeLevelUpController  → level-up attribute UI

TownScene
├── TownSaveController          → load/apply/make save
├── PauseMenuController         → pause menu UI
├── ShopInputController         → shop input com hysteresis
└── TownNpcInteractionController→ find nearest + handle interaction
```
