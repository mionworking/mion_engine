# 03a — Zones Content (Part 1: Zones 0-2)

Content for zones 0, 1, and 2: Vila Rosário, Centro Velho, Parque
Ibirapitanga. The opening of Act 1 — from gatilho to first revelations
about karma.

> Companion to `01_implementation_plan.md` § Sprints C2, C3.
> Narrative grounding in `ato1_historia.md`.
> Dialogues themselves live in `04_dialogues.md`; this doc references
> dialogue keys but doesn't write the lines.

---

# Zone 0 — Vila Rosário

## Narrative summary

Bairro periférico onde Dante cresceu. Casas simples, ruas mistas (terra
+ asfalto remendado), mercadinho, igreja, campo de várzea, figueira
centenária. Abertura do jogo: tutorial + gatilho com os pais. Após o
gatilho, vira **base do jogador** — Dante volta aqui para descansar,
falar com Lia, checar os pais.

A névoa de karma se aproxima da Vila ao longo do Ato 1. Nunca consome
ela completamente, mas a urgência cresce.

## Layout suggestion (procedural)

Tilemap aproximado 64×64. Áreas funcionais:

```
+---------------------------------------+
|  CAMPO DE VÁRZEA (figueira / altar)   |
|         [Altar de Dissolução]         |
+----------+--------------+-------------+
|          |              |             |
|  CASA    |   IGREJA     |  MERCADINHO |
|  DANTE   |   PEQUENA    |  DA ESQUINA |
|          |              |             |
+----------+--------------+-------------+
|  RUA PRINCIPAL (saída para Centro)    |
|         [Door → Centro Velho]         |
+---------------------------------------+
```

Pontos de interesse:
- **Casa de Dante:** entrada inicial, cama (save point), cozinha (cena
  do gatilho), quarto dos pais
- **Figueira centenária:** Altar de Dissolução (primeiro respec)
- **Mercadinho:** NPC vizinho, lore opcional
- **Igreja:** NPC sacerdote local (não confundir com Padre Inácio do
  morro), serviço básico de cura
- **Saída sul:** transição para Centro Velho (Zone 1)

## Inimigos

**Nenhum inimigo combatível.** Zona segura por design.

Conforme o Ato 1 avança, **névoa de karma na borda norte da Vila** se
intensifica visualmente (efeito de partículas, escurecimento de tile),
mas não atravessa pra dentro. Reforço narrativo de urgência sem virar
zona de combate.

## Mini-boss

Nenhum.

## NPCs

### `npc_lia` — Lia

**Função:** irmã de Dante, primeira aliada potencial. Cuidadora dos pais
após o gatilho. Hub emocional da Vila.

**Localização:** dentro da Casa de Dante (cozinha, depois quarto dos
pais).

**Gatilhos de diálogo:**
- `dlg_lia_intro_001` — primeira fala (telefonema do gatilho)
- `dlg_lia_apos_gatilho_001` — após Dante absorver karma dos pais
- `dlg_lia_visita_padrao_001` — sempre que Dante volta à Vila
- `dlg_lia_recrutamento_001` — momento de recrutamento (após Zone 1
  completa, ela quer ir junto)
- `dlg_lia_combat_*` — falas em combate (após recrutada)
- `dlg_lia_zone_<id>_001` — reação ao entrar em cada zona com Dante

### `npc_dona_celia` — Dona Célia

**Função:** mãe de Dante. Estado: estabilizada mas não curada. Dorme a
maior parte do tempo. Lúcida em momentos pontuais.

**Localização:** quarto dos pais.

**Gatilhos de diálogo:**
- `dlg_celia_pre_gatilho_001` — apenas no início, ainda saudável (rotina)
- `dlg_celia_lucida_001` — momento de lucidez breve, varia por faixa de karma
- `dlg_celia_lucida_002_alta` — variação da faixa alta (reage ao karma de Dante)

### `npc_seu_helio` — Seu Hélio

**Função:** pai de Dante. Estado: estabilizado, mais consumido que Dona
Célia. Quase não fala. Quando fala, peso enorme.

**Localização:** quarto dos pais.

**Gatilhos de diálogo:**
- `dlg_helio_pre_gatilho_001` — antes do gatilho (rotina, breve)
- `dlg_helio_gatilho_001` — momento do gatilho ("Cuida delas.")
- `dlg_helio_lucido_raro_001` — falas raras de lucidez ao longo do Ato 1
  (3-4 momentos no jogo inteiro, peso narrativo)

### `npc_seu_quinho` — Seu Quinho (vizinho)

**Função:** dono do mercadinho. Sobrevivente. Sabe de fofoca da vizinhança,
o que tá rolando ao redor. NPC de cor local — humor seco, tom "Direção 1".

