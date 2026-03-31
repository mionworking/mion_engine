# mion_engine

Motor de jogo 2D em C++17 com SDL3: dungeon com tilemap, iluminação estilo *fake lighting*, combate corpo a corpo e à distância, IA com pathfinding, áudio, save/load, diálogo básico e uma **cena de dungeon** jogável com progressão, maná, magias e árvore de talentos.

> **Modo atual:** placeholders geométricos (retângulos coloridos + seta de direção). **Tileset e sprites finais são a última fase** — depois do jogo estar funcionalmente completo; ver [ROADMAP.md](ROADMAP.md) (secção *Ordem de trabalho* e *Arte final*).

Ao iniciar, o jogo abre a **cena de título** (`title`) com menu completo: `New Game`, `Continue`, `Settings`, `Extras` e `Quit`. `New Game` abre o seletor de dificuldade e segue para a **cidade** (`town`); `Continue` carrega a run existente quando há save; `Extras` inclui `Credits`; `Settings` permite alterar **mute**, **volume master** e **idioma** (EN/PT-BR) com persistência em `config.ini`.

## Estado atual

- Cena de título com menu completo (`New Game`, `Continue`, `Settings`, `Extras`, `Quit`)
- Save/load automático da run
- `config.ini` para resolução, áudio e idioma de UI
- `data/*.ini` para stats de inimigos, magias e drops
- Testes locais cobrindo core, save/load, config/runtime, diálogo e integrações

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

- `config.ini` é procurado ao lado do executável e, se não existir lá, no diretório atual (`cwd`)
- `data/enemies.ini`, `data/spells.ini` e `data/items.ini` são carregados em runtime; se faltarem, o jogo usa defaults do código
- para uma release empacotada, o esperado é distribuir `data/` ao lado do binário

### Save/load

- `Play` no title abre a seleção de dificuldade e continua a run
- `N` no title apaga o save atual
- `Continue` no title fica desativado quando não há save
- o save atual é gravado em `SDL_GetPrefPath("mion", "mion_engine")/save.txt`
- o loader também aceita o save legado `mion_save.txt`

## Testes

```bash
make test
# ou
cd build && ctest --output-on-failure
```

O alvo `mion_tests` roda asserts locais (sem framework externo); na árvore atual cobre **580+ asserts** em dezenas de grupos, incluindo física, combate, save/load, config, menu de título/settings, diálogo, `SceneManager`/`SceneRegistry`, fluxo de portas e integrações com `ScriptedInputSource`.

No bloco de distribuição (keybinds), há testes dedicados para `scancode_from_name()` e `load_keybinds()`, cobrindo:
- defaults sem secção `[keybinds]`
- parsing de nomes válidos e inválidos
- fallback quando a tecla é inválida
- carregamento de todos os campos de rebinding

## Ferramentas opcionais

| Alvo Makefile   | Descrição |
|-----------------|-----------|
| `stress`        | Simulação headless com muitos inimigos |
| `render_stress` | Stress de render com janela SDL |
| `sprite_bench`  | Bench sintético focado em custo de sprites/frames |

Variáveis de ambiente úteis:

- `MION_STRESS_ENEMIES`: no boot, ajusta o número de inimigos na dungeon. `0` (omissão) = comportamento normal. `1`–`3` fixa o spawn count em salas normais. Valores **maiores que 3** ativam o *stress mode* (spawn massivo / headless em `mion_stress`).

## Controles (teclado)

| Ação | Tecla |
|------|--------|
| Mover | WASD ou setas |
| Ataque corpo a corpo | Espaço ou Z |
| Esquivar / dash | Shift esquerdo |
| Ataque à distância | X |
| Aparar | C |
| Magia 1 (ex.: Bolt) | Q |
| Magia 2 (ex.: Nova) | E |
| Avançar diálogo | Enter |
| Melhorias no level-up | 1 / 2 / 3 |
| Talentos (overlay) | 4 / 5 / 6 |
| Apagar save / nova run | N |
| Navegação em menus | Setas |
| Confirmar em menus | Enter |
| Voltar (menus/credits) | Backspace |

Maná e stamina regeneram com o tempo; magias consomem maná e obedecem cooldown do grimório. Ao ganhar XP (por exemplo ao derrotar inimigos), você pode receber **pontos de talento** para gastar na árvore (com pré-requisitos).

No título, o submenu **Extras** contém **Credits** (retorno via `Backspace`).

### Rebinding de teclado (`config.ini`)

Você pode customizar teclas na secção `[keybinds]` usando nomes aceitos por `SDL_GetScancodeFromName` (inglês, case-insensitive), por exemplo:

```ini
[keybinds]
attack=J
dash=Left Shift
spell_1=Q
pause=Escape
```

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
  - Movimento: analógico esquerdo
  - Ataque: South (A / Cruz)
  - Ranged: West (X / Quadrado)
  - Dash: East (B / Círculo)
  - Parry: Left Shoulder (LB / L1)
  - Spell 1: Right Shoulder (RB / R1)
  - Spell 2: Right Trigger
  - Spell 3/4: D-pad Left / Up
  - Confirm: South
  - Pause/Cancel: Start

## Layout do código

| Pasta | Conteúdo |
|-------|-----------|
| `src/core/` | Config, input, tempo, áudio, `SceneManager`, `SceneRegistry`, `register_scenes` |
| `src/components/` | Dados de gameplay (vida, combate, maná, grimório, talentos, stamina…) |
| `src/systems/` | Lógica por frame (combate, IA, projéteis, drops, magias…) |
| `src/world/` | Salas, tilemap, navgrid |
| `src/entities/` | `Actor`, projéteis, itens no chão |
| `src/scenes/` | Cenas de title/town/dungeon/game over/victory/credits |
| `assets/` | Áudio (WAVs); sprites e tiles reservados para fase de arte |
| `data/` | Definições externas de inimigos, magias e drops (INI) |
| `tools/asset_pipeline/` | Pipeline auxiliar para inventário/conversão de assets |
| `tests_legacy/` | suíte compartilhada + runner monolítico `test_main.cpp` (ctest `legacy`) |
| `tests_v2/` | suíte oficial modular (`mion_tests_v2`, etiqueta ctest `official`) |

**Testes:** após `cmake -S . -B build && cmake --build build`, executar `ctest --test-dir build` (todos) ou `ctest --test-dir build -L official` (apenas a suíte V2). O alvo `mion_tests_legacy` permanece disponível com a etiqueta `legacy`; desative com `-DMION_ENABLE_LEGACY_TESTS=OFF` se quiser só V2 no build.

## Licença / créditos

Defina conforme o projeto (dependências: SDL3, stb_image — ver licenças nos repositórios oficiais).
