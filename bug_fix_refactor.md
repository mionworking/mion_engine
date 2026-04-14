# Bug fix & refactor — inventário e plano

Documento único com problemas levantados na revisão de arquitectura (`main`/CMake/locale/RNG), na `WorldScene` e nos controllers. Cada secção inclui **o quê**, **porquê importa**, **onde mexer** e **plano sugerido**.

---

## Prioridade sugerida (ordem de ataque)

1. **Crítico / correcção de lógica:** `_all_enemies()` e footsteps (cópias de `Actor`).
2. **Alto:** run stats por delta de HP; input `_prev_*` vs early returns.
3. **Médio:** locale global (`L()` / `locale_bind`); Grimjaw hardcoded; diálogo de relíquia; NPC raio vs `find_nearest`.
4. **Estrutural (incremental):** extrair controllers / HUD; CMake explícito; cap de substeps no loop.

---

## Status de execução (worklog)

### 2026-04-06 — Tarefa A (`_all_enemies` / footsteps) — **Concluída**

- **Escopo aplicado:** `src/scenes/world_scene.hpp`, `src/systems/footstep_audio_system.hpp`, `src/systems/world_audio_system.hpp`, `src/systems/boss_state_controller.hpp`.
- **Mudança:** `_all_enemies()` deixou de copiar `Actor` por valor e passou a trabalhar com `std::vector<Actor*>` (atores reais).
- **Efeito esperado confirmado por revisão de código:** `FootstepAudioSystem` agora muta `footstep_prev_*`/`footstep_accum_dist` nos inimigos reais (não em cópias temporárias).
- **Validação:** build completo OK.

### 2026-04-06 — Tarefa B (sincronização `_prev_*` com early return) — **Concluída**

- **Escopo aplicado:** `src/scenes/world_scene.hpp`.
- **Mudança:** criado `_sync_gameplay_input_history(input)` e aplicado em todos os caminhos com early return (hitstop, diálogo, pausa, attr, skill tree, equipment, shop), além do caminho normal no fim de `fixed_update`.
- **Efeito esperado:** histórico de input de gameplay (`_prev_potion`, `_prev_confirm`, `_prev_upgrade_*`, `_prev_talent_*`) fica consistente mesmo quando o mundo pausa e retorna cedo.
- **Validação:** build completo OK.

### 2026-04-06 — Tarefa C (run stats: `damage_taken` por evento) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/scenes/world_scene.hpp`.
- **Mudança:** `damage_taken` deixou de usar delta `hp0` vs HP final no frame. A contabilização agora soma dano por eventos de impacto do frame:
  - `ProjectileSystem::last_hit_events` quando `target_name == _player.name`;
  - `MeleeCombatSystem::last_events` quando `target_name == _player.name`.
- **Efeito esperado:** cenários com dano + cura no mesmo frame deixam de mascarar dano recebido nas estatísticas da run.w
- **Nota:** esta etapa ainda usa identificação por `target_name` (string), coerente com o modelo atual dos eventos de combate/projétil. Migração para id/handle permanece no backlog arquitetural.
- **Validação:** build completo OK; suíte core mantém o mesmo estado observado (805 passed, 2 failed em restore de posição no `world_scene_lifecycle`).

### 2026-04-06 — Tarefa D (boss hardcoded por nome) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/scenes/world_scene.hpp`, `src/systems/boss_state_controller.hpp`, `src/systems/world_audio_system.hpp`, `src/systems/dungeon_audio_system.hpp`, `src/systems/enemy_death_controller.hpp`.
- **Mudança:** lógica de boss deixou de depender de comparação por nome (`\"Grimjaw\"`) nos pontos críticos de runtime e passou a usar `EnemyType::BossGrimjaw`.
- **Efeito esperado:** regras de boss ficam mais robustas para rename/localização de display name e menos frágeis a stringly-typed checks.
- **Nota:** conteúdo narrativo (falas/labels) continua com string “Grimjaw” em `dungeon_dialogue` e spawn display name em `enemy_spawner`, o que é esperado nesta etapa incremental (camada de conteúdo, não regra de decisão).
- **Validação:** build completo OK; suíte core mantém o mesmo estado observado (805 passed, 2 failed em restore de posição no `world_scene_lifecycle`).

### 2026-04-06 — Tarefa E (locale sem global) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/locale.hpp`, `src/main.cpp`, `src/core/title_menu.hpp`, `src/scenes/title_scene.hpp`, `src/scenes/victory_scene.hpp`, `src/scenes/game_over_scene.hpp`, `src/scenes/credits_scene.hpp`, `src/core/register_scenes.cpp`, `tests/input/test_locale.cpp`.
- **Mudança:** removidos `active_locale`, `locale_bind` e `L()` globais. Cenas/menus passaram a receber `LocaleSystem*` explicitamente (via setters + `SceneCreateContext`) e usar `locale->get(...)` através de helper local (`tr(...)`).
- **Efeito esperado:** elimina estado global implícito de locale; dependência fica explícita nas cenas/UI.
- **Validação:** build completo OK; suíte core mantém o mesmo estado observado (805 passed, 2 failed em restore de posição no `world_scene_lifecycle`).

### 2026-04-06 — Tarefa F (town NPC: `find_nearest` + raio unificado) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/town_npc_interaction.hpp`, `src/scenes/world_scene.hpp`.
- **Mudança:** criada API única `find_nearest_npc_for_interaction(...)` no `TownNpcInteractionController`, que retorna índice do NPC mais próximo e flag `in_interaction_range` calculada com `interact_radius`. A `WorldScene` passou a usar esse resultado para hint e confirmação, removendo cálculo duplicado de distância na cena.
- **Efeito esperado:** uma única fonte de verdade para geometria de interação de NPC em town, reduzindo risco de drift entre hint visual e gatilho de `confirm`.
- **Validação:** build completo OK; suíte core mantém o mesmo estado observado (805 passed, 2 failed em restore de posição no `world_scene_lifecycle`).

### 2026-04-06 — Tarefa G (diálogo de relíquia rara: política de repetição) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/scenes/world_scene.hpp`.
- **Mudança:** o trigger de `dungeon_rare_relic` foi separado em duas flags locais de sessão (`pending` + `shown`): pickup de lore marca pendência, e a abertura do diálogo ocorre apenas quando não há diálogo ativo; após abrir, marca como já mostrado na run.
- **Efeito esperado:** evita reabertura repetida do mesmo diálogo em múltiplos pickups e não perde o trigger quando o pickup acontece com outro diálogo aberto.
- **Validação:** build completo OK; suíte core mantém o mesmo estado observado (805 passed, 2 failed em restore de posição no `world_scene_lifecycle`).

### 2026-04-06 — Tarefa H (save load candidates: priorizar arquivo mais recente) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/save_system.hpp`.
- **Mudança:** `load_candidates(...)` deixou de priorizar cegamente `default_path`. Quando `default` e `legacy` coexistem, agora seleciona o arquivo com `last_write_time` mais recente e mantém fallback para o outro em caso de parse/load falhar.
- **Efeito esperado:** evita carregar save stale do path primário quando um save mais novo existe no legado; corrige cenário de restore de posição no lifecycle e reduz dependência frágil de ordem/caminho.
- **Validação:** build completo OK; suíte core passou após ajuste (807 passed, 0 failed).

### 2026-04-06 — Tarefa I (game loop: cap de substeps / acumulador) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/main.cpp`.
- **Mudança:** adicionado teto de substeps por frame (`max_substeps_per_frame = 5`) junto com clamp do acumulador (`accumulator = min(accumulator, fixed_dt * max_substeps_per_frame)`).
- **Efeito esperado:** reduz risco de spiral-of-death em frames atrasados, limitando backlog de simulação por frame sem alterar o fluxo geral de update/render.
- **Validação:** build completo OK; suíte core permanece verde (807 passed, 0 failed).

### 2026-04-06 — Tarefa J (HUD town: strings via locale em `WorldScene`) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/scenes/world_scene.hpp`, `src/core/register_scenes.cpp`, `data/locale_en.ini`, `data/locale_ptbr.ini`.
- **Mudança:** `WorldScene` passou a receber `LocaleSystem*` via `set_locale` (injetado no `register_scenes`) e a usar `tr(...)` para o hint de interação de NPC e label de ouro na HUD da town (`town_npc_interact_hint`, `town_gold_label`).
- **Efeito esperado:** remove hardcoded de texto de HUD em town da cena e alinha apresentação com o fluxo de localização já usado nas outras cenas.
- **Validação:** build completo OK; suíte core permanece verde (807 passed, 0 failed).

### 2026-04-06 — Tarefa K (autosave tag via locale em `WorldScene`) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/scenes/world_scene.hpp`, `data/locale_en.ini`, `data/locale_ptbr.ini`.
- **Mudança:** texto de autosave no HUD (`"Saved"`) foi substituído por `tr("ui_saved")` com chaves de locale adicionadas.
- **Efeito esperado:** elimina hardcoded restante de UI no render da `WorldScene` e mantém consistência de localização com as demais cenas.
- **Validação:** build completo OK; suíte core permanece verde (807 passed, 0 failed).

### 2026-04-06 — Tarefa L (Shop UI: strings via locale) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/shop_system.hpp`, `src/scenes/world_scene.hpp`, `data/locale_en.ini`, `data/locale_ptbr.ini`.
- **Mudança:** `ShopSystem::render_shop_ui` passou a aceitar `LocaleSystem*` opcional e usar chaves de locale para título, label de ouro e hints de compra/fechar (`shop_title`, `shop_gold_label`, `shop_buy_hint`, `shop_close_hint`). `WorldScene` repassa `_locale` ao render da loja.
- **Efeito esperado:** remove hardcoded restante de texto da UI da loja e mantém consistência de localização no fluxo town.
- **Validação:** build completo OK; suíte core permanece verde (807 passed, 0 failed).

### 2026-04-06 — Tarefa M (Pause UI: strings via locale) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/pause_menu.hpp`, `src/systems/pause_menu_controller.hpp`, `src/scenes/world_scene.hpp`, `data/locale_en.ini`, `data/locale_ptbr.ini`.
- **Mudança:** `PauseMenu` passou a renderizar título/settings/hints por chaves de locale; `PauseMenuController` ganhou `set_locale(...)` e usa labels localizados para itens padrão (`pause_resume`, `pause_skill_tree`, `pause_quit_to_menu`, etc.); `WorldScene` injeta locale no controller antes do `init`.
- **Efeito esperado:** remove hardcoded restante do fluxo de pause/settings na world scene e mantém UI modal alinhada ao sistema de localização.
- **Validação:** build completo OK; suíte core permanece verde (807 passed, 0 failed).

### 2026-04-06 — Tarefa N (Skill Tree UI: strings via locale) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/skill_tree_ui.hpp`, `src/systems/skill_tree_controller.hpp`, `src/scenes/world_scene.hpp`, `data/locale_en.ini`, `data/locale_ptbr.ini`.
- **Mudança:** `render_skill_tree_overlay` passou a receber `LocaleSystem*` opcional e usar chaves de locale para label de pontos, títulos de coluna e hint (`skill_points_label`, `skill_tree_col_*`, `skill_tree_hint`). `SkillTreeController` ganhou `set_locale(...)` e `WorldScene` injeta locale no controller.
- **Efeito esperado:** remove hardcoded de texto no overlay da skill tree e mantém consistência de localização entre overlays de UI.
- **Validação:** build completo OK; suíte core permanece verde (807 passed, 0 failed).

### 2026-04-06 — Tarefa O (Attribute Screen UI: strings via locale) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/attribute_screen_ui.hpp`, `src/systems/attribute_levelup_controller.hpp`, `src/scenes/world_scene.hpp`, `data/locale_en.ini`, `data/locale_ptbr.ini`.
- **Mudança:** `render_attribute_screen` passou a receber `LocaleSystem*` opcional e usar chaves de locale para título, pontos restantes, nomes/descrições dos atributos e hint (`attr_*`). `AttributeLevelUpController` ganhou `set_locale(...)` e `WorldScene` injeta locale no controller.
- **Efeito esperado:** remove hardcoded do overlay de distribuição de atributos e mantém consistência de localização entre overlays modais.
- **Validação:** build completo OK; suíte core permanece verde (807 passed, 0 failed).

### 2026-04-06 — Tarefa P (Equipment UI: strings via locale) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/equipment_screen_ui.hpp`, `src/systems/equipment_screen_controller.hpp`, `src/scenes/world_scene.hpp`, `data/locale_en.ini`, `data/locale_ptbr.ini`.
- **Mudança:** `render_equipment_screen` passou a receber `LocaleSystem*` opcional e usar chaves de locale para títulos, opções do menu contextual e hints de navegação (`equip_*`). `EquipmentScreenController` ganhou `set_locale(...)` e `WorldScene` injeta locale no controller.
- **Efeito esperado:** remove hardcoded do overlay de equipamento/inventário e mantém consistência de localização entre as UIs modais da world scene.
- **Validação:** build completo OK; suíte core permanece verde (807 passed, 0 failed).

### 2026-04-06 — Tarefa Q (Dungeon HUD: strings via locale) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/dungeon_hud.hpp`, `src/scenes/world_scene.hpp`, `data/locale_en.ini`, `data/locale_ptbr.ini`.
- **Mudança:** `render_dungeon_hud` passou a aceitar `LocaleSystem*` opcional e usar chaves de locale para labels/status/abreviações da HUD (`hud_*`), incluindo room/level/gold/talent points, tags de barras, status effects, labels da hotbar e qualidade de poção.
- **Efeito esperado:** remove hardcoded textual da HUD de dungeon e mantém consistência de localização entre HUD e overlays.
- **Validação:** build completo OK; suíte core permanece verde (807 passed, 0 failed).

### 2026-04-06 — Tarefa R (Boss intro FX: strings via locale) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/screen_fx.hpp`, `src/scenes/world_scene.hpp`, `data/locale_en.ini`, `data/locale_ptbr.ini`.
- **Mudança:** `ScreenFx::render_boss_intro` passou a aceitar `LocaleSystem*` opcional e usar chaves `boss_intro_name` / `boss_intro_subtitle` em vez de strings hardcoded. `WorldScene` passa `_locale` no callsite.
- **Efeito esperado:** remove hardcoded textual do overlay de intro de boss e mantém consistência de localização com HUD/overlays já migrados.
- **Validação:** build completo OK; suíte core permanece verde (807 passed, 0 failed).

### 2026-04-06 — Tarefa S (TitleMenu: header e labels de idioma via locale) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/title_menu.hpp`, `data/locale_en.ini`, `data/locale_ptbr.ini`.
- **Mudança:** título principal do menu (`MION ENGINE`) e labels de idioma em settings (`EN`/`PT-BR`) passaram a vir de chaves de locale (`title_main_header`, `lang_label_en`, `lang_label_ptbr`) em vez de literais hardcoded.
- **Efeito esperado:** reduz hardcoded residual no menu principal e mantém o fluxo de settings alinhado ao sistema de tradução já utilizado no restante da UI.
- **Validação:** build completo OK; suíte core permanece verde (807 passed, 0 failed).

### 2026-04-06 — Tarefa T (town NPC: persistência por owner da cena) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/town_npc_interaction.hpp`, `src/scenes/world_scene.hpp`.
- **Mudança:** `TownNpcInteractionController` deixou de chamar `SaveSystem` diretamente em callbacks de diálogo. A persistência passou a ser solicitada por callback injetado (`request_persist`), e a `WorldScene` (owner superior) agora decide e executa o save via `_persist_save()`.
- **Efeito esperado:** ownership de side effects fica explícito e consistente com os demais controllers (controller retorna/intenciona, scene orquestra persistência), reduzindo acoplamento de IO dentro de fluxo de interação de NPC.
- **Validação:** build completo OK; suíte core permanece verde (807 passed, 0 failed).

### 2026-04-06 — Tarefa U (TitleMenu: persistência por owner da cena) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/title_menu.hpp`, `src/scenes/title_scene.hpp`.
- **Mudança:** `TitleMenuController` deixou de chamar `SaveSystem` diretamente em fluxos de apagar save/novo jogo e seleção de dificuldade. O controller agora retorna intenções explícitas (`clear_default_saves`, `persist_difficulty_to_save`) via `TitleMenuActionResult`, e a `TitleScene` executa a persistência/remoção no owner superior.
- **Efeito esperado:** reduz side effects ocultos no controller de menu e padroniza ownership com o guideline (“controller devolve intenção; scene aplica IO/persistência”).
- **Validação:** build completo OK; suíte core permanece verde (807 passed, 0 failed).

### 2026-04-06 — Tarefa V (VictoryScene: persistência consolidada no service layer) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`, `src/scenes/victory_scene.hpp`.
- **Mudança:** extraído o fluxo duplicado de `load_default -> merge victory/run_stats -> save_default` da `VictoryScene` para `WorldSaveController::mark_victory_reached(...)`. A cena passou a orquestrar intenção de transição e delegar persistência para o service layer.
- **Efeito esperado:** reduz duplicação de pipeline de save em cena e concentra a regra de merge de estado persistente em módulo de persistência dedicado.
- **Validação:** build completo OK; suíte core permanece verde (807 passed, 0 failed).

### 2026-04-06 — Tarefa W (GameOverScene: remoção de save via service layer) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`, `src/scenes/game_over_scene.hpp`.
- **Mudança:** `GameOverScene` deixou de chamar `SaveSystem::remove_default_saves()` diretamente e passou a delegar para `WorldSaveController::clear_default_saves()`, mantendo a cena focada em fluxo/transição e o detalhe de persistência no service layer.
- **Efeito esperado:** reduz acoplamento direto de IO dentro da cena e mantém consistência com os passos anteriores de centralização de persistência.
- **Validação:** build completo OK; suíte core passou (807 passed, 0 failed). Em uma execução intermediária houve 1 falha intermitente em `Save.PrimaryPreferred`, não reproduzida na reexecução imediata.

### 2026-04-06 — Tarefa X (teste de save: eliminar intermitência de mtime) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/legacy/test_legacy.cpp`.
- **Mudança:** `test_save_load_candidates_prefers_primary` passou a fixar `last_write_time` explicitamente (fallback mais antigo, primary mais novo) antes de chamar `load_candidates(...)`, em vez de depender da granularidade/ordem implícita do filesystem.
- **Efeito esperado:** remove flakiness no `Save.PrimaryPreferred` ao alinhar o teste com a política atual de seleção por arquivo mais recente.
- **Validação:** build completo OK; suíte core passou de forma estável na validação local (809 passed, 0 failed).

### 2026-04-06 — Tarefa Y (TitleScene: leitura de unlock via service layer) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`, `src/scenes/title_scene.hpp`.
- **Mudança:** extraída de `TitleScene::enter()` a leitura direta de `SaveData` para verificar `victory_reached`. A cena agora usa `WorldSaveController::load_victory_unlock_flag(...)`, mantendo a lógica de leitura de save concentrada no service layer.
- **Efeito esperado:** reduz acoplamento da cena ao formato de `SaveData` e melhora consistência de ownership no fluxo de persistência/leitura.
- **Validação:** build completo OK; suíte core permanece verde (809 passed, 0 failed).

