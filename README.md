# mion_engine

Motor de jogo 2D em C++17 com SDL3: dungeon com tilemap, iluminação estilo *fake lighting*, combate corpo a corpo e à distância, IA com pathfinding, áudio, save/load, diálogo básico e uma **cena de dungeon** jogável com progressão, maná, magias e árvore de talentos.

> **Modo atual:** placeholders geométricos (retângulos coloridos + seta de direção) com spritesheets Puny Characters integradas. **Tileset e sprites finais são a última fase** — depois do jogo estar funcionalmente completo; ver [ROADMAP.md](ROADMAP.md).

Ao iniciar, o jogo abre a **cena de título** (`title`) com menu completo: `New Game`, `Continue`, `Settings`, `Extras` e `Quit`. `New Game` abre o seletor de dificuldade e segue para a **cidade** (`town`); `Continue` carrega a run existente quando há save; `Extras` inclui `Credits`; `Settings` permite alterar **mute**, **volume master** e **idioma** (EN/PT-BR) com persistência em `config.ini`.

## Estado atual

- Cena de título com menu completo (`New Game`, `Continue`, `Settings`, `Extras`, `Quit`)
- Save/load automático da run com migração de schema (`v1` → `v5`)
- `config.ini` para resolução, áudio, idioma de UI e rebinding de teclado
- `data/*.ini` para stats de inimigos, magias, talentos, progressão, drops, salas e diálogos
- Cenas e sistemas modularizados por responsabilidade (4 sprints SRP completos)
- `DungeonScene` e `TownScene` usam controllers stateful para pause, level-up de atributos e árvore de talentos
- Type safety: `Actor::sprite_sheet` usa `SDL_Texture*` com forward declaration (Sprint 4 — F-001)
- Suporte a gamepad SDL3 com detecção automática de conexão/desconexão
- Testes locais cobrindo core, save/load, config/runtime, diálogo, input, cenas e integrações (~660 asserts em 5 targets)

## Requisitos

- CMake 3.20+
- Compilador C++17
- Na **primeira** configuração, rede para o CMake baixar SDL3 e stb via FetchContent (depois pode usar `FETCHCONTENT_UPDATES_DISCONNECTED` como no `CMakeLists.txt`).

## Build e execução

```bash
make build          # Debug em build/
./build/mion_engine
```

Build otimizado (binário em `build_release/`):

```bash
make release
./build_release/mion_engine
```