**Localização:** balcão do mercadinho.

**Gatilhos de diálogo:**
- `dlg_quinho_intro_001` — primeira interação (rotina, antes do gatilho)
- `dlg_quinho_apos_gatilho_001` — depois que Dante começa a explorar
- `dlg_quinho_rumores_centro_001` — fofoca sobre o Centro Velho
- `dlg_quinho_zone_progress_*` — comentários conforme zonas são
  exploradas

**Serviço:** vende poções básicas (kind=potion, custo 25 karma). Lore-coerente:
ele tinha estoque de farmácia da vizinhança e agora distribui.

### `npc_padre_lucas` — Padre Lucas (igreja local)

**Função:** sacerdote da capela da Vila. **NÃO é Padre Inácio** (esse
está no Morro da Vigília). Lucas é mais simples, menos questionador.
Oferece cura básica.

**Localização:** porta da igreja.

**Gatilhos de diálogo:**
- `dlg_lucas_intro_001` — primeira fala
- `dlg_lucas_oferece_cura_001` — proposta de serviço

**Serviço:** cura completa (kind=heal, custo 50 karma).

## Documentos coletáveis

### `doc_carteira_de_trabalho_helio`
- **Localização:** gaveta do quarto dos pais (interagível após o gatilho)
- **Conteúdo (PT):** Carteira de trabalho de Seu Hélio. Anos e anos de
  registros como pedreiro. Empresas que faliram, salários congelados,
  três meses sem pagamento em 2019. Carimbos. Carimbos. Carimbos.
- **Conteúdo (EN):** Seu Hélio's work record. Years and years registered
  as a bricklayer. Companies that went bankrupt, frozen wages, three
  months unpaid in 2019. Stamps. Stamps. Stamps.

### `doc_caderno_lia`
- **Localização:** mesa do quarto da Lia
- **Conteúdo (PT):** Caderno de anotações da faculdade. Trabalho sobre
  "saúde mental e capitalismo tardio". Citações marcadas. No final, uma
  anotação à mão: "isso aqui não é crise individual. é crise estrutural
  com cara de privada."
- **Conteúdo (EN):** University notebook. Paper on "mental health and
  late capitalism." Highlighted quotes. Final handwritten note: "this
  isn't individual crisis. it's structural crisis dressed up as private."

### `doc_receita_dona_celia`
- **Localização:** cozinha
- **Conteúdo (PT):** Receita de bolo de fubá da Dona Célia, escrita à
  mão. Anotação no canto: "fazer no domingo quando o Dante vem."
- **Conteúdo (EN):** Dona Célia's cornmeal cake recipe, handwritten.
  Side note: "make Sunday when Dante comes."

## Action events

### `event_gatilho` — A primeira manifestação

**Triggers:**
- `ZoneEnter:vila_rosario` AND `flag:tutorial_done` AND NOT `flag:gatilho_done`

**Steps:**
1. **Dialogue:** `dlg_lia_intro_001` (Lia liga, voz no telefone)
2. **Cinematic:** Dante entra na cozinha. Dona Célia em crise visível
   (sprite alterado, partículas escuras).
