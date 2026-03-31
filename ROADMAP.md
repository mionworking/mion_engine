# mion_engine — Roadmap

## Estado atual

O projeto já tem um loop jogável completo com `title -> town -> dungeon`, combate melee/ranged, IA com pathfinding, progressão, magias, áudio SDL3, save/load, diálogo básico, configuração externa e tabelas de dados em INI.

## Ordem de trabalho: jogo pronto primeiro, arte no fim

**Prioridade explícita:** fechar um **jogo completo em funcionalidade** (fluxo início–fim, conteúdo, ritmo, controlos, save, eventual vitória ou run objetivo) usando o que o motor já suporta — **tilemap e atores com fallback geométrico** (retângulos, cores, partículas leves, texto).

**Tileset PNG, sprites finais dos atores e ícones de HUD “ricos”** são **última fase**: só depois o jogo estiver **jogável e fechado em regras**. Não bloqueiam balanceamento, novas salas, diálogos nem infra; encaixam-se quando os paths e o manifest estiverem estáveis.

---

## Feito

- [x] Loop de física fixo (accumulator pattern, 60 fps desacoplado do render)
- [x] `EngineConfig`, `Camera2D` (follow + clamp + screen shake com decay e teto de amplitude)
- [x] `IInputSource` + `KeyboardInputSource` (WASD/setas, Space/Z, Shift, X, C, Q/E/R/F, 1-6)
- [x] `ScriptedInputSource` para replay de `InputState` em testes de integração
- [x] `Transform2D`, `HealthState`, `AABB`, `CollisionBox`, `HurtBox`, `MeleeHitBox`
- [x] `CombatState` (Idle -> Startup -> Active -> Recovery, um hit por swing)
- [x] `Actor` unificado (player/inimigos via `Team`)
- [x] `RoomDefinition` com bounds, obstáculos e portas
- [x] `RoomCollisionSystem` (mundo + actor vs actor)
- [x] `MeleeCombatSystem`, knockback, hitstop e screen shake
- [x] `EnemyAISystem` com aggro, separação, perseguição, ataque e pathfinding; **ext.** dispatch por `AiBehavior` (Melee / Ranged / Patrol / Elite / BossPhased), projéteis de inimigo, patrulha com alerta a vizinhos, boss em 2 fases com carga — ver [`.cursor/plans/roadmap_enemy_variety.md`](.cursor/plans/roadmap_enemy_variety.md)
- [x] Tipos de inimigo alargados: `Archer`, `PatrolGuard`, `EliteSkeleton`, `BossGrimjaw`; `data/enemies.ini` com secções correspondentes; spawn em stress (ciclo 6 tipos) + Grimjaw na sala 3 com intro e gatilho de fase 2 (shake, diálogo, flash)
- [x] UI reutilizável [`src/core/ui.hpp`](src/core/ui.hpp) (`Panel`, `List`, `Label`, `ProgressBar`); menu de **pausa** na dungeon e na cidade; **Escape** não termina o processo no `main` (quit via fecho de janela) — ver [`.cursor/plans/roadmap_ui_system.md`](.cursor/plans/roadmap_ui_system.md)
- [x] **Skill tree** em overlay na dungeon (Tab e entrada pelo menu de pausa), 3 disciplinas, navegação por setas, Enter para gastar ponto; opção desativada na cidade
- [x] HUD dungeon: ouro, nível, barra XP no rodapé, hotbar Q/E/R/F com cooldown; HUD básica, HP bars, flash de dano, debug de facing/hitbox
- [x] `SceneManager`, `SceneRegistry`, `register_default_scenes`
- [x] Cena de título com menu completo (`New Game`, `Continue`, `Settings`, `Extras`, `Quit`), seletor de dificuldade, `Continue` desativado sem save e `Extras -> Credits`
- [x] Transição por zona (`RoomFlowSystem`) entre dungeon e title
- [x] `BitmapFont` embutida (`draw_text`, `text_width`)
- [x] Tilemap com fallback geométrico e iluminação fake
- [x] `TextureCache` + `draw_sprite` + `AnimPlayer` com fallback para retângulos
- [x] Stamina, dash, projéteis, ranged attack e parry
- [x] Progressão, XP, level-up, maná, grimório e árvore de talentos **D2-style** (17 nós / 3 disciplinas, níveis 0–3, spend dinâmico nas teclas 4–6)
- [x] Oito magias (`SpellId`), sinergias por talento, cleave / multishot+veneno / chain lightning / strafe / battle cry, projéteis com slow e poison on-hit, buff de dano temporário
- [x] Drops / itens no chão
- [x] Áudio SDL3 com SFX, música em loop e fade-out na volta ao título
- [x] Audio polish: `MusicState` (Town/DungeonCalm/DungeonCombat/DungeonBoss/Victory), ambient loops (`AmbientId`), SFX posicional (`play_sfx_at`), variação de pitch (`play_sfx_pitched`), footsteps player/inimigo, testes opcionais de integração
- [x] Sistema de diálogo com prólogo, vários gatilhos na dungeon, wrap de texto e hint `ENTER - continuar`
- [x] Save/load em texto (`key=value`) com auto-save, **formato v3** (talentos por nível int; sem `spell_unlocked` — grimório via `sync_from_talents`), migração v1/v2→v3 em memória, clamp de `room_index`, load automático e fallback para save legado `mion_save.txt`
- [x] Configuração externa via `config.ini` (resolução, `volume_master`, `mute`, `ui.autosave_indicator`, `ui.language`) + persistência runtime de opções do title mantendo `[keybinds]`
- [x] Dados externos em `data/enemies.ini`, `data/spells.ini`, `data/items.ini`, `data/player.ini`, `data/progression.ini`, `data/talents.ini`, **`data/rooms.ini`**
- [x] `ini_loader.hpp` (`get_int` / `get_float` / `get_string`, **`sections_with_prefix`**), `spell_defs` mutável, `apply_spell_ini_section` / `apply_enemy_ini_section`, configs globais de progressão e jogador; inimigos com campos opcionais de sprite no INI (documentados; arte no fim)
- [x] Geometria de dungeon por `room_index`: `dungeon_rules::room_bounds()` / `room_template()` / **`room_template_id_name()`**, obstáculos compostos em `room.hpp`, layouts `_layout_*` em `DungeonScene`; **`room_loader.hpp`** — obstáculos opcionais por template quando `[tpl].ini_obstacles=1` em `rooms.ini`; **`_load_data_files()`** carrega todos os INI incluindo salas
- [x] Testes automatizados (`mion_tests_v2` + `ctest`, etiqueta `official`; legado `mion_tests_legacy` em [`tests_legacy/`](tests_legacy/)): plano alargado em [`tests_legacy/test_plan.cpp`](tests_legacy/test_plan.cpp) — stamina/status/progression, save v3 / spell rank / Nova Arcane L2, **`Ini.PrefixSections`**, **`RoomLoader.*`**, **`DungeonRules.TemplateIds`**, `ResourceSystem` / `MovementSystem` / `StatusEffectSystem` / `ProjectileSystem`, `PlayerActionSystem` (incl. FrostBolt/Nova), `NavGrid` / `Pathfinder`, `AnimPlayer` / `Camera2D`, IA inimigo (incl. ranged/projétil, def boss, parse `ai_behavior`, nomes de tipo), **UI** (`UI.List*`, `UI.ProgressBarClamp`), menu de título/settings (`TitleScene.Continue`, `TitleScene.MainQuit`, `TitleScene.SettingsBack`, `Config.Language`, `Config.SaveRuntime`), [`dungeon_rules.hpp`](src/world/dungeon_rules.hpp), partículas, infra [`tests_legacy/test_common.hpp`](tests_legacy/test_common.hpp). *Nota:* cleave / chain / strafe / battle cry / multishot+veneno ainda sem testes unitários dedicados (smoke opcional `MION_DUNGEON_SMOKE`).
- [x] Testes opcionais por ambiente: `MION_AUDIO_INTEGRATION_TESTS`, `MION_DUNGEON_SMOKE` (smoke `DungeonScene` com SDL oculto)
- [x] Build release (`make release`) e `compile_commands.json` via CMake
- [x] Partículas simples (impacto melee/projétil, morte) e validação de assets no boot (`log_missing_assets_optional`)
- [x] CI GitHub Actions: build Release + `ctest`; jobs extra **Debug** com sanitizers (`MION_ENABLE_TEST_SANITIZERS`) e cobertura em `mion_tests` (`MION_ENABLE_COVERAGE`)

