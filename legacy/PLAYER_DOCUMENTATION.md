# Documentacao do Player

Este documento centraliza tudo que envolve o player no projeto: estado, progresso, magias, talentos, recursos, economia ("inventario"), controles, persistencia e configuracao.

## Visao geral

O player e representado por `Actor` com componentes agregados:

- `src/entities/actor.hpp`
  - Recursos: `HealthState`, `StaminaState`, `ManaState`
  - Progressao: `ProgressionState`
  - Build: `TalentState` e `SpellBookState`
  - Economia: `gold`
  - Combate e mobilidade: `CombatState`, dash, ranged cooldown, buffs temporarios

O ciclo principal de gameplay acontece em:

- `src/scenes/dungeon_scene.hpp`
  - Input -> acao do player
  - Regeneracao de recursos
  - Combate/projeteis/status
  - Morte de inimigos -> XP + pontos de talento + drops
  - UI de level up e talento

Na cidade:

- `src/scenes/town_scene.hpp`
  - Player mantem progressao/save
  - Loja (compra com gold)
  - Interacao com NPCs/quest

---

## Onde fica cada sistema pedido

### 1) Spell Book (livro de magias)

- Estado e regras: `src/components/spell_book.hpp`
  - Cooldown por magia (`cooldown_remaining`)
  - `unlocked[]` por `SpellId`
  - `can_cast(...)`, `start_cooldown(...)`, `sync_from_talents(...)`

- Definicao das magias: `src/components/spell_defs.hpp`
  - `SpellId`: FrostBolt, Nova, ChainLightning, MultiShot, PoisonArrow, Strafe, Cleave, BattleCry
  - `SpellDef` com custo de mana, cooldown, dano, raio, escala por nivel

- Valores ajustaveis em dados: `data/spells.ini`

- Uso em runtime:
  - Casting/consumo/cooldown: `src/systems/player_action.hpp`
  - Aplicacao dos efeitos: `src/systems/spell_effects.hpp`
  - Hotbar e cooldown na HUD: `src/scenes/dungeon_scene.hpp`

### 2) "Inventario"

Hoje o projeto nao tem um inventario classico por slots/itens equipaveis no player.
O que existe como "inventario funcional" e:

- Moeda do jogador:
  - Campo: `Actor::gold` em `src/entities/actor.hpp`
  - Drop de gold: `src/systems/drop_system.hpp`
  - Exibicao HUD: `src/scenes/dungeon_scene.hpp` e `src/scenes/town_scene.hpp`

- Itens de chao (pickup imediato, sem slot):
  - Estrutura: `src/entities/ground_item.hpp`
  - Tipos: Health, Damage, Speed, Gold
  - Spawn + pickup: `src/systems/drop_system.hpp`

- Loja (economia):
  - Modelos: `src/entities/shop.hpp`
  - Compra/aplicacao de efeito: `src/systems/shop_system.hpp`
  - Fluxo de UI e interacao com NPC Forge: `src/scenes/town_scene.hpp`

Se quiser inventario real (bag/slots/equip), ainda e uma feature nova.

### 3) Pontos de distribuicao (talentos)

Sistema de pontos e arvore:

- Estado dos pontos e niveis:
  - `src/components/talent_state.hpp`
  - `pending_points`, `levels[]`, `can_spend(...)`, `try_spend(...)`

- Definicao da arvore:
  - `src/components/talent_tree.hpp`
  - `TalentId`, pre-requisitos, custo por nivel, nivel maximo, disciplina
  - Override de nomes/custos por INI

- Dados:
  - `data/talents.ini`

- Geracao de pontos:
  - Em `DungeonScene`, ao matar inimigo:
    - `gained = progression.add_xp(...)`
    - `player.talents.pending_points += gained`

- Consumo de pontos:
  - Auto-prompt: teclas 4/5/6 escolhem entre ate 3 opcoes spendable
  - Skill tree overlay (TAB): gastar com ENTER
  - Lado codigo: `src/scenes/dungeon_scene.hpp` (`_try_spend_talent`)

