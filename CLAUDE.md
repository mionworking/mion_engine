# mion_engine — contrato de trabalho

Este arquivo é a fonte única de verdade para IA neste projeto. `README.md`
é para humanos. Se este arquivo e outro discordarem, **este vence** — ou
me avise para atualizar.

## O que é

Motor 2D C++17 sobre SDL3 para um **ARPG narrativo** estilo Diablo/POE +
Witcher/Disco Elysium. Código e comentários em português-BR.

Estado atual: open world unificado (town + dungeon) funcional.
Foco de desenvolvimento: **qualidade de conteúdo e sistemas do ARPG**.

## Filosofia

- Qualidade sobre quantidade: feature bem feita > três feitas pela metade.
- Sem puxadinhos: obstáculo técnico → parar e discutir, não contornar.
- Testes são contrato: código novo traz teste; alteração roda testes antes e depois.
- Código > docs: docs envelhecem; código é executável.
- Uma fonte de verdade por tópico.

## Comandos

| Ação                | Comando                                           |
|---------------------|---------------------------------------------------|
| Build debug + run   | `make run`                                        |
| Só build            | `make build`                                      |
| Release             | `make release` → `./build_release/mion_engine`    |
| Rodar testes        | `make test`                                       |
| Limpar build        | `make clean`                                      |
| Stress headless     | `make stress`                                     |

Gates de teste opcionais (env vars): ver `README.md` §Testes.

## Estrutura (índice — o código é a verdade)

```
src/
├── core/         SDL, scene, save, input, audio, UI, diálogo, locale, config
├── components/   Dados puros: health, combat, mana, stamina, equipment, bag, talents
├── entities/     Actor, Projectile, GroundItem, NPC, Shop, EnemyType
├── systems/      Lógica por frame: AI, combate, render, overlays, controllers
├── world/        Open world: WorldArea, WorldMap, ZoneManager, Tilemap
├── scenes/       title, world, game_over, victory, credits
└── main.cpp      Bootstrap + loop principal

data/             INI (conteúdo de jogo, não código): enemies, spells, items, ...
tests/            4 targets ctest (v2: main/core/scenes/input)
assets/           PNG + WAV (placeholders procedurais e finais)
tools/            Stress, bench, asset pipeline, geradores
```

Para encontrar algo, use Glob/Grep. Nomes de arquivo são descritivos.

**`legacy/` é arquivo morto** (test_legacy.cpp deletado na Sprint 3). Nunca leia de lá sem pedido explícito.

## Regras não-negociáveis

1. Nunca delete/comente teste para fazê-lo passar.
2. Nunca escreva `// TODO: fix later`, `// HACK:`, `#if 0`. Se aparecer, é puxadinho.
3. Nunca silencie warning com cast — resolva a causa.
4. Nunca mute `g_player_config`, `g_spell_defs`, `g_talent_nodes` fora de init.
5. Alteração de código com doc correspondente → atualize o doc no mesmo commit.
6. Nunca crie novo `.md` sem pedido explícito.
7. Nunca crie abstração especulativa. Resolva o caso de hoje.
8. Antes de criar controller/sistema/lógica, **BUSQUE** um específico existente
   (Grep/Glob por nome parecido). Redundância é puxadinho disfarçado de novidade.
9. Em dúvida, **PERGUNTE**. Não assuma. Não invente. A pergunta custa 1 linha;
   o puxadinho custa uma sessão inteira pra desfazer.
10. **Ensine, não só execute.** Quando houver trade-off (design, libs, arquitetura),
    mostre prós/contras e espere a decisão do usuário.

## Convenções de código

- C++17, sem `using namespace std`, sem exceções no hot path.
- Header-heavy: maior parte é `.hpp` com `inline`; `.cpp` só para unidades pesadas.
- Namespace único: `mion`. Tudo novo vai nele.
- Plain structs + free functions, estilo data-oriented. Sem herança (exceto `IScene`).
- Comentários em português.
- Paths resolvidos via `resolve_data_path()` / `SDL_GetBasePath()`.

## Processo de feature / sprint

Toda sprint segue estes 5 passos, sem pular:

1. **Brief curto** (usuário, no chat) — o que, por quê, como integra.
2. **Plano antes de código** (IA):
   - Arquivos **LIDOS** (contexto necessário para entender)
   - Arquivos **ESCRITOS** (alterados ou criados)
   - Riscos e trade-offs (prós/contras quando houver escolha real)
   - Testes novos ou afetados
   - Critério de DONE objetivo
3. **Aprovação** (usuário) — ok / ajuste / redireciona.
4. **Implementação** — testes junto quando aplicável.
5. **Diff review** (usuário) — aprova ou pede refazer.

## Onde mexer em...