---

## Conteúdo / Gameplay (roadmap fechado)

O plano detalhado em [`.cursor/plans/roadmap_conteudo_gameplay.md`](.cursor/plans/roadmap_conteudo_gameplay.md) está **concluído** no código. Checklist com itens ~~riscados~~ e mapa testes ↔ funcionalidade: [`.cursor/plans/roadmap_conteudo_gameplay_planilha.md`](.cursor/plans/roadmap_conteudo_gameplay_planilha.md).

- [x] ~~Skill tree D2-style (17 nós), 8 `SpellId`, efeitos/handlers, save v3 por nível de talento~~
- [x] ~~`rooms.ini` + `sections_with_prefix` + `room_loader` + integração em `_build_room` (fallback `_layout_*`)~~
- [x] ~~Base INI + salas por template (`player` / `progression` / `talents` / `spells` / `enemies` / `items` / `rooms`)~~
- [x] ~~Gatilhos de diálogo (primeira porta, salas profundas, relíquia de lore)~~
- [x] ~~Mini-boss `Grimjaw` (sala 3) + diálogo ao morrer~~
- [x] ~~Salas distintas por `room_index` (layouts em código; obstáculos opcionais por INI com `ini_obstacles=1`)~~

**Opcional / próximo conteúdo** (fora deste roadmap): testes unitários para cleave/chain/strafe/battle cry/multishot; mais salas; mecânicas dos nós de talento ainda sem gameplay (ex.: whirlwind); misturar Archer/Patrol/Elite nas salas com orçamento ≤3 se quiseres ver novos tipos antes do stress.