### 2026-04-06 — Tarefa Z (TitleScene: operações de save restantes via service layer) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`, `src/scenes/title_scene.hpp`.
- **Mudança:** as operações restantes de persistência da `TitleScene` (`exists_default`, `remove_default_saves`, persistência de dificuldade) foram delegadas para `WorldSaveController` (`has_default_save`, `clear_default_saves`, `persist_difficulty_if_save_exists`), removendo dependência direta adicional de `SaveSystem` na cena.
- **Efeito esperado:** consolida ownership de IO/save no service layer e deixa a `TitleScene` mais focada em fluxo de menu/transição.
- **Validação:** build completo OK; suíte core permanece verde (809 passed, 0 failed).

### 2026-04-06 — Tarefa AA (boot em `main`: dificuldade via service layer) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`, `src/main.cpp`.
- **Mudança:** leitura inicial de dificuldade no boot (`main`) deixou de usar `SaveSystem` diretamente e passou a delegar para `WorldSaveController::load_saved_difficulty(...)`.
- **Efeito esperado:** reduz acoplamento de `main` ao formato de save e mantém consistência da camada de persistência em um único service layer.
- **Validação:** build completo OK; suíte core permanece verde (809 passed, 0 failed).

### 2026-04-06 — Tarefa AB (`WorldScene::enter`: load_default via service layer) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`, `src/scenes/world_scene.hpp`.
- **Mudança:** carregamento de save em `WorldScene::enter()` deixou de chamar `SaveSystem::load_default(...)` diretamente; a cena agora usa `WorldSaveController::load_default_save(...)`.
- **Efeito esperado:** reduz acoplamento direto da cena ao backend de persistência e mantém consistência com os passos anteriores de centralização no service layer.
- **Validação:** build completo OK; suíte core permanece verde (809 passed, 0 failed).

### 2026-04-06 — Tarefa AC (service layer: contratos explícitos de sucesso) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** operações de escrita/remoção no service layer passaram a retornar `bool` de sucesso explícito (`mark_victory_reached`, `clear_default_saves`) em vez de `void`.
- **Efeito esperado:** melhora testabilidade e clareza de contrato para chamadas futuras que queiram reagir a falhas de persistência sem depender de side effects implícitos.
- **Validação:** build completo OK; suíte core permanece verde (809 passed, 0 failed).

### 2026-04-06 — Tarefa AD (service layer: snapshot de run com status explícito) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** `snapshot_last_run(...)` passou de `void` para `bool`, retornando sucesso/falha do fluxo `load_default -> merge last_run_stats -> save_default`.
- **Efeito esperado:** mantém consistência de contrato com os demais métodos de persistência já ajustados (`mark_victory_reached`, `clear_default_saves`) e facilita instrumentação futura de erro sem alterar comportamento atual.
- **Validação:** build completo OK; suíte core permanece verde (809 passed, 0 failed).

### 2026-04-06 — Tarefa AE (callsites: consumo de retorno de persistência com logging) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/scenes/victory_scene.hpp`, `src/scenes/game_over_scene.hpp`, `src/scenes/title_scene.hpp`, `src/scenes/world_scene.hpp`.
- **Mudança:** callsites críticos passaram a verificar retornos `bool` do `WorldSaveController` e registrar falhas via `debug_log(...)` (`mark_victory_reached`, `clear_default_saves`, `persist_difficulty_if_save_exists`, `snapshot_last_run`), mantendo o fluxo de jogo inalterado.
- **Efeito esperado:** aumenta observabilidade de falhas de IO/save em runtime sem impactar UX nem regras de transição de cena.
- **Validação:** build completo OK; suíte core permanece verde (809 passed, 0 failed).

### 2026-04-06 — Tarefa AF (persist world save com status explícito + consumo) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`, `src/scenes/world_scene.hpp`.
- **Mudança:** `WorldSaveController::persist(...)` passou de `void` para `bool` (retorno do `save_default`), e `WorldScene::_persist_save()` passou a consumir o retorno com `debug_log` em caso de falha.
- **Efeito esperado:** completa a padronização de contratos explícitos de persistência no service layer e melhora observabilidade de falhas no autosave sem alterar fluxo de jogo.
- **Validação:** build completo OK; suíte core permanece verde (809 passed, 0 failed).

### 2026-04-06 — Tarefa AG (persist/snapshot: semântica de no-op sem falso erro) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** retornos de `persist(...)` e `snapshot_last_run(...)` foram ajustados para tratar caminhos de no-op legítimos como sucesso lógico (`true`) — ex.: stress mode ativo ou ausência de save para snapshot.
- **Efeito esperado:** evita “falso negativo” nos callsites que agora logam falhas, preservando logs para erros reais de escrita/persistência.
- **Validação:** build completo OK; suíte core permanece verde (809 passed, 0 failed).

### 2026-04-06 — Tarefa AH (testes: cobertura de no-op em persist/snapshot) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionados testes para garantir semântica de no-op como sucesso lógico:
  - `world_save_snapshot_noop_paths_return_success`;
  - `world_save_persist_noop_stress_mode_returns_success`.
- **Efeito esperado:** evita regressões na nova semântica de retorno do service layer (`true` em no-op legítimo), mantendo estabilidade dos logs e do contrato público.
- **Validação:** build completo OK; suíte core passou com cobertura ampliada (812 passed, 0 failed).

### 2026-04-06 — Tarefa AI (no-op sem falso erro em clear/persist_difficulty) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** alinhada semântica de no-op em mais dois métodos:
  - `clear_default_saves()` retorna `true` quando não há save para remover;
  - `persist_difficulty_if_save_exists(...)` retorna `true` quando não há save para atualizar.
- **Efeito esperado:** reduz logs de “falha” que eram apenas ausência de trabalho, mantendo `false` para erro real de persistência.
- **Validação:** build completo OK; suíte core permanece verde (812 passed, 0 failed).

### 2026-04-06 — Tarefa AJ (estabilização de testes: sem dependência de `default_path`) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** mantida cobertura de no-op em caminhos determinísticos (`persist`/`snapshot` com `stress_mode`) e removida tentativa de teste que dependia do estado real do `default_path` do ambiente para `clear_default_saves`/`persist_difficulty_if_save_exists`.
- **Efeito esperado:** evita flakiness de teste ligada ao filesystem/perfil local, mantendo suíte reprodutível no CI e no ambiente local.
- **Validação:** build completo OK; suíte core permanece verde (812 passed, 0 failed).

### 2026-04-06 — Tarefa AK (service layer: documentação explícita da semântica de retorno) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** adicionados comentários curtos para documentar a semântica dos retornos `bool` (sucesso de escrita vs no-op legítimo) e pontos de no-op por método (`persist`, `snapshot_last_run`, `clear_default_saves`, `persist_difficulty_if_save_exists`).
- **Efeito esperado:** reduz ambiguidade para manutenção futura e evita uso incorreto dos retornos nos callsites.
- **Validação:** build completo OK; suíte core permanece verde (812 passed, 0 failed).

