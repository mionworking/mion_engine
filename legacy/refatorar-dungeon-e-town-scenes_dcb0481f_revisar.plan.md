---
name: refatorar-dungeon-e-town-scenes
overview: Refatorar DungeonScene e TownScene para extrair lógica de controle em módulos/controllers/systems reaproveitáveis, mantendo as cenas mais finas como orquestradoras.
todos:
  - id: analisar-cenas-e-systems-existentes
    content: Confirmar interfaces e limitações dos controllers/systems já existentes (PauseMenuController, AttributeLevelUpController, SkillTreeController, ShopSystem, DialogueSystem, RoomFlowSystem, RoomManager, SaveSystem) para garantir que os novos módulos apenas os orquestrem.
    status: completed
  - id: definir-contextos-dungeon-town
    content: Definir tipos leves de contexto (DungeonContext, TownContext) para passar referências necessárias aos novos controllers/serviços.
    status: completed
  - id: extrair-save-controllers
    content: Extrair lógica de save/load de DungeonScene e TownScene para DungeonSaveController e TownSaveController, reutilizando SaveSystem.
    status: completed
  - id: extrair-config-loaders
    content: Mover carregamento de INIs de dungeon/town de dentro das cenas para loaders/serviços dedicados, preservando apply_* existentes.
    status: completed
  - id: refatorar-fluxo-dungeon-town
    content: Extrair lógica de fluxo de run/salas no dungeon e de interação NPC/quest/loja no town para novos controllers, mantendo RoomFlowSystem e ShopSystem como motores centrais.
    status: completed
  - id: extrair-audio-fx-e-footsteps
    content: Criar sistemas dedicados para música/FX/footsteps e animação, reduzindo o tamanho de fixed_update nas cenas.
    status: completed
  - id: extrair-render-townspecifico
    content: Criar TownWorldRenderer/TownPropRenderer para isolar lógica de renderização específica da cidade fora de TownScene.
    status: completed
isProject: false
---

### Refatoração de DungeonScene e TownScene

#### Objetivos principais

- **Enxugar `DungeonScene` e `TownScene`** para que atuem mais como orquestradoras de sistemas já existentes, mantendo nelas apenas fluxo de cena e composição.
- **Extrair ou realocar lógicas de controle** (save, fluxo de salas, diálogos, música/FX, NPCs, shop, town layout) para módulos/controllers/systems dedicados.
- **Reaproveitar controllers/systems já existentes** (`PauseMenuController`, `AttributeLevelUpController`, `SkillTreeController`, `RoomFlowSystem`, `ShopSystem`, `DialogueSystem`, etc.), conectando a lógica da cena a eles em vez de criar duplicatas.

---

### 1. Mapa de módulos/controllers/systems existentes a reutilizar

- **HUD e UI in-game**
  - Reusar: `DungeonHud`, `SkillTreeController`, `AttributeLevelUpController`, `PauseMenuController`, `OverlayInputTracker` já incluídos em `[src/scenes/dungeon_scene.hpp](src/scenes/dungeon_scene.hpp)`.
  - Ações: garantir que qualquer nova orquestração de fluxo (save/autosave, transição de cena) passe por esses controllers quando fizer sentido, em vez de lógica duplicada.
- **Fluxo de salas/dungeon e transições de cena**
  - Reusar: `RoomFlowSystem` já incluso tanto no dungeon quanto no town; `RoomManager` para construção/avanço de salas.
  - Ações: mover a maior parte da lógica de "qual sala carregar, para onde transicionar" para funções utilitárias/serviços reutilizáveis que internamente chamem `RoomFlowSystem`/`RoomManager`.
- **Shop e economia**
  - Reusar: `ShopSystem` para lógica de compra e render da UI de loja.
  - Ações: extrair apenas o **controle de input/estado da loja** da `TownScene` para um pequeno controller, em cima de `ShopSystem`.
- **Diálogos**
  - Reusar: `DialogueSystem` e `render_dialogue_ui` já usados nas duas cenas.
  - Ações: mover o registro/carregamento de diálogos de dungeon/town para loaders ou arquivos dedicados, sem reinventar nenhum novo tipo de sistema de diálogo.
