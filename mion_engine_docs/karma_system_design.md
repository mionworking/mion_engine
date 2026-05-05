# Design Document — Sistema de Karma

Documento de design consolidado. Registra decisões tomadas em conversa.
Código é a autoridade quando implementado; até lá, este documento é a referência.

---

## Premissa narrativa

O mundo foi corrompido por **karma** — energia negativa acumulada pela pressão
social, pelas necessidades impostas pelo capital ao trabalhador, pela máquina
que ele alimenta mas não usufrui. Essa energia foi corroendo as pessoas
devagar até dar chabu: o karma se materializou, corrompeu corpos e mentes,
e o mundo colapsou.

O jogador absorve karma dos inimigos corrompidos que derrota. Usa essa mesma
energia pra se fortalecer — armas, armadura, habilidades. A ambiguidade
moral é intencional: o herói está usando exatamente o que destruiu o mundo.
Isso abre espaço para consequências narrativas futuras (reações de NPCs,
diálogos, mudanças visuais no mundo conforme karma total do jogador).

---

## Economia de karma

### Moeda única

Karma é o único recurso de progressão do jogo. Não existe XP, ouro ou
material de craft separado. Tudo é karma.

### Dois contadores

| Contador           | Descrição                                              | Desce? |
|--------------------|--------------------------------------------------------|--------|
| **Karma total**    | Tudo que o jogador já absorveu na vida. Funciona como nível. | Nunca  |
| **Karma disponível** | Saldo atual pra gastar. É o karma total menos o já investido. | Sim    |

Karma total determina progressão geral (gates de conteúdo, tiers de traje
acessíveis, escalonamento do mundo). Karma disponível é a moeda de decisão:
onde investir.

### Fonte de karma

Inimigos corrompidos ao morrer liberam karma que o jogador absorve. Quantidade
varia por tipo/tier do inimigo. Bosses dão quantidades significativas.
Possíveis fontes adicionais futuras: quests, eventos narrativos, exploração.

### Destinos de gasto

O jogador gasta karma disponível em três sistemas, todos competindo pelo
mesmo recurso:

1. **Skill tree** — desbloquear nós de habilidade
2. **Traje** — evoluir a linha de armadura escolhida (tier up)
3. **Armas** — evoluir a arma escolhida (tier up)

A tensão de escolha é a alma do sistema: cada gasto em um sistema é karma
*não investido* nos outros dois.

### Serviços no mundo (substitui loja tradicional)

Não existe shop convencional. Ninguém vende espadas num mundo corrompido
por karma. Em vez disso, o mundo tem **altares, forjas e NPCs** que
oferecem serviços por karma:

| Serviço           | Onde                     | O que faz                              | Custo   |
|-------------------|--------------------------|----------------------------------------|---------|
| Cura              | Altar / NPC curandeiro   | Restaura HP completo                   | Karma   |
| Respec            | Altar de dissolução      | Reseta build (skill tree + arma + traje)| Grátis  |
| Revelar mapa      | NPC explorador / oráculo | Desvenda zona oculta no mapa           | Karma   |
| Bênção (buff)     | Altar / NPC sacerdote    | Buff temporário antes de zona perigosa | Karma   |
| Poções            | NPC alquimista           | Reabastecer consumíveis                | Karma   |

Cada serviço tem custo em karma. O mundo se sente vivo e coerente: as
pessoas trocam o que podem em troca de energia. Serviços adicionais
surgem conforme conteúdo narrativo for criado.

### Escalonamento do mundo (tier fixo com piso e teto)

Cada zona tem um **tier base (piso)** e um **teto de escalonamento**.
Inimigos escalam com o karma total do jogador *até o teto*, depois param.

Exemplo: Zona 1 (Centro Velho) — piso tier 1, teto tier 3. Jogador com
karma tier 2 enfrenta tier 2. Jogador com karma tier 6 enfrenta tier 3
(teto) — é mais forte, mas inimigos não são triviais. Jogador com karma
tier 10 destrói tudo.

Isso combina liberdade de exploração (no começo toda zona é desafio
proporcional) com sensação real de poder (no late game, zonas antigas
viram fáceis). Curva natural: desafio → competência → dominância.