- Efeitos diretos de talentos especiais:
  - `ArcaneReservoir`: aumenta mana max/current
  - `ManaFlow`: aumenta mana regen
  - Sync de desbloqueio de magia apos gastar ponto (`sync_from_talents`)

### 4) Level up (XP e upgrades de nivel)

Progresso base:

- `src/components/progression.hpp`
  - `xp`, `level`, `xp_to_next`, `pending_level_ups`
  - Upgrades:
    - dano (`pick_upgrade_damage`)
    - hp (`pick_upgrade_hp`)
    - velocidade (`pick_upgrade_speed`)
  - Escala de dano de magia (`spell_damage_multiplier`)

Config de curva/bonus:

- `data/progression.ini`
  - `xp_base`, `xp_level_scale`
  - `damage_bonus`, `hp_bonus`, `speed_bonus`, `spell_mult_bonus`

Fluxo no dungeon:

- Morte de inimigo -> `add_xp(...)`
- Se subir nivel, `pending_level_ups > 0`
- Mundo pausa ate escolher upgrade:
  - Teclas 1/2/3 (dano/hp/velocidade)
  - Mensagem HUD: "LEVEL UP 1=DMG 2=HP 3=SPD"
- Implementacao: `src/scenes/dungeon_scene.hpp`

---

## Recursos do player

### Vida

- Componente: `src/components/health.hpp` (via `Actor`)
- Base configuravel: `data/player.ini` (`base_hp`)
- Bonus por level/drop/loja afeta max/current hp dependendo da origem

### Stamina

- Componente: `src/components/stamina.hpp`
  - `current`, `max`, `regen_rate`, `regen_delay`
- Custos principais em `src/systems/player_action.hpp`:
  - melee, dash, ranged
- Regen no tick:
  - `src/systems/resource_system.hpp`

### Mana

- Componente: `src/components/mana.hpp`
- Consumo em magias via `SpellBookState::can_cast` + `mana.consume`
- Regen no tick:
  - `src/systems/resource_system.hpp`
- Base em `data/player.ini`

---

## Entrada do jogador (teclas)

- Estrutura de input: `src/core/input.hpp`
- Leitura teclado/gamepad: `src/core/input.cpp`
- Mapeamento configuravel: `src/core/keybind_config.hpp`
- Overrides via `config.ini` (`[keybinds]`)

Defaults relevantes:

- Ataque: Z/SPACE
- Ranged: X
- Dash: LSHIFT
- Parry: C
- Magias: Q/E/R/F
- Level up: 1/2/3
- Talentos rapidos: 4/5/6
- Skill tree: TAB
- Pause: ESC

---

## Persistencia (save)

Arquivo e serializacao:

- `src/core/save_system.hpp`
- Estrutura `SaveData` inclui:
  - HP, gold
  - progressao (xp/level/bonus)
  - talentos (pontos + niveis por talento)
  - mana e stamina
  - quest state e estatisticas de run

Fluxo:

- Dungeon: autosave em transicoes/eventos relevantes
- Town: salva ao sair da cena e em eventos de quest/compra

Importante: `SpellBookState` nao e salvo diretamente; ele e reconstruido e sincronizado a partir dos talentos ao carregar.

---

## Cenas que controlam o player

### Dungeon

- Arquivo: `src/scenes/dungeon_scene.hpp`
- Controla combate completo, XP, level-up, talentos, drops e HUD completa.

### Town

- Arquivo: `src/scenes/town_scene.hpp`
- Controla locomocao segura, dialogo, loja, gold e quest reward.
- Mantem progressao/talentos/recursos a partir do save.

---

## Configuracoes de dados que impactam o player

- `data/player.ini`: stats base de combate/movimento/mana/stamina/dash
- `data/progression.ini`: curva de XP e bonus por level up
- `data/spells.ini`: custo/cooldown/dano das magias
- `data/talents.ini`: custos/nomes/max level da arvore
- `data/items.ini`: drop chance, pickup radius e bonus de drops