### 2026-04-06 — Tarefa AL (áudio: fonte canônica única para asset paths) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/audio_asset_paths.hpp`, `src/core/audio.cpp`, `src/core/audio.hpp`, `src/core/asset_manifest.hpp`.
- **Mudança:** centralizados os paths de SFX/música/ambiente em `audio_asset_paths.hpp` (fonte única) e removidas listas duplicadas de `audio.cpp` e `asset_manifest.hpp` para essas categorias (mantendo apenas extras não mapeados por enums no manifesto).
- **Efeito esperado:** reduz drift entre manifesto de assets e carregamento real de áudio, evitando divergência silenciosa quando um path é alterado em apenas um ponto.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa AM (talentos: defaults sem duplicação literal) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/components/talent_data.hpp`.
- **Mudança:** extraídas tabelas canônicas `kDefaultTalentNodes` e `kDefaultTalentDisplayNames`; `g_talent_nodes`/`g_talent_display_names` agora inicializam e resetam a partir dessas fontes únicas, removendo duplicação literal entre inicialização global e `reset_talent_tree_defaults()`.
- **Efeito esperado:** reduz risco de drift entre defaults compilados e caminho de reset, mantendo contrato existente de mutabilidade via `apply_talents_ini()` e restauração por reset.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa AN (spells: defaults canônicos + reset antes de INI) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/components/spell_defs.hpp`, `src/systems/dungeon_config_loader.hpp`.
- **Mudança:** extraída tabela canônica `kDefaultSpellDefs`; `g_spell_defs` agora inicializa a partir dela; adicionado `reset_spell_defs_defaults()` e integrado ao bootstrap de dungeon antes de `apply_spell_ini_section(...)`.
- **Efeito esperado:** elimina duplicação literal de defaults de spells e evita herança acidental de estado anterior quando `spells.ini` omite campos (cada load parte de baseline determinístico).
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa AO (player/progression: defaults canônicos explícitos) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/components/player_config.hpp`, `src/components/progression.hpp`.
- **Mudança:** adicionadas constantes canônicas `kDefaultPlayerConfig` e `kDefaultProgressionConfig`; `g_player_config`/`g_progression_config` agora inicializam por essas fontes e os resets usam as mesmas constantes em vez de `Type{}` inline.
- **Efeito esperado:** padroniza o contrato de defaults com o mesmo modelo aplicado em talentos/spells, reduzindo ambiguidade e risco de drift quando defaults forem evoluídos.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa AP (bootstrap compartilhado: player/progression/talents) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/common_player_progression_loader.hpp`, `src/systems/town_config_loader.hpp`, `src/systems/dungeon_config_loader.hpp`.
- **Mudança:** extraído helper compartilhado `CommonPlayerProgressionLoader::load_defaults_and_ini_overrides()` para concentrar reset + aplicação de INI de `player/progression/talents`; loaders de town e dungeon passaram a reutilizar o mesmo fluxo.
- **Efeito esperado:** remove duplicação de bootstrap estático e reduz risco de drift entre town/dungeon ao evoluir configuração base.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa AQ (spells.ini: aplicação centralizada por helper único) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/components/spell_defs.hpp`, `src/systems/dungeon_config_loader.hpp`.
- **Mudança:** extraído `apply_spells_ini_overrides(const IniData&)` em `spell_defs.hpp` para encapsular a aplicação de todas as seções conhecidas de `spells.ini` sobre `g_spell_defs`; `DungeonConfigLoader` passou a usar esse helper em vez de sequência inline repetitiva.
- **Efeito esperado:** reduz duplicação local no bootstrap de dungeon e centraliza o contrato de mapeamento seção->`SpellId`, diminuindo risco de drift em futuras evoluções de spells.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa AR (enemies.ini: aplicação centralizada por helper único) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/entities/enemy_type.hpp`, `src/systems/dungeon_config_loader.hpp`.
- **Mudança:** extraído `apply_enemies_ini_overrides(...)` em `enemy_type.hpp` para encapsular o mapeamento de seções conhecidas de `enemies.ini` e sua aplicação em `enemy_defs` + `enemy_sprite_paths`; `DungeonConfigLoader` passou a chamar esse helper.
- **Efeito esperado:** reduz duplicação local no bootstrap de dungeon e centraliza o contrato seção->`EnemyType`, diminuindo risco de drift em evoluções de catálogo de inimigos.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa AS (items.ini/drops: aplicação centralizada por helper único) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/drop_system.hpp`, `src/systems/dungeon_config_loader.hpp`.
- **Mudança:** extraído `apply_drop_ini_overrides(const IniData&, DropConfig&)` em `drop_system.hpp` para concentrar parsing da seção `drops` de `items.ini`; `DungeonConfigLoader` passou a usar esse helper.
- **Efeito esperado:** reduz duplicação local no bootstrap de dungeon e centraliza o contrato de configuração de drops em módulo de domínio já dono de `DropConfig`.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa AT (carregamento de INI de `data/` via helper único) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/data_ini_loader.hpp`, `src/systems/common_player_progression_loader.hpp`, `src/systems/dungeon_config_loader.hpp`, `src/systems/town_config_loader.hpp`.
- **Mudança:** criado helper `load_data_ini(relative_path)` para encapsular `ini_load(resolve_data_path(...))`; loaders de town/dungeon/common passaram a usar esse helper.
- **Efeito esperado:** reduz repetição de boilerplate de IO/config e centraliza um contrato único para leitura de INIs do diretório `data/`, facilitando futuras mudanças de resolução de path.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa AU (nomes de arquivos INI canônicos em constantes) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/data_file_names.hpp`, `src/systems/common_player_progression_loader.hpp`, `src/systems/dungeon_config_loader.hpp`, `src/systems/town_config_loader.hpp`.
- **Mudança:** criadas constantes canônicas para nomes de INI de `data/` (`kPlayer`, `kProgression`, `kTalents`, `kEnemies`, `kSpells`, `kItems`, `kRooms`, `kTownDialogues`) e substituídos literais string nos loaders pelos símbolos compartilhados.
- **Efeito esperado:** reduz stringly-typed config names espalhados e facilita manutenção/rename de arquivos de dados com menor risco de typo.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa AV (seções INI canônicas para bootstrap de dungeon) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/data_section_names.hpp`, `src/components/spell_defs.hpp`, `src/entities/enemy_type.hpp`, `src/systems/drop_system.hpp`.
- **Mudança:** centralizados nomes de seções INI usados no bootstrap (`spells`, `enemies`, `drops`) em constantes compartilhadas; removidos literais string equivalentes dos módulos de aplicação de overrides.
- **Efeito esperado:** reduz stringly-typed section ids espalhados, facilita renomear/expandir seções de dados e diminui risco de typo entre loaders/helpers.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa AW (invariantes de catálogo: `EnemyType` x seções INI) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/entities/enemy_type.hpp`.
- **Mudança:** adicionado `static_assert` em `apply_enemies_ini_overrides(...)` para garantir, em compile-time, que `kEnemyIniSections.size()` continua alinhado com `kEnemyTypeCount`.
- **Efeito esperado:** evita drift silencioso quando novos `EnemyType` forem adicionados sem atualização correspondente das seções de `enemies.ini`.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa AX (spells.ini: mapeamento tabela-driven + invariante) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/components/spell_defs.hpp`.
- **Mudança:** `apply_spells_ini_overrides(...)` migrou de sequência manual de chamadas para tabela estática de bindings `section -> SpellId` (incluindo alias legado `bolt`), com laço único de aplicação e `static_assert` para cobertura mínima do catálogo de spells.
- **Efeito esperado:** reduz repetição e risco de drift ao evoluir o catálogo de spells/aliases, mantendo o contrato em um único ponto declarativo.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa AY (WorldSaveController: helper único para mutate+save) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** extraído helper interno `detail::mutate_default_save(...)` para consolidar o fluxo recorrente `load_default -> mutação -> save_default`, com políticas explícitas para “missing = no-op” e “missing = seed vazio”; aplicado em `snapshot_last_run`, `mark_victory_reached` e `persist_difficulty_if_save_exists`.
- **Efeito esperado:** reduz duplicação no service layer de persistência e deixa mais explícita a semântica de no-op vs criação de baseline no mesmo ponto de decisão.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa AZ (WorldSaveController: clamp de dificuldade centralizado) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** extraído helper interno `detail::clamp_difficulty_level(int)` e reaproveitado nos pontos que convertiam `int -> DifficultyLevel` com `std::clamp` (`apply_world_save`, `load_saved_difficulty`).
- **Efeito esperado:** reduz duplicação de regra de normalização de dificuldade e centraliza o contrato de conversão em um único ponto.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BA (spells.ini: bindings canônicos separados de aliases) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/components/spell_defs.hpp`.
- **Mudança:** `apply_spells_ini_overrides(...)` passou a usar duas tabelas distintas: `kCanonicalBindings` (cobertura 1:1 do catálogo de `SpellId`) e `kAliasBindings` (aliases legados, ex. `bolt`). Adicionado `static_assert` de correspondência exata `kCanonicalBindings.size() == kSpellCount`.
- **Efeito esperado:** evita que aliases “mascarem” falta de cobertura canônica ao evoluir o catálogo de spells e deixa explícita a separação entre contrato principal e compatibilidade legada.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BB (WorldSaveController: regra de no-op por stress mode centralizada) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** extraído helper interno `detail::persistence_disabled_by_stress_mode(const WorldContext&)` e aplicado em `persist(...)` e `snapshot_last_run(...)` no lugar da checagem duplicada de `ctx.stress_mode`.
- **Efeito esperado:** reduz duplicação de regra de no-op de persistência e mantém semântica consistente quando stress mode estiver ativo.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BC (WorldSaveController: checagem de save default centralizada) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** extraído helper interno `detail::default_save_exists()` e aplicado em `clear_default_saves()` e `has_default_save()` no lugar de chamadas duplicadas diretas a `SaveSystem::exists_default()`.
- **Efeito esperado:** reduz duplicação de regra de existência de save default e mantém um ponto único para eventual ajuste de política de detecção.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BD (WorldSaveController: leitura de save default centralizada) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** extraído helper interno `detail::load_default_save_data(SaveData&)` para encapsular leitura do save default; aplicado em `mutate_default_save`, `load_victory_unlock_flag`, `load_saved_difficulty` e `load_default_save`.
- **Efeito esperado:** reduz duplicação de chamadas de leitura no service layer e concentra em um ponto único a operação de load do backend.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BE (WorldSaveController: escrita de save default centralizada) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** extraído helper interno `detail::save_default_save_data(const SaveData&)` para encapsular escrita no save default; aplicado em `mutate_default_save(...)` e `persist(...)`.
- **Efeito esperado:** complementa a centralização de leitura/escrita no service layer, reduzindo duplicação de chamadas ao backend `SaveSystem` e facilitando evolução futura de política de persistência.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BF (WorldSaveController: merge de victory flag centralizado) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** extraído helper interno `detail::merge_victory_flag_from_existing_save(SaveData&)` para encapsular leitura do save existente e preservação de `victory_reached` durante `persist(...)`.
- **Efeito esperado:** reduz duplicação de leitura pontual no fluxo de persist e mantém explícito, em um único ponto, o contrato de preservação de flag de vitória no autosave.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BG (WorldSaveController: snapshot debug de autosave encapsulado) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** extraído helper interno `detail::save_autosave_debug_snapshot(const SaveData&)` para encapsular o `SaveSystem::save_debug(..., "world_autosave")` usado no fluxo de `persist(...)`.
- **Efeito esperado:** completa a centralização de chamadas ao backend `SaveSystem` no service layer e reduz acoplamento direto de callsites internos a tags literais de debug.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BH (testes: clamp de dificuldade no apply do save) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_apply_clamps_difficulty_range` cobrindo `WorldSaveController::apply_world_save(...)` com entradas fora de faixa (`-99` e `99`) e verificação de clamp para o intervalo válido de dificuldade (`0..2`).
- **Efeito esperado:** evita regressão na normalização de dificuldade durante aplicação de save no contexto de mundo, especialmente após refactors internos de helpers.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BI (WorldSaveController: remoção de save default encapsulada) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** extraído helper interno `detail::remove_default_save_data()` para encapsular remoção de saves default e aplicado em `clear_default_saves()`.
- **Efeito esperado:** completa a centralização das operações básicas de backend (`exists/load/save/remove`) no service layer e reduz acoplamento direto a `SaveSystem` nos callsites de alto nível.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BJ (WorldSaveController: persist sem leitura direta de backend) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** `merge_victory_flag_from_existing_save(...)` passou a usar explicitamente `detail::load_default_save_data(...)`, eliminando leitura direta remanescente do backend dentro do fluxo de `persist(...)`.
- **Efeito esperado:** reforça o encapsulamento de operações de backend no conjunto de helpers internos do service layer e reduz pontos de acesso direto ao `SaveSystem`.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BK (WorldSaveController: remover redundância de difficulty no persist) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** removida atribuição redundante de `difficulty` em `persist(...)`; o campo já é populado por `make_world_save(ctx)` quando `ctx.difficulty` está presente.
- **Efeito esperado:** reduz código duplicado sem alterar comportamento do autosave.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BL (WorldSaveController: limites de dificuldade sem números mágicos) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** adicionadas constantes internas `kMinDifficultyIndex`/`kMaxDifficultyIndex` e reaproveitadas em `clamp_difficulty_level(...)`, removendo literais mágicos `0`/`2` do clamp.
- **Efeito esperado:** melhora legibilidade/manutenibilidade da regra de normalização de dificuldade e evita divergência futura de limites em caso de ajuste.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BM (WorldSaveController: duração do flash de autosave sem número mágico) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** extraída constante interna `kAutosaveFlashDurationSec` e aplicada ao fluxo `persist(...)` no lugar do literal `1.35f`.
- **Efeito esperado:** melhora legibilidade e manutenção do contrato visual do indicador de autosave, evitando valor hardcoded disperso.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BN (WorldSaveController: tag de debug de autosave sem literal mágico) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`.
- **Mudança:** extraída constante interna `kAutosaveDebugTag` e aplicada no helper `save_autosave_debug_snapshot(...)` no lugar do literal `"world_autosave"`.
- **Efeito esperado:** reforça padronização de constantes internas e evita string hardcoded no callsite de debug de persistência.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BO (testes: clamp de dificuldade no load_saved_difficulty) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_load_saved_difficulty_clamps_range`, cobrindo `WorldSaveController::load_saved_difficulty(...)` com saves default contendo dificuldade fora da faixa (`-42` e `99`) e verificando clamp para `0..2`.
- **Efeito esperado:** amplia cobertura da regra de normalização de dificuldade também no caminho de leitura de boot/save default, reduzindo risco de regressão em refactors internos do service layer.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BP (testes: `persist` preserva `victory_reached` no save existente) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_persist_preserves_existing_victory_flag`, cobrindo fluxo real de `WorldSaveController::persist(...)` com save default pré-semeado com `victory_reached=true`, validando preservação da flag e atualização dos campos de posição do player.
- **Efeito esperado:** aumenta cobertura do comportamento de merge de persistência no caminho de produção (não apenas no teste “like persist”), protegendo regressões no helper de merge de vitória.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BQ (testes: `clear_default_saves` no-op retorna sucesso) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_clear_default_saves_noop_when_missing_returns_success`, garantindo que, sem save default presente, `has_default_save()` reporta ausência e `clear_default_saves()` mantém contrato de no-op bem-sucedido (`true`).
- **Efeito esperado:** protege a semântica pública de no-op de persistência já documentada no service layer, evitando regressão para falso erro quando não há trabalho a executar.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BR (testes: `has_default_save` reflete presença real de save) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_has_default_save_reflects_presence`, cobrindo transições de estado de presença de save (`ausente -> presente -> removido`) com `has_default_save()` e `clear_default_saves()`.
- **Efeito esperado:** protege contrato público de detecção de save existente no service layer e evita regressões silenciosas na integração entre `exists_default` e remoção.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BS (testes: `clear_default_saves` remove save existente) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_clear_default_saves_removes_existing_save`, cobrindo cenário com save default presente e validando que `clear_default_saves()` retorna sucesso, remove o save e impede novo load.
- **Efeito esperado:** complementa a cobertura do caso no-op com o caso operacional real de remoção, protegendo o contrato completo de limpeza de save no service layer.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BT (testes de save default: estabilização para ambiente sandbox) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado guard `world_save_default_backend_available()` e aplicado aos testes dependentes de `SaveSystem::save_default(...)`/`default_path`, para que virem no-op em ambientes sem backend de persistência default gravável (ex.: sandbox com escrita restrita fora do workspace).
- **Efeito esperado:** remove falsos negativos de CI/local em ambiente restrito sem perder cobertura funcional quando o backend default está disponível.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BT (testes: `persist` aplica flash timer com indicador ativo) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_persist_sets_flash_timer_on_success_when_enabled`, cobrindo caminho de sucesso de `WorldSaveController::persist(...)` com `show_indicator=true` e validando que `flash_timer` é atualizado para valor positivo.
- **Efeito esperado:** protege o contrato visual do autosave indicator no fluxo real de persistência, evitando regressões onde o save funciona mas o feedback ao usuário deixa de aparecer.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BU (testes: `persist` preserva `flash_timer` com indicador desativado) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_persist_keeps_flash_timer_when_indicator_disabled`, cobrindo caminho de sucesso de `persist(...)` com `show_indicator=false` e verificando que `flash_timer` não é alterado.
- **Efeito esperado:** protege o contrato de UI do autosave indicator para não sobrescrever timer quando o feedback visual estiver explicitamente desativado.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BV (testes: `load_victory_unlock_flag` em ausência/presença de save) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_load_victory_unlock_flag_reports_missing_and_present_save`, cobrindo: (1) ausência de save default (retorno `false` e `out=false`) e (2) save presente com `victory_reached=true` (retorno `true` e `out=true`).
- **Efeito esperado:** protege o contrato público de leitura de unlock de vitória, garantindo comportamento previsível tanto em boot limpo quanto após progresso salvo.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BW (testes: `snapshot_last_run` grava stats com save existente) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_snapshot_last_run_writes_stats_when_save_exists`, cobrindo caminho operacional de `WorldSaveController::snapshot_last_run(...)` com `run_stats` válido e save default presente, verificando persistência de campos de estatística da run.
- **Efeito esperado:** protege regressões no fluxo de snapshot de fim de run, garantindo que no-op/merge não substitua indevidamente o comportamento de gravação quando há baseline de save.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BX (testes: `mark_victory_reached` sem save prévio) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_mark_victory_reached_seeds_save_when_missing`, cobrindo o contrato de `WorldSaveController::mark_victory_reached(...)` quando não existe save default: deve criar save, marcar `victory_reached = true` e persistir `last_run_stats`.
- **Efeito esperado:** protege regressões no fluxo de seed+mutate (via `mutate_default_save` com `seed_empty_on_missing = true`), garantindo comportamento estável após refactors internos.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BY (testes: `persist_difficulty_if_save_exists` em ausência/presença de save) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_persist_difficulty_if_save_exists_handles_missing_and_present`, cobrindo os dois contratos públicos: (1) quando não há save default, a operação retorna sucesso como no-op sem criar arquivo; (2) quando há save default, o campo `difficulty` é atualizado e persistido.
- **Efeito esperado:** protege regressões no comportamento de atualização condicional de dificuldade, especialmente após extração para helper comum de mutação de save.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa BZ (testes: `load_default_save` em ausência/presença de save) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_load_default_save_reports_missing_and_present_data`, validando o contrato de `WorldSaveController::load_default_save(...)` para os dois cenários principais: sem arquivo (retorna `false`) e com save existente (retorna `true` e restaura campos persistidos).
- **Efeito esperado:** protege regressões no wrapper público de carregamento default, garantindo semântica estável de presença/ausência e roundtrip básico de dados.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CA (testes: `snapshot_last_run` sem save não cria arquivo) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_snapshot_last_run_missing_save_is_noop_without_creating_file`, cobrindo o contrato de no-op de `WorldSaveController::snapshot_last_run(...)` quando há `run_stats`, mas não existe save default em disco.
- **Efeito esperado:** protege regressões na política de merge/snapshot, garantindo que o fluxo não passe a criar save implicitamente em caminhos destinados a no-op.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CB (testes: `mark_victory_reached` preserva campos existentes) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_mark_victory_reached_preserves_existing_fields`, cobrindo o cenário com save default pré-existente e validando que `mark_victory_reached(...)` marca vitória e atualiza `last_run_stats` sem perder campos já persistidos (`gold` e posição).
- **Efeito esperado:** protege regressões no fluxo de mutação incremental de save, garantindo compatibilidade com dados previamente gravados.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CC (testes: `load_saved_difficulty` sem save preserva saída) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_load_saved_difficulty_missing_save_preserves_output_value`, validando que `WorldSaveController::load_saved_difficulty(...)` retorna `false` quando não há save default e não sobrescreve `out_difficulty` em caso de falha.
- **Efeito esperado:** protege a semântica de erro do carregamento de dificuldade, evitando regressões em chamadores que dependem da preservação do valor corrente no caminho de falha.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CD (testes: persistência bruta de dificuldade sem clamp local) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_persist_difficulty_stores_raw_value_without_local_clamp`, validando que `WorldSaveController::persist_difficulty_if_save_exists(...)` grava o índice informado sem clamp local.
- **Efeito esperado:** documenta e protege o contrato atual de responsabilidade separada (persistência grava; clamp ocorre no carregamento/aplicação), reduzindo risco de regressões semânticas em refactors futuros.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CE (testes: `persist` em stress mode não cria save) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_persist_stress_mode_does_not_create_save_file`, validando que `WorldSaveController::persist(...)` em `stress_mode` retorna sucesso como no-op, não altera `flash_timer` e não cria save default.
- **Efeito esperado:** protege regressões no guard de persistência em modo de stress, garantindo isolamento de I/O quando o modo está ativo.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CF (testes: `mark_victory_reached` torna save visível via `has_default_save`) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_mark_victory_reached_creates_visible_default_save`, validando que o caminho de seed em `mark_victory_reached(...)` efetivamente cria save default observável por `has_default_save()`.
- **Efeito esperado:** protege regressões de integração entre gravação de vitória e detecção de presença de save usada por fluxos de UI/boot.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CG (testes: `persist` sem save prévio cria save e indicador) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_persist_without_existing_save_creates_and_sets_indicator`, cobrindo `WorldSaveController::persist(...)` quando não existe save default: deve criar save, persistir dados do player e ajustar `flash_timer` quando `show_indicator = true`.
- **Efeito esperado:** protege regressões no caminho de autosave inicial (primeira gravação), validando criação de arquivo e feedback visual de sucesso.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CH (testes: `load_victory_unlock_flag` com save existente bloqueado) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_load_victory_unlock_flag_existing_false_reports_locked`, cobrindo cenário com save default existente e `victory_reached = false`.
- **Efeito esperado:** protege a semântica do retorno de carregamento (`true` para save encontrado) separada do estado de desbloqueio (`unlocked = false`), evitando regressões de interpretação no fluxo de menu/boot.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CI (testes: `apply_world_save` limpa estado transitório) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_apply_clears_transient_projectiles_and_ground_items`, validando que `WorldSaveController::apply_world_save(...)` limpa `projectiles` e `ground_items` quando esses ponteiros estão presentes no contexto.
- **Efeito esperado:** protege regressões de restauração de save que poderiam manter estado transitório indevido entre sessões/loads.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CJ (testes: `apply_world_save` restaura quest e scene flags) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_apply_restores_quest_state_and_scene_flags`, validando que `WorldSaveController::apply_world_save(...)` propaga corretamente `quest_state` e `scene_flags` do `SaveData` para o `WorldContext` quando os ponteiros existem.
- **Efeito esperado:** protege regressões de restauração de progresso global após load, especialmente para gates de fluxo e estado de quest.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CK (testes: `persist` preserva `victory_reached=false`) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_persist_preserves_existing_non_victory_flag`, cobrindo `WorldSaveController::persist(...)` com save default pré-existente e `victory_reached = false`.
- **Efeito esperado:** protege regressões na etapa de merge de `persist`, garantindo que o fluxo não promova vitória indevidamente quando o save existente ainda está bloqueado.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CL (testes: `make_world_save` copia estado global de runtime) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_make_world_save_copies_runtime_global_fields`, validando que `WorldSaveController::make_world_save(...)` copia corretamente `difficulty`, `scene_flags` e `quest_state` quando presentes no `WorldContext`.
- **Efeito esperado:** protege regressões no snapshot de estado global para persistência, evitando perdas silenciosas de progresso/flags no momento da serialização.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CM (testes: `make_world_save` copia `run_stats`) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_make_world_save_copies_run_stats_snapshot`, validando a cópia de métricas de `run_stats` para `last_run_stats` durante `WorldSaveController::make_world_save(...)`.
- **Efeito esperado:** protege regressões no snapshot estatístico da run, garantindo persistência consistente de telemetria usada em telas de resultado/progressão.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CN (testes: `make_world_save` com contexto vazio mantém defaults) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_make_world_save_empty_context_keeps_defaults`, validando que `WorldSaveController::make_world_save(...)` com `WorldContext` vazio retorna `SaveData` consistente com valores default.
- **Efeito esperado:** protege regressões em caminhos defensivos/bootstrapping, garantindo estabilidade quando dependências opcionais não estão conectadas.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CO (testes: `clear_default_saves` idempotente) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_clear_default_saves_is_idempotent`, validando sucesso em chamadas consecutivas de `WorldSaveController::clear_default_saves()` (remoção real seguida de no-op).
- **Efeito esperado:** protege regressões em fluxos de limpeza repetidos (boot/reset), garantindo semântica estável e segura para chamadas redundantes.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CP (testes: `load_default_save` sem save preserva saída) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_load_default_save_missing_preserves_output_value`, validando que `WorldSaveController::load_default_save(...)` retorna `false` sem sobrescrever o objeto de saída quando não existe save default.
- **Efeito esperado:** protege a semântica de erro do carregamento default e evita regressões em chamadores que reutilizam estado prévio no objeto de saída.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CQ (testes: clamp de dificuldade na leitura após persistência bruta) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_persist_then_load_difficulty_applies_read_time_clamp`, cobrindo fluxo integrado onde `persist_difficulty_if_save_exists(99)` grava valor bruto e `load_saved_difficulty(...)` retorna valor clampado (`Hard`).
- **Efeito esperado:** protege a separação de responsabilidades entre escrita e leitura de dificuldade, evitando regressões em que o clamp deixe de ocorrer no carregamento.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CR (testes: `snapshot_last_run` em stress mode não cria save) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_snapshot_stress_mode_does_not_create_save_file`, validando que `WorldSaveController::snapshot_last_run(...)` com `stress_mode=true` retorna sucesso como no-op e não cria save default.
- **Efeito esperado:** protege regressões na política de I/O desativado em modo stress para snapshots de run.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CS (testes: `make_world_save` serializa múltiplas áreas visitadas) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_make_world_save_serializes_multiple_visited_area_bits`, validando que `WorldSaveController::make_world_save(...)` serializa corretamente `visited_area_mask` com múltiplos bits (`DungeonRoom0`, `DungeonRoom2`, `Boss`).
- **Efeito esperado:** protege regressões na codificação de progresso de exploração no save (bitmask de áreas), evitando perdas silenciosas em mapas com múltiplas áreas já visitadas.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CT (testes: `apply_world_save` restaura máscara de áreas com múltiplos bits) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_apply_world_save_restores_multiple_visited_area_bits`, validando que `WorldSaveController::apply_world_save(...)` reidrata corretamente `AreaEntrySystem::visited_areas` a partir de `visited_area_mask` com múltiplos bits ativos.
- **Efeito esperado:** protege regressões no caminho de load para progresso de exploração, garantindo restauração fiel da máscara de áreas visitadas.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CU (testes: `mark_victory_reached` sobrescreve `last_run_stats` anterior) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_mark_victory_reached_overwrites_previous_last_run_stats`, validando que `WorldSaveController::mark_victory_reached(...)` atualiza `last_run_stats` com os novos dados mesmo quando já existe snapshot antigo no save.
- **Efeito esperado:** protege regressões no fluxo de atualização de estatísticas de vitória, evitando manutenção indevida de métricas defasadas.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CV (testes: `apply_world_save` restaura campos de runtime do player) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_apply_restores_player_runtime_fields_from_save`, validando que `WorldSaveController::apply_world_save(...)` reidrata campos essenciais do player (`gold`, `mana`, `stamina`, `attributes`, `progression`) a partir do `SaveData`.
- **Efeito esperado:** protege regressões no caminho de load que poderiam deixar runtime do jogador parcialmente desatualizado após aplicar save.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CW (testes: `make_world_save` sincroniza `attr_points_available`) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_make_world_save_syncs_attr_points_with_pending_level_ups`, validando que `WorldSaveController::make_world_save(...)` propaga `player.progression.pending_level_ups` para `attr_points_available` no `SaveData`.
- **Efeito esperado:** protege regressões no contrato de snapshot entre progressão em runtime e campo persistido de pontos de atributo.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CX (testes: `snapshot_last_run` preserva campos não-estatísticos) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_snapshot_last_run_preserves_existing_non_stats_fields`, validando que `WorldSaveController::snapshot_last_run(...)` atualiza `last_run_stats` sem sobrescrever campos preexistentes do save (`gold` e posição do player).
- **Efeito esperado:** protege regressões no fluxo de mutação parcial de save para snapshot de run, garantindo que apenas os campos-alvo sejam alterados.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CY (testes: `persist_difficulty_if_save_exists` preserva demais campos) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_persist_difficulty_preserves_other_save_fields`, validando que `WorldSaveController::persist_difficulty_if_save_exists(...)` atualiza `difficulty` sem sobrescrever campos não relacionados (`gold` e posição do player).
- **Efeito esperado:** protege regressões no caminho de mutação parcial de save, garantindo isolamento da atualização de dificuldade.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa CZ (testes: `persist` em stress mode não altera save existente) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_persist_stress_mode_preserves_existing_save_unchanged`, validando que `WorldSaveController::persist(...)` com `stress_mode=true` mantém intacto um save default já existente.
- **Efeito esperado:** protege regressões em que o caminho no-op de stress mode poderia sobrescrever dados persistidos indevidamente.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa DA (testes: `snapshot_last_run` sem `run_stats` não altera save existente) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_snapshot_without_run_stats_preserves_existing_save_unchanged`, validando que `WorldSaveController::snapshot_last_run(...)` com `run_stats = nullptr` retorna sucesso como no-op sem modificar um save default já existente.
- **Efeito esperado:** protege regressões no caminho defensivo de snapshot quando não há estatísticas disponíveis no contexto.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa DB (testes: clamp inferior de dificuldade na leitura após persistência bruta) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_persist_then_load_negative_difficulty_clamps_to_easy`, cobrindo fluxo integrado onde `persist_difficulty_if_save_exists(-42)` grava valor bruto e `load_saved_difficulty(...)` retorna valor clampado (`Easy`/`0`).
- **Efeito esperado:** protege regressões no clamp inferior de dificuldade no caminho de leitura, mantendo consistência com o contrato já validado para overflow positivo.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa DC (testes: `load_victory_unlock_flag` sem save força saída `false`) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_load_victory_unlock_missing_forces_output_false`, validando que `WorldSaveController::load_victory_unlock_flag(...)` sem save default retorna `false` e normaliza `out_victory_unlock` para `false` mesmo quando a entrada estava `true`.
- **Efeito esperado:** protege regressões no contrato defensivo do carregamento de unlock de vitória em ausência de persistência.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa DD (testes: `persist` em stress mode preserva `flash_timer` existente) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_persist_stress_mode_keeps_existing_flash_timer_value`, validando que `WorldSaveController::persist(...)` com `stress_mode=true` mantém `flash_timer` inalterado mesmo quando `show_indicator=true`.
- **Efeito esperado:** protege regressões no comportamento visual de no-op em stress mode, evitando sobrescrita indevida de feedback já ativo.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa DE (testes: `mark_victory_reached` preserva `difficulty` existente) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_mark_victory_reached_preserves_existing_difficulty`, validando que `WorldSaveController::mark_victory_reached(...)` marca vitória sem alterar o valor de `difficulty` já persistido.
- **Efeito esperado:** protege regressões de mutação parcial em save, evitando efeitos colaterais sobre configuração global de dificuldade durante o fluxo de vitória.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa DF (testes: `snapshot_last_run` preserva `difficulty` existente) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_snapshot_last_run_preserves_existing_difficulty`, validando que `WorldSaveController::snapshot_last_run(...)` atualiza estatísticas de run sem alterar `difficulty` previamente persistida.
- **Efeito esperado:** protege regressões de mutação parcial no snapshot de run, garantindo isolamento da configuração global de dificuldade.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa DG (testes: `load_default_save` presente sobrescreve saída anterior) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_load_default_save_present_overwrites_previous_output_data`, validando que `WorldSaveController::load_default_save(...)` com save existente substitui corretamente dados prévios do objeto de saída.
- **Efeito esperado:** protege regressões no contrato de carga bem-sucedida, evitando retenção acidental de valores stale no objeto de destino.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa DH (testes: seed de `mark_victory_reached` mantém defaults esperados) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_mark_victory_reached_seeds_expected_default_difficulty`, validando que `WorldSaveController::mark_victory_reached(...)` sem save prévio cria save válido com defaults esperados (incluindo `difficulty=1`) e marca vitória.
- **Efeito esperado:** protege regressões no caminho de seed em ausência de save, garantindo consistência de defaults persistidos.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa DI (testes: `apply_world_save` com contexto vazio é no-op seguro) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_apply_world_save_empty_context_is_safe_noop`, validando que `WorldSaveController::apply_world_save(...)` com `WorldContext` sem ponteiros opcionais executa sem falha e sem efeitos colaterais.
- **Efeito esperado:** protege regressões em caminhos defensivos durante load parcial/inicialização, onde o contexto pode estar incompleto.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa DJ (testes: `apply_world_save` tolera `difficulty` nulo) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_apply_world_save_handles_null_difficulty_pointer`, validando que `WorldSaveController::apply_world_save(...)` com `ctx.difficulty == nullptr` não falha e continua aplicando os demais campos.
- **Efeito esperado:** protege regressões em caminhos de aplicação parcial onde nem todas as dependências opcionais do contexto estão conectadas.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa DK (testes: `apply_world_save` tolera `scene_flags` nulo) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_save_controller.cpp`.
- **Mudança:** adicionado teste `world_save_apply_world_save_handles_null_scene_flags_pointer`, validando que `WorldSaveController::apply_world_save(...)` com `ctx.scene_flags == nullptr` não falha e mantém aplicação dos demais campos.
- **Efeito esperado:** protege regressões em cenários de aplicação parcial com ponteiros opcionais ausentes no contexto.
- **Validação:** build/testes locais executados após a mudança (resultado abaixo nesta sessão).