Karma drop de inimigos é proporcional ao tier efetivo do inimigo. Zonas
no teto ainda dão karma relevante (não ótimo, mas não inútil).

| Zona                    | Piso | Teto |
|-------------------------|------|------|
| Zona 0 — Vila Rosário   | —    | —    |
| Zona 1 — Centro Velho   | 1    | 3    |
| Zona 2 — Parque         | 2    | 4    |
| Zona 4 — Morro (bordas) | 2    | 5    |
| Zona 3 — Industrial     | 3    | 6    |
| Zona 5 — Zona Rica      | 4    | 7    |
| Zona 6 — Rio Turvão     | 5    | 8    |
| Zona 7 — Torre          | 6    | 9    |

> Valores são placeholder. Calibrar durante balanceamento.

---

## Skill tree

### Estrutura: 3 árvores × 2 variações (formato Y)

Cada árvore corresponde a uma classe base. Cada uma tem um tronco comum que
bifurca em duas especializações.

| Classe base | Variação A (placeholder) | Variação B (placeholder) |
|-------------|--------------------------|--------------------------|
| **Warrior** | Berserker (dano bruto, sustain) | Guardian (tanque, controle) |
| **Mage**    | Elementalist (AoE, burst) | Warlock (DoT, debuff) |
| **Archer**  | Sharpshooter (single target, crit) | Trapper (AoE, controle de área) |

> Nomes e identidades das variações são placeholders. Definir quando o
> conteúdo de skills for desenhado.

### Acesso e liberdade

- Sem classe inicial. O jogador começa neutro.
- Ao adquirir karma e abrir a skill tree, **todas as 3 árvores são visíveis**.
- O jogador investe onde quiser, sem restrição de ordem entre árvores.
- Investir em múltiplas árvores é permitido (multiclasse orgânico).
- Sinergias entre nós de árvores diferentes recompensam combinações.
- Especializar fundo numa árvore recompensa foco.
- A bifurcação Y acontece num ponto do tronco. Após a bifurcação,
  o jogador pode investir nos dois ramos, mas o custo de karma escala.

### Sinergias (direção de design, não implementação)

Nós específicos de árvores diferentes podem criar sinergias quando ambos
estão ativos. Exemplo: nó de "armadura em chamas" (warrior) + nó de
"dano de fogo" (mage) = bônus combinado. Isso emerge do design de
conteúdo das skills, não de um sistema genérico de sinergia.

---

## Atributos (foco por level)

### Conceito

Os 5 atributos existentes (vigor, força, destreza, inteligência, endurance)
permanecem. A cada level (determinado por karma total), o jogador escolhe
**um atributo para focar**. Esse atributo sobe significativamente; os
outros 4 sobem um incremento menor automaticamente.

### Por que esse modelo

- **Manual clássico** (distribuir X pontos entre 5 atributos): micro-gestão
  chata quando o jogo já tem skill tree + traje + arma competindo por atenção.
- **Automático puro** (atributos sobem pela build): tira sensação de controle,
  incomoda jogadores que gostam de decidir.
- **Foco por level**: uma decisão por level, leva 2 segundos, tem peso real.
  O jogador sente que escolheu sem ficar preso num menu.

### Funcionamento

1. Karma total atinge threshold do próximo level.
2. Tela de level-up aparece mostrando os 5 atributos.
3. Jogador aponta um atributo.
4. Atributo focado: +N pontos (valor significativo).
5. Demais atributos: +1 cada (crescimento base garantido).
6. Confirma e segue jogando.

### Na engine

Reutiliza a tela de `AttributeLevelUpController` existente, simplificada:
em vez de pontos livres pra distribuir, é seleção de foco. A lógica de
bônus por atributo em `attributes.ini` permanece.

---

### Acesso universal

Todo personagem começa com acesso às três categorias de arma:

| Categoria  | Uso primário               |
|------------|----------------------------|
| **Espada** | Melee, dano corpo a corpo  |
| **Arco**   | Ranged, dano a distância   |
| **Magia**  | Spells, dano mágico / utility |

O jogador pode usar qualquer uma a qualquer momento. Não há restrição
de classe. A diferença é onde investe karma.

