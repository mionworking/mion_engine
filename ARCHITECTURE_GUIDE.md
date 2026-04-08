# ARCHITECTURE_GUIDE — mion_engine_cpp

Referencia de arquitetura da engine. Consultar antes de criar modulos novos, nomear tipos ou decidir onde colocar logica.

Nao e backlog. Nao lista bugs. Define a estrutura que queremos manter e os contratos que nao devem ser violados.

---

## 1. Mapa de camadas

A engine segue uma hierarquia de dependencia explicita. Camadas superiores podem depender de camadas inferiores; o inverso e divida tecnica.

```
scenes/          <- orquestracao de alto nivel, composicao de sistemas
  |
systems/         <- logica de gameplay, controllers, renderers, services de IO
  |
world/           <- estruturas de mapa e contexto (sem logica de cena)
entities/        <- tipos de entidade (Actor, Projectile, NPC, GroundItem, Shop)
components/      <- dados puros de gameplay (sem side effects)
  |
core/            <- primitivas da engine (input, audio, camera, sprite, save, locale, UI)
```

### 1.1 Regras de dependencia

| Camada | Pode importar | Nao pode importar |
|--------|---------------|-------------------|
| `scenes/` | tudo | nada (e o topo) |
| `systems/` | `core/`, `components/`, `entities/`, `world/` | `scenes/` |
| `world/` | `core/`, `components/`, `entities/` | `systems/`, `scenes/` |
| `entities/` | `core/`, `components/` | `systems/`, `world/`, `scenes/` |
| `components/` | `core/` | tudo acima |
| `core/` | stdlib, SDL3, stb | nada do projeto |

Violacoes de dependencia devem ser tratadas como bug arquitetural, nao como conveniencia.

---

## 2. Taxonomia de modulos

O nome de um modulo deve deixar claro o papel que ele desempenha. Antes de criar um modulo, escolher a categoria correta.

### 2.1 Controller (classe stateful)

**Quando usar:** fluxo com estado local persistente entre frames, ciclo de vida (abrir/fechar/navegar), abertura e fechamento controlados pela cena.

**Contrato obrigatorio:**
- Guarda estado interno privado.
- Expoe `update(...) -> Result` que retorna intencoes (`world_paused`, `should_save`, `should_close`, etc.).
- Expoe `render(...)` separado do update.
- **Nao chama** `SaveSystem` diretamente — devolve `should_save` e deixa o owner persistir.
- **Nao dispara** transicoes de cena — devolve `requested_transition`.

**Exemplos corretos:** `SkillTreeController`, `AttributeLevelUpController`, `EquipmentScreenController`, `PauseMenuController`.

**Sinal de violacao:** controller que chama `SaveSystem::save_default` ou `SceneRegistry::push` internamente.

---

### 2.2 System (struct com fixed_update)

**Quando usar:** logica de gameplay que roda todo frame sobre um conjunto de entidades/atores. Stateless ou com estado minimo de cache.

**Contrato obrigatorio:**
- `fixed_update(actors, dt, ...)` como metodo principal.
- Pode ter campos de resultado do ultimo tick (ex.: `last_events`, `projectile_hit_actor`) para consumo pelo chamador no mesmo frame.
- **Nao renderiza.**
- **Nao persiste.**
- **Nao toca audio** diretamente — pode retornar eventos que o chamador repassa ao audio.

**Exemplos corretos:** `MeleeCombatSystem`, `ProjectileSystem`, `MovementSystem`, `ResourceSystem`, `StatusEffectSystem`, `RoomCollisionSystem`.

**Sinal de violacao:** system que abre arquivo, chama `AudioSystem::play_sfx` ou desenha com SDL_Renderer.

---

### 2.3 Namespace utilitario (stateless)

**Quando usar:** conjunto de funcoes relacionadas sem estado proprio, sem ciclo de vida, sem render.

**Contrato obrigatorio:**
- Namespace, nao classe.
- Funcoes puras ou que recebem todos os dependentes como parametro.
- Retorna resultado explicito (struct, bool, etc.) — sem side effects escondidos.