### 2026-04-06 — Tarefa DL (`g_attribute_scales`: constante canônica + reset) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/components/attributes.hpp`.
- **Mudança:** adicionada constante canônica `kDefaultAttributeScales` (`constexpr`) e `reset_attribute_scales_defaults()`; `g_attribute_scales` passa a inicializar a partir da constante canônica. Mesmo padrão de AM-AO.
- **Efeito esperado:** reduz débito de seção 12.1 (global mutável nunca mutado), torna explícito o baseline de escalas e prepara reset para futuro `attributes.ini`.
- **Validação:** build completo OK; suíte core permanece verde (849 passed, 0 failed).

### 2026-04-06 — Tarefa DM (`g_theme`: constante canônica + enforced const) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/ui.hpp`.
- **Mudança:** adicionada constante `kDefaultTheme` (`constexpr`) e `g_theme` passou a ser `inline const`, inicializado a partir dela. Nunca mutado em runtime; `const` enforça o contrato.
- **Efeito esperado:** elimina global mutável de tema da lista de hotspots (seção 12.1), evita mutações acidentais futuras e documenta que `g_theme` é uma constante de estilo, não estado de sessão.
- **Validação:** build completo OK; suíte core permanece verde (849 passed, 0 failed).

### 2026-04-06 — Tarefa DN (`apply_world_save`: HP restoration + clamping consolidado) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/world_save_controller.hpp`, `src/scenes/world_scene.hpp`, `tests/world/test_world_save_controller.cpp`.
- **Mudança:** `apply_world_save` passou a restaurar `health.current_hp` a partir de `sd.player_hp` (junto com mana/stamina já restaurados). O bloco inline em `enter()` foi simplificado para somente clamping pós-`configure_player`, removendo a re-atribuição redundante de mana/stamina. Adicionados: teste unitário `world_save_apply_restores_player_hp_from_save` e verificação de HP no roundtrip `world_save_make_and_apply_restores_position_and_mask`.
- **Efeito esperado:** `apply_world_save` passa a ter ownership completo de restauração do estado de recursos do player; elimina duplicação de atribuição de mana/stamina em `enter()`; fecha o gap de cobertura de HP no roundtrip de save.
- **Validação:** build completo OK; suíte core passou com cobertura ampliada (851 passed, 0 failed).

### 2026-04-06 — Tarefa DO (`SceneExitRequest`: encapsular delayed transition) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/scenes/world_scene.hpp`.
- **Mudança:** criado `struct SceneExitRequest` (nested private) com `schedule(target, delay)` e `tick(dt) → string`; os três campos separados `_scene_exit_pending / _scene_exit_timer / _scene_exit_target` foram substituídos por `_scene_exit`; callsites atualizados (`enter()` reset, callback de post-mortem dialogue, `fixed_update` tick block).
- **Efeito esperado:** protocolo de transição atrasada explicitamente encapsulado com método nomeado; reduz 3 campos soltos a 1 struct com semântica clara; elimina 6 linhas de boilerplate no `fixed_update`.
- **Validação:** build completo OK; suíte core permanece verde (851 passed, 0 failed).

### 2026-04-06 — Tarefa DP (IDs de diálogo de dungeon: constantes canônicas) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/dungeon_dialogue.hpp`, `src/scenes/world_scene.hpp`, `src/systems/boss_state_controller.hpp`, `src/systems/enemy_death_controller.hpp`.
- **Mudança:** criado `namespace DungeonDialogueId` com constantes `constexpr const char*` para todos os IDs de diálogo de dungeon (`kPrologue`, `kRoom2`, `kDeeper`, `kRareRelic`, `kMinibossDeath`, `kBossPhase2`); substituídos todos os string literals equivalentes nos callsites.
- **Efeito esperado:** elimina stringly-typed IDs de diálogo espalhados em camadas de lógica; typos em IDs passam a ser erros de compilação; alinhado com seção 12.4 (identidade não deve ser baseada em string livre na lógica de domínio).
- **Validação:** build completo OK; suíte core permanece verde (851 passed, 0 failed).

### 2026-04-06 — Tarefa DQ (`PlayerDeathFlow`: encapsular estado de morte) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/combat_fx_controller.hpp`, `src/scenes/world_scene.hpp`, `tests/systems/test_dungeon_flow_and_animation.cpp`.
- **Mudança:** criado `struct PlayerDeathFlow` em `combat_fx_controller.hpp` com `reset()` e `tick(player_alive, dt, on_snapshot) → string`; campos soltos `_death_snapshot_done` e `_death_fade_remaining` (+ constante `kDeathFadeSeconds`) foram substituídos por `_death_flow`; `CombatFxController::update_death_flow` (template com 7 parâmetros por referência) foi removido; teste atualizado para usar `PlayerDeathFlow::tick` diretamente.
- **Efeito esperado:** morte do player passa a ter semântica explícita encapsulada, com interface nomeada (mesmo padrão do `SceneExitRequest`); reduz 2 campos + 1 constante a 1 struct; elimina função de controle com contrato implícito de mutação de referências externas.
- **Validação:** build completo OK; suíte core permanece verde (851 passed, 0 failed).

### 2026-04-06 — Tarefa DR (`dungeon_audio_system.hpp`: remoção de dead code) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/dungeon_audio_system.hpp`.
- **Mudança:** removido header `dungeon_audio_system.hpp` que não era incluído em nenhum ponto de `src/` nem de `tests/` — código morto remanescente de extração anterior.
- **Efeito esperado:** elimina módulo paralelo sem chamador, reduzindo ruído no grafo de dependências e risco de cópia acidental do padrão obsoleto.
- **Validação:** build completo OK; suíte core permanece verde (851 passed, 0 failed).

### 2026-04-06 — Tarefa DS (`ShopInputController`: stateful, encapsular estado de input) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/shop_input_controller.hpp`, `src/scenes/world_scene.hpp`, `tests/systems/test_town_scene_modules.cpp`.
- **Mudança:** `ShopInputController` deixou de ser namespace com função que recebia `prev_move_y`/`prev_confirm`/`prev_cancel` por referência externa e passou a ser classe que encapsula esses três campos internamente; `WorldScene` substituiu os 3 campos soltos por `_shop_input` (instância do controller); teste atualizado para usar `ctrl.update(...)` diretamente.
- **Efeito esperado:** ownership do estado de input de loja fica explícito no controller, removendo estado auxiliar vazado na cena e alinhando ao padrão dos demais controllers de UI.
- **Validação:** build completo OK; suíte core permanece verde (851 passed, 0 failed).

### 2026-04-06 — Tarefa DT (`PostMortemDialogueFlow`: encapsular diálogo pós-morte) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/scenes/world_scene.hpp`.
- **Mudança:** criado `struct PostMortemDialogueFlow` (inner private) com `trigger(id)` e `tick(dialogue, scene_transition_pending) → string`; campo solto `_post_mortem_dialogue_id` substituído por `_post_mortem_flow`; callsites atualizados (trigger em `EnemyDeathController::process_deaths`, consumo em `fixed_update`).
- **Efeito esperado:** o fluxo de diálogo pós-morte passa a ter semântica explícita e nomeada, consistente com `SceneExitRequest` e `PlayerDeathFlow`; elimina string solta com papel implícito de estado de máquina de estados.
- **Validação:** build completo OK; suíte core permanece verde (851 passed, 0 failed).

### 2026-04-06 — Tarefa DU (IDs de cena: constantes canônicas) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/scene_ids.hpp` (novo), `src/core/register_scenes.cpp`, `src/main.cpp`, `src/scenes/world_scene.hpp`, `src/scenes/game_over_scene.hpp`, `src/scenes/victory_scene.hpp`, `src/scenes/credits_scene.hpp`, `src/scenes/title_scene.hpp`, `src/systems/combat_fx_controller.hpp`, `src/core/title_menu.hpp`.
- **Mudança:** criado `namespace SceneId` com constantes `constexpr const char*` para os 5 IDs de cena (`kTitle`, `kWorld`, `kGameOver`, `kVictory`, `kCredits`) e o sentinel especial `kQuit` (`__quit__`); substituídos todos os ~20 literais string equivalentes nos callsites.
- **Efeito esperado:** elimina stringly-typed scene IDs espalhados em 9 arquivos; typos em IDs passam a ser erros de compilação; alinhado com o padrão de `DungeonDialogueId` (Tarefa DP) e seção 12.4 (identidade não deve ser baseada em string livre na lógica de domínio).
- **Validação:** build completo OK; suíte core permanece verde (851 passed, 0 failed).

### 2026-04-06 — Tarefa DV (IDs de diálogo da town: constantes canônicas) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/town_dialogue_ids.hpp` (novo), `src/systems/town_builder.hpp`, `src/systems/town_npc_interaction.hpp`, `tests/core/test_town_config_loader.cpp`.
- **Mudança:** criado `namespace TownDialogueId` com `constexpr const char*` para `mira_default`, `mira_quest_offer`, `mira_quest_active`, `mira_quest_done` e `forge_greeting`; substituídos literais nos defaults de NPC, no fluxo de quest da Mira e no teste de registo do INI.
- **Efeito esperado:** alinha town ao padrão de `DungeonDialogueId` / `SceneId`; typos em IDs cruzados entre builder, interação e dados passam a ser erros de compilação; reforça seção 12.4 (identidade de conteúdo referenciada na lógica via chaves centralizadas).
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (**820 passed, 0 failed**).
### 2026-04-07 — Tarefa DW (`PlayerActorId::kName`: nome canónico do Actor do jogador) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/player_actor_id.hpp` (novo), `src/systems/player_configurator.hpp`, `tests/legacy/test_legacy.cpp`.
- **Mudança:** introduzido `namespace PlayerActorId` com `kName` (`"player"`) como única fonte para o literal usado em `configure_player`; testes legados de melee que simulam o jogador passam a usar a mesma constante. A `WorldScene` mantém `ev.target_name == _player.name` (correto se o nome vier de save/config no futuro).
- **Efeito esperado:** documenta e centraliza o contrato entre `Actor::name` do player e os eventos `CombatEvent` / `ProjectileSystem::HitEvent` que gravam `target_name`; reduz risco de typo ao alinhar testes com o runtime.
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (**820 passed, 0 failed**).

### 2026-04-07 — Tarefa DX (portas / layout mundo: `WorldLayoutId` + teste de porta → cena) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/world_layout_ids.hpp` (novo), `src/systems/town_builder.hpp`, `src/systems/room_manager.hpp`, `src/world/world_map_builder.hpp`, `tests/world/test_town_layout.cpp`, `tests/legacy/test_legacy.cpp`.
- **Mudança:** criado `namespace WorldLayoutId` com `kTown`, `kCorridor`, `kDungeon` para `RoomDefinition::name` e `DoorZone::target_scene_id` no layout contínuo; substituídos literais em builder de town, portas de retorno ao town em salas de dungeon e nome do corredor de transição. Teste legado `RoomFlow.TargetScene` passou a usar `SceneId::kTitle` em vez do literal `"title"`.
- **Efeito esperado:** tokens de transição por porta e nomes de room estáveis ficam centralizados (alinhado a DP/DU/DV); documenta que `target_scene_id` em portas pode ser tag semântica, não só id de registo de cena.
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (contagem atual no bloco abaixo).

### 2026-04-07 — Tarefa DY (teste do mapa: validar `kCorridor` canónico) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_world_map_builder.cpp`.
- **Mudança:** o teste passou a incluir `world_layout_ids.hpp` e validar `corridor->room.name == WorldLayoutId::kCorridor` no `WorldMapBuilder::build(...)`.
- **Efeito esperado:** aumenta cobertura do contrato de naming canónico do layout (porta/transição), evitando regressão silenciosa para literal solto no corredor.
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (**821 passed, 0 failed**).

### 2026-04-07 — Tarefa DZ (`DoorZone::target_scene_id`: contrato semântico + teste de passthrough) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/world/room.hpp`, `tests/legacy/test_legacy.cpp`.
- **Mudança:** comentário de contrato em `DoorZone` expandido para explicitar semântica de `target_scene_id` (vazio = avanço de sala; não vazio = passthrough para `RoomFlowSystem::scene_exit_to`) e os dois tipos de token esperados (`SceneId::*` e `WorldLayoutId::*`). Adicionado teste `RoomFlow.PassthroughTag` garantindo que uma porta com `WorldLayoutId::kTown` é repassada sem interpretação no `RoomFlowSystem`.
- **Efeito esperado:** reduz ambiguidade arquitetural no fluxo de portas e fixa por teste o comportamento transport-only do `RoomFlowSystem`, evitando regressão para acoplamento com parser local de strings.
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (**823 passed, 0 failed**).

### 2026-04-07 — Tarefa EA (`RoomManager` portas de dungeon: cobertura de contrato) — **Concluída (versão incremental)**

- **Escopo aplicado:** `tests/world/test_room_manager_doors.cpp` (novo).
- **Mudança:** adicionados testes para `RoomManager::build_room(...)` fixando o contrato atual de portas: (a) sala não-boss cria porta de avanço (`target_scene_id` vazio) + porta de retorno para `WorldLayoutId::kTown`; (b) sala boss cria apenas porta de saída para `WorldLayoutId::kTown`.
- **Efeito esperado:** reforça por teste a semântica de transição por porta no layout de dungeon e protege contra regressões de tokens/quantidade de portas ao evoluir builders.
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (**828 passed, 0 failed**).

### 2026-04-07 — Tarefa EB (boss data-driven: `EnemyDef::is_zone_boss` como critério canônico) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/entities/enemy_type.hpp`, `src/systems/boss_state_controller.hpp`, `src/systems/world_audio_system.hpp`, `src/systems/enemy_death_controller.hpp`, `src/scenes/world_scene.hpp`.
- **Mudança:** `EnemyDef` ganhou `is_zone_boss` (com default canônico em `BossGrimjaw` e override via `enemies.ini`); decisões centrais de boss em runtime deixaram de comparar `EnemyType::BossGrimjaw` diretamente e passaram a usar `EnemyDef::is_zone_boss` em `BossState::update`, `WorldAudioSystem::update`, `EnemyDeathController::process_deaths` e no reset de estado de boss em transição de zona na `WorldScene`.
- **Efeito esperado:** reduz acoplamento da lógica central a um boss específico, preparando evolução para múltiplos bosses sem espalhar novos `if (EnemyType::BossX)`; mantém strings apenas em conteúdo/narrativa.
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (**828 passed, 0 failed**).

### 2026-04-07 — Tarefa EC (town NPC interaction: API única com semântica de alcance) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/town_npc_interaction.hpp`, `tests/systems/test_town_scene_modules.cpp`.
- **Mudança:** removida a API pública paralela `find_nearest_npc(...)` (que retornava índice sem semântica de alcance); `find_nearest_npc_for_interaction(...)` passou a ser a única entrada para seleção de NPC no fluxo de interação, calculando nearest + `in_interaction_range` no mesmo contrato. Teste de módulos da town foi migrado para a API consolidada.
- **Efeito esperado:** elimina caminho alternativo que podia reintroduzir divergência entre “NPC mais próximo” e “NPC realmente interagível”, reforçando fonte única de verdade geométrica do item 7.1.
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (**829 passed, 0 failed**).