### Progressão por investimento

Cada categoria de arma tem uma linha de evolução em tiers. Gastar karma
na arma sobe o tier: dano base, velocidade, efeitos especiais. A arma
evolui com o jogador — não é loot, é progressão.

O jogador pode investir em mais de uma categoria. Generalista fraco,
especialista forte — mesma filosofia da skill tree.

### Relação com skill tree

Skills da árvore de warrior potencializam espada. Skills de archer
potencializam arco. Skills de mage potencializam magia. Mas não é
exclusivo: um mage que investiu em espada tier alto tem uma espada forte,
só não tem os nós de skill que a potencializam ao máximo.

---

## Traje (armadura)

### Conceito

O traje é karma materializado no corpo do personagem. Evolui em tiers,
muda de aparência, e reflete o caminho escolhido.

### 3 linhas de evolução

O jogador escolhe **uma linha** de evolução do traje. Cada linha é
associada a uma classe base, mas a escolha é **livre e independente**
da skill tree.

| Linha             | Identidade                                    |
|-------------------|-----------------------------------------------|
| **Traje Guerreiro** | Armadura pesada, HP, resistência física     |
| **Traje Mago**      | Barrier mágico, regeneração, resist. elemental |
| **Traje Arqueiro**   | Evasão, velocidade, esquiva, mobilidade      |

### Evolução em tiers

Cada linha tem tiers (1 → 2 → 3 → ...). Gastar karma no traje sobe o tier
da linha ativa: stats base sobem, aparência muda. Custo cresce por tier.

Mesma mecânica das armas: investimento direto de karma, sem loot.

### Sinergia e trade-off com a build

Combinar a linha do traje com a classe dominante na skill tree dá
**sinergia** (bônus naturais se complementam). Combinar cruzado cria
**trade-offs interessantes**:

| Build (skill tree) | Traje Guerreiro       | Traje Mago            | Traje Arqueiro        |
|---------------------|-----------------------|-----------------------|-----------------------|
| **Warrior**         | Sinergia (tank puro)  | Battle mage defensivo | Berserker ágil        |
| **Mage**            | Spell blade tanky     | Sinergia (caster puro)| Mage móvel            |
| **Archer**          | Archer pesado         | Archer mágico         | Sinergia (glass cannon)|

9 combinações base antes das variações Y. Cada uma com identidade emergente.

### Visual

Cada tier muda a aparência. O traje cresce, ganha detalhes, pulsa com energia.
A cor/estilo reflete a linha escolhida:
- Guerreiro: karma denso, cristalizado, pesado
- Mago: karma etéreo, runas flutuantes, aura
- Arqueiro: karma leve, aderente, aerodinâmico

---

## Respec

### Mecanismo

**Altar de dissolução** — ponto fixo no mundo onde o karma se dissolve e
retorna ao jogador. O jogador interage com o altar e **reseta tudo**: skill
tree (todos os nós), tiers de arma, linha e tier do traje. Todo karma
investido retorna ao pool de karma disponível. O jogador reinveste do zero.

### Custo

Gratuito. O custo natural é o deslocamento até o altar e o tempo de
reinvestir tudo. Isso é fricção suficiente pra não ser abusado em combate,
mas não pune quem quer experimentar builds. As pessoas só querem sobreviver
— não faz sentido penalizar por tentar.

### Escopo

Reset total. Não existe reset parcial. Simplicidade: uma ação, resultado
claro, sem menus de "o que resetar".

### Narrativa

O altar é um lugar onde a energia de karma se desfaz. Coerente com a lore:
se karma é energia que se materializa, faz sentido que exista um ponto
onde ela se dissolve de volta à forma pura. O jogador está devolvendo
temporariamente o que absorveu pra remoldá-lo.

---

## Mapeamento com a engine existente

### O que muda