---

## Falta — Apresentação (sem depender de sprites / tileset novos)

- [x] Partículas simples para impacto e morte
- [x] HUD mais legível (rótulos XP/ST/MP e indicadores de status em texto)
- [x] SFX de UI (`UiConfirm`, `UiDelete`), parry distinto, indicador opcional de auto-save (`config.ini`)
- [x] Mensagem de game over explicitando que o restart limpa o save atual
- [ ] Opcional: SFX ainda mais granulares; afinação da curva de shake por design

---

## Falta — Arte final (última fase, após jogo funcional)

> **Só depois** de o loop de jogo estar fechado (regras, conteúdo, fluxo). Até lá, placeholders geométricos e retângulos são o visual esperado.

- [ ] Tileset real no lugar do fallback geométrico
- [ ] Sprites finais dos atores no fluxo principal
- [ ] Ícones gráficos no HUD (além do texto; pode acompanhar ou seguir os sprites)

---

## Falta — Infraestrutura

- [x] Manifest / validação de assets no boot (lista mínima + log; alinhar com `tools/asset_pipeline` quando o inventário global existir)
- [x] Contrato de texturas runtime: `tools/texture_manifest.json` + geração `tools/gen_placeholder_textures.py` + preview `tools/preview_placeholders.py`
- [x] Contrato de texturas planejado (backlog): `tools/texture_backlog_contract.json` + auditoria `tools/audit_texture_contract.py` + relatório `tools/texture_contract_inventory.md`
- [x] Alvos Make para placeholders: `gen-placeholders`, `preview-placeholders`, `verify-placeholders`, `gen-placeholders-backlog`, `preview-placeholders-backlog`
- [ ] Integração contínua da `tools/asset_pipeline` ao empacotamento (além da lista em C++) **(adiado para depois)**
- [x] CI para `cmake` + `ctest` (Linux), mais jobs `linux-asan` e `linux-coverage`
- [x] Opções CMake `MION_ENABLE_COVERAGE` e `MION_ENABLE_TEST_SANITIZERS` no alvo `mion_tests`
- [ ] Windows cross-compile / validação fora de Linux **(adiado para depois)**
- [x] Versionamento do save v3 (talentos int), compatibilidade v1/v2, `migrate_v1_to_v2`, clamp de `room_index`
- [x] Rebinding de teclas no `config.ini`

---

## Ferramentas (`tools/`)

- `mion_stress`, `mion_render_stress`, variável `MION_STRESS_ENEMIES` — ver [README.md](README.md)
- Alinhar inventários: `tools/asset_pipeline` ↔ lista em [src/core/asset_manifest.hpp](src/core/asset_manifest.hpp) quando o contrato global evoluir

---

## Próximas prioridades

### 1. Jogo jogável completo (sem arte final)

Conteúdo/gameplay do plano `roadmap_conteudo_gameplay` está fechado. Roadmaps **UI** e **enemy variety** (planos em `.cursor/plans/`) estão espelhados em código e referenciados em **Feito** acima. Seguinte: clarificar objectivos de run (vitória / fim de run; ver `roadmap_run_loop.md` se aplicável), rebinding em `config.ini`, e testes extra para magias avançadas se desejares cobertura total.

### 2. Infra de distribuição

Empacotamento/install com pipeline de assets e CI/build em Windows ficam **adiados para depois**.  
Rebinding em `config.ini` já foi implementado.

### 2.1 Contrato de assets (produção)

Manter o fluxo em duas camadas:
- runtime obrigatório em `tools/texture_manifest.json` (já ligado ao verify);
- backlog planejado em `tools/texture_backlog_contract.json` (não bloqueia runtime).

Próximo passo: integrar `MION_TEXTURE_INTEGRATION_TESTS=1` na CI para travar regressão.

### 3. Arte final

Tileset, sprites de atores e ícones de HUD ricos — **por último**, quando as regras e o fluxo estiverem validados.