**Exemplos corretos:** `CombatFxController` (namespace), `EnemyDeathController` (namespace), `EnemySpawner` (namespace), `TownNpcInteractionController` (namespace).

**Nota:** o sufixo `Controller` em namespaces e legado e causa confusao. Novos modulos stateless devem usar sufixos como `System`, `Helpers`, `Utils` ou simplesmente o nome da operacao.

---

### 2.4 Service layer / facade de IO

**Quando usar:** encapsulamento de operacoes de IO (save, load, remoção de arquivo), sem estado proprio de gameplay.

**Contrato obrigatorio:**
- Namespace.
- Retorna `bool` de sucesso/falha em operacoes de escrita/remocao.
- No-op legitimo retorna `true` (nao e erro; nao havia trabalho a fazer).
- **Nao contem logica de gameplay** — apenas serializa/deserializa e faz IO.
- Unico ponto de acesso ao backend de persistencia (`SaveSystem`).

**Exemplos corretos:** `WorldSaveController` (namespace de servico), `SaveSystem`.

**Sinal de violacao:** service layer que calcula XP, altera atributos ou decide transicao de cena.

---

### 2.5 Renderer / UI helper

**Quando usar:** desenho e layout de uma tela ou componente visual.

**Contrato obrigatorio:**
- Funcao livre ou namespace.
- Recebe estado ja preparado — **nao calcula gameplay**.
- Recebe `LocaleSystem*` para textos — **sem strings hardcoded** de UI.
- Usa `ui::g_theme` para cores — sem literais RGB espalhados.
- **Nao altera** quest, save, atributos ou transicoes.

**Exemplos corretos:** `render_dungeon_hud`, `render_skill_tree_overlay`, `TownWorldRenderer::render_town_world`, `DungeonWorldRenderer::render`, `render_dialogue_ui`.

**Sinal de violacao:** renderer que chama `quest_state.set(...)` ou `SaveSystem::save_default(...)`.

---

### 2.6 Loader / bootstrap de dados

**Quando usar:** leitura de arquivos INI e aplicacao de overrides sobre globais de configuracao.

**Contrato obrigatorio:**
- Namespace com funcao `load_*` ou `apply_*`.
- Sempre faz reset dos defaults antes de aplicar INI (baseline determinístico).
- Usa `load_data_ini(kDataFileNames::k...)` — sem `ini_load` + `resolve_data_path` inline.
- **Nao guarda estado** — aplica e termina.

**Exemplos corretos:** `DungeonConfigLoader`, `TownConfigLoader`, `CommonPlayerProgressionLoader`.

**Sinal de violacao:** loader que chama `SDL_Renderer` ou guarda ponteiro para a cena.

---

### 2.7 State / data struct

**Quando usar:** armazenamento de dados com ciclo de vida claro, sem side effects.

**Contrato obrigatorio:**
- Struct simples, sem metodos com IO ou audio.
- Pode ter metodos de conveniencia puros (`is_alive`, `add_xp`, `can_receive_hit`).
- **Nao chama** subsistemas externos.

**Exemplos corretos:** `Actor`, `SaveData`, `HealthState`, `ManaState`, `StaminaState`, `TalentState`, `QuestState`, `BossState`.

---

## 3. Fluxo de dados canonico

```
InputState (lido uma vez por frame em main)
    |
    v
Scene::fixed_update(dt, input)
    |
    +-- OverlayInputTracker::capture  (edges de UI)
    |
    +-- Controllers (UI stateful)
    |       update(player, overlay_input, dt) -> Result
    |       se Result.should_save -> _persist_save()
    |       se Result.world_paused -> return
    |
    +-- Systems (gameplay)
    |       fixed_update(actors, dt, ...)
    |       last_events[] -> consumido pelo chamador no mesmo frame
    |
    +-- Namespace helpers (orquestracao pontual)
    |       process_deaths(...) -> DeathResult
    |       apply_combat_feedback(...) -> void
    |
    +-- WorldSaveController::persist (service layer)
            chamado apenas pelo owner (Scene ou controller via Result)
```