| Assunto                         | Olhe primeiro em                                      |
|---------------------------------|-------------------------------------------------------|
| Comportamento do player         | `systems/player_action.hpp`                           |
| IA de inimigos                  | `systems/enemy_ai.hpp` + `ai_combat/ai_patrol/ai_boss`|
| Dano / combate                  | `systems/melee_combat.hpp`, `systems/spell_system.hpp`|
| Fluxo do mundo                  | `scenes/world_scene.hpp`                              |
| Save / load                     | `core/save_data.hpp`, `save_migration.hpp`, `save_system.hpp` |
| UI / overlays                   | `core/ui.hpp` + `systems/*_controller.hpp`            |
| Progressão                      | `components/progression.hpp` + atributos controller   |
| Conteúdo de jogo (stats, drops) | `data/*.ini` — sem recompilar                         |

---

# Sprints

Disciplina: **apenas sprint atual + próximas 2-3**. Sprint concluída sai
daqui (git log é a memória). Sprints adiadas ficam como parking lot
explícito — não executar antes do gatilho.

---

## Sprint atual — Sprint 5: Actor split (`PlayerData` / `EnemyAIData`)

> **Modelo:** Opus 4.6 — maior refactor da engine, decisões arquiteturais reais (quais campos vão onde, impacto em save/combate/IA/render). Use Opus para o plano; Sonnet pode executar passos mecânicos aprovados.

**Objetivo:** Quebrar `Actor` monolítico em base pequena (campos comuns a
todo personagem) + sub-structs opcionais para responsabilidades específicas.

**Por quê:** `Actor` hoje carrega campos de player, inimigo, boss, patrulha,
equipamento, talentos — tudo junto. Cada inimigo aloca memória para coisas
que nunca usa; player carrega campos de IA que nunca usa. Vai destravar
NPCs com diálogo, companheiros, bosses com fases, sem inchar mais o struct.

**Escopo inicial (detalhar no plano):**
- Extrair `PlayerData` (spell_book, talents, equipment, bag, potion, progression, stamina, mana).
- Extrair `EnemyAIData` (patrol_waypoints, boss_phase, ranged_*, aggro_range).
- `Actor` base fica com: transform, collision, health, combat state, animação, team, movimento.
- Abordagem incremental: `std::optional<PlayerData>` / `std::optional<EnemyAIData>` dentro de `Actor` — uma extração por vez, cada commit compila e passa testes.
- Save v7 → v8: migração cuida da nova estrutura.

**Risco:** alto. Maior refactor possível na engine. Toca combate, IA, render,
save, colisão. Por isso requer Opus para o plano.
**Estimativa:** 2-3 sessões concentradas.

---

## Adiada — Sprint A: `ItemId` → `enum class` (parking lot)

> **Gatilho:** primeiro item de equipamento real sendo criado (drop de inimigo OU item de lore/quest como "AncientSeal"). Sem item real, criar o enum é abstração especulativa.

**Quando executar:** junto com (ou imediatamente antes de) a sprint que cria os primeiros itens concretos.

**O que será:**
- `enum class ItemId` + `to_string(ItemId)` / `from_string(std::string_view)` na borda de save.
  - "Borda de save": save file continua com strings (`bag_0=iron_sword`); memória usa `ItemId::IronSword`. A conversão acontece só no load/save, não no hot path.
- Migrar `BagSlot.name`, `GroundItem.item_name`, `ItemDef.name` para `ItemId`.
- `SaveData.bag_names[]` e `equipped_names[]` continuam `std::string` (compatibilidade de arquivo).
- Modelo: Sonnet 4.6 — mecânico após o gatilho.

---

## Adiada — Sprint B: Event bus (parking lot)

> **Modelo (quando executar):** Opus 4.6 — infraestrutura nova do zero, design de API de eventos, sem caso real ainda para validar.

**Não executar antes do gatilho real aparecer.**

**Quando executar:** quando surgir o primeiro caso concreto de reação em
cadeia a morte de inimigo (ex.: "boss X morre → área Y desbloqueada →
cutscene Z"). Sem caso real, construir event bus viola a regra #7
(abstração especulativa).

**O que será:** `DeathEvent` emitido por `EnemyDeathController`; sistemas
(`NarrativeSystem`, `QuestSystem`, etc.) se registram como subscribers.
Adicionar gatilho novo = novo subscriber, sem tocar o controller.

---

# Memória de longo prazo

Em `.claude/projects/-home-danten-Documents-G-v2-mion-engine-cpp/memory/`:
- `user_profile.md` — quem é o usuário e como colaborar.
- `feedback_ia_antipadroes.md` — anti-padrões a evitar (puxadinho, docs desatualizados, redundância).
- `project_overview.md` — snapshot histórico (ver data; **o código é autoridade**).

Memórias são observações no tempo. Quando divergem do código, **o código vence**
e a memória é atualizada.