| Sistema atual               | Estado atual                          | O que muda com karma                             |
|-----------------------------|---------------------------------------|--------------------------------------------------|
| `progression.hpp`           | XP → level → pontos                  | Karma total → level equivalente. Karma disponível → recurso de gasto. Elimina XP. |
| `talents.ini` / skill tree  | 1 árvore, pontos de talento           | 3 árvores formato Y. Custo em karma, não em pontos. |
| `equipment.hpp`             | 11 slots, peças individuais           | Substituído por traje único com linha + tier. Slots viram aspectos visuais, não funcionais. |
| `item_bag.hpp`              | Bag 4×6 de peças de equip             | Reduzido ou repensado. Sem loot de armadura, bag guarda consumíveis + itens especiais (reset, etc.). |
| `spells.ini`                | 5 spells hardcoded                    | Spells migram pra nós da skill tree. Desbloqueia por investimento de karma. |
| `attributes.ini`            | 5 atributos com pontos manuais        | Mantém 5 atributos. Distribuição manual → foco por level (1 escolha por level up). |
| `data/items.ini`            | Drops com stats                       | Drops viram karma (todos os inimigos) + itens especiais (reset, consumíveis). |
| `potion_quickslot.hpp`      | Poções com stack e qualidade          | Mantém. Poções compradas por karma em NPC alquimista ou dropadas. |
| `shop_system.hpp`           | Loja tradicional com menu de compra   | Substituído por altares/NPCs com serviços por karma (cura, buff, poções, respec, mapa). |
| `save_data.hpp`             | Save v7                               | Save v8+: karma_total, karma_available, skill_tree_state[3], weapon_tiers[3], traje_line, traje_tier. |
| `enemy_death_controller.hpp`| Drops + XP + quest                    | Drops → karma + chance de consumível. XP eliminado. |

### O que não muda

- Sistema de combate (melee, projéteis, colisão) — intacto.
- Open world (WorldScene, ZoneManager, WorldMap) — intacto.
- IA de inimigos — intacta.
- Actor split (PlayerData / EnemyAIData) — intacto, PlayerData ganha campos de karma.
- Save/load — migração incremental, estrutura existente.
- Config, locale, áudio, input — intactos.

### Consequências narrativas do karma alto (faixas híbridas)

Três faixas de karma com efeitos distintos, combinando reação de NPCs e
mudança visual do mundo.

**Faixa baixa (início do Ato 1):** nada muda. Dante é invisível, mais
um sobrevivente.

**Faixa média (meio do Ato 1):** NPCs comentam que "tem algo diferente
nele" — curiosidade, não medo. O mundo começa a reagir visualmente:
plantas crescem perto dele, objetos vibram levemente.

**Faixa alta (final do Ato 1):** NPCs divididos. Alguns admiram (o cara
que tá limpando São Chico). Outros temem (ele carrega a mesma energia que
destruiu tudo). Mundo reage forte: vegetação brota onde ele pisa, névoa
recua na sua presença, mas energia pulsa ao redor dele. Dante é uma
figura ambígua — respeitado e temido ao mesmo tempo.

Reações variam por NPC. Dona Marta fica cautelosa. Lia se preocupa. Padre
Inácio questiona. Dra. Marina se fascina. Zé faz piada. O Sindicalista
não liga.

Na engine: faixas de karma total definem variável `karma_tier_visual`
que sistemas de diálogo e render consultam. Não é um sistema complexo —
são flags por faixa.

### Decisões em aberto (balanceamento)

1. **Custo dos serviços no mundo:** quanto karma custa cada serviço
   (cura, buff, poções, mapa). Definir durante balanceamento.

2. **Valores de piso/teto por zona:** tabela provisória acima, calibrar
   com playtest.

---

## Plano de implementação

Sprints maiores, cada uma dividida em sub-sprints seguras. Cada sub-sprint
compila, passa testes, e o jogo é jogável. Nunca fica num estado quebrado
entre sub-sprints.

---

### Sprint K1 — Fundação de karma

Objetivo: karma existe como recurso, substitui XP, persiste no save.
Gameplay existente continua funcionando; karma é aditivo, não destrutivo.

**K1.1 — KarmaData + drop de karma**
- Criar `KarmaData { int total; int available; }` em `PlayerData`.
- `EnemyDeathController` emite karma em vez de (ou além de) XP.
- Karma por tipo de inimigo definido em `data/enemies.ini` (novo campo).
- Testes: karma sobe ao matar, total nunca desce, available = total - investido.

