# Plano — Migração para Open World
> mion_engine_cpp · 2026-04-04

---

## Status por fase

| Fase | Descrição | Status |
|------|-----------|--------|
| 1 | WorldMap + broadphase collision | **Feito** |
| 2 | ZoneManager | **Feito** |
| 3 | WorldContext + WorldSaveController + AreaEntry | **Feito** |
| 4 | WorldScene + WorldAudioSystem | **Feito** |
| 5 | Limpeza / testes apontando para `"world"` | **Feito** |
| 6 | Testes da nova estrutura + remoção cenas legadas | **Feito** |
| 7 | Auditoria (checklist × testes / revisão) | **Feito** |
| 8 | Pipeline Python de assets (texturas + áudio) | **Feito** (pipeline *externo* opcional; ver abaixo) |
| 9 | Bug transição town → 1ª sala (tela preta, combate) | **Feito** |
| 10 | Auditoria **room** → **world** (APIs e dados ainda “por sala”) | **Inventário feito** (ver § abaixo; refatorações futuras opcionais) |

**Build:** ok · **CTest** (`mion_tests_v2` + core/scenes/input): 0 failed.

---

## Premissa (1 linha)

Mundo contínuo em coordenadas globais; uma cena `WorldScene`; câmera segue o player em todo o mapa.

---

## O que já está feito (síntese)

- **Mundo:** `WorldArea` / `WorldMap` / `WorldMapBuilder`, colisão broadphase, `ZoneManager`, `WorldContext`, save v6, `WorldSaveController`, `AreaEntrySystem`.
- **Cena:** `WorldScene` (enter/fixed_update/render, sistemas por zona), `WorldAudioSystem`, registro `"world"`, título/vitória apontam para world.
- **Testes:** `tests/world/*`, `test_save_v6`, `test_world_scene_lifecycle`, migrações de testes legados; `WorldScene::player_actor()` para asserts.
- **Fase 9:** iluminação uma vez por frame (não por área); spawn de inimigos com `world_origin` da área; corredor com altura 900 alinhada às salas; assert no builder.
- **Corredor:** `corridor_floor.png` no chão da zona `Transition`; `AmbientId::TransitionWind`; parágrafo Assets no `README.md`.
- **Contrato de texturas:** `tools/texture_manifest.json` cobre as **23** PNG referenciadas em `src/`; `python3 tools/audit_texture_contract.py` → `missing=0` / `extra=0` (regenera `tools/texture_contract_inventory.md`).

---

## WorldScene — combate, colisão e IA (debugging; 2026-04-04)

### Sintomas (evolução)

1. **Primeira queixa:** inimigos, magias e flechas **não apareciam** no ecrã.
2. **Após correções de render/projéteis:** passaram a **ser visíveis**, mas **inimigos não atacavam** e o jogador **não conseguia atingi-los**.
3. **Após correção do pathfinder em espaço local:** esperava-se que a IA **fechasse distância** e o melee **registasse hits** (distâncias e `attack_range` voltariam a fazer sentido).
4. **Estado actual (confirmado pelo autor):** **ainda não resolveu** — inimigos **não atacam nem recebem dano**, e o jogador **passa por cima** deles (sem colisão física credível entre player e inimigo).

### O que já foi feito no código (registo técnico detalhado)

Esta secção documenta **alterações já aplicadas** no repositório; serve para não repetir trabalho e para orientar a próxima investigação.

#### A) Render: coordenadas de mundo vs. câmera “offset” da área

- **Problema:** No open world, posições de **atores** e **projéteis** estão em **coordenadas globais** (soma do `offset_x` / `offset_y` da `WorldArea` ao espaço local da sala). O **tilemap**, **obstáculos** e **portas** da `RoomDefinition` continuam em **espaço local** da sala. A renderização usava uma única `Camera2D` com `x/y` deslocados (`_camera` menos o offset da área) — correcto para o chão/paredes, **incorrecto** para entidades já expressas em mundo.
- **Efeito:** Sprites e rectângulos de inimigos/projéteis eram projectados como se as suas coordenadas fossem locais → apareciam **fora do ecrã** ou em posição errada.
- **Correção:** `DungeonWorldRenderInputs` passou a ter duas câmeras:
  - **`camera`** — alinhada ao espaço **local** da área (tiles, obstáculos, portas).
  - **`world_camera`** — a câmera global usada para o jogador no resto da cena.
- **Uso de `world_camera`:** actores (só **inimigos** na lista passada ao renderer; o player continua a ser desenhado num bloco posterior com `_camera`), projéteis, drops no chão, partículas, textos flutuantes, e posicionamento do overlay de luz quando aplicável.
- **Player na lista da área:** removido de `area_actors` no ramo dungeon para **evitar duplo desenho** do jogador com câmeras diferentes.
- **Ficheiros tocados:** `src/systems/dungeon_world_renderer.hpp`, `src/scenes/world_scene.hpp`.

