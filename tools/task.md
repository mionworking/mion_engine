# Refatoração SRP — Task Tracker

## Sprint 1: Fases 1-3 ✓ COMPLETO

### Fase 1: Extrair Rendering + Player Config
- [x] Criar `src/systems/world_renderer.hpp`
- [x] Criar `src/systems/player_configurator.hpp`
- [x] Modificar `dungeon_scene.hpp` — remover funções static + player config
- [x] Modificar `town_scene.hpp` — remover draw_rect + player config
- [x] Verificar compilação (652 testes OK)

### Fase 2: Extrair Pause Menu
- [x] Criar `src/core/pause_menu.hpp`
- [x] Modificar `dungeon_scene.hpp` — usar PauseMenu
- [x] Modificar `town_scene.hpp` — usar PauseMenu
- [x] Verificar compilação (652 testes OK)

### Fase 3: Extrair UI do Dungeon
- [x] Criar `src/systems/dungeon_hud.hpp`
- [x] Criar `src/systems/skill_tree_ui.hpp`
- [x] Criar `src/systems/attribute_screen_ui.hpp`
- [x] Modificar `dungeon_scene.hpp` — usar novos módulos de UI
- [x] Verificar compilação (652 testes OK)

---

## Sprint 2: Fases 4-6 — dungeon_scene + save_system ✓ COMPLETO

### Fase 4: Extrair Spawn & Room Management do DungeonScene (1711→~1400)
- [x] Criar `src/systems/enemy_spawner.hpp` — spawn budget, _spawn_enemies_for_budget, enemy type selection
- [x] Criar `src/systems/room_manager.hpp` — room transitions, door flow, room reset, `_advance_room` logic
- [x] Modificar `dungeon_scene.hpp` — usar EnemySpawner + RoomManager
- [x] Verificar compilação + testes

### Fase 5: Extrair World Render & FX do DungeonScene (~1400→~1100)
- [x] Criar `src/systems/dungeon_world_renderer.hpp` — render do mundo (tiles, obstacles, actors, projectiles, drops, particles, floating texts world-space, lighting)
- [x] Criar `src/systems/screen_fx.hpp` — overlays full-screen (death fade, boss intro, screen flash)
- [x] Modificar `dungeon_scene.hpp` — usar novos módulos de render
- [x] Verificar compilação + testes

### Fase 6: Refatorar save_system.hpp (403 linhas)
- [x] Criar `src/core/save_data.hpp` — struct SaveData + constantes de versão
- [x] Criar `src/core/save_migration.hpp` — funções de migração v1→v2→v3→v4→v5
- [x] Simplificar `save_system.hpp` — apenas load/save/path helpers
- [x] Verificar compilação + testes

---

## Sprint 3: Fases 7-9 — enemy_ai + title_scene + player_action

### Fase 7: Refatorar enemy_ai.hpp (364 linhas)
- [ ] Criar `src/systems/ai_patrol.hpp` — patrol/waypoint/separation behavior
- [ ] Criar `src/systems/ai_combat.hpp` — chase/melee/ranged behavior + projectile spawn
- [ ] Criar `src/systems/ai_boss.hpp` — boss phased logic (phase2 trigger, special attacks)
- [ ] Simplificar `enemy_ai.hpp` — orquestrador que delega aos sub-sistemas
- [ ] Verificar compilação + testes

### Fase 8: Refatorar title_scene.hpp (350 linhas)
- [ ] Criar `src/core/title_menu.hpp` — UI modes (main/difficulty/settings/extras), navigation, rendering
- [ ] Extrair ações de menu para helpers/controller — continue/new game, erase save, settings, persistência de dificuldade/config
- [ ] Simplificar `title_scene.hpp` — lifecycle + estado de cena + delegação ao menu/controller
- [ ] Verificar compilação + testes

### Fase 9: Refatorar player_action.hpp (311 linhas)
- [ ] Criar `src/systems/dash_system.hpp` — dash logic + stamina drain + iframes
- [ ] Criar `src/systems/spell_system.hpp` — spell casting, cooldowns, projectile spawn
- [ ] Simplificar `player_action.hpp` — orquestrador melee/ranged/dash/spells
- [ ] Verificar compilação + testes

---

## Backlog (prioridade menor, candidatos futuros)

| Linhas | Arquivo | Ação sugerida |
|-------:|---------|---------------|
| 229 | `systems/world_renderer.hpp` | Separar primitivas de actor rendering |
| 201 | `systems/dialogue_system.hpp` | Separar render de runtime logic + INI loader |
| 179 | `components/talent_tree.hpp` | Separar data tables de runtime API de INI loader |
| 176 | `entities/actor.hpp` | Considerar split Player/Enemy ou composition |
| 160 | `systems/melee_combat.hpp` | Separar parry system |
| 157 | `systems/pathfinder.hpp` | Separar NavGrid de Pathfinder |
| 156 | `core/audio.hpp` | Separar enums/IDs de AudioSystem |
| 154 | `entities/enemy_type.hpp` | Separar EnemyDef table de INI parser |
| 99 | `systems/shop_system.hpp` | Separar try_buy (lógica) de render_shop_ui |