**K1.2 — Level por karma total**
- Tabela de thresholds de level em `data/progression.ini` (reusa estrutura).
- Função `karma_level(int karma_total) → int` substitui cálculo de XP→level.
- Testes: level sobe nos thresholds corretos.

**K1.3 — Atributos: foco por level**
- Adaptar `AttributeLevelUpController` pra modo foco: 1 escolha por level.
- Atributo focado ganha +N, demais ganham +1.
- `data/attributes.ini`: novo campo `focus_bonus` por atributo.
- Testes: foco aplica bônus correto, demais sobem +1.

**K1.4 — HUD de karma**
- `DungeonHud` mostra karma total / disponível no lugar de XP.
- Barra ou número — decisão visual na sub-sprint.

**K1.5 — Save migration v7→v8**
- Persistir `karma_total`, `karma_available`, `attribute_focus_history`.
- `migrate_v7_to_v8()`: converte XP existente em karma total equivalente.
- Testes: save v7 migra corretamente, round-trip v8 OK.

**Critério de DONE K1:** jogador ganha karma ao matar inimigos, sobe de
level por karma, escolhe foco de atributo, HUD reflete karma, save persiste.
Skill tree e equip ainda funcionam do jeito antigo.

---

### Sprint K2 — Skill tree expandida

Objetivo: 3 árvores com formato Y, custo em karma, liberdade total.

**K2.1 — Estrutura de dados: 3 árvores**
- Expandir `data/talents.ini` pra 3 seções (`[warrior]`, `[mage]`, `[archer]`).
- Cada nó tem: `karma_cost`, `prerequisites`, `tree_id`, `branch` (pre-Y, A, B).
- `TalentNode` e `TalentTree` adaptados pra múltiplas árvores.
- Testes: parse de 3 árvores, nós com branch correto.

**K2.2 — Lógica de desbloqueio por karma**
- Substituir "talent points" por gasto de karma disponível.
- Validação: karma disponível >= custo do nó, pré-requisitos atendidos.
- Testes: desbloqueia nó com karma suficiente, rejeita sem karma,
  respeita pré-requisitos entre árvores.

**K2.3 — UI: navegação entre árvores**
- `SkillTreeController` ganha navegação lateral (warrior ↔ mage ↔ archer).
- Cada árvore renderiza com layout Y (tronco + bifurcação).
- Testes: navegação não quebra, estado persiste entre trocas.

**K2.4 — Sinergias (estrutura)**
- Nós podem declarar `synergy_with = [outro_nó]` em INI.
- Quando ambos os nós estão ativos, bônus adicional aplicado.
- Conteúdo real das sinergias é da fase de design de skills (não aqui).
- Testes: sinergia ativa quando par completo, inativa quando não.

**K2.5 — Save: estado da skill tree**
- Persistir nós desbloqueados das 3 árvores no save.
- Migração: nós antigos mapeiam pra árvore warrior (compatibilidade).
- Testes: round-trip save/load com nós de múltiplas árvores.

**Critério de DONE K2:** jogador navega 3 árvores, gasta karma pra desbloquear
nós, formato Y funciona, sinergias aplicam bônus, save persiste.

---

### Sprint K3 — Armas universais

Objetivo: 3 categorias de arma com progressão por karma, acesso livre.

**K3.1 — WeaponData: 3 categorias com tiers**
- `WeaponData { tier_sword, tier_bow, tier_magic }` em `PlayerData`.
- Tabela de stats por tier em `data/weapons.ini` (novo arquivo):
  dano base, velocidade, efeitos por tier.
- Testes: stats corretos por tier, tier 0 = base funcional.

**K3.2 — Investimento de karma em arma**
- UI (nova ou integrada no HUD/overlay) pra gastar karma em tier de arma.
- Validação: karma disponível >= custo do próximo tier.
- Custo crescente por tier em `data/weapons.ini`.
- Testes: tier sobe com karma, rejeita sem karma suficiente.

**K3.3 — Integração com combate**
- `MeleeCombatSystem`, `SpellSystem`, `ProjectileSystem` leem dano base
  do tier da arma correspondente em vez de stats fixos do player.