#### B) Projéteis: limites da sala em espaço local vs. posição em mundo

- **Problema:** `ProjectileSystem::fixed_update` testava `room.bounds` e `obstacle.bounds` como se **x/y** dos projéteis estivessem no mesmo referencial — mas no open world **estão em mundo**.
- **Efeito:** Projéteis eram considerados “fora da sala” quase de imediato → **desapareciam** sem serem vistos ou sem acertar.
- **Correção:** Parâmetros `world_origin_x` / `world_origin_y` somados aos bounds da sala e aos AABBs dos obstáculos antes dos testes de intersecção.
- **Ficheiros tocados:** `src/systems/projectile_system.hpp`, `src/scenes/world_scene.hpp`, `tests/systems/test_projectiles.cpp` (origem `0,0` nos testes unitários que simulam uma única sala).

#### C) IA e pathfinding: grelha local vs. coordenadas globais

- **Problema:** O `NavGrid` é construído a partir do **tilemap** e da **room** em coordenadas **locais** (índices de tile 0…cols/rows). `Pathfinder::find_path` converte `sx,sy` para tile com `px / tile_size` **sem** subtrair o offset da área. Com posições globais (ex.: milhares em X), os índices **saturavam** nas bordas da grelha pequena → caminhos errados ou degenerados.
- **Efeito:** Inimigos **não se aproximavam** correctamente do jogador; `dist` e ranges de ataque **não refletiam** a geometria esperada; sintoma parecido a “não há combate”.
- **Correção:** Threading de **`nav_area_ox` / `nav_area_oy`** (iguais a `cur_area->offset_x/y` na dungeon) em `EnemyAISystem::fixed_update` e nas rotinas `replan_chase_path`, `move_along_path_toward`, `steer_on_nav_path`, `patrol_along_waypoints`, `update_patrol`, `update_boss`, `chase_and_melee_attack`, `update_ranged`. Conversão **mundo → local** para `find_path`; conversão **local → mundo** ao seguir waypoints.
- **Comentário:** `pathfinder.hpp` esclarece que o caminho é em espaço **local** da sala.
- **Ficheiros tocados:** `src/systems/ai_combat.hpp`, `src/systems/ai_patrol.hpp`, `src/systems/ai_boss.hpp`, `src/systems/enemy_ai.hpp`, `src/systems/pathfinder.hpp` (documentação), `src/scenes/world_scene.hpp`.

### O que **não** está resolvido (segundo feedback)

- Colisão **jogador ↔ inimigo** (passar “por cima”).
- **Dano recebido** por inimigos (melee / possivelmente outras fontes).
- **Ataques dos inimigos** que conectem com o jogador.

Isto sugere que, para além de render e pathfinding, subsiste **pelo menos um** problema em: lista de actores, ordem dos sistemas, colisão actor–actor, referencial em algum subsistema de combate, ou estado/zona que desactiva combate sem ser óbvio.

### O que mais se tentaria fazer (só investigação / próximos passos — não executado nesta edição do plano)

Ordem sugerida por **custo vs. informação**:

1. **Verificar `_actors` em runtime na dungeon**  
   - Confirmar que, ao entrar na zona de dungeon, `_rebuild_all_actors()` inclui ponteiros para **todos** os `area.enemies` vivos.  
   - Confirmar que `_combat_sys.fixed_update(_actors, …)` e `MeleeCombatSystem` iteram os mesmos ponteiros que o utilizador “vê” no ecrã.  
   - *Hipótese:* inimigos visíveis por render a partir de `area.enemies` mas **ausentes** de `_actors` → sem colisão/combate.

2. **Colisão actor–actor (`RoomCollisionSystem::resolve_actors`)**  
   - Hoje chama-se `resolve_actors(_actors, nullptr)`. Com `room == nullptr`, `_resolve_pair` faz um empurrão inicial e **não** volta a projectar contra `RoomDefinition::resolve` nas fases seguintes.  
   - Avaliar se isso é suficiente para separar player e inimigo ou se, em mundo contínuo, falta **sempre** resolver pares com bounds/obstáculos **globais** (ou repetir `resolve` contra `cur_area->room` **com offset** aplicado a uma cópia temporária dos bounds — desenho de solução, não implementado aqui).  
   - *Hipótese:* “passar por cima” = **nenhuma** resolução efectiva entre equipas.