- **Save/Load**
  - Reusar: `SaveSystem` e `SaveData` definidos em `[src/core/save_system.hpp](src/core/save_system.hpp)`.
  - Ações: criar funis de leitura/gravação específicos (p.ex. `dungeon_save_profile_*`, `town_save_profile_*`) em helpers/serviços, sem outro tipo de mecanismo de save.

---

### 2. Refatorar `DungeonScene` – novos serviços/controllers propostos

#### 2.1. DungeonSaveController / DungeonRunStatsService

- **Local sugerido**: novo header `[src/systems/dungeon_save_controller.hpp](src/systems/dungeon_save_controller.hpp)` ou dentro de `[src/core/save_system.hpp](src/core/save_system.hpp)` se você preferir agrupar.
- **Responsabilidades a mover de `DungeonScene`**:
  - `_capture_save_data` e a montagem de `SaveData` específica do dungeon.
  - `_persist_save` (incluindo preservação de `victory_reached`, difficulty, autosave flash e debug save).
  - `_snapshot_last_run_to_save` (espelhando `RunStats` da run atual para save permanente).
- **API sugerida**:
  - `SaveData make_dungeon_save(const DungeonContext& ctx);`
  - `void persist_dungeon_save(const DungeonContext& ctx, bool show_autosave_indicator);`
  - `void snapshot_last_run(const DungeonContext& ctx);`
- **Mudanças na cena**:
  - Em `fixed_update` e helpers (_apply_save_and_build_world, _advance_room, level up, pickups, transição de cena), substituir chamadas diretas a `SaveSystem::load_default/save_default/save_debug/remove_default_saves` por chamadas a esse serviço.

#### 2.2. DungeonConfigLoader (dados de ini do dungeon)

- **Local sugerido**: `[src/core/config_loader.hpp](src/core/config_loader.hpp)` ou novo `[src/systems/dungeon_config_loader.hpp](src/systems/dungeon_config_loader.hpp)`.
- **Responsabilidades a mover de `_load_data_files`**:
  - Reset de defaults (`reset_progression_config_defaults`, `reset_player_config_defaults`, `reset_talent_tree_defaults`).
  - Leitura de `progression.ini`, `player.ini`, `talents.ini`, `enemies.ini`, `spells.ini`, `items.ini`, `rooms.ini`.
  - Preenchimento de `EnemyDef` por tipo e `DropConfig`, assim como `g_spell_defs`.
- **API sugerida**:
  - `DungeonData load_dungeon_static_data();` (contendo arrays de `EnemyDef`, `DropConfig`, `IniData rooms_ini`, etc.).
- **Mudanças na cena**:
  - Em `enter`, substituir `_load_data_files` por uma chamada ao loader que retorna estrutura pronta; armazenar no estado privado da cena (`_enemy_defs`, `_drop_config`, `_rooms_ini`).

#### 2.3. DungeonDialogueRegistry (ou carregamento via INI)

- **Opção A (hardcoded, mas fora da cena)**:
  - Criar `[src/core/dungeon_dialogue.hpp](src/core/dungeon_dialogue.hpp)` com função
    - `void register_dungeon_dialogue(DialogueSystem& dlg);`
  - Mover o corpo de `_register_dungeon_dialogue` para essa função.
- **Opção B (dados externos)**:
  - Seguir o padrão de town, com um `dungeon_dialogues.ini` + `register_dialogue_sequences_from_ini`.
- **Mudanças na cena**:
  - Em `enter`, substituir `_register_dungeon_dialogue();` por `register_dungeon_dialogue(_dialogue);` ou equivalente.

#### 2.4. DungeonFlowController (fluxo de run & sala)

- **Local sugerido**: novo `[src/systems/dungeon_flow_controller.hpp](src/systems/dungeon_flow_controller.hpp)`.
- **Responsabilidades a mover de `DungeonScene`**:
  - `_full_run_reset_initial` (novo run do zero).
  - `_apply_save_and_build_world` (carregar save e reconstruir mundo).
  - `_advance_room` (avanço de sala, spawn de inimigos, diálogos de transição e persistência).
  - Partes de `fixed_update` que tratam de `_room_flow_sys` e decisão de `scene_exit_to`, `_scene_exit_pending`, `_pending_next_scene` e `_scene_exit_target`.
