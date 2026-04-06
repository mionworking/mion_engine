---
name: Mana Magia Skill Tree
overview: "Plano histórico: mana, spells ativas e skill tree. Estado 2026-04-04: entregue no código (ResourceSystem, spell_book, talent_data/loader, skill tree UI, testes em tests/)."
todos:
  - id: mana-component
    content: Criar ManaState, integrar Actor + ResourceSystem + HUD + reset de run
    status: completed
  - id: mana-tests
    content: Adicionar testes de mana (consume, delay, regen, clamp) — coberto em suíte V2 / legado
    status: completed
  - id: spell-defs-book
    content: SpellId, SpellDef, SpellBook + cooldown/mana; wired em PlayerAction + input
    status: completed
  - id: spell-bolt-nova
    content: Implementar Bolt (projétil) e Nova (AoE) + chamadas no pipeline
    status: completed
  - id: spell-tests
    content: Testes de cooldown, gate de mana, Nova raio/hit
    status: completed
  - id: talent-data-state
    content: TalentNode table + TalentState + pontos por level em Progression/Actor
    status: completed
  - id: talent-ui
    content: Overlay com edge detection e aplicação de passivas/unlock de spell
    status: completed
  - id: talent-tests
    content: "Testes prereq, spend point, efeito passivo (ex: +max mana)"
    status: completed
  - id: regression-ci
    content: "Após cada fase: build + ctest; smoke manual de combate existente"
    status: completed
isProject: false
---

> **Estado 2026-04-04:** todos os itens acima estão **completed** no repositório actual. Este ficheiro permanece como arquivo do plano original.