3. **`MeleeCombatSystem` e hitboxes**  
   - Tudo usa `transform` global + `melee_hit_box` / `hurt_box` em mundo — em princípio coerente **se** as posições forem as mesmas que no render.  
   - Confirmar `facing`, fases de ataque (`is_in_active_phase`), `attack_hit_landed`, e se `hurt_stun` ou `can_receive_hit()` bloqueiam sem feedback visual.  
   - Log pontual (numa futura sessão de código): centros e AABBs no frame do active phase.

4. **`cur_area` nulo ou área errada**  
   - Se `area_at(player)` falhar em bordas ou durante transições, ramos que dependem de `cur_area` (projéteis, pathfinder) podem comportar-se mal.  
   - *Hipótese menos provável* se o pathfinder já recebe offset correcto, mas vale confirmar nas mesmas frames em que o jogador “atravessa” inimigos.

5. **Zonas e flags**  
   - Garantir que `in_dungeon()` é verdadeiro durante o teste e que nenhum ramo (diálogo, pause, boss intro, shop) está a **saltar** combat ou movimento sem ser evidente.

6. **Teste de integração mínimo**  
   - Cena ou teste que constrói um `WorldMap` com **uma** área dungeon, um inimigo spawnado com offset conhecido, player a `fixed_update` N frames, e asserts sobre `dist`, overlap de AABBs, e `CombatEvent` / HP — **reproduzível** na CI.

7. **Comparação com commit pré–open-world**  
   - Se existir histórico git de `DungeonScene` vs `WorldScene`, diff no **pipeline** `movement → collision → combat → AI` para detectar passos perdidos na migração.

8. **Documentar offsets verticais**  
   - `WorldMapBuilder`: town `offset_y = 0`; corredor e salas dungeon `offset_y = 300` (alinhamento intencional com altura 900 do corredor). **Não** é, por si só, bug — mas qualquer código que misture “Y da town” com “Y da dungeon” sem somar offset continua candidato a revisão.

---

## O que falta

| Item | Nota |
|------|------|
| **Combate / colisão em `WorldScene`** | **Em aberto** — ver secção **WorldScene — combate, colisão e IA** acima; sintomas persistem após fixes de render, projéteis e pathfinder. |
| Pipeline com pacote fonte externo | Ver **Ainda por tua conta** |

### Ainda por tua conta (só quando fizer sentido)

- **`tools/asset_pipeline/main.py`** — correr `scan` / `plan` / `convert` quando houver um **pacote fonte externo** de arte; `tools/asset_pipeline/README.md`.
- **Novas PNG em `src/`** — ao adicionar referências, atualizar `texture_manifest.json` e voltar a correr `python3 tools/audit_texture_contract.py`.

### Fase 10 — inventário (room vs world; 2026-04-04)

| Área | Conclusão curta |
|------|-----------------|
| Save v6 | `WorldSaveController` fixa `room_index = 0` com comentário *legacy*; gameplay não depende disso para posição (usa coords globais + bitmask de áreas). |
| HUD | `render_dungeon_hud` usa `ZoneManager::room_index_equiv()` — alinhado à zona atual, não a um índice global antigo. |
| `DungeonAudioSystem` | Header existe; **não** incluído em `WorldScene` — áudio de zona é `WorldAudioSystem`. |
| `room_flow_system` | Apenas testes legados (`tests/legacy/test_legacy.cpp`). |
| `RoomManager::build_room` | Usado por `WorldMapBuilder` por área (`rooms.ini` + índice de sala na construção) — modelo “uma `RoomDefinition` por `WorldArea`” mantido de propósito. |
| `rooms.ini` / `room_loader` | Templates por índice de sala na construção; cada área recebe o seu layout — não há “sala atual” única em runtime. |
| Combate / projéteis / morte | `room_index_equiv()` no `WorldScene` para spawn e `EnemyDeathController`; projéteis atualizam contra `cur_area->room` (local). |
| Colisão / path | `WorldScene` usa `get_obstacles_near` + `total_bounds` e pathfinder **por** `WorldArea`. |
| Próximos passos opcionais | Renomear templates `dungeon_r00` por zona; remover `room_index` do schema quando não houver migração v1–v5 em uso; revisar projéteis se surgir bug de bounds. |

**Fase 10:** inventário acima fecha o critério “room local por área vs mundo global”; refactors são opcionais (última linha).

---

## Ordem histórica das fases

Fases 1–10 do plano **fechadas** para o âmbito open world “estrutural”; 8 inclui scripts locais + manifesto completo. **Excepção activa:** combate/colisão player–inimigo em `WorldScene` ainda não validado em jogo (secção de debugging). Restam o **asset_pipeline** opcional e refactors opcionais da fase 10.