- **API sugerida** (recebendo contextos já existentes como `RoomManager`, `RoomFlowSystem`, `QuestState`, `RunStats`, etc.):
  - `void start_full_run(DungeonContext& ctx, bool clear_saved_progress);`
  - `void apply_save_and_build_world(DungeonContext& ctx, const SaveData& sd);`
  - `void advance_room(DungeonContext& ctx);`
  - `void update_room_flow(DungeonContext& ctx, float dt);` (retornando talvez um enum/struct com `next_scene`, `trigger_victory_scene`, etc.).
- **Mudanças na cena**:
  - Deixar `DungeonScene::enter` chamar `DungeonFlowController::start_full_run` ou `apply_save_and_build_world`.
  - Em `fixed_update`, substituir blocos de lógica de saída de cena/transição de sala por chamadas a esse controller, atualizando somente `_pending_next_scene`.

#### 2.5. DungeonAudioStateSystem (música, ambience e FX globais)

- **Local sugerido**: `[src/systems/dungeon_audio_system.hpp](src/systems/dungeon_audio_system.hpp)`.
- **Responsabilidades a mover**:
  - Trechos de `enter` que inicializam ambience/música do dungeon.
  - Bloco em `fixed_update` que calcula `MusicState` baseado em boss vivo, aggro e `room_index`.
  - Trechos isolados que apenas decidem que `MusicState` setar ou que ambience tocar/parar, mas não dependem de decisão de fluxo de cena.
- **API sugerida**:
  - `void init_dungeon_audio(AudioSystem& audio);`
  - `void update_dungeon_music(AudioSystem& audio, const DungeonContext& ctx);`
- **Mudanças na cena**:
  - Substituir lógica inline de música por chamadas a esse sistema, mantendo apenas o `tick_music()` dentro da cena.

#### 2.6. CombatFxController (hitstop, camera shake, screen flash, morte)

- **Local sugerido**: `[src/systems/combat_fx_controller.hpp](src/systems/combat_fx_controller.hpp)`.
- **Responsabilidades a mover**:
  - Bloco de `fixed_update` que reage a `_combat_sys.pending_hitstop`, `player_hit_landed`, `parry_success` e aplica shake e sons.
  - Bloco que trata `_death_snapshot_done`, `_death_fade_remaining`, e dispara `game_over`.
- **API sugerida**:
  - `void apply_combat_fx(CombatFxState& fx, const CombatSystem& combat, Camera2D& cam, AudioSystem* audio, float dt, RunStats* run_stats, DungeonSceneOutputs& outputs);`
- **Mudanças na cena**:
  - Manter apenas armazenamento de estado (timers), delegando a lógica de atualização desses timers ao novo controller.

#### 2.7. FootstepAudioSystem

- **Local sugerido**: `[src/systems/footstep_audio_system.hpp](src/systems/footstep_audio_system.hpp)`.
- **Responsabilidades a mover**:
  - Bloco de `fixed_update` que acumula distâncias, toca `SoundId::FootstepPlayer`/`FootstepEnemy` e zera acumuladores para player e inimigos.
- **API sugerida**:
  - `void update_footsteps(AudioSystem& audio, Actor& player, std::vector<Actor>& enemies, const Camera2D& cam, float dt);`
- **Mudanças na cena**:
  - Em `fixed_update`, após atualizar movimentos, chamar apenas a função do sistema.

#### 2.8. Pequenas utilidades

- **AnimationDriver compartilhado**:
  - `_update_animations` em `DungeonScene` e o pequeno bloco de animação do player em `TownScene` são muito parecidos.
  - Criar algo como `[src/systems/animation_driver.hpp](src/systems/animation_driver.hpp)` com:
    - `void update_actor_anims(std::vector<Actor*>& actors, float dt);`
    - `void update_player_town_anim(Actor& player, float dt);`

---

### 3. Refatorar `TownScene` – novos serviços/controllers propostos

#### 3.1. TownSaveController

