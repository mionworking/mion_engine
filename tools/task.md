# Refatoração SRP — Task Tracker

## Sprint 1: Fases 1-3

### Fase 1: Extrair Rendering + Player Config
- [x] Verificar compilação antes de começar
- [x] Criar `src/systems/world_renderer.hpp`
- [x] Criar `src/systems/player_configurator.hpp`
- [x] Modificar `dungeon_scene.hpp` — remover funções static + player config
- [x] Modificar `town_scene.hpp` — remover draw_rect + player config
- [x] Verificar compilação após Fase 1 (652 testes OK)

### Fase 2: Extrair Pause Menu
- [x] Criar `src/core/pause_menu.hpp`
- [x] Modificar `dungeon_scene.hpp` — usar PauseMenu
- [x] Modificar `town_scene.hpp` — usar PauseMenu
- [x] Verificar compilação após Fase 2 (652 testes OK)

### Fase 3: Extrair UI do Dungeon
- [x] Criar `src/systems/dungeon_hud.hpp`
- [x] Criar `src/systems/skill_tree_ui.hpp`
- [x] Criar `src/systems/attribute_screen_ui.hpp`
- [x] Modificar `dungeon_scene.hpp` — usar novos módulos de UI
- [x] Verificar compilação após Fase 3 (652 testes OK)