### 3.1 Quem pode chamar quem

| Chamador | Pode chamar |
|----------|-------------|
| Scene | tudo |
| Controller | System, Renderer, service layer via Result |
| System | components, entities, core |
| Renderer | core (draw), components (leitura) |
| Service layer | SaveSystem, debug_log |
| Loader | ini_loader, engine_paths, globals de config |

---

## 4. Modelo de persistencia

Uma unica regra: **quem persiste e o owner superior, nunca o controller ou o renderer.**

```
Controller.update() -> Result { should_save = true }
    |
    v
Scene._persist_save()
    |
    v
WorldSaveController::persist(ctx, show_indicator, flash_timer) -> bool
    |
    v
SaveSystem::save_default(data) -> bool
```

### 4.1 No-op vs erro

- No-op legitimo (stress mode, sem trabalho): retorna `true`.
- Falha real de IO: retorna `false` + `debug_log`.
- Nunca retornar `false` quando nao havia trabalho a fazer.

### 4.2 Apply vs persist

- `apply_world_save` restaura estado em memoria — nao faz IO.
- `persist` serializa estado atual para disco — nao altera estado em memoria.
- Os dois nunca devem ser chamados no mesmo fluxo sem motivo explicito.

---

## 5. Modelo de input

Tres camadas de input coexistem; nunca misturar:

| Camada | Onde | Para que |
|--------|------|----------|
| `InputState` raw | `main` → `fixed_update` | Valores absolutos de frame (move_x, attack_pressed, etc.) |
| `OverlayInputEdges` | `OverlayInputTracker::capture` | Edges de overlay (pause, tab, inventario, navegacao, confirm/back) para menu de pausa e demais UIs modais |
| `_prev_*` de gameplay | `WorldScene::_sync_gameplay_input_history` | Edges de acoes de gameplay (pocao, confirmar, talentos) |

**Regra:** `_prev_*` de gameplay devem ser sincronizados em **todos** os caminhos de `fixed_update`, inclusive early returns. Nunca criar um quarto mecanismo de edge detection sem primeiro verificar se os tres acima cobrem o caso.

---

## 6. Modelo de coordenadas (open world)

Duas origens coexistem no render da dungeon. Confundir as duas causa bugs visuais silenciosos.

| Tipo | Descricao | Usado por |
|------|-----------|-----------|
| **Global** (mundo) | Origem `(0, 0)` absoluta do `WorldMap` | Posicoes de `Actor`, `Projectile`, `GroundItem`, broadphase de colisao |
| **Local** (area) | Origem relativa ao `offset_x/offset_y` da `WorldArea` | `RoomDefinition`, `Tilemap`, `Pathfinder`, `NavGrid` |

**Conversao:**
- Mundo → local: `local_x = world_x - area.offset_x`
- Local → mundo: `world_x = local_x + area.offset_x`
- IA recebe `nav_ox/nav_oy` e converte para `find_path`; waypoints voltam a mundo ao steer.

**Render:**
- `DungeonWorldRenderInputs::camera` (local, com `offset_cam`): tiles, obstaculos, paredes.
- `DungeonWorldRenderInputs::world_camera`: inimigos, projeteis, drops, particulas.
- Player desenhado por ultimo com `_camera` global.

---

## 7. Anti-padroes conhecidos

Estes padroes existem no codigo e estao documentados como divida tecnica. Nao copiar; nao reforcar.

### 7.1 Identidade por string em logica de dominio

**Problema:** `ev.target_name == _player.name` em `CombatFxController`, nomes de cena como `"world"/"title"/"victory"` em literais, boss por nome string.

**Consequencia:** typos quebram em runtime; multiplas entidades com mesmo nome visivel conflitam.

**Regra:** strings ficam na camada de conteudo (dialogo, display name, serializacao). Identidade estrutural usa enum, id estavl ou handle.

---

### 7.2 Globais mutaveis de configuracao