- **Local sugerido**: `[src/systems/town_save_controller.hpp](src/systems/town_save_controller.hpp)` ou reutilizar o mesmo arquivo de save com variante de perfil.
- **Responsabilidades a mover**:
  - `_capture_save`, `_apply_save`, `_apply_stats_after_configure`.
  - Chamadas a `SaveSystem::save_default` diretamente em `_start_npc_interaction` (quest giver) e em transição de cena.
- **API sugerida**:
  - `SaveData make_town_save(const TownContext& ctx);`
  - `void apply_town_save(TownContext& ctx, const SaveData& sd);`
  - `void apply_town_stats_after_configure(TownContext& ctx, const SaveData& sd);`
- **Mudanças na cena**:
  - No `enter`, usar `TownSaveController::apply_town_save` em vez de `_apply_save`.
  - No `exit` e em pontos onde hoje salva imediato (aceitar quest, completar quest, sair para dungeon), usar as helpers do controller.

#### 3.2. TownConfigLoader

- **Local sugerido**: reutilizar `config_loader` ou criar `town_config_loader.hpp`.
- **Responsabilidades a mover de `enter`**:
  - Reset de player/progression/talents e load de `player.ini`, `progression.ini`, `talents.ini`.
  - Carregamento de `town_dialogues.ini` via `register_dialogue_sequences_from_ini`.
- **API sugerida**:
  - `void load_town_player_and_progression_config();`
  - `void load_town_dialogues(DialogueSystem& dlg);`
- **Mudanças na cena**:
  - Em `enter`, substituir blocos manuais por chamadas a esse loader.

#### 3.3. TownLayoutBuilder

- **Local sugerido**: `[src/world/town_layout.hpp](src/world/town_layout.hpp)` ou `[src/systems/town_builder.hpp](src/systems/town_builder.hpp)`.
- **Responsabilidades a mover de `_build_town`**:
  - Configuração de `_room.bounds`, obstáculos, portas e tilemap de grama.
  - Criação da lista `_npcs` com `NpcEntity` preenchidos e `ShopInventory` do forge.
  - Posição inicial do player.
- **API sugerida**:
  - `void build_town_world(TownWorld& world, TextureCache* tc);` contendo `RoomDefinition`, `Tilemap`, `TilemapRenderer`, `std::vector<NpcEntity>`, `ShopInventory`, posição do player.
- **Mudanças na cena**:
  - Substituir `_build_town()` por algo como `TownLayoutBuilder::build_town_world(ctx);` e copiar apenas os resultados para campos privados da cena.

#### 3.4. NpcActorFactory

- **Local sugerido**: `[src/systems/npc_actor_factory.hpp](src/systems/npc_actor_factory.hpp)`.
- **Responsabilidades a mover de `_rebuild_npc_actors`**:
  - Conversão de cada `NpcEntity` em um `Actor` configurado (posição, half_w/half_h, move_speed, sprite_scale).
- **API sugerida**:
  - `void rebuild_npc_actors(const std::vector<NpcEntity>& npcs, Actor& player, std::vector<Actor>& npc_actors, std::vector<Actor*>& actors);`
- **Mudanças na cena**:
  - Em `enter` e onde mais precisar reconstruir NPCs, chamar essa função e manter `_npcs`, `_npc_actors` e `_actors` como storage apenas.

#### 3.5. TownNpcWanderSystem

- **Local sugerido**: `[src/systems/town_npc_wander.hpp](src/systems/town_npc_wander.hpp)`.
- **Responsabilidades a mover de `_update_npcs`**:
  - RNG xorshift, cálculo de alvos de wander, clamping ao raio de spawn, update de `is_moving` e sincronização `npc.x/y`.
- **API sugerida**:
  - `void update_town_npcs(std::vector<NpcEntity>& npcs, float dt);` (usando `npc.actor` internamente).
- **Mudanças na cena**:
  - Em `fixed_update`, trocar `_update_npcs(dt);` por chamada a esse sistema.

#### 3.6. TownNpcInteractionController

