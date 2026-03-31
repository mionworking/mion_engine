---
name: "Plano atualizado: Passo 0 + 5 passos (mana/magia/skill tree)"
overview: Adicionar uma pré-refatoração (Passo 0) para preparar contratos de dados, pipeline e testabilidade antes de implementar mana, magias e skill tree, reduzindo regressões.
todos:
  - id: step0-contracts
    content: Criar spell_defs/talent_tree e componentes mana/spell_book/talent_state
    status: completed
  - id: step0-pipeline-input
    content: Preparar pipeline da cena e input para spells/talentos sem ativar features
    status: completed
  - id: step0-tests
    content: Adicionar testes base de contratos (mana/spellbook/talentos)
    status: completed
  - id: step1-mana
    content: Implementar mana completa e HUD
    status: completed
  - id: step2-bolt
    content: Implementar Bolt com custo/cooldown/mana
    status: completed
  - id: step3-nova
    content: Implementar Nova (AoE) e testes de raio
    status: completed
  - id: step4-talent-tree
    content: Implementar talent tree com pontos e prerequisitos
    status: completed
  - id: step5-hardening
    content: Balance + regressão automática build/ctest
    status: completed
isProject: false
---

# Plano atualizado: Passo 0 + 5 passos (mana/magia/skill tree)

## Passo 0 — Pré-refatoração (obrigatório)

Objetivo: criar os contratos e pontos de extensão para evitar espalhar lógica nova em `DungeonScene`.

### 0.1 Contratos de gameplay

- Criar módulos de dados:
  - `[/home/danten/Documents/G_v2/mion_engine_cpp/src/gameplay/spell_defs.hpp](/home/danten/Documents/G_v2/mion_engine_cpp/src/gameplay/spell_defs.hpp)`
  - `[/home/danten/Documents/G_v2/mion_engine_cpp/src/gameplay/talent_tree.hpp](/home/danten/Documents/G_v2/mion_engine_cpp/src/gameplay/talent_tree.hpp)`
- Inserir enums e structs estáticas (`SpellId`, `SpellDef`, `TalentId`, `TalentNode`) com lookup O(1).

### 0.2 Componentização do player

- Criar componentes dedicados:
  - `[/home/danten/Documents/G_v2/mion_engine_cpp/src/components/mana.hpp](/home/danten/Documents/G_v2/mion_engine_cpp/src/components/mana.hpp)`
  - `[/home/danten/Documents/G_v2/mion_engine_cpp/src/components/spell_book.hpp](/home/danten/Documents/G_v2/mion_engine_cpp/src/components/spell_book.hpp)`
  - `[/home/danten/Documents/G_v2/mion_engine_cpp/src/components/talent_state.hpp](/home/danten/Documents/G_v2/mion_engine_cpp/src/components/talent_state.hpp)`
- Agregar em `[/home/danten/Documents/G_v2/mion_engine_cpp/src/entities/actor.hpp](/home/danten/Documents/G_v2/mion_engine_cpp/src/entities/actor.hpp)`.

### 0.3 Pipeline e UI gates (sem features ainda)

- Em `[/home/danten/Documents/G_v2/mion_engine_cpp/src/scenes/dungeon_scene.hpp](/home/danten/Documents/G_v2/mion_engine_cpp/src/scenes/dungeon_scene.hpp)`:
  - reservar pontos claros para `spell intents` e `talent selection`;
  - manter prioridade de estado (ex.: level/talent prompt pausa ações ativas);
  - preparar `_render_hud()` para barra de mana e texto de pontos de talento.

### 0.4 Input preparado

- Em `[/home/danten/Documents/G_v2/mion_engine_cpp/src/core/input.hpp](/home/danten/Documents/G_v2/mion_engine_cpp/src/core/input.hpp)` e `[/home/danten/Documents/G_v2/mion_engine_cpp/src/core/input.cpp](/home/danten/Documents/G_v2/mion_engine_cpp/src/core/input.cpp)`:
  - reservar `spell_1_pressed`, `spell_2_pressed`, `talent_1/2/3_pressed` com edge-detection na cena.

### 0.5 Test harness primeiro

- Expandir `[/home/danten/Documents/G_v2/mion_engine_cpp/tests/test_main.cpp](/home/danten/Documents/G_v2/mion_engine_cpp/tests/test_main.cpp)` com testes de contratos (sem SDL):
  - `ManaState` e `SpellBookState` básicos;
  - validação de prerequisitos de talentos.
- Critério de aceite Passo 0: `build + ctest` verde sem mudar gameplay atual.

---

## Passo 1 — Mana

- Implementar `ManaState` completo (consume/tick/regen delay/clamp).
- Integrar em `Actor`, `ResourceSystem` e `_render_hud()`.
- Testes: consumo, delay, regen até max.

## Passo 2 — Magias (MVP)

- Implementar cast de `Bolt` (projétil) usando `SpellBookState + SpellDef + mana cost + cooldown`.
- Integrar no `PlayerActionSystem` e pool de projéteis existente.
- Testes: gate por mana e cooldown.

## Passo 3 — Segunda magia (AoE)

- Implementar `Nova` com função pura de aplicação em raio.
- Integrar no pipeline sem quebrar melee/ranged.
- Testes: alvo dentro/fora do raio.

## Passo 4 — Skill tree

- Implementar `TalentState` com pontos, prerequisitos e unlock único.
- Relacionar progressão de level com ganho de ponto.
- UI mínima de seleção (teclas) com edge-detection.
- Testes: parent required, point spend, não desbloquear duas vezes.

## Passo 5 — Integração e hardening

- Balance inicial (custos, cooldowns, regen).
- Regressão de combate existente (dash/parry/status/combo/ranged).
- Rodar sempre: `cmake --build build` + `ctest --output-on-failure`.

## Critérios de segurança/otimização

- Manter `Projectile` fora de `_actors`.
- Usar arrays fixos por enum para cooldown/unlock (evita alocação e simplifica testes).
- Lógica de regra em módulos testáveis; `DungeonScene` apenas orquestra.

