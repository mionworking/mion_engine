---
name: Mana Magia Skill Tree
overview: Implementar mana, spells ativas e skill tree de forma incremental, espelhando o padrão já validado (Stamina + ResourceSystem + componentes em header), mantendo lógica testável sem SDL e expandindo `tests/test_main.cpp` para evitar regressões.
todos:
  - id: mana-component
    content: Criar ManaState, integrar Actor + ResourceSystem + HUD + reset de run
    status: pending
  - id: mana-tests
    content: Adicionar testes de mana (consume, delay, regen, clamp) em test_main.cpp
    status: pending
  - id: spell-defs-book
    content: SpellId, SpellDef, SpellBook + cooldown/mana; wired em PlayerAction + input
    status: pending
  - id: spell-bolt-nova
    content: Implementar Bolt (projétil) e Nova (AoE) + chamadas no pipeline
    status: pending
  - id: spell-tests
    content: Testes de cooldown, gate de mana, Nova raio/hit
    status: pending
  - id: talent-data-state
    content: TalentNode table + TalentState + pontos por level em Progression/Actor
    status: pending
  - id: talent-ui
    content: Overlay com edge detection e aplicação de passivas/unlock de spell
    status: pending
  - id: talent-tests
    content: "Testes prereq, spend point, efeito passivo (ex: +max mana)"
    status: pending
  - id: regression-ci
    content: "Após cada fase: build + ctest; smoke manual de combate existente"
    status: pending
isProject: false
---