3. **Dialogue:** `dlg_celia_gatilho_001` ("...você é quem? quem é
   você...")
4. **Dialogue:** `dlg_helio_gatilho_001` ("Cuida delas.")
5. **Cinematic:** Dante toca a mãe. Tela escurece, flashes de imagens
   (memórias de uma vida — turnos, contas, a saída do pai sendo
   evitada). Tela ilumina.
6. **Effect:** `karma_add(50)` — Dante absorveu seu primeiro karma
7. **Effect:** `set_flag:gatilho_done`
8. **Effect:** `unlock_zone:centro_velho`
9. **Dialogue:** `dlg_lia_apos_gatilho_001` (Lia entrando, confusa)
10. **Effect:** `set_objective:investigar_centro`

**Consequence:** karma sistema oficialmente ativo. HUD atualiza.

### `event_figueira_descoberta` — O primeiro altar

**Triggers:**
- `flag:gatilho_done` AND `ZoneEnter:vila_rosario_campo_varzea` AND NOT
  `flag:figueira_descoberta`

**Steps:**
1. **Cinematic:** Dante chega ao campo de várzea. A figueira centenária
   pulsa levemente — partículas dispersam ao redor.
2. **Dialogue:** `dlg_dante_figueira_001` (linha interna curta — Dante
   sente algo)
3. **Tutorial popup:** "Altares de Dissolução permitem reorganizar
   karma investido. Sem custo." (PT) / "Dissolution Altars allow you
   to reorganize invested karma. No cost." (EN)
4. **Effect:** `unlock_altar:figueira_vila`
5. **Effect:** `set_flag:figueira_descoberta`

### `event_nevoa_aproxima` — Pressão narrativa

**Triggers:**
- `karma_total >= faixa_media_threshold` AND `ZoneEnter:vila_rosario`

**Steps:**
1. **Visual:** Borda norte da Vila ganha névoa de karma mais densa.
   Efeito visível mas não bloqueia movimento.
2. **Dialogue:** `dlg_lia_nevoa_aproxima_001`
3. **Effect:** `set_flag:nevoa_proxima_vila`

**Consequence:** reforço de urgência. Dante percebe que a Vila não vai
ficar segura pra sempre.

## Pontos de interesse

| Tipo            | Local                    | Função                                |
|-----------------|--------------------------|---------------------------------------|
| Save point      | Cama da casa de Dante    | Save manual                           |
| Altar           | Figueira do campo        | Respec total (free)                   |
| Cura            | Padre Lucas (igreja)     | Cura HP completo (50 karma)           |
| Poções          | Seu Quinho (mercado)     | Compra de poções (25 karma)           |
| Transição       | Saída sul                | Door → Centro Velho                   |
| Lore documents  | Quarto dos pais, caderno Lia, cozinha | 3 documentos iniciais |

---

# Zone 1 — Centro Velho

## Narrative summary

Coração comercial de São Chico. Prédios de escritório, lojas de rua,
terminal de ônibus, praça com chafariz seco. Carros abandonados, fiação
exposta, papéis de empresa voando. Primeiros inimigos do jogo: trabalhadores
corrompidos. Primeira informação real sobre o que karma é, vinda de um
ex-professor de filosofia.

Mini-boss: O Gerente — executivo cuja autoridade virou armadura de karma.

## Layout suggestion (procedural)

Tilemap aproximado 96×96. Estrutura urbana grid:

```
+----------------------------------------------+
|  PRAÇA CENTRAL (chafariz seco)               |
|         [Mini-boss arena: O Gerente]         |
+----------------+----------+------------------+
|                |          |                  |
| TERMINAL       | RUA      | EDIFÍCIO         |
| ÔNIBUS         | LOJAS    | COMERCIAL        |
| (entrada N)    |          | (npc professor)  |
+----------------+----------+------------------+
|     RUA PRINCIPAL (acesso a outras zonas)    |
| [Door S → Vila]  [Door E → Parque]  [Door W → Industrial] |
+----------------------------------------------+
```

Pontos de interesse:
- **Praça central:** arena do mini-boss
- **Edifício comercial (térreo):** NPC ex-professor, primeiros documentos
  corporativos
- **Terminal de ônibus:** NPC alquimista improvisado (farmácia da rua)
- **Ruas laterais:** spawn de inimigos, exploração

## Inimigos

### `enemy_office_worker` — Trabalhador de Escritório
- **Tier base:** 1
- **HP base:** 40
- **Damage base:** 6
- **AI:** Lento, ataca corpo a corpo com objetos improvisados (grampeador,
  caneta — narrativamente irônico). Em grupos de 3-5.
- **Karma drop:** 8
- **Comportamento:** wander → aggro on player sight → melee chase
- **Visual placeholder:** sprite de NPC office worker tingido com névoa
  escura

### `enemy_security_guard` — Segurança Corrompido
- **Tier base:** 2
- **HP base:** 70
- **Damage base:** 10
- **AI:** Patrulha em rotas fixas (calçadas, hall de prédio). Ataca com
  cassetete. Mais agressivo que office worker.
- **Karma drop:** 15
- **Comportamento:** patrol → aggro → melee
- **Visual placeholder:** uniforme de segurança escurecido

### `enemy_delivery_corrupted` — Entregador Corrompido
- **Tier base:** 1
- **HP base:** 30
- **Damage base:** 8
- **AI:** Rápido, em moto/bicicleta corrompida. Investidas curtas.
- **Karma drop:** 10
- **Comportamento:** circle player → dash attack → retreat → repeat
- **Visual placeholder:** sprite de motoboy com névoa de karma

## Mini-boss: O Gerente

### Identidade narrativa

Executivo de andar inteiro que acumulou pressão até cristalizar. O
karma virou armadura corporativa — terno blindado, gravata como chicote,
crachá como ímã de inimigos. Ele não está mais consciente. Atos
restantes são repetições compulsivas de gestão: "ordens" que ele dá no
ar, e o karma as materializa como projéteis.

### Stats

- **Tier:** 3
- **HP:** 400 (escala com `effective_tier` da zona)
- **Damage:** 18
- **Phases:** 1 (sem fases — é o primeiro mini-boss, foco em mecânica única)

### Mecânicas

**Ataque 1 — Ordem Direta** (cooldown 4s)
- O Gerente grita uma "ordem" (texto flutuante: "RELATÓRIO!")
- Projétil de karma em linha reta na direção do player
- Dano: 18, atordoa por 0.5s se acertar

**Ataque 2 — Reunião Forçada** (cooldown 12s)
- O Gerente convoca 3 office workers que spawn ao redor do player
- Office workers atacam por 8s ou até morrerem
- Útil pra player com lifesteal/AoE

**Ataque 3 — Memorando Geral** (cooldown 18s)
- O Gerente bate o crachá no chão
- 5 projéteis radiais saem dele em padrão fixo (oito direções menos as
  diagonais traseiras)
- Dano cada: 12, mas overlap possível

**Passive — Autoridade Imposta**
- Player perto dele (3m) tem -10% velocidade de movimento
- Lore: a presença dele literalmente pesa

### Arena

Praça central. Espaço aberto com chafariz seco no centro (cobertura
parcial). Bordas da arena são fachadas de prédios — sem saída até o
boss morrer.

### Drop

- 200 karma garantido
- Documento: `doc_relatorio_gerente`
- Ativa flag: `flag:centro_clear`

### Diálogo no início e fim

- **Início:** sem diálogo. O Gerente apenas grita uma ordem ininteligível
  e ataca.
- **Morte:** breve momento de lucidez — `dlg_gerente_morte_001` ("...mas
  o trimestre... eu não posso..."). Morre.

## NPCs

### `npc_professor_olavo` — Olavo (ex-professor de filosofia)

**Função:** primeiro NPC com informação **real** sobre o karma. Dá ao
jogador o vocabulário pra entender o que tá acontecendo.

**Localização:** térreo do Edifício Comercial. Sentado num sofá de espera,
fumando.

**Gatilhos de diálogo:**
- `dlg_olavo_intro_001` — primeira fala
- `dlg_olavo_explicacao_karma_001` — quando perguntado sobre karma
  (Direção 2 — denso, poético)
- `dlg_olavo_explicacao_karma_002_alta` — variação faixa alta (reage ao
  karma de Dante: "você tá começando a sentir, né")
- `dlg_olavo_recomenda_centro_001` — direciona pra explorar o resto do
  centro
- `dlg_olavo_apos_gerente_001` — após o mini-boss morrer

**Lore que ele entrega:**
- Karma é energia de sofrimento coletivo materializada
- Não é punição moral
- A natureza reage diferente (gancho pra Zone 2)
- "O peso do mundo se tornando real."

### `npc_alquimista_terminal` — Eunice (a do terminal)

**Função:** NPC alquimista improvisada. Era enfermeira de UPA antes
do colapso. Montou farmácia improvisada no terminal de ônibus.

**Localização:** terminal de ônibus, atrás do balcão.

**Gatilhos de diálogo:**
- `dlg_eunice_intro_001`
- `dlg_eunice_oferece_servicos_001`

**Serviços:**
- Cura completa (60 karma)
- Poções de HP médias (40 karma cada)
- Buff temporário de resistência (80 karma, dura 5 min)

## Documentos coletáveis

### `doc_aviso_demissao`
- **Localização:** cubículo no segundo andar do Edifício Comercial
- **Conteúdo (PT):** Aviso de demissão em massa datado de 6 meses antes
  do início do jogo. Empresa "TecnoSul Soluções." 142 demitidos. "Em
  função de reestruturação de prioridades estratégicas..."
- **Conteúdo (EN):** Mass layoff notice dated 6 months before game start.
  "TecnoSul Solutions." 142 laid off. "Due to restructuring of strategic
  priorities..."

### `doc_relatorio_gerente`
- **Localização:** drop garantido após O Gerente
- **Conteúdo (PT):** Relatório trimestral inacabado. Metas de produtividade
  marcadas com canetas vermelhas. Anotação à mão: "Não sei mais o que
  fazer. Eles querem 20% a mais com 30% menos pessoal. Eu não durmo."
  Assinado: O Gerente (nome real ilegível).
- **Conteúdo (EN):** Unfinished quarterly report. Productivity targets
  marked in red. Handwritten note: "I don't know what to do anymore.
  They want 20% more with 30% less staff. I can't sleep." Signed: O
  Gerente (real name illegible).

### `doc_email_assedio`
- **Localização:** terminal de computador funcionando no térreo
- **Conteúdo (PT):** Cadeia de emails entre uma funcionária e RH. Ela
  reporta assédio do superior. RH: "vamos investigar." 3 meses depois:
  "decidimos arquivar por falta de evidências." Última mensagem dela:
  "Tá. Já entendi."
- **Conteúdo (EN):** Email chain between an employee and HR. She reports
  harassment from her boss. HR: "we'll investigate." 3 months later:
  "we've decided to archive due to lack of evidence." Her last message:
  "OK. I get it."

### `doc_panfleto_meditacao_corporativa`
- **Localização:** chão da praça central (perto do chafariz)
- **Conteúdo (PT):** Panfleto de "workshop de mindfulness corporativo."
  Promete reduzir estresse com 15 minutos de meditação por dia, sem
  alterar carga de trabalho. Logo da empresa "TecnoSul" no canto.
- **Conteúdo (EN):** Pamphlet for "corporate mindfulness workshop."
  Promises to reduce stress with 15 minutes of meditation per day,
  without changing workload. "TecnoSul" logo in the corner.

## Action events

### `event_primeiro_combate` — Tutorial de combate

**Triggers:**
- `ZoneEnter:centro_velho` AND NOT `flag:primeiro_combate_done`

**Steps:**
1. Spawn 1 office worker próximo do player
2. **Tutorial popup:** controles básicos de combate
3. Após o inimigo morrer: **karma_add(8)** + popup "Você absorveu karma
   do inimigo derrotado." (PT) / "You absorbed karma from the defeated
   enemy." (EN)
4. **Effect:** `set_flag:primeiro_combate_done`

### `event_encontro_olavo` — Primeira informação

**Triggers:**
- `flag:primeiro_combate_done` AND `ZoneEnter:edificio_comercial_terreo`

**Steps:**
1. Olavo levanta do sofá quando Dante entra
2. **Dialogue:** `dlg_olavo_intro_001`
3. Player pode perguntar sobre karma → `dlg_olavo_explicacao_karma_001`
4. Após explicação: `set_flag:karma_explicado`
5. Olavo direciona pra explorar a praça central

### `event_arena_gerente` — Spawn do mini-boss

**Triggers:**
- `flag:karma_explicado` AND `ZoneEnter:praca_central`

**Steps:**
1. Cinematic: portões da praça se fecham (escombros, carros viram barreira)
2. **Sound cue:** grito distorcido (O Gerente)
3. Spawn O Gerente no centro da praça
4. Combate começa
5. Após morte: `set_flag:centro_clear`, `unlock_zone:parque_ibirapitanga`,
   `unlock_zone:distrito_industrial`, drop garantido
6. Portões da praça se abrem (carros se movem com partículas de karma
   dispersando)

### `event_olavo_apos_gerente` — Reflexão pós-combate

**Triggers:**
- `flag:centro_clear` AND retorna a Olavo

**Steps:**
1. **Dialogue:** `dlg_olavo_apos_gerente_001`
2. Olavo entrega gancho pro Parque (ele leu a tese da bióloga, sabe que
   ela tá lá): `dlg_olavo_recomenda_parque_001`

## Pontos de interesse

| Tipo            | Local                          | Função                                |
|-----------------|--------------------------------|---------------------------------------|
| Cura            | Eunice (terminal)              | Cura HP completo (60 karma)           |
| Poções          | Eunice (terminal)              | Compra (40 karma média)               |
| Buff            | Eunice (terminal)              | Resistência temporária (80 karma)     |
| Mini-boss       | Praça central                  | O Gerente (depois clear)              |
| Lore principal  | Olavo (Edifício Comercial)     | Tese sobre karma                      |
| Documentos      | 4 espalhados                   | Lore corporativo                      |
| Transição S     | Saída sul da rua principal     | Door → Vila Rosário                   |
| Transição E     | Saída leste                    | Door → Parque Ibirapitanga (após boss)|
| Transição W     | Saída oeste                    | Door → Distrito Industrial (após boss)|

---

# Zone 2 — Parque Ibirapitanga

## Narrative summary

Grande parque urbano. A natureza absorveu karma e prosperou — árvores
gigantes, raízes invadindo asfalto, bioluminescência. Primeira pista
importante: karma não é puramente destrutivo. É energia. A natureza não
tem o filtro emocional que corrompe humanos.

Mini-boss: O Jardineiro — ex-funcionário fundido com a vegetação.
Momento pesado (ele morre em paz).

NPC chave: Dra. Marina (bióloga), refugiada na estação de guarda-parque.
Aliada potencial. Tom Direção 2.

## Layout suggestion (procedural)

Tilemap aproximado 80×80. Layout orgânico, irregular:

```
                 +-------+
                 | LAGO  |
                 |(biolum)|
                 +-------+
                     |
+----------------+   |   +----------------+
|  TRILHA OESTE  |   |   |  TRILHA LESTE  |
|  (cães)        |   |   |  (pássaros)    |
+----------------+   |   +----------------+
                 +---+---+
                 | MATA  |
                 | DENSA |
                 |(boss) |
                 +-------+
                     |
+----------------+---+---+----------------+
|  PLAYGROUND    |  ESTAÇÃO DE GUARDA     |
|  (insetos)     |  (Dra. Marina, save)   |
+----------------+------------------------+
                     |
              [Door W → Centro Velho]
```

Pontos de interesse:
- **Estação de guarda-parque:** hub do NPC Marina, save point
- **Lago:** centro do parque, bioluminescência forte
- **Mata densa:** arena do mini-boss
- **Trilhas:** spawn de inimigos diferentes

## Inimigos

### `enemy_corrupted_dog` — Cão Corrompido
- **Tier base:** 2
- **HP base:** 60
- **Damage base:** 12
- **AI:** Rápido, ataca em matilha (3-4 ao mesmo tempo). Cerca o player.
- **Karma drop:** 12
- **Comportamento:** flank → bite → reposition
- **Visual placeholder:** sprite de cachorro de rua tingido + partículas

### `enemy_corrupted_bird` — Pássaro Corrompido
- **Tier base:** 2
- **HP base:** 35
- **Damage base:** 8
- **AI:** Voa em círculos sobre o player. Picada vertical (mergulho).
  Difícil de acertar com melee.
- **Karma drop:** 10
- **Comportamento:** circle aerial → dive attack → retreat aerial
- **Visual placeholder:** sprite de pássaro de cidade (pombo, andorinha)
  ampliado e tingido

### `enemy_insect_swarm` — Enxame de Insetos
- **Tier base:** 1
- **HP base:** 80 (mas aplica dano em área constante)
- **Damage base:** 4 por tick (a cada 0.5s)
- **AI:** Persegue lentamente. Causa dano a quem fica perto.
- **Karma drop:** 15 (alto pra incentivar matar)
- **Comportamento:** slow chase → area damage zone follows enemy
- **Visual placeholder:** partícula de enxame, sem sprite individual

## Mini-boss: O Jardineiro

### Identidade narrativa

Ex-funcionário da prefeitura. Cuidava do parque há 30 anos. Quando o
karma estourou, sua conexão com as plantas literalizou — fundiu com
elas. Meio humano, meio orgânico. Não totalmente consciente, mas
sente. Quando derrotado, volta brevemente à consciência. Uma das
mortes mais pesadas do Ato 1.

### Stats

- **Tier:** 3
- **HP:** 500 (escala com effective_tier)
- **Damage:** 14
- **Phases:** 1 (mas com regen agressivo)

### Mecânicas

**Ataque 1 — Raízes Vivas** (cooldown 5s)
- 3 raízes saem do chão em posições próximas ao player
- Telegraph: marcadores no chão por 1s antes do ataque
- Dano: 14, knockback leve

**Ataque 2 — Esporos** (cooldown 8s)
- Libera nuvem de esporos numa área ao seu redor
- Causa dano contínuo (5/s) por 4s a quem ficar dentro
- Telegraph: visível, esporos verdes amarelados

**Ataque 3 — Cipó Puxador** (cooldown 12s)
- Cipó dispara em linha reta, puxa o player na direção dele
- Após puxar, golpe corpo a corpo (20 dano)

**Passive — Regeneração Vegetal**
- Regenera 8 HP/s enquanto não está sob efeito de fogo
- Lore: a vegetação ao redor o nutre
- **Dica de design:** spells de fogo (mage tree) anulam isso. Player que
  investiu em fogo tem vantagem clara.

### Arena

Mata densa. Espaço fechado por árvores enormes. Algumas raízes no chão
servem de cobertura/obstáculo. Bioluminescência ambiente.

### Drop

- 250 karma garantido
- Documento: `doc_diario_jardineiro`
- Ativa flag: `flag:parque_clear`

### Diálogo no início e fim

- **Início:** apenas grunhidos vegetais, raízes se movendo
- **Morte:** O Jardineiro cai. Raízes recuam. Por 5 segundos, ele volta
  à consciência. `dlg_jardineiro_morte_001`:

  PT: *"...as árvores. Tão felizes. Nunca tiveram... tanta energia. Eu
  cuidei delas trinta anos. Agora... agora elas cuidam de mim. Tá tudo
  bem. Tá tudo... bem."*

  EN: *"...the trees. They're happy. They've never had... this much
  energy. I cared for them thirty years. Now... now they care for me.
  It's okay. It's all... okay."*

  Morre em paz. Visualmente: o corpo se dissolve em pétalas e raízes
  que voltam à terra.

## NPCs

### `npc_marina` — Dra. Marina (bióloga)

**Função:** segunda aliada potencial. Cientista. Dá a primeira teoria
formal sobre o karma — energia neutra, destrutiva quando processada
por dor humana.

**Localização:** estação de guarda-parque, perto do mapa de campo
afixado na parede.

**Gatilhos de diálogo:**
- `dlg_marina_intro_001`
- `dlg_marina_teoria_karma_001` — explicação científica (Direção 2)
- `dlg_marina_oferece_servico_001`
- `dlg_marina_recrutamento_001` — após Jardineiro derrotado, ela quer ir
  junto pra estudar mais
- `dlg_marina_combat_*`
- `dlg_marina_zone_<id>_001`

**Serviço:** revelar mapa de zona próxima (60 karma). Lore: ela
conhece o terreno por pesquisa de campo.

### `npc_velho_andre` — Velho André (banco da praça)

**Função:** NPC menor. Lore local. Velho que mora há décadas perto do
parque. Tom Direção 1 — humor seco, observador. Não faz nada
mecanicamente, mas dá atmosfera.

**Localização:** banco perto do playground.

**Gatilhos de diálogo:**
- `dlg_andre_intro_001` — primeira fala
- `dlg_andre_observacao_001` — comentário sobre o parque pré-karma
  (PT: "esse parque era horrível, viu. mato alto, cheio de bicho. agora
  tá... bonito. esquisito.")

## Documentos coletáveis

### `doc_relatorio_marina_001`
- **Localização:** mesa da estação de guarda-parque
- **Conteúdo (PT):** Relatório de pesquisa da Dra. Marina, datado da
  semana do colapso. Observações de mudança de comportamento em
  pequenos mamíferos. Conclusão preliminar: "fauna está se adaptando
  a uma fonte de energia ambiental não identificada."
- **Conteúdo (EN):** Dr. Marina's research report, dated the week of
  the collapse. Behavioral observations on small mammals. Preliminary
  conclusion: "fauna is adapting to an unidentified environmental
  energy source."

### `doc_relatorio_marina_002`
- **Localização:** mesa da estação (segundo papel)
- **Conteúdo (PT):** Anotação posterior. "Hipótese revisada: a energia
  é processada de forma neutra pela vida não-consciente. Plantas
  prosperam. Animais se adaptam. **Apenas humanos quebram.** O
  filtro emocional parece ser o vetor de corrupção. Preciso testar
  isso, mas como?"
- **Conteúdo (EN):** Later note. "Revised hypothesis: the energy is
  processed neutrally by non-conscious life. Plants prosper. Animals
  adapt. **Only humans break.** The emotional filter appears to be
  the corruption vector. I need to test this, but how?"

### `doc_diario_jardineiro`
- **Localização:** drop após o boss
- **Conteúdo (PT):** Caderninho amassado. Últimas anotações do Jardineiro
  antes de fundir com o parque. "Sinto a vinda. Não é dor. As ipês
  estão mais fortes. As raízes pedem espaço. Acho que vou ficar com
  elas. Cuide dos jacarandás, são frágeis. Não corte os galhos novos."
- **Conteúdo (EN):** Crumpled notebook. The Jardineiro's last notes
  before fusing with the park. "I feel it coming. It's not pain. The
  ipês are stronger. The roots ask for space. I think I'll stay with
  them. Take care of the jacarandás, they're fragile. Don't cut the
  new branches."

### `doc_panfleto_corrida`
- **Localização:** quiosque desativado perto do lago
- **Conteúdo (PT):** Panfleto de uma corrida de rua que ia acontecer
  no parque. Patrocinada pela TecnoSul. Datada para uma semana após
  o colapso. Nunca aconteceu.
- **Conteúdo (EN):** Pamphlet for a street race that was supposed to
  happen at the park. Sponsored by TecnoSul. Dated for one week after
  the collapse. Never happened.

## Action events

### `event_primeira_visao_natureza` — A revelação

**Triggers:**
- `ZoneEnter:parque_ibirapitanga` AND NOT `flag:natureza_revelada`

**Steps:**
1. Cinematic curta: câmera lenta sobre raízes pulsando, flores
   bioluminescentes, partículas verdes-douradas
2. **Internal monologue:** `dlg_dante_natureza_001` (linha curta — Dante
   estranha que aqui é "bonito, esquisito")
3. **Effect:** `set_flag:natureza_revelada`

### `event_encontro_marina` — Aliada potencial

**Triggers:**
- `ZoneEnter:estacao_guarda_parque` AND NOT `flag:marina_conhecida`

**Steps:**
1. **Dialogue:** `dlg_marina_intro_001`
2. Player pode perguntar sobre o que ela tá pesquisando →
   `dlg_marina_teoria_karma_001`
3. **Effect:** `set_flag:marina_conhecida`
4. Marina libera serviço de revelação de mapa

### `event_arena_jardineiro` — O confronto pesado

**Triggers:**
- `flag:marina_conhecida` AND `ZoneEnter:mata_densa`

**Steps:**
1. **Sound cue:** sons orgânicos crescendo (raízes se movendo, folhas)
2. Spawn O Jardineiro emergindo das raízes
3. Combate
4. Ao morrer, animação de morte estendida (5s), sem skip permitido
5. **Dialogue (auto):** `dlg_jardineiro_morte_001`
6. Pétalas se dispersam, corpo desaparece
7. **Effect:** `set_flag:parque_clear`, `karma_add(250)` (drop), gain
   `doc_diario_jardineiro`
8. **Effect (delayed 5s):** silence ambient, partículas calmas
9. Music fades to soft

### `event_marina_recrutamento` — Aliada se junta

**Triggers:**
- `flag:parque_clear` AND `ZoneEnter:estacao_guarda_parque`

**Steps:**
1. Marina já no portão, esperando
2. **Dialogue:** `dlg_marina_recrutamento_001`
3. Player escolhe: aceitar / recusar
4. Se aceita: `recruit_ally:marina`, ela vira aliada disponível no
   roster (Sprint C7 implementa o sistema)
5. Se recusa: ela permanece como NPC de serviços

## Pontos de interesse

| Tipo            | Local                          | Função                                |
|-----------------|--------------------------------|---------------------------------------|
| Save point      | Estação de guarda-parque       | Save manual                           |
| Revelação mapa  | Marina (estação)               | Revela zona próxima (60 karma)        |
| Mini-boss       | Mata densa                     | O Jardineiro                          |
| Lore principal  | Marina (estação)               | Teoria do karma neutro                |
| Documentos      | 4 espalhados                   | Pesquisa + diário                     |
| Transição W     | Saída oeste                    | Door → Centro Velho                   |
| Pontos de cura  | Lago (passivo: HP regen lento perto do lago) | Recurso ambiental |

---

# Notas técnicas para C2 e C3

## Para implementação (Sprint C2)

Vila Rosário (Z0) e Centro Velho (Z1) implementados juntos:

1. **Z0 primeiro:** sem combate, foco em tutorial + scripted events.
   Validar fluxo de gatilho.
2. **Z1 segundo:** primeiros inimigos (3 tipos), primeiro mini-boss
   (O Gerente), primeiros documentos.
3. **Reaproveitar AI existente:** office_worker e security_guard usam
   `ai_combat` genérico. Delivery_corrupted requer dash — verificar se
   `ai_patrol` cobre, ou criar comportamento simples.
4. **Mini-boss O Gerente:** usar `ai_boss` existente como base. Mecânicas
   1, 2, 3 são variações de comportamentos já existentes (projétil em
   linha, summon, projétil radial).

## Para implementação (Sprint C3)

Parque Ibirapitanga (Z2) implementado depois de C2:

1. **Inimigos novos:** corrupted_dog (rápido), corrupted_bird (aéreo,
   provavelmente novo), insect_swarm (área, novo). Verificar engine
   pra suporte a inimigos aéreos.
2. **Mini-boss O Jardineiro:** mecânica de regen anulado por fogo é
   simples (flag de "im_fire_resistant" → false durante 4s após hit
   de fogo). Animação de morte estendida requer scripted event que
   trava input por 5s.
3. **Bioluminescência ambiente:** efeito visual procedural (partículas
   + light overlay). Não bloqueia gameplay se não funcionar perfeito.

## Decisões em aberto (resolver durante C2/C3)

- [ ] Renato ou Dante? "Dante" é apelido confirmado, mas o easter egg
      do `doc_receita_dona_celia` precisa ser checado pra coerência
      narrativa.
- [ ] Visual de O Gerente: terno blindado é metaforico ou literal? Na
      engine procedural, sprite tingido + tamanho aumentado pode bastar.
- [ ] Pássaros aéreos: a engine suporta inimigos voadores? Se não,
      converter `corrupted_bird` em "morcego que rasteja em árvores"
      (mesma mecânica de dive, sem voo real).
- [ ] Animação de morte estendida do Jardineiro: 5s é ideal mas pode ser
      cortado pra 2-3s se o tempo virar problema técnico.