**Problema:** `ui::g_theme`, `g_player_config`, `g_progression_config`, `g_talent_nodes`, `g_spell_defs` sao `inline` mutaveis globais.

**Consequencia:** ordem de carregamento importa; testes paralelos ficam frageis; dois mundos simultaneos no mesmo processo sao inviaveis.

**Regra:** nenhuma funcionalidade nova deve depender desses globais sem justificativa. Em medio prazo, migrar para contexto explicito injetado.

---

### 7.3 Side effects escondidos em helpers

**Problema:** controller ou helper que chama `SaveSystem`, `AudioSystem::play_sfx` ou `quest_state.set` sem deixar claro na assinatura.

**Regra:** se o modulo faz IO, toca audio, muda quest ou dispara transicao, isso deve estar claro no nome, na assinatura ou no `Result`. Ver secao 4.3 do `refactor_guidelines.md`.

---

### 7.4 Pipeline de apply_save fragmentado

**Problema:** `apply_world_save` restaura parte do estado, `configure_player` recomputa stats, e `WorldScene::enter` clampeia HP/mana/stamina manualmente depois. A ordem dessas tres chamadas e contrato implicito nao documentado.

**Regra:** ao tocar `enter()`, documentar e manter a ordem. Nao adicionar mais uma etapa manual sem encapsular no pipeline.

---

### 7.5 Actor monolitico

**Problema:** `Actor` agrega dados de player, inimigo, boss, pathing, inventario, talentos, pocoes e derived stats. Cópias sao custosas; invariantes sao dificeis de manter.

**Regra:** nao adicionar novos campos em `Actor` sem justificativa forte. Novos tipos de entidade devem ser avaliados como struct proprio.

---

### 7.6 Namespace `Controller` ambiguo

**Problema:** `Controller` hoje nomeia tanto classes stateful de UI (`SkillTreeController`) quanto namespaces stateless (`CombatFxController`, `WorldSaveController`).

**Regra para modulos novos:**
- Classe stateful com ciclo de vida → sufixo `Controller`.
- Namespace stateless de orquestracao → sem sufixo `Controller`; usar nome descritivo da operacao ou sufixo `System`/`Service`/`Helpers`.

---

## 8. Contrato de overlay / UI modal

Todo overlay modal (tela que pausa o mundo) deve seguir este contrato:

```
struct ResultXxx {
    bool world_paused  = false;  // mundo deve pausar este tick
    bool should_save   = false;  // cena deve chamar _persist_save()
    bool just_opened   = false;  // feedback visual/audio de abertura
};

class XxxController {
public:
    void set_locale(LocaleSystem*);   // injetado antes de enter/init
    void open();
    void close();
    bool is_open() const;
    ResultXxx update(Actor& player, const OverlayInputEdges&, ...);
    void render(const Actor& player, SDL_Renderer*, int vw, int vh);
};
```

- `update` nunca chama `SaveSystem` diretamente.
- `render` nunca altera estado de gameplay.
- `LocaleSystem*` injetado por setter antes do primeiro uso.
- Cores via `ui::g_theme`.
- Textos via `locale->get("chave")` — sem literais hardcoded.

---

## 9. Contrato de loader de dados

Todo loader de INI deve seguir:

1. Reset dos defaults canonicos (`kDefaultXxx`) antes de qualquer leitura.
2. `IniData ini = load_data_ini(kDataFileNames::kXxx)`.
3. Aplicacao de overrides via helper centralizado (`apply_xxx_ini_overrides(ini)`).
4. Nenhuma chamada a `SDL`, `AudioSystem` ou `SaveSystem`.

Invariante: dois loads consecutivos sem alteracao de arquivo devem produzir o mesmo estado.

---

## 10. Relacao com outros documentos

- `refactor_guidelines.md` — regras operacionais de processo (como mexer, quando parar, anti-duplicacao).
- `bug_fix_refactor.md` — inventario de hotspots e worklog de tarefas executadas.
- `map_controllers_revisar.md` — mapa de controllers e fluxo da `WorldScene` (referencia de runtime).