- Testes: dano escala com tier, todas as 3 armas funcionam.

**K3.4 — Relação skill ↔ arma**
- Nós de skill tree que dão bônus a categoria específica de arma.
- Bônus aplicados como multiplicador sobre stats do tier.
- Testes: nó de warrior potencializa espada, não afeta arco/magia.

**K3.5 — Save: tiers de arma**
- Persistir `weapon_tier_sword`, `weapon_tier_bow`, `weapon_tier_magic`.
- Testes: round-trip save/load com tiers.

**Critério de DONE K3:** jogador usa espada/arco/magia livremente, investe
karma pra evoluir cada uma, dano reflete tier + bônus de skill, save persiste.

---

### Sprint K4 — Traje

Objetivo: 3 linhas de traje com evolução por karma e impacto nos stats.

**K4.1 — TrajeData: linha + tier**
- `TrajeData { TrajeType line; int tier; }` em `PlayerData`.
- `enum class TrajeType { Warrior, Mage, Archer }`.
- Tabela de stats por linha × tier em `data/traje.ini` (novo arquivo).
- Testes: stats corretos por combinação linha/tier.

**K4.2 — Escolha de linha e investimento**
- UI pra escolher linha (primeira vez) e gastar karma pra tier up.
- Mudar de linha reseta o tier (decisão pesada, ou via respec).
- Testes: escolha de linha, tier up, troca de linha reseta tier.

**K4.3 — Stats do traje no personagem**
- Bônus do traje aplicados nos stats do player (HP, defesa, evasão,
  barrier, velocidade — conforme a linha).
- Sinergia/trade-off com build: bônus extra quando linha combina com
  classe dominante na skill tree.
- Testes: stats corretos por combinação, sinergia aplica bônus extra.

**K4.4 — Visual por tier**
- Spritesheet do player varia conforme linha + tier.
- Pode ser palette swap da base por tier/linha no MVP.
- Testes: visual muda, não quebra render.

**K4.5 — Save: traje**
- Persistir `traje_line` e `traje_tier`.
- Testes: round-trip save/load.

**Critério de DONE K4:** jogador escolhe linha de traje, evolui com karma,
stats refletem linha + tier + sinergia com build, visual muda, save persiste.

---

### Sprint K5 — Serviços no mundo + respec

Objetivo: altares e NPCs oferecem serviços por karma. Respec funcional.

**K5.1 — Sistema de serviços**
- `ServiceNpc` ou `ServiceAltar` como entidade no mundo (análogo a Shop).
- Menu de interação lista serviços disponíveis com custo em karma.
- Testes: interação abre menu, serviço cobra karma, aplica efeito.

**K5.2 — Serviço: cura**
- Restaura HP por karma. Altar ou NPC curandeiro.
- Testes: cura aplica, cobra karma.

**K5.3 — Serviço: poções**
- NPC alquimista vende poções por karma.
- Reusa `PotionQuickslot` existente.
- Testes: compra funciona, stack atualiza.

**K5.4 — Serviço: respec (altar de dissolução)**
- Altar de dissolução como entidade no mundo.
- Interação reseta: skill tree (todos os nós), tiers de arma, linha + tier de traje.
- Karma investido retorna ao pool de karma disponível.
- Karma total não muda (level permanece). Gratuito.
- Testes: respec devolve karma, zera nós/tiers, level inalterado.

**K5.5 — Serviço: buff e revelar mapa**
- Buff temporário (timer) por karma.
- Revelar zona: marca zona no mapa como descoberta.
- Testes: buff aplica e expira, zona revelada persiste.

**K5.6 — Save: serviços usados**
- Persistir buffs ativos e zonas reveladas.
- Testes: round-trip.

**Critério de DONE K5:** mundo tem altares/NPCs com serviços, respec
funciona completamente, jogador pode resetar e reinvestir.

---

### Sprint K6 — Escalonamento do mundo (piso/teto)

Objetivo: dificuldade por zona com tier fixo, piso e teto.

**K6.1 — Tabela de piso/teto por zona**
- `data/zones.ini` (novo): cada zona define `tier_min` e `tier_max`.
- Função `effective_tier(karma_total, zone) → clamp(karma_tier, min, max)`.
- Testes: tier efetivo respeita piso e teto.