### 2026-04-07 — Tarefa ED (run stats por evento: extrair tracker + cobertura de cenários) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/run_stats_tracker.hpp` (novo), `src/scenes/world_scene.hpp`, `tests/systems/test_run_stats_tracker.cpp` (novo).
- **Mudança:** extraída a soma de dano recebido para `RunStatsTracker::damage_taken_for_actor(...)`, que agrega apenas eventos de dano (`MeleeCombatSystem::last_events` + `ProjectileSystem::last_hit_events`) por `target_name`; `WorldScene` passou a consumir esse tracker em vez de loops ad hoc no fluxo principal. Adicionados 3 testes dedicados cobrindo: dano puro, frame de cura sem dano e dano+cura no mesmo frame (sem depender de delta de HP final).
- **Efeito esperado:** cristaliza por contrato que `damage_taken` é orientado a eventos de dano e não a snapshot agregado de HP; reduz risco de regressão para o bug de mascaramento em frames com dano e cura simultâneos.
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (**832 passed, 0 failed**).

### 2026-04-07 — Tarefa EE (game loop: cap de substeps contra “spiral of death”) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/main.cpp`.
- **Mudança:** loop principal passou a usar `max_substeps_per_frame` e clamp de acumulador (`accumulator = min(accumulator, fixed_dt * max_substeps_per_frame)`), limitando backlog de física por frame sem alterar o modelo de fixed timestep.
- **Efeito esperado:** evita explosão de substeps após frame lento, reduzindo picos de CPU e degradação perceptível de responsividade.
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (**832 passed, 0 failed**).

### 2026-04-07 — Tarefa EF (locale: remover dependência de estado global implícito) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/locale.hpp`, `src/main.cpp`, cenas/UI que consumem locale via contexto.
- **Mudança:** runtime de locale ficou orientado a dependência explícita (`SceneCreateContext::locale` + ponteiro/referência por cena/componente); não há uso ativo de `locale_bind`, `detail::active_locale` ou `L(...)` em `src/`.
- **Efeito esperado:** elimina acoplamento a estado global oculto de locale e melhora previsibilidade/testabilidade de texto por contexto.
- **Validação:** guardrail `rg 'detail::active_locale|locale_bind\(|\bL\(' src/` sem ocorrências; build e suíte core verdes (**832 passed, 0 failed**).

### 2026-04-07 — Tarefa EG (town NPC interaction: unificar nearest + range no contrato) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/town_npc_interaction.hpp`, `src/scenes/world_scene.hpp`, `tests/systems/test_town_scene_modules.cpp`.
- **Mudança:** consolidado uso de `find_nearest_npc_for_interaction(...)` como API de decisão para hint/confirm de interação, removendo caminho paralelo sem semântica de alcance.
- **Efeito esperado:** fonte única de verdade geométrica para interação de NPC na town; menor risco de divergência entre “NPC mais próximo” e “NPC interagível”.
- **Validação:** build e suíte core verdes (**832 passed, 0 failed**).

### 2026-04-07 — Tarefa EH (HUD/town UI strings: locale como fonte canônica no render) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/scenes/world_scene.hpp`, `data/locale_en.ini`, `data/locale_ptbr.ini`.
- **Mudança:** hints e labels de town/HUD em `WorldScene::render` usam chaves de locale (`town_npc_interact_hint`, `town_gold_label`, `ui_saved`) em vez de literais fixos.
- **Efeito esperado:** reduz string hardcode na cena, melhora localização e consistência de UI.
- **Validação:** build e suíte core verdes (**832 passed, 0 failed**).

### 2026-04-07 — Tarefa EI (`WorldScene::enter`: extrair pipeline de bootstrap) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/scenes/world_scene.hpp`.
- **Mudança:** `enter()` foi decomposto em helpers nomeados sem alterar comportamento: `_load_save_or_start_fresh(...)`, `_configure_player_for_session(...)` e `_initialize_runtime_after_enter(...)`.
- **Efeito esperado:** reduz densidade de orquestração inline em `enter()`, explicita ordem do bootstrap (save/fresh -> configure -> runtime) e facilita testes/refactors locais futuros.
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (**832 passed, 0 failed**).

### 2026-04-07 — Tarefa EJ (transição pós-morte/vitória: unificar estado explícito) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/scenes/world_scene.hpp`.
- **Mudança:** estados de transição de cena ligados a pós-morte foram consolidados em `SceneTransitionFlow` (fila de diálogo post-mortem + exit diferido por timer), substituindo structs/estado separados (`PostMortemDialogueFlow` e `SceneExitRequest`) por um owner único.
- **Efeito esperado:** reduz máquina de estados implícita distribuída pela cena, com contrato único de tick/agenda para transições pós-combate.
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (**832 passed, 0 failed**).

### 2026-04-07 — Tarefa EK (`RunStatsTracker`: consolidar delta de frame além de dano) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/run_stats_tracker.hpp`, `src/scenes/world_scene.hpp`, `tests/systems/test_run_stats_tracker.cpp`.
- **Mudança:** adicionado `RunStatsTracker::FrameStatsDelta` + `apply_frame_delta(...)`, movendo para tracker a aplicação de métricas por frame (`damage_taken`, `gold_collected`, `spells_cast`, `time_seconds`) que estavam inline na `WorldScene`.
- **Efeito esperado:** reduz responsabilidade de estatísticas agregadas dentro da cena e estabelece ponto único para evolução de métricas de run.
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (**835 passed, 0 failed**).

### 2026-04-07 — Tarefa EL (UI controller contract incremental: `ShopInputController` retorna intenção) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/shop_input_controller.hpp`, `src/scenes/world_scene.hpp`, `tests/systems/test_town_scene_modules.cpp`.
- **Mudança:** `ShopInputController::update(...)` passou a retornar `ShopInputResult` (`world_paused`, `should_save`) no mesmo modelo de “controller retorna intenção”; `WorldScene` agora persiste save por `should_save` após compra na loja.
- **Efeito esperado:** reduz divergência de contrato entre controllers de UI (skill tree/attribute/equipment já retornavam intenção), aproximando a loja do padrão canônico de ownership de side effect no owner superior.
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (**835 passed, 0 failed**).

### 2026-04-07 — Tarefa EM (input de loja alinhado ao modelo unificado de `OverlayInputEdges`) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/shop_input_controller.hpp`, `src/scenes/world_scene.hpp`, `tests/systems/test_town_scene_modules.cpp`.
- **Mudança:** `ShopInputController` deixou de depender de `InputState` bruto + estado interno de edge para usar `OverlayInputEdges` (`up/down/confirm/back`) como entrada canônica de UI modal; `WorldScene` passou a repassar `overlay_input` para o controller da loja.
- **Efeito esperado:** reduz fragmentação de mecanismos de input de UI e evita criação de mais um tracker paralelo para menu/overlay.
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (**835 passed, 0 failed**).

### 2026-04-07 — Tarefa EN (persistência por intenção: loja sem side effect escondido) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/shop_input_controller.hpp`, `src/scenes/world_scene.hpp`.
- **Mudança:** compra na loja sinaliza `should_save` no resultado do controller; persistência permanece no owner superior (`WorldScene`) via `_persist_save()`.
- **Efeito esperado:** reforça regra de ownership para side effects de persistência (controller retorna intenção, cena decide quando persistir), alinhando loja ao padrão já usado por skill tree/atributos/equipamento.
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (**835 passed, 0 failed**).

### 2026-04-07 — Tarefa EO (UI text/render: hint de diálogo via locale) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/dialogue_render.hpp`, `src/scenes/world_scene.hpp`, `data/locale_en.ini`, `data/locale_ptbr.ini`.
- **Mudança:** `render_dialogue_ui(...)` passou a receber locale opcional e consumir `dialogue_continue_hint` em vez de string hardcoded no renderer de diálogo.
- **Efeito esperado:** reduz hardcode de texto na camada de render e mantém consistência de tradução com o idioma ativo.
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (**835 passed, 0 failed**).

### 2026-04-07 — Tarefa EP (contrato mínimo canônico para UI controllers) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/systems/ui_controller_contract.hpp` (novo), `src/systems/equipment_screen_controller.hpp`, `src/systems/shop_input_controller.hpp`.
- **Mudança:** introduzido `UiControllerResult` (`world_paused`, `should_save`) como contrato mínimo reutilizável para controllers modais; `EquipmentResult` e `ShopInputResult` passaram a reutilizar esse contrato.
- **Efeito esperado:** reduz deriva de contratos de retorno entre módulos de UI e torna explícita a semântica canônica de pausa/persistência por intenção.
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (**835 passed, 0 failed**).

### 2026-04-07 — Tarefa EQ (`WorldMapBuilder`: preservar portas da town após `TownBuilder`) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/world/world_map_builder.hpp`, `tests/world/test_world_map_builder.cpp`, `bug_fix_refactor.md` (resolução do aviso DX).
- **Mudança:** removido `town.room.doors.clear()` após `TownBuilder::build_town_world(...)`, mantendo a `DoorZone` town→dungeon (`WorldLayoutId::kDungeon`) para render (`town_world_renderer`) e alinhamento ao contrato de dados já coberto por `test_town_layout.cpp`. Dungeon/corridor continuam sem portas após o passo de build (transição continua por `ZoneManager`). Teste do map builder passou a exigir 1 porta na área Town e 0 nas demais.
- **Efeito esperado:** elimina divergência entre layout definido no builder e mapa montado; evita “porta fantasma” só em testes isolados de town.
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (**836 passed, 0 failed**).

### 2026-04-07 — Tarefa ER (pause menu: `OverlayInputEdges` + `UiControllerResult`; edges em `core`) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/input.hpp` (`OverlayInputEdges`), `src/systems/overlay_input.hpp`, `src/core/pause_menu.hpp` (`handle_overlay_edges`), `src/systems/pause_menu_controller.hpp` (`PauseMenuResult` deriva `UiControllerResult`; `update(edges)`), `src/scenes/world_scene.hpp`, `src/systems/skill_tree_controller.hpp`, `src/systems/attribute_levelup_controller.hpp`, `tests/systems/test_dungeon_controllers.cpp`, `ARCHITECTURE_GUIDE.md` (tabela de input).
- **Mudança:** `OverlayInputEdges` passou a viver em `core/input.hpp` (evita acoplamento `PauseMenu`↔`systems`). `PauseMenu` deixou de calcular flancos internos a partir de `InputState` e consome o mesmo pacote de edges que skill tree / equip / loja. `WorldScene` trata pausa com `overlay_input` já capturado. `SkillTreeResult` e `AttributeLevelUpResult` passam a estender `UiControllerResult`. Testes de `PauseMenuController` usam `OverlayInputTracker` como em produção.
- **Efeito esperado:** um único roteador de bordas para UI modal; contrato de retorno alinhado ao EP em mais controllers; reduz débito 11.2 (pause vs `OverlayInputEdges`).
- **Validação:** build completo OK; `./build/mion_tests_v2_core` verde (contagem no bloco abaixo).

### 2026-04-07 — Tarefa ES (`_prev_upgrade_*`/`_prev_talent_*`: remoção de dead state) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/scenes/world_scene.hpp`.
- **Mudança:** removidos 6 campos `_prev_upgrade_1/2/3` e `_prev_talent_1/2/3` e 6 atribuições correspondentes em `_sync_gameplay_input_history`; campos eram escritos mas nunca lidos (dead state após refactors anteriores que moveram lógica de upgrade/talent para controllers).
- **Efeito esperado:** reduz estado desnecessário na cena e simplifica `_sync_gameplay_input_history` ao contrato real (`_prev_potion`, `_prev_confirm`).
- **Validação:** build completo OK; suíte core permanece verde (867 passed, 0 failed).

### 2026-04-07 — Tarefa ET (`configure_player`: mana/stamina não preservados ao carregar save) — **Concluída**

- **Escopo aplicado:** `src/systems/player_configurator.hpp`, `src/scenes/world_scene.hpp`.
- **Problema:** `apply_world_save` restaurava `player.mana` e `player.stamina` do save, mas `configure_player` logo a seguir sobrescrevia incondicionalmente `.current` com os valores base (`g_player_config.base_mana`, `base_stamina`). Os valores do save eram descartados silenciosamente. As linhas de clamp em `_configure_player_for_session` (mana/stamina) eram dead code consequente.
- **Mudança:**
  - Adicionado campo `bool preserve_resources = false` em `PlayerConfigureOptions`.
  - Em `configure_player`, os campos `.current` de stamina e mana capturam o valor anterior antes do reset do struct; quando `preserve_resources = true`, o `.current` é restaurado com clamp ao novo `.max`.
  - Em `_configure_player_for_session`, `opts.preserve_resources = has_save`.
  - Removidas as linhas de clamp de mana/stamina em `_configure_player_for_session` (eram dead code; o clamp agora acontece dentro de `configure_player`). O clamp de HP permanece.
- **Efeito esperado:** ao retomar um save, o jogador mantém a mana e stamina salvas (dentro do max recalculado).
- **Validação:** build completo OK; ctest 100% (4/4 suítes passam, 867 core passed, 0 failed).

### 2026-04-07 — Tarefa EU (`TitleScene`: edge tracking consolidado no topo de `fixed_update`) — **Concluída**

- **Escopo aplicado:** `src/scenes/title_scene.hpp`.
- **Problema:** `_prev_ui_up`, `_prev_ui_down`, `_prev_confirm`, `_prev_attack`, `_prev_cancel` eram atualizados dentro de cada branch da state machine de UI, com early returns (cancel em DifficultySelect/Settings/Extras) deixando `_prev_ui_up` e `_prev_ui_down` desatualizados — mesmo padrão de bug resolvido na Tarefa B para `WorldScene`.
- **Mudança:** todos os edges são calculados no topo de `fixed_update` a partir dos `_prev_*` do tick anterior; em seguida, todo o histórico é sincronizado em um bloco único antes de qualquer branch. Os branches passam a usar as variáveis locais `edge_*` (erase, pause, up, down, confirm, cancel) — sem atribuições inline de `_prev_*` espalhadas.
- **Efeito esperado:** edge tracking do title screen é consistente independente de early returns; código mais legível (estado machine separado do gerenciamento de histórico).
- **Validação:** build completo OK; ctest 100%.

### 2026-04-07 — Tarefa EV (auditoria 12.7/12.8: já resolvidos) — **Registrado**

- **Resultado da auditoria:**
  - **12.7** (audio paths / talent defaults duplicados): `audio.cpp` e `asset_manifest.hpp` já usam `audio_asset_paths.hpp` como fonte única. `talent_data.hpp` já usa `kDefaultTalentNodes` em `reset_talent_tree_defaults()`. Não há duplicação real — débito já liquidado.
  - **12.8** (encapsulamento fraco): `DialogueSystem` já tem todos os internals privados (`_sequences`, `_active_lines`, `_line_index`, `_is_active`, `_prev_confirm`, `_on_finish`). `PauseMenu`/`PauseMenuController` já expõem apenas accessors (`was_pause_pressed()`, etc.), com `_prev_*` privados. Débito já liquidado.

### 2026-04-07 — Tarefa EW (auditoria 1.4 / 5.1 / 12.2: guardrails e bootstrap) — **Concluída**

- **1.4 (rand/srand):** zero ocorrências de `rand(`/`srand(` em `src/`. Guardrail confirmado. `main.cpp` usa `std::mt19937`; sistemas recebem `std::mt19937&`. Nenhuma ação necessária.

- **5.1 (Grimjaw hardcoded):** auditoria mostrou que o problema estrutural já estava resolvido. Todos os pontos de lógica (`BossState::update`, `WorldAudioSystem`, `_handle_zone_transition`) usam `enemy_defs[...].is_zone_boss` — sem comparação por string. `EnemyType::BossGrimjaw` e `QuestId::DefeatGrimjaw` são enums. Strings `"Grimjaw"` remanescentes são apenas na camada de conteúdo (nome de exibição no spawner, texto de diálogo). Corrigido também comentário stale em `save_data.hpp:41` que documentava bits 2/3 de `scene_flags` como implementados — na realidade nunca foram escritos ou lidos por lógica de runtime.

- **12.2 (bootstrap duplicado):** `WorldScene::enter()` chamava `DungeonConfigLoader::load_dungeon_static_data` (que internamente chama `CommonPlayerProgressionLoader::load_defaults_and_ini_overrides`) e depois `TownConfigLoader::load_town_player_and_progression_config` (que chama a mesma função novamente). Removida a segunda chamada redundante de `enter()`. `TownConfigLoader::load_town_player_and_progression_config` permanece no header — válida para uso standalone e coberta por teste isolado.

- **Validação:** build completo OK; ctest 100%.

### 2026-04-07 — Tarefa EX (auditoria 6.1 / 11.3 / cenas restantes) — **Registrado**

- **6.1 (diálogo de relíquia rara):** política já existe. `WorldScene` tem `_rare_relic_dialogue_pending` (set em cada pickup) e `_rare_relic_dialogue_shown` (impede reexibição no mesmo session). Diálogo dispara no máximo uma vez por `enter()`, sem spam. Sem ação adicional necessária — débito liquidado.

- **11.3 residual (TownNpcInteraction + persistência):** `handle_npc_interaction` recebe `request_persist` como callback opcional explícito; o callsite em `WorldScene` passa `[this]{ _persist_save(); }`. Side effect está na assinatura (parâmetro), não escondido. Padrão difere do `should_save` canônico, mas é explicito e controlado pelo owner. Débito aceitável sem tarefa dedicada — registrado como padrão alternativo documentado.

- **Auditoria de cenas restantes (early returns / stale `_prev_*`):**
  - `VictoryScene`: sem early returns em `fixed_update`; `_prev_up/_down` atualizados inline, `_prev_confirm` ao final — sem risco de stale.
  - `GameOverScene`: mesma estrutura; sem early returns.
  - `CreditsScene`: `_prev_cancel` único; atualizado após o único check, sem branches.
  - Nenhuma ação necessária.

### 2026-04-07 — Tarefa EY (auditoria 12.4 / 11.6: identidade por string e EnemyDeathController) — **Registrado**

- **12.4 (SceneId como enum class):** avaliação completa do escopo. Todos os callers já usam `SceneId::k*` constantes (typos são erros de compilação no lado do caller). O bloqueador para migração completa é `DoorZone::target_scene_id` (`room.hpp`): campo `std::string` que mistura dois namespaces distintos — `SceneId` (ex.: `"title"`) para transições de cena e `WorldLayoutId` (ex.: `"town"`, `"dungeon"`) para roteamento intra-world. Antes de converter `SceneId` para `enum class`, o campo precisa ser separado em dois campos tipados (`SceneId exit_to_scene` e `WorldZoneId exit_to_zone`), o que expande o escopo para `room.hpp`, `RoomFlowSystem`, `world_map_builder.hpp`, WorldScene e testes. **Decisão: adiar para tarefa dedicada com escopo explícito para esse split.** `SceneId` como namespace de constantes é seguro para uso atual.