---

## Mapa rapido por tema

- Player agregado: `src/entities/actor.hpp`
- Acoes do player (ataques/dash/magias): `src/systems/player_action.hpp`
- Efeitos de magia: `src/systems/spell_effects.hpp`
- Recursos (regen): `src/systems/resource_system.hpp`
- Spellbook: `src/components/spell_book.hpp`
- Defs de magia: `src/components/spell_defs.hpp` + `data/spells.ini`
- Progressao/levelup: `src/components/progression.hpp` + `data/progression.ini`
- Talentos: `src/components/talent_state.hpp`, `src/components/talent_tree.hpp`, `data/talents.ini`
- Drop/economia basica: `src/systems/drop_system.hpp`, `src/entities/ground_item.hpp`
- Loja: `src/entities/shop.hpp`, `src/systems/shop_system.hpp`, `src/scenes/town_scene.hpp`
- Save: `src/core/save_system.hpp`
- Input/keybinds: `src/core/input.hpp`, `src/core/input.cpp`, `src/core/keybind_config.hpp`

---

## Prioridade de evolucao: atributos vs inventario vs game feel

Recomendacao para este projeto (escopo atual):

1. Primeiro: game feel do nucleo de combate
   - Melhorar feedback de impacto, telegraph de inimigos, clareza de leitura e resposta de controles.
   - Objetivo: o loop basico ficar divertido antes de adicionar muita complexidade sistemica.

2. Em paralelo: definir modelo minimo de atributos
   - Mesmo sem UI completa de inventario, os atributos devem existir cedo para evitar retrabalho.
   - Isso tira o level-up de hardcode e cria base consistente para talentos e equipamentos.

3. Depois: inventario/equipamento em fases
   - V1 simples: slots basicos (arma, armadura, acessorio), equipar/desequipar, bonus aplicados.
   - V2: raridade, comparacao de itens, filtros, tooltips e economia mais rica.

### Decisao de ordem (resumo)

- Nao priorizar inventario completo antes do combate estar com boa sensacao.
- Sim priorizar definicao de atributos agora (mesmo sem menu final), para sustentar progressao, talentos e equipamentos futuros.

### Atributos minimos sugeridos

- `Vigor`: vida maxima e resistencia geral.
- `Forca`: dano melee.
- `Destreza`: eficiencia ranged (dano/cadencia, conforme balanceamento).
- `Inteligencia`: dano magico e mana maxima.
- `Endurance` (ou `Vitalidade`): stamina maxima e regeneracao.

### Beneficios esperados

- Menos logica hardcoded em level-up.
- Melhor escalabilidade para novos sistemas (itens, classes, build variety).
- Integracao mais limpa entre progressao, talentos, magia e economia.

---

## Plano de evolucao tecnica (integracao total + testes)

### Objetivo

Evoluir atributos, progressao e inventario/equip sem quebrar o que ja existe (combate, talentos, magias, save, HUD e cenas).

### Ordem recomendada de implementacao

1. Base de atributos (sem UI completa ainda)
   - Criar estado base/final de atributos no player.
   - Remover hardcode gradual de bonus em progressao e acao.
2. Integracao com formulas de combate e recursos
   - Conectar atributos ao dano melee/ranged/spell, HP, mana, stamina e move speed.
3. Inventario/equipamento v1
   - Slots minimos (arma, armadura, acessorio) + aplicacao de modificadores no player.
4. UI e fluxo
   - Exibir atributos finais, bonus de equipamento e preview de level-up.
5. Persistencia completa
   - Salvar/carregar atributos base, distribuicao, itens/slots equipados.
6. Cobertura de testes de regressao
   - Garantir que progressao antiga, talentos e spellbook continuem consistentes.

### Onde mexer no codigo (mapa de integracao)

- Estado do player
  - `src/entities/actor.hpp`
  - Inserir `AttributesState` e (opcional) `EquipmentState`.
