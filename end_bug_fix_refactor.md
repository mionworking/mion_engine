# Encerramento do ciclo bug_fix_refactor

Documento de encerramento. Registra as decisões tomadas sobre os 5 itens pendentes identificados no final do worklog de `bug_fix_refactor.md`, antes de avançar para o ciclo de conteúdo/narrativa.

---

## Itens com decisão tomada

### Item 1 — `SceneId` → `enum class` ✅ FAZER

**Decisão:** Opção A — converter de `namespace SceneId { const char* }` para `enum class SceneId`.

**Por quê:** o jogo vai crescer em cenas (cutscenes, hub, mapa). Com `enum class`, é impossível referenciar uma cena que não existe — o compilador avisa. Com `const char*`, alguém pode escrever uma string literal errada e o erro aparece só em runtime.

**O que precisa mudar:**
- `SceneRegistry::register_scene` e `::create` passam a aceitar `SceneId`
- `IScene::next_scene()` retorna `SceneId` em vez de `const char*`
- A borda de string (serialização, registry interno) fica encapsulada dentro do `SceneRegistry`
- `DoorZone::exit_to_scene` (já separado no EZ) precisa de conversão na borda

**Quando fazer:** antes de adicionar qualquer cena nova de narrativa.

---

### Item 2 — `ItemId` como identificador tipado ✅ FAZER

**Decisão:** Opção A — converter `std::string name` em `ItemBag`/`DropSystem`/`GroundItem` para `enum class ItemId`.

**Por quê:** quando o primeiro item de lore/quest aparecer (artefato, item que desbloqueia área, item de narrativa), a lógica vai usar `if (item.name == "ancient_seal")` — o mesmo padrão stringly-typed que foi resolvido para boss, diálogos e cenas. Fazer antes de ter saves no campo evita o risco de quebrar compatibilidade de save depois.

**O que precisa mudar:**
- `enum class ItemId` com `to_string(ItemId)` para a borda de serialização
- `SaveData` continua serializando strings (`to_string(ItemId::AncientSeal)`) — compatibilidade mantida
- `ItemBag`, `GroundItem`, `DropSystem`, `ShopSystem` migram para o enum

**Quando fazer:** antes de criar o primeiro item com comportamento específico de narrativa.

---

## Itens pendentes de decisão

### Item 3 — Globais mutáveis (`g_player_config`, `g_spell_defs`, etc.) — DECIDIR

**Contexto técnico (para referência):**

Hoje existe isso em `player_config.hpp`:
```cpp
inline PlayerConfig g_player_config = kDefaultPlayerConfig;
```

Qualquer arquivo que incluir esse header pode **ler e escrever** essa variável. O compilador não reclama. A variável deve ser configurada uma vez no boot e nunca mais alterada, mas nada garante isso.

**Opção B — wrapper `const` (recomendada):**
```cpp
namespace detail {
    inline PlayerConfig _g_player_config_mutable = kDefaultPlayerConfig;
}
// Todo o código do jogo lê por aqui — const, não dá pra escrever:
inline const PlayerConfig& g_player_config = detail::_g_player_config_mutable;

// Só o bootstrap pode inicializar:
inline void init_player_config(const IniData& d) {
    detail::_g_player_config_mutable = make_player_config_from_ini(d);
}
```

- **Todo o código existente continua igual** — leitura de `g_player_config.base_hp` funciona sem mudança
- Qualquer tentativa de escrever `g_player_config.base_hp = X` vira **erro de compilação**
- Trabalho: ~15 min por global (4 globais afetados)
- Risco de regressão: zero

**Opção C — fechar como está:**
- Correto em comportamento hoje
- Risco: feature nova que tente "mudar config por dificuldade em runtime" vai escrever no global sem aviso

**Para decidir:** você quer que o compilador impeça mutações acidentais (Opção B, 15 min), ou aceita que isso seja responsabilidade de revisão de código (Opção C)?

---

### Item 4 — `Actor` monolítico — DECIDIR (quando, não se)

**Contexto:**

`Actor` é o tipo central do jogo. Hoje ele agrega tudo: dados do player (talentos, spell book, inventário), dados de inimigo (IA, spawn config) e dados compartilhados (posição, vida, animação). Todo inimigo aloca memória para campos de talentos e inventário que nunca vai usar. O player carrega campos de IA que nunca usa.

**O problema na prática:** quando você criar NPCs com diálogos próprios, companheiros que seguem o player, ou chefes com fases distintas — todos vão ser `Actor` por default. O struct vai crescer com campos que só fazem sentido pra um subconjunto dos personagens.

**Solução correta:** separar em `PlayerActor`, `EnemyActor`, `NpcActor` com base comum. Isso toca **todos** os sistemas (combate, IA, render, áudio, save, colisão). É o maior refactor possível na engine.

**Não é para agora.** É um investimento de engine que faz sentido entre ciclos de conteúdo, com o jogo em estado estável.

**Decisão a tomar:** em que ponto do roadmap de conteúdo você quer pausar e fazer isso? Sugestão: antes de adicionar o terceiro tipo de personagem distinto com dados próprios (hoje são player + inimigo + NPC simples).

---

### Item 5 — `EnemyDeathController` sem event bus — DECIDIR (quando, não se)

**Contexto:**

Hoje, quando um inimigo morre, `EnemyDeathController::process_deaths()` orquestra tudo diretamente: drops, XP, quest, partículas, áudio, run stats, lógica de boss. Funciona porque os "reações à morte" são fixas e conhecidas.

**O problema na narrativa:** quando você adicionar `"boss X morre → área Y desbloqueada → cutscene Z"`, você vai precisar tocar esse módulo que já tem 6 responsabilidades. Cada gatilho narrativo novo aumenta o acoplamento.

**Solução correta:** event bus — `process_deaths()` emite um `DeathEvent` e cada sistema se registra para reagir ao que lhe interessa. `NarrativeSystem` escuta morte de boss. `QuestSystem` escuta morte de miniboss. Adicionar novo gatilho = novo assinante, sem tocar o controller.

**Não é para agora.** A infraestrutura de event bus não existe. Criar agora sem conteúdo real para validar o design é especulativo.

**O estado atual é gerenciável:** `EnemyDeathController` já retorna `DeathResult` com flags explícitas (`boss_defeated`, `quest_completed`). A cena decide o que fazer. Não é o padrão ideal mas é controlado e extensível de forma incremental.

**Decisão a tomar:** quando o primeiro gatilho narrativo por morte de inimigo aparecer e o controller ficar difícil de estender — esse é o momento de construir o event bus. Não antes.

---

## Estado do projeto ao encerrar este documento

- Suíte de testes: **867 passed, 0 failed** (último estado registrado no worklog)
- Build: OK em todos os targets
- Todos os bugs críticos e refactors estruturais do inventário original foram resolvidos (Tarefas A → FF-2)
- O que permanece são investimentos de arquitetura com pré-requisito de conteúdo definido