- **11.6 (EnemyDeathController side effects):** auditoria confirmou uso de `is_zone_boss` flag (não string) e `QuestId::DefeatGrimjaw` enum. O módulo orquestra mortes de forma explícita: todos os subsistemas (drops, partículas, áudio, run_stats, quest) recebem por parâmetro. Retorna `DeathResult` com `boss_defeated`, `quest_completed`, `xp_gained` — a cena decide sobre persistência. Áudio chamado internamente é aceitável (parâmetro explícito). **Nenhuma ação necessária.**

### 2026-04-07 — Estado de encerramento do ciclo de refactor incremental

Todos os itens concretos e incrementais do plano foram executados ou auditados. O que permanece é dívida técnica de médio/longo prazo explicitamente documentada:

| Item | Pré-requisito antes de atacar |
|------|-------------------------------|
| **12.4 (SceneId enum)** | ~~Split de `DoorZone::target_scene_id` em `exit_to_scene` + `exit_to_zone`~~ **Concluído (EZ)** |
| **12.1 (globais mutáveis)** | Decisão de escopo: qual subsistema migrar primeiro para contexto injetado |
| **12.4 (diálogo/item IDs)** | Enum/handles para `DialogueId`, `ItemId` — requer refactor de `DialogueSystem` e `ItemBag` |
| **12.5 (Actor monolítico)** | Separação de dados específicos de player vs enemy — requer redesign |
| **11.6 (EnemyDeathController)** | Extrair audio e quest para events retornados — requer barramento de eventos |

### Estado de testes no ambiente atual

- `./build/mion_tests_v2_core` está **verde** no ambiente atual (**867 passed, 0 failed**).

### 2026-04-11 — Tarefa FF-2 (12.4: `TownDialogueId` como `enum class`) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/town_dialogue_ids.hpp`, `src/systems/town_npc_interaction.hpp`, `src/systems/town_builder.hpp`, `tests/core/test_town_config_loader.cpp`.
- **Mudança:** o antigo `namespace TownDialogueId { constexpr const char* k* }` foi substituído por `enum class TownDialogueId { MiraDefault, MiraQuestOffer, MiraQuestActive, MiraQuestDone, ForgeGreeting }` com `to_string(TownDialogueId)` em `town_dialogue_ids.hpp` (chaves continuam casando com seções de `data/town_dialogues.ini`).
  - `TownNpcInteractionController::handle_npc_interaction` passa enum + `to_string(...)` em `dialogue.start(...)`. Os fallbacks ternários `(npc.dialogue_default.empty() ? <default> : npc.dialogue_default)` constroem `std::string(to_string(TownDialogueId::Mira*))` no ramo "default" — borda explícita entre identidade tipada do código e o id em string vindo do data-layer (`NpcEntity::dialogue_*`).
  - `TownBuilder::build_town_world` atribui `mira.dialogue_default = to_string(TownDialogueId::MiraDefault)` etc.; `villager_a`/`villager_b` permanecem strings literais (não são canonical: vivem só no INI, sem referência por código).
  - `tests/core/test_town_config_loader.cpp` checa `dialogue.has_sequence(to_string(TownDialogueId::MiraDefault))`/`ForgeGreeting`.
- **Por que `NpcEntity::dialogue_*` continua `std::string`:** o campo é data-driven (carregável de config no futuro) e não é lógica de domínio — strings ficam confinadas à interface com o registry/INI. Migrar `NpcEntity` a `optional<TownDialogueId>` exigiria fechar primeiro o conjunto de ids canônicos (incluindo `villager_a/b`) e fica fora do escopo desta tarefa.
- **Efeito esperado:** elimina o anti-padrão "constants de string com prefixo `k`" para identidades de diálogo de town; typos viram erro de compilação. Linha alinhada com FF-1 (`DungeonDialogueId`).
- **Não fazer:** `ItemId` (FF-3) — ainda em aberto, separado por tocar borda de save/serialização.
- **Validação:** `cmake --build build` OK; `ctest --test-dir build -L official --output-on-failure` OK (4/4).

### 2026-04-11 — Tarefa FF-1 (12.4: `DungeonDialogueId` como `enum class`) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/core/dungeon_dialogue_id.hpp` (novo), `src/core/dungeon_dialogue.hpp`, `src/systems/enemy_death_controller.hpp`, `src/systems/boss_state_controller.hpp`, `src/scenes/world_scene.hpp`.
- **Mudança:** o antigo `namespace DungeonDialogueId { constexpr const char* k* }` foi substituído por `enum class DungeonDialogueId { Prologue, Room2, Deeper, RareRelic, MinibossDeath, BossPhase2 }` com `to_string(DungeonDialogueId)` em `dungeon_dialogue_id.hpp`. Strings ficam confinadas à borda do registry (`register_dungeon_dialogue`) e ao callsite de `DialogueSystem::start(...)`. Logica de runtime passa a usar o enum:
  - `EnemyDeathController::DeathResult::post_mortem_dialogue_id` (string) virou `std::optional<DungeonDialogueId> post_mortem_dialogue`.
  - `WorldScene::SceneTransitionFlow::pending_post_mortem_dialogue_id` (string) virou `std::optional<DungeonDialogueId> pending_post_mortem_dialogue`; `take_post_mortem_dialogue_to_start(...)` retorna `optional<DungeonDialogueId>`.
  - `BossState::update` chama `dialogue.start(to_string(DungeonDialogueId::BossPhase2))`.
  - `WorldScene::fixed_update` usa `optional<DungeonDialogueId>` para a seleção de prólogo/room2/deeper por zona.
- **Efeito esperado:** elimina identidade por string livre na lógica de domínio para diálogo de dungeon (seção 12.4). Typos viram erros de compilação; a borda de serialização (registry/`DialogueSystem`) é o único ponto onde o `to_string(...)` é necessário. Alinhado com o padrão já validado em `SceneId`/`WorldLayoutId` (EZ) e `EnemyType`/`TalentId`.
- **Não fazer:** `TownDialogueId` (FF-2) e `ItemId` (FF-3) ficam para tarefas dedicadas — escopo separado para não misturar borda de save (`ItemId`) com gameplay.
- **Validação:** `cmake --build build -j$(nproc)` OK; `ctest --test-dir build -L official --output-on-failure` OK (4/4).

### 2026-04-10 — Tarefa FE (12.1: split runtime de `g_talent_nodes`) — **Cancelada / premissa invalidada**

- **Premissa original:** passar `g_talent_nodes` por referência explícita ao `SkillTreeController` para remover mutabilidade global em runtime (gasto de talentos).
- **Verificação (`rg g_talent_nodes`):** o global **não é mutado em runtime**. Escrita só ocorre em:
  - `src/components/talent_loader.hpp:62-63` — wrapper `apply_talents_ini` (bootstrap, agora via factory).
  - `src/components/talent_data.hpp:98-99` — `reset_talent_tree_defaults()` (init/reset de teste).
  Leitura única em runtime: `src/systems/skill_tree_controller.hpp:28` — só lê `.discipline`. O `SkillTreeController` muta o estado do *jogador* (talent points, ranks no `Player`/`TalentTree`), não a tabela estática.
- **Conclusão:** após `load_defaults_and_ini_overrides()` o global comporta-se como `const`; passar referência ao controller seria churn sem benefício. FE como descrita foi **descartada**.
- **Próximos passos propostos para 12.1 (a decidir em sessão futura):**
  1. **FE-revisado:** tornar o invariante "read-only após bootstrap" explícito — mover a inicialização para uma função `init_talent_tables(const IniData&)` única e documentar o contrato no header (`talent_data.hpp`). Sem mudar assinaturas de controllers.
  2. **12.4 (DialogueId/ItemId enums):** continuar a guideline 4.2 (typed identity) na mesma linha do `SceneId`/`WorldLayoutId` que fechámos no EZ. **Recomendação prioritária** por seguir técnica já validada.
  3. **11.6 (`EnemyDeathController` event bus):** muda de área e arejar o ciclo de refactor focado em globals.
- **Validação:** nenhuma — não houve mudança de código nesta tarefa.

### 2026-04-09 — Tarefas FA/FB/FC (12.1: factories para globals de configuração estática) — **Concluídas (versão incremental)**

- **Escopo aplicado:** `src/components/player_config.hpp`, `src/components/progression.hpp`, `src/components/spell_defs.hpp`, `src/components/talent_loader.hpp`, `src/systems/common_player_progression_loader.hpp`, `src/systems/dungeon_config_loader.hpp`, `tests/components/test_player_config.cpp`, `tests/components/test_progression.cpp`, `tests/core/test_ini.cpp`.
- **Mudança:**
  - **FA:** `apply_player_ini(d)` e `apply_progression_ini(d)` (mutavam o global diretamente) foram substituídos por `make_player_config_from_ini(d)` e `make_progression_config_from_ini(d)` (factories puras que retornam valor). O global passa a ser atribuído explicitamente no bootstrap (`g_player_config = make_player_config_from_ini(...)`). Comentário documenta que o global é read-only após o bootstrap.
  - **FB:** `apply_spells_ini_overrides(d)` substituído por `make_spell_defs_from_ini(d)`. `DungeonConfigLoader` passa a atribuir `g_spell_defs = make_spell_defs_from_ini(...)`.
  - **FC:** `apply_talents_ini(d)` mantido como wrapper de conveniência, mas agora delega para `make_talent_data_from_ini(d)` (factory pura que retorna `TalentData`). Testes de player_config/progression migrados para verificar valor retornado pelas factories, sem tocar o global.
- **Efeito esperado:** factories puras são testáveis sem side effects em globais; escrita nos globais fica visível e explícita no callsite do bootstrap; elimina o padrão de stash/restore em testes de configuração.
- **Validação:** build completo OK; ctest 100% (4/4 suítes passam, 867 core passed, 0 failed).

### 2026-04-08 — Tarefa EZ (`DoorZone::target_scene_id`: split em `exit_to_zone` + `exit_to_scene`) — **Concluída (versão incremental)**

- **Escopo aplicado:** `src/world/room.hpp`, `src/systems/room_flow_system.hpp`, `src/systems/room_manager.hpp`, `src/systems/town_builder.hpp`, `tests/legacy/test_legacy.cpp`, `tests/world/test_room_manager_doors.cpp`, `tests/world/test_town_layout.cpp`, `tests/world/test_world_map_builder.cpp`.
- **Mudança:** `DoorZone::target_scene_id` (`std::string` de namespace misto) foi substituído por dois campos explícitos:
  - `exit_to_zone: std::string` — tokens `WorldLayoutId::k*` (handoff intra-world);
  - `exit_to_scene: std::string` — tokens `SceneId::k*` (saída para cena do registry).
  - `add_door(…, target)` com argumento extra foi removido; adicionados `add_zone_door(…, zone_id)` e `add_scene_door(…, scene_id)` com semântica clara.
  - `RoomFlowSystem::fixed_update` passou a checar `exit_to_zone` antes de `exit_to_scene`, copiando o valor não vazio para `scene_exit_to` como antes.
  - Callsites atualizados: `RoomManager::build_room` usa `add_zone_door`; `TownBuilder::build_town_world` usa `add_zone_door`.
  - Testes atualizados: `test_legacy` usa `add_scene_door`/`add_zone_door`; testes de layout usam `exit_to_zone` nas assertions.
- **Efeito esperado:** elimina namespace misto no campo de transição de porta; prepara migração `SceneId → enum class` (o único bloqueador restante era esse campo misto).
- **Validação:** build completo OK; ctest 100% (4/4 suítes passam, 867 core passed, 0 failed).

---

## 1. Motor e infraestrutura

### 1.1 CMake — `GLOB_RECURSE` vs lista explícita

| | |
|--|--|
| **Estado** | `ENGINE_SOURCES` e `TEST_SOURCES` já usam `CONFIGURE_DEPENDS` em `CMakeLists.txt` — adicionar/remover `.cpp` nas pastas cobertas reconfigura o CMake. |
| **Risco residual** | `.cpp` novo fora dos globs (nova pasta) não entra no alvo; revisões de PR não mostram lista explícita de fontes. |
| **Onde mexer** | `CMakeLists.txt` (raiz). |
| **Plano** | Opcional: substituir globs por `target_sources` / lista explícita por módulo, ou manter globs e documentar a convenção de pastas. **Porquê:** previsibilidade de build e CI, não correção de bug imediato. |

### 1.2 Game loop (`main.cpp`)

| | |
|--|--|
| **Estado** | `SDL_PollEvent` no topo do frame; input lido uma vez fora do loop físico; fixed timestep com acumulador + `fixed_dt`; cap de backlog aplicado com `max_substeps_per_frame` e clamp de acumulador. |
| **Risco residual** | Baixo: backlog extremo agora é truncado por design; o tradeoff é descartar parte do atraso em cenários de hitch severo (preferível a espiral de substeps). |
| **Onde mexer** | `src/main.cpp` — ciclo `while (running)`. |
| **Plano** | **Concluído (EE).** Manter `max_substeps_per_frame` como parâmetro explícito de estabilidade e revisar apenas se houver necessidade de tuning por plataforma. |

### 1.3 Locale — estado global (`locale_bind`, `L()`, `detail::active_locale`)

| | |
|--|--|
| **Estado** | `LocaleSystem` consumido por dependência explícita via `SceneCreateContext::locale`; cenas/controllers recebem locale por ponteiro/referência controlada. |
| **Onde mexer** | `src/core/locale.hpp`, `src/main.cpp` (`locale_bind`), `src/core/title_menu.hpp`, `src/scenes/title_scene.hpp` (reload por idioma), `victory_scene.hpp`, `game_over_scene.hpp`, `credits_scene.hpp`. |
| **Plano** | **Concluído (EF).** Guardrail de regressão: manter `rg 'detail::active_locale|locale_bind\(|\bL\(' src/` sem ocorrências em mudanças de UI/cenas. |

### 1.4 RNG — `rand()` / `srand()`

| | |
|--|--|
| **Estado** | `main` usa `std::mt19937` e `ctx.rng`; sistemas como `DropSystem` recebem `std::mt19937&` e `<random>`. |
| **Onde auditar** | `rg '\\\\brand\\\\(|\\\\bsrand\\\\(' src/` |
| **Plano** | Garantir zero uso de `rand`/`srand` em `src/`; testes podem manter seeds fixos com `<random>`. **Porquê:** reprodutibilidade e clareza de ownership do gerador. |

---

## 2. `WorldScene` — performance e bug de footsteps

### 2.1 `_all_enemies()` — cópias por valor de `Actor`

| | |
|--|--|
| **Problema** | `_cached_all_enemies` é `std::vector<Actor>`; cada chamada a `_all_enemies()` faz `clear` + `push_back(e)` por valor para todos os inimigos de todas as áreas. `Actor` é pesado (`std::string`, vários `std::vector`, spell book, bag, etc.). |
| **Chamadas por tick (dungeon)** | Pelo menos: `BossState::update`, `WorldAudioSystem::update`, `FootstepAudioSystem::update_footsteps` — triplica trabalho e alocações. |
| **Bug lógico** | `FootstepAudioSystem::update_footsteps` **escreve** `footstep_prev_*` e `footstep_accum_dist` nos elementos do vector — que são **cópias**. O estado real em `WorldArea::enemies` **não** é actualizado; passos de inimigo ficam incoerentes com o movimento real. |
| **Onde mexer** | `src/scenes/world_scene.hpp` (`_all_enemies`, `_cached_all_enemies`, chamadas); `src/systems/boss_state_controller.hpp` (assinatura `update`); `src/systems/world_audio_system.hpp` (`update`); `src/systems/footstep_audio_system.hpp` (`update_footsteps`). |
| **Plano** | (1) Trocar cache para `std::vector<Actor*>` preenchido a partir de `area.enemies` (endereços dos actores reais). (2) Ajustar assinaturas para `const std::vector<Actor*>&` ou `span` onde só há leitura (`BossState`, `WorldAudioSystem`). (3) `FootstepAudioSystem` deve receber `std::vector<Actor*>&` ou iterar ponteiros para mutar os mesmos objectos que o gameplay. Alternativa: deixar de ter cache e passar `_actors` filtrado (só `Team::Enemy`) se o modelo de listas já for a fonte única. **Porquê:** menos alocações/cópias, cache-friendly, e footsteps consistentes com o estado do mundo. |

---

## 3. Input — flancos (`_prev_*`) e early returns

### 3.1 Desincronização após gates de UI

| | |
|--|--|
| **Problema** | `_prev_potion`, `_prev_upgrade_*`, `_prev_talent_*`, `_prev_confirm` (town) só são actualizados **depois** de hitstop, diálogo, pausa, level-up, skill tree, equipamento e loja. Qualquer `return` antes desse bloco deixa os `_prev_*` desactualizados relativamente ao input do(s) tick(s) saltado(s). |
| **Contraste** | `OverlayInputTracker::flush` alinha bordas de overlays (tab, inventário, etc.), **não** os `_prev_*` de gameplay da cena. |
| **Onde mexer** | `src/scenes/world_scene.hpp` — `fixed_update`, possivelmente `src/systems/overlay_input.hpp` ou novo helper. |
| **Plano** | Opção A: no **fim** de `fixed_update` (ou num `finally` lógico), sincronizar **todos** os `_prev_*` com o `input` actual, independentemente de early return (requer reestruturar para não sair cedo sem esse passo). Opção B: componente `InputEdgeTracker` que regista `InputState` completo do tick e expõe `edge_potion`, `edge_confirm_world`, etc., actualizado **uma vez** no início e consumido pelos subsistemas. Opção C: duas fases — “processar UI que bloqueia mundo” vs “actualizar histórico de input” sempre na mesma ordem. **Porquê:** evitar poção/confirm duplicados ou engolidos ao sair de pausa/diálogo. |

---

## 4. Run stats — dano por delta de HP

### 4.1 `hp0` vs `current_hp` no fim do frame

| | |
|--|--|
| **Problema** | `damage_taken` só incrementa quando `hp0 > _player.health.current_hp`. No mesmo tick, cura + dano pode mascarar dano real (HP final igual ou superior a `hp0`). |
| **Onde mexer** | `src/scenes/world_scene.hpp` (bloco `if (_run_stats)` após combate); idealmente origem de dano/cura. |
| **Plano** | Emitir eventos ou contadores onde o dano é **aplicado** (`MeleeCombatSystem`, projéteis, `HealthState`, etc.), ou `RunStatsTracker` que subscreve `CombatEvent::DamageTaken` / cura explícita. Remover o delta agregado da cena. **Porquê:** estatísticas alinhadas com a simulação, testáveis sem simular frame inteiro. |

---

## 5. Conteúdo hardcoded — boss Grimjaw

### 5.1 Nome `"Grimjaw"` em vários sítios

| | |
|--|--|
| **Problema** | Lógica de zona boss, música e reset de `_boss_state` assumem um boss com nome fixo. Dificulta vários bosses ou dados-driven. |
| **Onde mexer** | `src/scenes/world_scene.hpp` (`_handle_zone_transition`), `src/systems/boss_state_controller.hpp`, `src/systems/world_audio_system.hpp`. |
| **Plano** | Introduzir identificador em dados (`EnemyDef`, flag `is_zone_boss`, `boss_id` na `WorldArea`) e comparar por id/flag em vez de string literal; ou tabela “zona Boss → critério de vitória”. **Porquê:** escalabilidade e menos erros ao renomear ou adicionar bosses. |