- Novos componentes
  - Novo arquivo sugerido: `src/components/attributes.hpp`
  - Novo arquivo sugerido: `src/components/equipment.hpp` (v1 simples)
  - Opcional para inventario: `src/components/inventory.hpp`
- Progressao e level-up
  - `src/components/progression.hpp`
  - `src/scenes/dungeon_scene.hpp` (fluxo de escolha e aplicacao de upgrades)
- Talentos e desbloqueios
  - `src/components/talent_state.hpp`
  - `src/components/talent_tree.hpp`
  - `src/components/spell_book.hpp` (sync com talentos e requisitos)
- Formulas de combate/acao
  - `src/systems/player_action.hpp`
  - `src/systems/spell_effects.hpp`
  - `src/systems/resource_system.hpp`
- Economia / itens / loja
  - `src/entities/ground_item.hpp`
  - `src/systems/drop_system.hpp`
  - `src/entities/shop.hpp`
  - `src/systems/shop_system.hpp`
  - `src/scenes/town_scene.hpp`
- HUD / overlays
  - `src/scenes/dungeon_scene.hpp`
  - `src/core/ui.hpp`
- Persistencia
  - `src/core/save_system.hpp` (versao de save e migracao)
- Config data-driven
  - `data/player.ini`
  - `data/progression.ini`
  - `data/talents.ini`
  - `data/spells.ini`
  - (novo sugerido) `data/attributes.ini`
  - (novo sugerido) `data/items.ini` expandido para equip/tiers

### O que inserir (escopo tecnico minimo)

- Atributos minimos
  - `vigor`, `forca`, `destreza`, `inteligencia`, `endurance`
- Pipeline de calculo
  - Base -> bonus de level-up -> bonus de talentos -> bonus de equipamento -> valor final
- Regras de aplicacao
  - Recalculo centralizado (ex.: funcao unica `recompute_player_derived_stats`)
  - Evitar espalhar formula em multiplos sistemas
- Inventario/equip v1
  - Slots fixos + lista simples de itens
  - Aplicacao de bonus por item equipado
  - Sem raridade complexa no inicio

### Boas praticas obrigatorias para manter integracao

- Single source of truth
  - Valores finais do player calculados em um ponto central.
- Data-driven first
  - Evitar numero magico em sistemas; preferir INI/config.
- Compatibilidade de save
  - Subir `kSaveFormatVersion` e criar migracao explicita.
- Feature flags simples
  - Ativar inventario/equip por etapa para facilitar rollout.
- Nao quebrar fluxo atual
  - Level-up, talentos e spell unlock continuam funcionando durante migracao.

### Plano de testes (obrigatorio antes de considerar pronto)

Base existente de testes:

- `tests/legacy/test_legacy.cpp` (suíte legada; mains em `tests/test_v2_*.cpp`)

Adicionar/expandir testes para:

1. Atributos e formulas
   - Escala de dano melee/ranged/spell por atributo.
   - Escala de HP/mana/stamina/move_speed por atributo.
2. Progressao
   - Level-up aplicando pontos em atributos (sem regressao dos upgrades atuais).
3. Talentos + spellbook
   - Unlock e cooldown continuam corretos apos mudancas de atributo.
4. Inventario/equip
   - Equipar/desequipar atualiza stats finais corretamente.
   - Compra/drop/loot integra com inventario sem perder gold/estado.
5. Save/load
   - Roundtrip de novos campos.
   - Migracao de saves antigos (v1/v2/v3 -> novo formato) preservando progresso.
6. Smoke de cena
   - Entrada/saida em `DungeonScene` e `TownScene` com novo estado ativo.

### Criterios de pronto (Definition of Done)

- Atributos ativos no gameplay sem hardcode espalhado.
- Inventario/equip v1 funcional e persistente.
- Save migration validada para arquivos antigos.
- Nenhuma regressao nos testes legados.
- HUD exibindo estado final do player de forma clara.