**K6.2 — Escalonamento de inimigos**
- `AreaEntrySystem` aplica `effective_tier` ao spawnar inimigos.
- Stats de inimigos multiplicados pelo tier efetivo da zona.
- Zonas revisitadas com karma maior: inimigos escalam até o teto, depois param.
- Testes: mesma zona com karma diferente gera inimigos diferentes (até o teto).

**K6.3 — Karma drop proporcional**
- Karma drop = f(tier efetivo do inimigo). Inimigos no teto dão drop
  relevante mas não ótimo comparado a zonas com teto mais alto.
- Testes: karma drop proporcional, zonas no teto não dão drop trivial.

**Critério de DONE K6:** cada zona tem piso/teto, inimigos escalam
dentro da faixa, drop proporcional, progressão sente-se real.

---

### Sprint K7 — Remoção de sistemas legados + polish

Objetivo: limpar XP, ouro, equipamento antigo, shop tradicional.

**K7.1 — Remover XP**
- Eliminar campos de XP do `PlayerData`, HUD, save.
- Migração final: save antigo converte XP→karma se ainda não converteu.

**K7.2 — Remover ouro**
- Eliminar campos de ouro do `PlayerData`, HUD, drops, save.

**K7.3 — Simplificar inventário**
- Bag reduzida: só consumíveis + itens especiais (reset, chaves, quest).
- 11 equipment slots removidos ou repensados.
- `EquipmentScreenController` adaptado ou substituído.

**K7.4 — Remover shop tradicional**
- `ShopSystem` removido ou convertido em base para `ServiceNpc`.

**Critério de DONE K7:** nenhum sistema legado (XP, ouro, equip tradicionais,
shop) permanece. Todos os fluxos usam karma.

---

### Sprint K8 — Consequências narrativas do karma (faixas)

Objetivo: mundo e NPCs reagem ao nível de karma total de Dante.

**K8.1 — Sistema de faixas de karma**
- Definir thresholds de faixa (baixa/média/alta) em `data/progression.ini`.
- `karma_presence_tier(karma_total) → low | mid | high`.
- Testes: faixa correta por threshold.

**K8.2 — Variações de diálogo por faixa**
- NPCs com diálogos alternativos por faixa. Estrutura em `data/dialogues/`.
- Faixa baixa: diálogo padrão. Faixa média: variações de curiosidade.
  Faixa alta: variações divididas (admiração ou medo, por NPC).
- Testes: diálogo correto por faixa × NPC.

**K8.3 — Efeitos visuais por faixa**
- Faixa média: vegetação cresce perto do player (partículas leves).
  Objetos vibram sutilmente.
- Faixa alta: névoa recua na presença do player. Energia pulsa ao redor.
  Vegetação brota onde ele passa.
- Pode ser palette shift + partículas no MVP.
- Testes: efeitos visuais ativam na faixa correta, não quebram render.

**Critério de DONE K8:** NPCs reagem ao karma de Dante por faixa,
mundo muda visualmente, ambiguidade moral é sentida pelo jogador.

---

## Riscos

| Risco                                    | Mitigação                                    |
|------------------------------------------|----------------------------------------------|
| Karma como moeda única sem tensão de escolha | Custos calibrados pra forçar priorização (balanceamento em K6/K7) |
| Skill tree genérica / nós sem identidade   | Design de conteúdo de skills como fase separada após estrutura pronta |
| Traje sem dopamina de loot                 | Evolução visual marcante a cada tier (K4.4) |
| Respec sem custo → sem consequência         | Fricção natural: deslocamento até altar + tempo de reinvestir tudo |
| 9 combinações build×traje impossíveis de balancear | Balancear por arquétipo, não por combinação individual |
| Refactor grande no sistema de equip existente | Sistemas legados removidos só em K7 (último), tudo funciona até lá |
| Escalonamento cria grind infinito          | Cap de escalonamento por zona + piso/teto de karma drop |
| Sprints longas demais                      | Cada sub-sprint é autônoma: compila, testa, joga |