---

## 6. Diálogo de relíquia rara (`dungeon_rare_relic`)

### 6.1 Disparo e repetição

| | |
|--|--|
| **Nota** | O `DropSystem::pickup_near_player` **não** inicia o diálogo; quem chama é `WorldScene::fixed_update` após pickup bem-sucedido. |
| **Risco** | Vários pickups de lore/relíquia podem reabrir o mesmo diálogo; política de “já mostrado” inexistente. |
| **Onde mexer** | `src/scenes/world_scene.hpp`; opcionalmente novo `PickupDialogueController` ou flags em `QuestState` / save. |
| **Plano** | Definir regra (uma vez por run, por item, por flag em save); fila vs substituir diálogo actual. **Porquê:** UX previsível e não bloquear acções por spam de diálogo. |

---

## 7. Town — interacção NPC

### 7.1 `find_nearest` vs raio de interacção

| | |
|--|--|
| **Estado** | Seleção de NPC de interação consolidada via `find_nearest_npc_for_interaction(...)`, que retorna índice + `in_interaction_range` no mesmo contrato. |
| **Onde mexer** | `src/systems/town_npc_interaction.hpp`, `src/scenes/world_scene.hpp` (bloco town em `fixed_update`). |
| **Plano** | **Concluído (EG).** Guardrail: evitar reintroduzir API “nearest sem range” para fluxos de interação de town. |

---

## 8. Apresentação — HUD e strings na cena

### 8.1 `draw_text` e texto fixo na `WorldScene::render`

| | |
|--|--|
| **Estado** | Texto de town/HUD nessa área do render usa chaves de locale (`town_npc_interact_hint`, `town_gold_label`, `ui_saved`) via helper `tr(...)`. |
| **Onde mexer** | `src/scenes/world_scene.hpp` (`render`); candidatos: renderizadores world/town já referidos no `map_controllers_revisar.md`. |
| **Plano** | **Concluído parcialmente (EH):** strings canônicas migradas para locale. **Débito residual:** extração total para `TownHud` dedicado continua opcional como melhoria de arquitetura (não bloqueante). |

---

## 9. `enter()` — sessão, save e player

### 9.1 Orquestração pesada na cena

| | |
|--|--|
| **Estado** | Fluxo de `enter()` foi dividido em pipeline nomeado (`_load_save_or_start_fresh`, `_configure_player_for_session`, `_initialize_runtime_after_enter`), mantendo ordem e comportamento. |
| **Onde mexer** | `src/scenes/world_scene.hpp` (`enter`); `src/systems/player_configurator.hpp`; possível `WorldSessionBootstrap` / extensão de `WorldSaveController`. |
| **Plano** | **Concluído incrementalmente (EI).** Débito residual opcional: mover pipeline para módulo dedicado (`WorldSessionBootstrap`) se `enter()` voltar a crescer com novos passos de sessão. |

---

## 10. Transições de cena e estatísticas (extracção modular)

### 10.1 Pós-morte, vitória, timers

| | |
|--|--|
| **Estado** | Fluxo pós-morte/vitória está encapsulado em `SceneTransitionFlow` com estado explícito (fila de diálogo post-mortem + saída diferida por timer), consumido por tick único na cena. |
| **Onde mexer** | `src/scenes/world_scene.hpp`; novo `SceneTransitionController` (header em `src/systems/` ou `src/world/`). |
| **Plano** | **Concluído incrementalmente (EJ).** Evolução futura opcional: promover `SceneTransitionFlow` para módulo próprio se houver reutilização em outras cenas. |

### 10.2 `RunStatsTracker` / barramento de eventos

| | |
|--|--|
| **Estado** | `RunStatsTracker` concentra cálculo de dano por evento e aplicação de delta de frame (`FrameStatsDelta`), removendo agregação inline principal de métricas na cena. |
| **Onde mexer** | `src/core/run_stats.hpp`, sistemas que alteram ouro/HP/dano, `world_scene.hpp`. |
| **Plano** | **Concluído incrementalmente (EK).** Evolução futura opcional: ampliar eventos de origem para métricas adicionais sem expandir lógica na `WorldScene`. |

---

## 11. God object — visão geral

| | |
|--|--|
| **Constatação** | `WorldScene` concentra dungeon + town, combate, áudio de mundo, pickups, NPCs, colisão, áreas, loja, transições, parte do HUD. |
| **Plano longo** | Depois dos fixes críticos (itens 2–4), ir delegando blocos já identificados (10, 7, 8, 9) sem big-bang rewrite. **Porquê:** reduzir ponto único de falha e facilitar features narrativas/combate em cima de bases estáveis. |

### 11.1 UI e controllers — deriva de padrões / duplicação estrutural

| | |
|--|--|
| **Estado** | Contrato canônico “controller retorna intenção” foi reforçado incrementalmente na loja: `ShopInputController` agora retorna `ShopInputResult` (`world_paused`, `should_save`) e a cena aplica side effects globais (persistência). |
| **Sintoma** | Novas telas e fluxos acabam com ownership diferente de estado, retorno e input. Isso explica porque menu, skill tree, inventário, pausa e loja parecem “da mesma família”, mas funcionam e evoluem de jeitos diferentes. |
| **Onde observar** | `src/systems/skill_tree_controller.hpp`, `src/systems/attribute_levelup_controller.hpp`, `src/systems/equipment_screen_controller.hpp`, `src/systems/pause_menu_controller.hpp`, `src/core/pause_menu.hpp`, `src/systems/shop_input_controller.hpp`, `src/core/title_menu.hpp`. |
| **Plano** | **Concluído parcialmente (EL):** `ShopInputController` alinhado ao padrão de retorno por intenção. Débito residual: convergir `PauseMenu`/`TitleMenu` para o mesmo contrato mínimo quando esses módulos forem tocados por feature/refactor dedicado. |

### 11.2 Input de UI fragmentado em vários mecanismos

| | |
|--|--|
| **Estado** | `PauseMenu`, loja e overlays principais consomem `OverlayInputEdges` de `OverlayInputTracker::capture` (`OverlayInputEdges` definido em `core/input.hpp`). |
| **Risco** | Novas UIs podem ignorar o tracker e reintroduzir deteção de bordas paralela. |
| **Onde mexer** | `src/core/input.hpp`, `src/systems/overlay_input.hpp`, `src/core/pause_menu.hpp`, `src/scenes/world_scene.hpp`. |
| **Plano** | **Concluído incrementalmente (ER):** pausa alinhada ao mesmo pacote de edges. Débito residual: `TitleMenu` / outras cenas fora da world scene se ainda usarem `InputState` bruto para UI. |

### 11.3 Persistência e side effects inconsistentes entre controllers

| | |
|--|--|
| **Estado** | Fluxo de loja foi alinhado ao contrato de intenção (`should_save`) com persistência feita no owner (`WorldScene`), reduzindo side effect escondido no controller. |
| **Onde observar** | `src/systems/skill_tree_controller.hpp`, `src/systems/equipment_screen_controller.hpp`, `src/systems/attribute_levelup_controller.hpp`, `src/core/title_menu.hpp`, `src/systems/town_npc_interaction.hpp`. |
| **Plano** | **Concluído parcialmente (EN):** loja aderiu ao padrão. Débito residual: revisar callbacks assíncronos de persistência em `TownNpcInteractionController`/`TitleMenu` para o mesmo contrato quando houver tarefa dedicada nesses módulos. |

### 11.4 Renderização e strings de UI espalhadas / não padronizadas

| | |
|--|--|
| **Estado** | Módulos tocados no fluxo principal de jogo (`WorldScene`, `ShopSystem`, `DungeonHud`, `dialogue_render`) usam chaves de locale para textos de UI/hints em runtime.
| **Risco** | Cada nova tela copia um estilo de hint, labels e posicionamento diferente; localização fica parcial; a IA replica o exemplo local mais próximo em vez de seguir um padrão global. |
| **Onde observar** | `src/core/title_menu.hpp`, `src/core/pause_menu.hpp`, `src/systems/shop_system.hpp`, `src/scenes/world_scene.hpp`, `src/systems/dungeon_hud.hpp`, `src/systems/skill_tree_ui.hpp`, `src/systems/attribute_screen_ui.hpp`. |
| **Plano** | **Concluído parcialmente (EO/EH):** remover hardcodes críticos de texto em renderers ativos e manter locale como fonte canônica. Débito residual: consolidar módulos legados restantes quando forem tocados por tarefa dedicada. |

### 11.5 Conceito de `Controller` inconsistente no projecto

| | |
|--|--|
| **Estado** | Controllers modais de UI na world scene estendem ou aliasam `UiControllerResult` (`EquipmentResult`, `ShopInputResult`, `PauseMenuResult`, `SkillTreeResult`, `AttributeLevelUpResult`). |
| **Risco** | A IA e humanos deixam de saber o que esperar de um “controller”: ele guarda estado? só orquestra? pode salvar? pode tocar áudio? pode renderizar? Resultado: módulos novos seguem contratos diferentes e a arquitetura deriva sem perceber. |
| **Onde observar** | `src/systems/skill_tree_controller.hpp`, `src/systems/pause_menu_controller.hpp`, `src/systems/equipment_screen_controller.hpp`, `src/systems/combat_fx_controller.hpp`, `src/systems/world_save_controller.hpp`, `src/systems/enemy_death_controller.hpp`, `src/systems/shop_input_controller.hpp`, `src/systems/town_npc_interaction.hpp`, `src/core/title_menu.hpp`, `src/systems/boss_state_controller.hpp`. |
| **Plano** | **Concluído parcialmente (EP):** contrato mínimo canônico documentado em código e reaproveitado por controllers de UI tocados. Débito residual: consolidar taxonomia global (`Controller/System/Service/Renderer/State`) no guia arquitetural e aplicar em módulos legados quando forem evoluídos. |

### 11.6 Side effects e identificação frágil em controllers/sistemas de gameplay

| | |
|--|--|
| **Problema** | Alguns módulos “controller” de gameplay fazem side effects em várias camadas ao mesmo tempo. `EnemyDeathController` mexe em drops, XP, talentos, quest, `run_stats`, partículas, áudio e ainda contém lógica hardcoded de boss por nome. `CombatFxController` resolve alvos comparando `ev.target_name` com `Actor::name`, o que é frágil se existirem inimigos com nomes repetidos. |
| **Risco** | Quando se adiciona um novo boss, novo evento de combate ou novo tipo de inimigo, a mudança pode se espalhar por vários pontos ocultos. A comparação por string também é sujeita a colisão semântica e dificulta evoluir para múltiplas instâncias com o mesmo nome visível. |
| **Onde observar** | `src/systems/enemy_death_controller.hpp`, `src/systems/combat_fx_controller.hpp`, `src/systems/melee_combat.hpp`, `src/systems/projectile_system.hpp`, `src/systems/boss_state_controller.hpp`. |
| **Plano** | Em médio prazo, mover identificação de alvo/boss para ids/handles estáveis e reduzir side effects misturados via eventos/result structs mais específicos. No curto prazo, pelo menos registrar esses módulos como hotspots de refactor para evitar que novos casos especiais sejam “colados” ali. **Porquê:** diminuir acoplamento implícito e bugs difíceis de rastrear. |

---

## Checklist rápido de ficheiros

| Área | Ficheiros principais |
|------|----------------------|
| Loop | `src/main.cpp` |
| Locale | `src/core/locale.hpp`, `src/main.cpp`, `src/core/title_menu.hpp`, `src/scenes/title_scene.hpp`, `victory_scene.hpp`, `game_over_scene.hpp`, `credits_scene.hpp` |
| Enemies / footsteps / boss / áudio mundo | `src/scenes/world_scene.hpp`, `src/systems/footstep_audio_system.hpp`, `src/systems/world_audio_system.hpp`, `src/systems/boss_state_controller.hpp` |
| Input / overlay | `src/scenes/world_scene.hpp`, `src/systems/overlay_input.hpp` |
| Run stats | `src/scenes/world_scene.hpp`, `src/core/run_stats.hpp`, sistemas de combate/recursos |
| NPC town | `src/systems/town_npc_interaction.hpp`, `src/scenes/world_scene.hpp` |
| UI / controllers | `src/systems/skill_tree_controller.hpp`, `src/systems/attribute_levelup_controller.hpp`, `src/systems/equipment_screen_controller.hpp`, `src/systems/pause_menu_controller.hpp`, `src/core/pause_menu.hpp`, `src/systems/shop_input_controller.hpp`, `src/core/title_menu.hpp` |
| CMake | `CMakeLists.txt` |

---

## 12. Varredura geral de `src` — problemas recorrentes

### 12.1 Estado global mutável espalhado em configuração e tema

| | |
|--|--|
| **Problema** | Além do locale global, o projecto usa vários `inline` globais mutáveis como fonte implícita de verdade: `ui::g_theme`, `g_player_config`, `g_progression_config`, `g_attribute_scales`, `g_talent_nodes`, `g_spell_defs`. Esses dados são lidos e mutados em múltiplos pontos de `src/`, sem owner explícito. |
| **Risco** | Ordem de carregamento/importação passa a importar; testes isolados ficam frágeis; duas cenas/mundos simultâneos no mesmo processo tornam-se inviáveis; a IA tende a “plugar” mais coisas nesses globais porque já existe precedente. |
| **Onde observar** | `src/core/ui.hpp`, `src/core/locale.hpp`, `src/components/player_config.hpp`, `src/components/progression.hpp`, `src/components/attributes.hpp`, `src/components/talent_data.hpp`, `src/components/spell_defs.hpp`, loaders em `src/systems/*config_loader*.hpp`. |
| **Plano** | Caminho de médio prazo: agrupar configuração/theme/data tables em contexto explícito (`GameData`, `UiTheme`, `RuntimeConfig`) injectado nos subsistemas, removendo escrita direta em variáveis process-wide. No curto prazo, registrar esses globais como hotspots: nenhuma funcionalidade nova deve depender deles sem justificativa. **Porquê:** reduzir acoplamento invisível e ordem frágil de inicialização. |

### 12.2 Bootstrap de dados estáticos duplicado e sujeito a divergência

| | |
|--|--|
| **Problema** | Carregamento de dados base está espalhado e duplicado: loaders de town e dungeon resetam/reaplicam partes de player, progressão e talentos, e `WorldScene::enter()` chama ambos. Isso cria contrato implícito de ordem e “double work”. |
| **Risco** | Uma alteração num loader pode não ser replicada no outro; defaults podem ser reescritos sem perceber; a IA pode copiar o loader “errado” e reforçar duplicação. |
| **Onde observar** | `src/systems/town_config_loader.hpp`, `src/systems/dungeon_config_loader.hpp`, `src/scenes/world_scene.hpp`. |
| **Plano** | Consolidar bootstrap estático em um ponto (`GameDataLoader`, `StaticDataBootstrap` ou equivalente) com fases explícitas: defaults -> INI -> validação -> disponibilização. Scene não deve decidir quais pedaços resetar duas vezes. **Porquê:** fonte única de configuração e menor chance de drift entre dungeon/town. |

### 12.3 Pipeline de save/apply fragmentado e dependente de ordem

| | |
|--|--|
| **Problema** | `WorldSaveController::apply_world_save` aplica parte do estado, enquanto `WorldScene::enter()` complementa HP/mana/stamina e clamps após `configure_player`. O funcionamento depende de uma ordem não óbvia entre save, configuração e recomputação de stats. |
| **Risco** | Refactors em `enter()` ou em `configure_player` podem quebrar restauração de save sem erro de compilação. O mesmo padrão favorece “puxadinhos” onde cada call site completa o que o helper não fez. |
| **Onde observar** | `src/systems/world_save_controller.hpp`, `src/scenes/world_scene.hpp`, `src/systems/player_configurator.hpp`. |
| **Plano** | Transformar restauração de sessão em pipeline único e fechado: `apply_save` precisa restaurar completamente o contrato que promete, ou devolver um resultado explícito dizendo o que ainda falta. **Porquê:** eliminar dependências de ordem escondidas. |

### 12.4 Identidade e fluxo baseados em strings

| | |
|--|--|
| **Problema** | Identificadores críticos usam strings livres: nomes de cenas (`"world"`, `"title"`, `"victory"`), ids de diálogo (`"dungeon_prologue"`, `"mira_default"`), boss por nome (`"Grimjaw"`), itens por `name`, eventos de combate por `target_name`. |
| **Risco** | Typos e renames quebram em runtime; múltiplas entidades com o mesmo nome visível podem conflitar; conteúdo e lógica ficam espalhados sem registro único. |
| **Onde observar** | `src/core/scene_registry.hpp`, `src/core/register_scenes.cpp`, `src/scenes/world_scene.hpp`, `src/core/dungeon_dialogue.hpp`, `src/systems/town_npc_interaction.hpp`, `src/systems/enemy_death_controller.hpp`, `src/systems/boss_state_controller.hpp`, `src/systems/combat_fx_controller.hpp`, `src/components/item_bag.hpp`, `src/entities/ground_item.hpp`. |
| **Plano** | Evoluir gradualmente para ids tipados/enum/handles estáveis onde a identidade é estrutural (scene, boss, alvo, item, diálogo), mantendo strings apenas na camada de conteúdo/serialização. **Porquê:** diminuir acoplamento textual e bugs de runtime difíceis de rastrear. |

### 12.5 `Actor` monolítico e modelo de identidade frágil

| | |
|--|--|
| **Problema** | `Actor` agrega dados de player, inimigo, boss, pathing, inventário, talentos, poções, derived stats, etc. Ao mesmo tempo, o mundo usa mistura de valores, `std::vector<Actor*>`, rebuild de listas e `NpcEntity::actor` apontando para storage auxiliar. |
| **Risco** | Cópias custosas, invariantes difíceis de manter, maior risco de ponteiros inválidos após rebuilds/realocações, e dificuldade para evoluir para sistemas mais robustos de identidade/ownership. |
| **Onde observar** | `src/entities/actor.hpp`, `src/scenes/world_scene.hpp`, `src/entities/npc.hpp`, `src/systems/npc_actor_factory.hpp`, `src/systems/enemy_spawner.hpp`. |
| **Plano** | Não fazer rewrite total agora. Primeiro, registrar `Actor` e listas de ponteiros como hotspot e evitar novas responsabilidades no tipo. Em médio prazo, separar pelo menos identidade/estado comum de blocos específicos de player/enemy, e caminhar para ids/handles mais estáveis. **Porquê:** reduzir custo cognitivo e fragilidade estrutural do núcleo do gameplay. |

### 12.6 Regras de domínio e recomputação espalhadas