Equivalente com CMake direto:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
./build/mion_engine
```

### Configuração e dados externos

- `config.ini` é procurado ao lado do executável (`SDL_GetBasePath()`) e, se não existir lá, no diretório atual (`cwd`)
- os arquivos `data/*.ini` seguem a mesma lógica de resolução de path (`resolve_data_path()`)
- para uma release empacotada, o esperado é distribuir `data/` ao lado do binário

Arquivos de dados carregados em runtime:

| Arquivo | Conteúdo |
|---------|----------|
| `data/enemies.ini` | Stats, sprite path e IA por tipo de inimigo |
| `data/spells.ini` | Dano, cooldown, velocidade e área de cada magia |
| `data/items.ini` | Taxas de drop, bônus de HP/dano/velocidade |
| `data/player.ini` | Stats base do player (HP, velocidade, dano, dash, etc.) |
| `data/progression.ini` | XP por level, bônus de progressão |
| `data/talents.ini` | Custos e pré-requisitos da árvore de talentos |
| `data/rooms.ini` | Layout e templates de salas da dungeon |
| `data/town_dialogues.ini` | Linhas de diálogo dos NPCs da cidade |
| `data/locale_en.ini` | Strings de UI em inglês |
| `data/locale_ptbr.ini` | Strings de UI em português |

Se um arquivo `.ini` faltar, o jogo usa defaults do código.

### Save/load

- `New Game` no title abre a seleção de dificuldade e inicia a run
- `N` no title apaga o save atual
- `Continue` no title fica desativado quando não há save
- o save atual é gravado em `SDL_GetPrefPath("mion", "mion_engine")/save.txt`
- o loader também aceita o save legado `mion_save.txt`
- migrações automáticas de `v1` até `v5` garantem compatibilidade com saves antigos

## Testes

```bash
make test
# ou
cd build && ctest --output-on-failure
```

**5 targets de teste** via CTest:

| Target | Etiqueta | Escopo |
|--------|----------|--------|
| `mion_tests_legacy` | `legacy` | Suíte original: física, combate, save/load, config, menus, diálogo, portas, fluxo de cenas |
| `mion_tests_v2` | `official` | Gaps de cobertura, player action, integrações |
| `mion_tests_v2_core` | `official` | Core systems |
| `mion_tests_v2_scenes` | `official` | Testes de cena |
| `mion_tests_v2_input` | `official` | Input (teclado, gamepad, keybinds) |

**~660 asserts** no total (macros `EXPECT_TRUE`, `EXPECT_EQ`, etc. — framework interno, sem dependência externa).

Para rodar apenas a suíte V2:

```bash
ctest --test-dir build -L official
```

O alvo legacy pode ser desativado com `-DMION_ENABLE_LEGACY_TESTS=OFF`.

### Gates de teste opcionais (variáveis de ambiente)

| Variável | Efeito |
|----------|--------|
| `MION_AUDIO_INTEGRATION_TESTS=1` | Habilita testes que precisam do subsistema de áudio |
| `MION_TEXTURE_INTEGRATION_TESTS=1` | Habilita testes que carregam texturas reais |
| `MION_DUNGEON_SMOKE=1` | Habilita smoke test da dungeon completa |

## Ferramentas opcionais

| Alvo Makefile | Descrição |
|---------------|-----------|
| `stress` | Simulação headless com muitos inimigos |
| `render_stress` | Stress de render com janela SDL |
| `sprite_bench` | Bench sintético focado em custo de sprites/frames |
| `gen-placeholders` | Gera texturas placeholder via `tools/gen_placeholder_textures.py` |
| `preview-placeholders` | Visualiza placeholders gerados |
| `verify-placeholders` | Gera + visualiza + roda testes de textura |

### Variáveis de ambiente

| Variável | Onde | Descrição |
|----------|------|-----------|
| `MION_STRESS_ENEMIES` | `mion_engine`, `mion_stress` | Número de inimigos. `0` = normal; `1–3` = spawn fixo; `>3` = stress mode |
| `MION_STRESS_FRAMES` | `mion_stress` | Frames a simular (default 600) |
| `RS_WIDTH`, `RS_HEIGHT` | `mion_render_stress` | Resolução da janela |
| `RS_MAX`, `RS_STEP`, `RS_FRAMES` | `mion_render_stress` | Máximo de actors, passo de incremento, frames por passo |
| `RS_FRAMES_MULT` | `mion_render_stress` | Multiplicador de frames de animação |
| `SB_WIDTH`, `SB_HEIGHT` | `mion_sprite_bench` | Resolução da janela |
| `SB_MAX_ACTORS`, `SB_STEP_ACTORS` | `mion_sprite_bench` | Máximo de actors e passo de incremento |
| `SB_FRAMES_PER_ANIM`, `SB_RENDER_FRAMES` | `mion_sprite_bench` | Frames de animação e frames de render |

## Controles (teclado)

| Ação | Tecla padrão |
|------|--------------|
| Mover | WASD ou setas |
| Ataque corpo a corpo | Z (alt: Espaço) |
| Esquivar / dash | Shift esquerdo |
| Ataque à distância | X |
| Aparar | C |
| Magia 1 (ex.: Bolt) | Q |
| Magia 2 (ex.: Nova) | E |
| Magia 3 | R |
| Magia 4 | F |
| Avançar diálogo / confirmar | Enter |
| Pausar | Escape |
| Árvore de talentos | Tab |
| Melhorias no level-up | 1 / 2 / 3 |
| Talentos (overlay) | 4 / 5 / 6 |
| Apagar save / nova run | N |
| Navegação em menus | Setas |
| Voltar (menus/credits) | Backspace |

Maná e stamina regeneram com o tempo; magias consomem maná e obedecem cooldown do grimório. Ao ganhar XP (por exemplo ao derrotar inimigos), você pode receber **pontos de talento** para gastar na árvore (com pré-requisitos).

### Rebinding de teclado (`config.ini`)

Você pode customizar teclas na secção `[keybinds]` usando nomes aceitos por `SDL_GetScancodeFromName` (inglês, case-insensitive):

```ini
[keybinds]
attack=J
dash=Left Shift
spell_1=Q
pause=Escape
```

Campos disponíveis: `attack`, `attack_alt`, `ranged`, `dash`, `parry`, `spell_1`–`spell_4`, `confirm`, `pause`, `skill_tree`, `erase_save`, `upgrade_1`–`upgrade_3`, `talent_1`–`talent_3`.

Se uma chave faltar ou for inválida, o jogo usa automaticamente o default interno.

### Localização (`config.ini` + `data/locale_*.ini`)

- Defina idioma em `config.ini`:

```ini
[ui]
language=en   # en | ptbr
```

- Arquivos de idioma:
  - `data/locale_en.ini`
  - `data/locale_ptbr.ini`
- Se a chave não existir no locale carregado, o sistema mostra a própria chave como fallback.

## Gamepad (SDL3)

- O jogo tenta detectar gamepad automaticamente no boot e também ao conectar/desconectar durante a execução.
- Se houver gamepad conectado, a leitura de input prioriza gamepad; sem gamepad, usa teclado normalmente.
- Mapeamento atual:

| Ação | Botão |
|------|-------|
| Movimento | Analógico esquerdo |
| Ataque | South (A / Cruz) |
| Ranged | West (X / Quadrado) |
| Dash | East (B / Círculo) |
| Parry | Left Shoulder (LB / L1) |
| Spell 1 | Right Shoulder (RB / R1) |
| Spell 2 | Right Trigger |
| Spell 3 / 4 | D-pad Left / Up |
| Confirm | South |
| Pause / Cancel | Start |
| Navegação de menus | D-pad |
| Árvore de talentos | Back (Select) |

## `config.ini` — opções completas

```ini
[window]
width=1280
height=720

[audio]
volume_master=1.0
mute=0

[ui]
language=en
autosave_indicator=0

[keybinds]
# ver seção Rebinding acima
```

## Layout do código

| Pasta | Conteúdo |
|-------|-----------|
| `src/core/` | Engine: config, input, áudio, camera, sprites, animação, bitmap font, UI widgets, scene manager/registry, save system (`save_data`, `save_migration`, `save_system`), pause menu, locale, diálogo, scripted input, `title_menu`, `scene_fader`, `engine_paths`, `asset_manifest` |
| `src/components/` | Dados puros de gameplay: health, combat, collision, transform, mana, stamina, spell book/defs, talent tree/state, attributes, progression, equipment, status effects, player config |
| `src/systems/` | Lógica por frame, overlays e UI: combate melee, IA inimiga (`ai_patrol`, `ai_combat`, `ai_boss`, `enemy_ai`), projéteis, drops, magias (`spell_system`, `spell_effects`), movement, room collision/flow, resource regen, lighting, pathfinder, tilemap renderer, partículas, floating text, diálogo, shop; **controllers stateful**: `pause_menu_controller`, `attribute_levelup_controller`, `skill_tree_controller`, `overlay_input`; **rendering**: world renderer, dungeon world renderer, screen FX, dungeon HUD, skill tree UI, attribute screen UI; **gameplay modularizado**: enemy spawner, room manager, dash system; **config**: player configurator |
| `src/world/` | Salas (`Room`), tilemap, dungeon rules, room loader |
| `src/entities/` | `Actor`, `Projectile`, `GroundItem`, `NPC`, `Shop`, `EnemyType` |
| `src/scenes/` | Cenas: title, town, dungeon, game over, victory, credits |
| `src/vendor/` | `stb_image_impl.cpp` (implementação de stb_image) |
| `assets/` | Áudio (WAVs), sprites, tiles, props, NPCs, Puny Characters |
| `data/` | Definições externas (INI): inimigos, magias, items, player, progressão, talentos, salas, diálogos, locales |
| `tools/` | Stress tests, sprite bench, geradores de placeholders, asset pipeline, task tracker |
| `tools/asset_pipeline/` | Pipeline de classificação e conversão de assets (Python + Pillow vendorado) |
| `tests_legacy/` | Suíte compartilhada + runner `test_main.cpp` (ctest etiqueta `legacy`) |
| `tests_v2/` | Suíte oficial modular (4 targets, etiqueta ctest `official`) |
| `legacy/` | Documentação arquivada (`PLAYER_DOCUMENTATION.md`, `README_ASSET_SCRIPTS.md`, `ROADMAP_FALHAS.md`) |

## Módulos extraídos na refatoração SRP

Os arquivos abaixo foram criados ou promovidos para concentrar responsabilidades específicas. O efeito prático é que `DungeonScene`, `TitleScene`, `EnemyAISystem` e `PlayerActionSystem` hoje atuam mais como orquestradores do que como "god objects".

### Sprint 1

| Módulo | O que contém |
|--------|--------------|
| `src/systems/world_renderer.hpp` | Primitivas de render em coordenadas de mundo, render de actor com sprite/fallback, overlays de status, barras de HP e helpers compartilhados entre cenas. Inclui `facing_to_puny_row()` — fonte única, usada por dungeon e town. |
| `src/systems/player_configurator.hpp` | Configuração unificada do `Actor` do player a partir de `g_player_config`, atributos derivados, cooldowns, recursos, spell book e stats base. |
| `src/core/pause_menu.hpp` | State machine de pause compartilhada por dungeon/town, com navegação, callbacks por item e render do overlay. |
| `src/systems/dungeon_hud.hpp` | HUD principal da dungeon: XP, level, ouro, HP, stamina, mana, status e atalhos de habilidades. |
| `src/systems/skill_tree_ui.hpp` | Overlay da árvore de talentos com colunas, highlight de cursor, níveis e pré-requisitos. |
| `src/systems/attribute_screen_ui.hpp` | Overlay de level-up para distribuição de atributos com seleção de linha e descrições. |

### Sprint 2

| Módulo | O que contém |
|--------|--------------|
| `src/systems/enemy_spawner.hpp` | Cálculo de budget por sala/dificuldade/stress mode, spawn default/stress, spawn de boss e montagem do `Actor` inimigo a partir de `EnemyDef`. |
| `src/systems/room_manager.hpp` | Reset de runtime da sala, avanço de sala, construção de layout/tilemap, portas, templates/INI e posicionamento do player por contexto. |
| `src/systems/dungeon_world_renderer.hpp` | Pass completo de render do mundo da dungeon: tilemap, obstáculos, portas, actors, projéteis, drops, floating texts e lighting. |
| `src/systems/screen_fx.hpp` | Overlays full-screen independentes do mundo: intro de boss, death fade e screen flash. |
| `src/core/save_data.hpp` | Schema persistido do save (`SaveData`) e constantes de versão/limites. |
| `src/core/save_migration.hpp` | Cadeia de migração de save (`v1` → `v5`) e clamps/sanitização de compatibilidade. |
| `src/core/save_system.hpp` | Resolução de paths, parser do formato key-value, serialização, load/save e fallback entre save atual e legado. |

### Sprint 3

| Módulo | O que contém |
|--------|--------------|
| `src/core/title_menu.hpp` | Estado do menu de título, listas (`main`, `difficulty`, `settings`, `extras`), render e controller de ações (`New Game`, `Continue`, `Settings`, `Extras`, `Quit`). |
| `src/systems/ai_combat.hpp` | Separação de inimigos, replan de path, chase melee, ranged AI e spawn de projéteis inimigos. |
| `src/systems/ai_patrol.hpp` | Waypoints de patrulha, alerta de vizinhos, steering em nav path e transições `Patrol → Alert → Chase`. |
| `src/systems/ai_boss.hpp` | Lógica faseada de boss, trigger de phase 2 e charge attack. |
| `src/systems/enemy_ai.hpp` | Orquestrador da IA: localiza o player, filtra actors válidos e delega por `AiBehavior`. |
| `src/systems/dash_system.hpp` | Tick de cooldown do dash, dash ativo, consumo de stamina, iframes e áudio do dash. |
| `src/systems/spell_system.hpp` | Cooldowns de ranged/spells, cálculo de dano mágico/físico, spawn de projéteis e casts (`FrostBolt`, `Nova`, `ChainLightning`, `Strafe`, `BattleCry`). |
| `src/systems/player_action.hpp` | Orquestrador das ações do player: movimento básico, parry, melee e delegação para dash/ranged/spells. |

### Sprint 4 — F-001: Type Safety

| Mudança | Detalhe |
|---------|---------|
| `src/entities/actor.hpp` | `void* sprite_sheet` → `SDL_Texture* sprite_sheet` com forward declaration `struct SDL_Texture;` |
| 4 arquivos de rendering/cena | Removidos todos os `static_cast<SDL_Texture*>` (acesso direto ao campo tipado) |
| 2 arquivos de spawn/config | Removidos `static_cast<void*>` na atribuição de texturas |

### Sprint 5 — Limpeza de diálogo e dados de talentos

| Módulo | O que contém |
|--------|--------------|
| `src/systems/dialogue_system.hpp` | Runtime de diálogo (estado, avanço por input, callbacks) sem acoplamento a renderização ou parsing de INI. |
| `src/systems/dialogue_render.hpp` | Renderização da UI de diálogo (barra, texto, hint) a partir do estado exposto por `DialogueSystem`. |
| `src/core/dialogue_loader.hpp` | Loader de diálogos a partir de `IniData` (`data/town_dialogues.ini`), registrando sequências no `DialogueSystem`. |
| `src/components/talent_data.hpp` | Dados e defaults da árvore de talentos (`TalentId`, `TalentNode`, tabelas globais, helpers como `talent_def`). |
| `src/components/talent_loader.hpp` | Aplicação de overrides de `data/talents.ini` sobre `g_talent_nodes`/`g_talent_display_names` (`apply_talents_ini`). |
| `src/components/talent_tree.hpp` | Fachada fina que reexporta `talent_data` + `talent_loader` para os consumidores existentes. |

### Sprint 6 — Controllers stateful dos overlays da dungeon

| Módulo | O que contém |
|--------|--------------|
| `src/systems/pause_menu_controller.hpp` | Controller stateful que encapsula `PauseMenu`, callbacks de menu, update/flush/render e requests de abrir skill tree ou sair para o title. |
| `src/systems/attribute_levelup_controller.hpp` | Controller stateful da tela de atributos: decide abertura automática por `pending_level_ups`, navegação, aplicação de ponto e render. |
| `src/systems/skill_tree_controller.hpp` | Controller stateful da árvore de talentos: colunas persistentes, cursor, auto-open por pontos pendentes, gasto de talento e render. |
| `src/systems/overlay_input.hpp` | Tracker de edges compartilhado pelos overlays, para remover estado transitório de input da `DungeonScene`. |

### Como ler a arquitetura

- `src/scenes/dungeon_scene.hpp`: coordena fluxo de dungeon e delega overlays para `PauseMenuController`, `AttributeLevelUpController` e `SkillTreeController`.
- `src/scenes/title_scene.hpp`: coordena áudio/locale e delega menu/render/ações para `title_menu`.
- `src/systems/enemy_ai.hpp` e `src/systems/player_action.hpp`: permanecem como fachadas pequenas para integrar submódulos durante o `fixed_update`.

## Licença / créditos

Defina conforme o projeto (dependências: SDL3, stb_image — ver licenças nos repositórios oficiais).