- **Local sugerido**: `[src/systems/town_npc_interaction.hpp](src/systems/town_npc_interaction.hpp)`.
- **Responsabilidades a mover**:
  - `_nearest_npc_index` (cálculo de NPC mais próximo).
  - `_start_npc_interaction` (lógica de abrir shop, escolher diálogos de quest, conceder recompensa, salvar).
- **API sugerida**:
  - `int find_nearest_npc(const Actor& player, const std::vector<NpcEntity>& npcs);`
  - `void handle_npc_interaction(int idx, TownContext& ctx);`
- **Mudanças na cena**:
  - Em `fixed_update`, manter apenas detecção de `confirm_edge` e `_interacting_npc_hint`, delegando o resto para esse controller.

#### 3.7. ShopInputController (em cima de ShopSystem)

- **Local sugerido**: `[src/systems/shop_input_controller.hpp](src/systems/shop_input_controller.hpp)`.
- **Responsabilidades a mover de `_update_shop`**:
  - Navegação de índice da loja com `move_y` (debounce pelo threshold).
  - Disparo de tentativa de compra (`ShopSystem::try_buy` + áudio).
  - Fechar shop com `ui_cancel`.
- **API sugerida**:
  - `void update_shop_input(ShopInventory& inv, Actor& player, const InputState& input, AudioSystem* audio, bool& shop_open, float& prev_move_y, bool& prev_confirm, bool& prev_cancel);`
- **Mudanças na cena**:
  - Em `fixed_update`, dentro do ramo `_shop_open`, chamar apenas esse controller e deixar o resto como está.

#### 3.8. TownWorldRenderer / TownPropRenderer

- **Local sugerido**: `[src/systems/town_world_renderer.hpp](src/systems/town_world_renderer.hpp)`.
- **Responsabilidades a mover de `render`**:
  - Desenho dos prédios com seleção de sprite por nome de obstáculo.
  - Desenho das portas de saída para dungeon.
  - Desenho de NPCs com mapeamento `nome → sprite` e posição baseada em `NpcEntity`/`Actor`.
- **API sugerida**:
  - `void render_town_world(SDL_Renderer* r, const TownRenderContext& ctx);`
- **Mudanças na cena**:
  - Em `render`, deixar apenas a orquestração de ordem de draw: fundo → town_world_renderer → player → hints/labels → UI (gold, diálogo, shop, pause).

---

### 4. Passo a passo de implementação recomendado

1. **Criar contextos de dados leves** (`DungeonContext`, `TownContext`) em um header comum, contendo apenas referências/pointers necessários (player, enemies/npcs, room, quest_state, run_stats, audio, camera, etc.).
2. **Extrair primeiro os helpers de save** (DungeonSaveController e TownSaveController), pois são relativamente mecânicos e reduzem acoplamento forte a `SaveSystem` dentro das cenas.
3. **Extrair loaders/configs de ini** (DungeonConfigLoader e TownConfigLoader), reaproveitando o código atual apenas movido e limpando as cenas.
4. **Mover construção de mundo** (`_build_town`, `_load_room_visuals` e partes de `_apply_save_and_build_world`/`_advance_room`) para `TownLayoutBuilder` e `DungeonFlowController`.
5. **Extrair controllers de fluxo** (`DungeonFlowController`, `TownNpcInteractionController`, `ShopInputController`) e conectar às cenas, garantindo que a interface use apenas controllers já existentes para baixo nível (RoomFlowSystem, ShopSystem, DialogueSystem, PauseMenuController).
6. **Criar sistemas de FX/música/footsteps** (`DungeonAudioStateSystem`, `CombatFxController`, `FootstepAudioSystem`, `AnimationDriver`) e trocar a lógica inline em `fixed_update` por chamadas a esses sistemas.
7. **Por fim, extrair renderers específicos** de town (`TownWorldRenderer`) para deixar `TownScene::render` mais enxuta e semelhante ao padrão `DungeonWorldRenderer`.

Cada uma dessas etapas pode ser feita em PRs pequenos, rodando os testes existentes (`tests/test_scenes_v2.cpp`, `tests/test_v2_main.cpp`, etc.) para garantir que o comportamento continue igual, já que você está apenas redistribuindo responsabilidades sem introduzir novas features nem novos tipos de controller redundantes.