| | |
|--|--|
| **Problema** | Relações entre talentos, spells, atributos e derived stats estão espalhadas por múltiplos pontos: `SpellBookState::sync_from_talents`, `skill_tree_try_spend_talent`, `AttributeLevelUpController`, `ShopSystem`, `PlayerConfigurator`, além de dados globais mutáveis de talento/spell. |
| **Risco** | Ao adicionar um novo talento/spell/item, é fácil esquecer um dos lugares que precisa ser atualizado; a IA tende a duplicar mais uma regra “perto de onde mexeu”. |
| **Onde observar** | `src/components/spell_book.hpp`, `src/systems/skill_tree_controller.hpp`, `src/systems/attribute_levelup_controller.hpp`, `src/systems/shop_system.hpp`, `src/systems/player_configurator.hpp`, `src/components/talent_data.hpp`. |
| **Plano** | Futuramente concentrar regras de progressão/derived em um serviço ou pipeline único de “recompute + apply caps”, e reduzir hardcodes locais de efeito por talento/item. **Porquê:** uma única fonte de verdade para evolução de atributos e unlocks. |

### 12.7 Duplicação de dados estáticos e manifests paralelos

| | |
|--|--|
| **Problema** | Algumas tabelas/listas existem em mais de um lugar. Exemplo forte: paths de áudio aparecem no manifesto e novamente em `audio.cpp`; defaults de talento são repetidos no array global e em função de reset. |
| **Risco** | Um ajuste de conteúdo pode atualizar só metade dos lugares; verificações de bootstrap e comportamento runtime passam a divergir. |
| **Onde observar** | `src/core/asset_manifest.hpp`, `src/core/audio.cpp`, `src/components/talent_data.hpp`. |
| **Plano** | Declarar cada tabela/lista estática uma única vez e derivar o restante dela; `reset` deve reaproveitar a fonte canônica, não copiar o literal. **Porquê:** diminuir drift silencioso entre dados equivalentes. |

### 12.8 Encapsulamento fraco em tipos centrais

| | |
|--|--|
| **Problema** | Alguns tipos “de sistema” expõem internals como API de facto. Exemplo forte: `DialogueSystem` deixa armazenamento e estado interno públicos; `PauseMenu` expõe `prev_*` para outros módulos computarem edges; vários módulos dependem desses detalhes. |
| **Risco** | Refactors ficam perigosos porque qualquer caller pode depender da implementação concreta; a IA aprende o detalhe e o reutiliza, consolidando um acoplamento acidental. |
| **Onde observar** | `src/systems/dialogue_system.hpp`, `src/core/pause_menu.hpp`, `src/systems/overlay_input.hpp`. |
| **Plano** | Ao tocar nesses módulos, preferir APIs explícitas para o que é realmente suportado (`capture_edges`, `is_active`, `start`, `advance`, etc.) e esconder storage/prev states privados. **Porquê:** reduzir dependência de detalhes internos que hoje vazam entre camadas. |

---

## 13. Execução assistida por IA (anti-regressão)

### 12.1 Template obrigatório por tarefa

Usar este template para cada item implementado:

```md
### Tarefa: <id + título curto>

- Objetivo:
- Escopo (arquivos permitidos):
- Não-fazer (fora de escopo):
- Dependências:
- Risco (baixo/médio/alto):
- Invariantes que não podem quebrar:
- Passos de implementação:
  1)
  2)
  3)
- Testes obrigatórios:
  - Unit:
  - Integração:
  - Manual:
- Critérios de aceite (DoD):
  - [ ] 
  - [ ] 
```

### 12.2 Definition of Done (DoD) por item crítico

#### Item 2.1 (`_all_enemies` / footsteps)
- [x] `WorldScene` não mantém cache de inimigos por valor (`std::vector<Actor>`).
- [x] `FootstepAudioSystem` muta actores reais (não cópias temporárias).
- [x] `BossState` e `WorldAudioSystem` continuam funcionais com a nova assinatura.
- [x] Build e testes de sistemas de combate/áudio passam.

#### Item 3.1 (input edges e early return)
- [x] `_prev_*` fica sincronizado mesmo quando há retorno antecipado.
- [x] Pausa, diálogo, loja e menus não disparam ações duplicadas ao sair.
- [ ] Testes manuais de input em sequência (abrir/fechar pausa e confirmar ação) passam.

#### Item 4.1 (run stats por evento)
- [x] `damage_taken` não depende mais de delta `hp0` vs `hp_final`.
- [x] Dano e cura no mesmo tick não mascaram dano recebido.
- [x] Cenários de teste cobrem dano puro, cura pura e dano+cura no mesmo frame.

### 12.3 Dependências e ordem sugerida de execução

1. **Tarefa A:** corrigir `_all_enemies`/assinaturas (`WorldScene`, `BossState`, `WorldAudioSystem`, `FootstepAudioSystem`).
2. **Tarefa B:** corrigir sincronização de `_prev_*` e padronizar edge tracking.
3. **Tarefa C:** migrar `run_stats` para eventos/contadores na origem do dano.
4. **Tarefa D:** eliminar hardcode de boss por nome.
5. **Tarefa E:** locale sem global (`L`, `locale_bind`, `active_locale`).
6. **Tarefa F (estrutural):** transição de cena, HUD e bootstrap.

Regra: **não** misturar itens sem dependência direta no mesmo PR.

### 12.4 Matriz de risco e mitigação

| Tarefa | Impacto | Probabilidade de regressão | Mitigação |
|-------|---------|-----------------------------|-----------|
| A (`_all_enemies`) | Alto | Alto | Testes de combate/áudio + validação manual de footsteps e música boss |
| B (input edges) | Alto | Médio/Alto | Checklist manual de UI gates (pausa, diálogo, loja, skill tree) |
| C (run stats) | Médio | Médio | Testes determinísticos com cenários de dano/cura |
| D (boss hardcode) | Médio | Médio | Testar zona boss + transição/vitória |
| E (locale) | Médio | Médio | Smoke test de cenas com texto e troca de idioma |

### 12.5 Checklist de validação padrão (rodar sempre)

1. Build Debug/Release do alvo principal.
2. Testes automatizados relevantes para os arquivos tocados.
3. `rg` de guardrails arquiteturais:
   - `rg '\brand\(|\bsrand\(' src/`
   - `rg 'detail::active_locale|locale_bind\(|\bL\(' src/` (quando estiver na tarefa de locale)
   - `rg '\"Grimjaw\"' src/` (depois da tarefa de boss data-driven)
4. Smoke test manual mínimo:
   - Entrar dungeon/town
   - Combate básico
   - Abrir/fechar UI de pausa
   - Interagir com NPC/loja

### 12.6 Guardrails para IA (não negociar)

- Não introduzir novos globais de estado mutável.
- Não usar `rand`/`srand` em `src/`.
- Não hardcodear boss por nome de string em regras centrais.
- Não refatorar módulos fora do escopo da tarefa ativa.
- Não alterar comportamento de gameplay e UI no mesmo PR sem teste dedicado.

### 12.7 Prompt pack (copiar/colar para execução por IA)

```md
Tarefa: <id>
Objetivo: <resultado mensurável>
Escopo: <arquivos permitidos>
Restrições:
- Não editar arquivos fora do escopo.
- Não introduzir globais novos.
- Manter compatibilidade com testes atuais.
Passos:
1) Implementar mudança mínima.
2) Ajustar assinaturas/chamadores.
3) Rodar validações obrigatórias.
Saída esperada:
- Lista de arquivos alterados.
- Resumo do porquê técnico.
- Resultado dos testes/validações.
```

---

### Task DL - Prune redundant WorldSaveController incrementals (batch cleanup)

**Applied scope**
- `tests/world/test_world_save_controller.cpp`

**Changes made**
- Removed a large batch of overlapping incremental tests added in recent steps.
- Kept only the core contract set for `WorldSaveController` (persist/snapshot/apply/load/clear and key clamp + unlock flows).
- Trimmed matching `REGISTER_TEST(...)` entries to prevent dangling test references.

**Expected effect**
- Lower maintenance noise and faster iteration while preserving meaningful behavior coverage.
- Cleaner baseline before next round of fixes requested by user.

**Validation**
- Build + official tests executados apos o corte em lote:
  - `cmake --build build -j$(nproc)` OK
  - `ctest --test-dir build -L official --output-on-failure` OK (4/4)

### Task DM - Consolidate static data file/section names and shared INI loading

**Applied scope**
- `src/core/data_file_names.hpp`
- `src/core/data_section_names.hpp`
- `src/core/data_ini_loader.hpp`
- `src/systems/common_player_progression_loader.hpp`
- `src/systems/town_config_loader.hpp`
- `src/systems/dungeon_config_loader.hpp`
- `src/core/audio_asset_paths.hpp`
- `src/core/asset_manifest.hpp`
- `src/core/audio.cpp`

**Changes made**
- Centralized canonical data INI filenames into `data_files::*` constants.
- Centralized canonical INI section names used by config loaders into `data_sections::*`.
- Added shared helper `load_data_ini(...)` to standardize data-path resolution and ini reads.
- Extracted duplicated player/progression/talent default+override bootstrap into `CommonPlayerProgressionLoader`.
- Updated town/dungeon loaders to consume the shared bootstrap and canonical file constants.
- Consolidated audio asset path tables into `audio_assets::*` and reused them from both `AudioSystem` and asset manifest checks.

**Expected effect**
- Reduces string duplication and drift risk across loaders/manifests.
- Makes static-data bootstrap flow more explicit and reusable.
- Keeps one source of truth for asset/data canonical paths.

**Validation**
- Build/testes oficiais verdes apos consolidacao:
  - `cmake --build build -j$(nproc)` OK
  - `ctest --test-dir build -L official --output-on-failure` OK (4/4)
- Guardrails executados:
  - `rg '\brand\(|\bsrand\(' src/` -> sem ocorrencias
  - `rg 'detail::active_locale|locale_bind\(|\bL\(' src/` -> sem ocorrencias
  - `rg '"Grimjaw"' src/` -> apenas conteudo narrativo/spawn display name

### Task DN - Reuse canonical enemy section names in dungeon rules

**Applied scope**
- `src/world/dungeon_rules.hpp`

**Changes made**
- `enemy_type_name(...)` deixou de repetir literais de nome de secao de inimigo.
- O mapeamento agora reutiliza `data_sections::kEnemyIniSections` como fonte canonica, com fallback seguro para `"enemy"` fora de faixa.

**Expected effect**
- Remove duplicacao de strings de identidade de inimigo em camada de regras.
- Reduz risco de drift entre regras de dungeon e nomes de secao usados por loaders.

**Validation**
- `cmake --build build -j$(nproc)` OK
- `ctest --test-dir build -L official --output-on-failure` OK (4/4)

### Task DO - PauseMenu edge state encapsulation (anti-leak internals)

**Applied scope**
- `src/core/pause_menu.hpp`
- `src/systems/overlay_input.hpp`

**Changes made**
- `PauseMenu` deixou de expor os campos `prev_*` publicamente.
- O estado interno de edge tracking passou para campos privados (`_prev_*`).
- Foram adicionados getters read-only explícitos (`was_pause_pressed`, `was_ui_up_pressed`, `was_ui_down_pressed`, `was_confirm_pressed`, `was_ui_cancel_pressed`).
- `OverlayInputTracker` foi ajustado para consumir esses getters, sem acesso direto a internals do menu.

**Expected effect**
- Melhora encapsulamento do tipo central de UI e reduz acoplamento acidental a detalhes internos.
- Mantém o mesmo comportamento funcional de edge detection, com contrato mais explícito.

**Validation**
- `cmake --build build -j$(nproc)` OK
- `ctest --test-dir build -L official --output-on-failure` OK (4/4)

### Task DP - DialogueSystem encapsulation with public query API

**Applied scope**
- `src/systems/dialogue_system.hpp`
- `tests/core/test_dungeon_dialogue_registry.cpp`
- `tests/core/test_town_config_loader.cpp`

**Changes made**
- Encapsulated `DialogueSystem` runtime storage (`_sequences`, `_active_lines`, `_line_index`, `_is_active`, etc.) as `private`.
- Added minimal public query API `has_sequence(const std::string&)` for safe registry assertions.
- Updated tests that previously accessed `dialogue._sequences` directly to use `dialogue.has_sequence(...)`.

**Expected effect**
- Reduces coupling to `DialogueSystem` internals and strengthens encapsulation boundaries.
- Keeps test intent intact (verify expected dialogue ids are registered) using stable public contract.

**Validation**
- `cmake --build build -j$(nproc)` OK
- `ctest --test-dir build -L official --output-on-failure` OK (4/4)

### Task DQ - Dialogue render uses single-line public accessor

**Applied scope**
- `src/systems/dialogue_system.hpp`
- `src/systems/dialogue_render.hpp`

**Changes made**
- Added `DialogueSystem::current_line()` that returns `const DialogueLine*` when active/index is valid.
- Updated `render_dialogue_ui(...)` to consume `current_line()` directly.
- Reduced renderer coupling to internal dialogue container/index pairing (`active_lines` + `current_line_index`) for the main render path.

**Expected effect**
- Simpler and safer rendering contract for dialogue UI.
- Better encapsulation boundary with less dependence on internal storage structure.

**Validation**
- `cmake --build build -j$(nproc)` OK
- `ctest --test-dir build -L official --output-on-failure` OK (4/4)

### Task DR - DialogueSystem API cleanup (remove legacy line/index accessors)

**Applied scope**
- `src/systems/dialogue_system.hpp`

**Changes made**
- Removed legacy public accessors `active_lines()` and `current_line_index()`.
- Kept `current_line()` as the canonical read API for dialogue UI consumption.
- Confirmed no remaining usages in `src/` or `tests/` before removal.

**Expected effect**
- Tightens the public contract and discourages new callsites from coupling to internal sequence/index representation.
- Keeps API surface minimal and aligned with encapsulation goals from item 12.8.

**Validation**
- `cmake --build build -j$(nproc)` OK
- `ctest --test-dir build -L official --output-on-failure` OK (4/4)

### Task DS - PauseMenuController no longer exposes mutable PauseMenu internals

**Applied scope**
- `src/systems/pause_menu_controller.hpp`
- `src/systems/overlay_input.hpp`
- `src/scenes/world_scene.hpp`

**Changes made**
- Removed `PauseMenuController::menu()` mutable/const accessors.
- Added narrow read-only forwarding API on controller (`was_pause_pressed`, `was_ui_up_pressed`, `was_ui_down_pressed`, `was_ui_cancel_pressed`).
- `OverlayInputTracker` now depends on `PauseMenuController` (not raw `PauseMenu`) and uses the forwarded read API.
- Updated `WorldScene` callsites to pass `_pause_controller` directly to overlay capture/flush.

**Expected effect**
- Reduces leakage of mutable UI internals across module boundaries.
- Preserves current overlay edge behavior while tightening ownership/encapsulation.

**Validation**
- `cmake --build build -j$(nproc)` OK
- `ctest --test-dir build -L official --output-on-failure` OK (4/4)

### Task DT - PauseMenu internal state (`paused/settings_open`) encapsulated

**Applied scope**
- `src/core/pause_menu.hpp`
- `src/systems/pause_menu_controller.hpp`

**Changes made**
- `PauseMenu` no longer exposes raw public booleans for modal state.
- Moved `paused/settings_open` to private storage (`_paused`, `_settings_open`).
- Added explicit state/transition methods:
  - `is_paused()`, `is_settings_open()`
  - `open()`, `close()`, `open_settings()`
- Updated `PauseMenuController` to use these methods (including menu item callbacks and state accessors).

**Expected effect**
- Further reduces accidental coupling to mutable internals in central UI type.
- Keeps behavior unchanged while making state transitions explicit in API.

**Validation**
- `cmake --build build -j$(nproc)` OK
- `ctest --test-dir build -L official --output-on-failure` OK (4/4)

### Task DU - PauseMenu locale dependency encapsulated

**Applied scope**
- `src/core/pause_menu.hpp`
- `src/systems/pause_menu_controller.hpp`

**Changes made**
- Removed direct public locale field usage from `PauseMenu`.
- Added explicit setter `set_locale(LocaleSystem*)` and moved locale storage to private `_locale`.
- Updated `PauseMenuController` callsites to use `_menu.set_locale(...)` during setup/init.

**Expected effect**
- Prevents external modules from mutating locale dependency via raw field access.
- Keeps dependency injection explicit and aligned with encapsulation rules.

**Validation**
- `cmake --build build -j$(nproc)` OK
- `ctest --test-dir build -L official --output-on-failure` OK (4/4)

### Task DV - PauseMenu list internals encapsulated

**Applied scope**
- `src/core/pause_menu.hpp`

**Changes made**
- Moved `ui::List` storage from public field to private `_list`.
- Updated internal menu flow (`init`, navigation, selection dispatch, open/reset and render) to use `_list`.
- Kept external behavior unchanged; no external module depended on direct `list` access.

**Expected effect**
- Further reduces mutable UI internals exposed by `PauseMenu`.
- Reinforces single-owner principle for menu navigation/render state.

**Validation**
- `cmake --build build -j$(nproc)` OK
- `ctest --test-dir build -L official --output-on-failure` OK (4/4)

### Task DW - Pause menu API surface trim (unused accessors)

**Applied scope**
- `src/core/pause_menu.hpp`
- `src/systems/pause_menu_controller.hpp`

**Changes made**
- Removed unused `PauseMenu::was_confirm_pressed()` read accessor.
- Removed unused `PauseMenuController::settings_open()` read accessor.
- Kept `PauseMenuController::paused()` because tests rely on it as contract/assertion point.

**Expected effect**
- Smaller public API surface with fewer accidental dependencies.
- No behavioral impact; only interface cleanup around unused reads.

**Validation**
- `cmake --build build -j$(nproc)` OK
- `ctest --test-dir build -L official --output-on-failure` OK (4/4)

## Decisões de escopo (deliberadamente fora de tarefas)

Este registro vive aqui para que decisões de "não fazer" não sejam reabertas por engano em sessões futuras. Sempre que adiar deliberadamente uma migração, escreva a razão aqui — não no corpo da tarefa.

### `NpcEntity::dialogue_*` continua `std::string` (decidido em FF-2, 2026-04-11)

- **O que ficou de fora:** migrar `NpcEntity::dialogue_default` / `dialogue_quest_active` / `dialogue_quest_done` para `std::optional<TownDialogueId>`.
- **Por quê:**
  1. O campo é **data-driven**: hoje é setado por código (`TownBuilder`) com `to_string(TownDialogueId::...)`, mas o destino natural é vir de config (e.g. `town_npcs.ini`). Strings nesse ponto são parte da borda com o data-layer.
  2. Existem ids canônicos *parciais*: `villager_a`/`villager_b` são usados só dentro do INI, sem referência por código. Migrar `NpcEntity` para enum exige primeiro fechar o conjunto canônico (decidir se viram `TownDialogueId::VillagerA`/`VillagerB` ou se ganham outro tipo).
  3. O ganho é marginal: a interpretação errada do id já é detectada em runtime via `DialogueSystem::start` (ignora id desconhecido) — não há lógica de domínio que ramifique pelo valor da string.
- **Quando reabrir:** se/quando NPCs forem migrados para serem definidos por INI (ponto 11 do refactor), aí faz sentido fechar o conjunto canônico de ids e tipar o campo. Até lá, custo > benefício.

*Última actualização: consolidado a partir da revisão de arquitectura e do relatório WorldScene/controllers; expandido com guia de execução assistida por IA.*
