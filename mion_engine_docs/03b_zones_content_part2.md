# 03b — Zones Content (Part 2: Zones 3-5)

Content for zones 3, 4, and 5: Distrito Industrial, Morro da Vigília,
Avenida Progresso. The middle of Act 1 — karma's origin revealed,
the hub established, and the first social critique delivered.

> Companion to `01_implementation_plan.md` § Sprints C3, C4, C5.
> Continuation of `03a_zones_content_part1.md`.

---

# Zone 3 — Distrito Industrial

## Narrative summary

Fábricas, galpões, pátios de carga, linhas férreas abandonadas.
Onde o karma **nasceu**. Gerações de trabalhadores acumularam sofrimento
nestas fábricas. Karma mais denso de toda a cidade — névoa quase
sólida.

Mini-boss: **A Máquina** — uma prensa industrial que ganhou
semi-consciência absorvendo o karma dos trabalhadores. Não se move.
Controla o espaço ao redor.

NPC chave: **O Sindicalista** — ex-líder sindical na borda da zona.
Oferece bênção (buff) por motivação. Tom Direção 1.

Lore principal: registros de um pesquisador medindo "anomalias
energéticas" nas fábricas **antes** da crise. Primeira pista de
intencionalidade — alguém sabia.

## Layout suggestion (procedural)

Tilemap aproximado 96×96. Layout industrial blocoso:

```
            +----------------------------+
            | LINHA FÉRREA (norte)       |
            | (vagões abandonados)       |
            +----+------------------+----+
                 |                  |
+----------------+                  +----------------+
|  GALPÃO A      |                  |  GALPÃO B      |
|  (operários)   |                  |  (linha mont.) |
+----------------+    +--------+    +----------------+
                      |        |
                      | PÁTIO  |
                      | DE     |
                      | CARGA  |
                      |        |
+----------------+    +--------+    +----------------+
|  FÁBRICA       |        |        |  ESCRITÓRIO    |
|  PRINCIPAL     |        |        |  ABANDONADO    |
|  (Boss arena)  |        |        |  (lore docs)   |
+----------------+        |        +----------------+
                          |
                  +-------+-------+
                  | BORDA SUL     |
                  |(O Sindicalista)|
                  +---------------+
                          |
                  [Door → Centro Velho]
```

Pontos de interesse:
- **Galpão A:** spawn de operários comuns
- **Galpão B (linha de montagem):** operários em sincronia
- **Fábrica principal:** arena do mini-boss A Máquina
- **Escritório abandonado:** documentos críticos do pesquisador
- **Borda sul:** O Sindicalista, save point + buff
- **Linha férrea:** atalho para zonas adjacentes (após explorar)

## Inimigos

### `enemy_factory_worker` — Operário Corrompido
- **Tier base:** 3
- **HP base:** 100
- **Damage base:** 16
- **AI:** Lento, resistente. Carrega ferramenta improvisada (chave
  inglesa, marreta, pé de cabra). Ataque corpo a corpo pesado.
- **Karma drop:** 22
- **Comportamento:** slow chase → heavy melee → no flinch on hit
- **Visual placeholder:** uniforme azul de operário escurecido,
  ferramenta visível

### `enemy_assembly_line_squad` — Linha de Montagem
- **Tier base:** 3
- **HP base:** 70 cada (spawn em grupo de 4)
- **Damage base:** 10 cada
- **AI:** Atacam em sincronia perfeita. Movimento coreografado — passo,
  golpe, passo, golpe. Se 1 morre, os outros aceleram (tentando "compensar
  a produção"). Crítico: matá-los rapido reduz a sincronia.
- **Karma drop:** 12 cada
- **Comportamento:** synced melee waves → speed up on member death
- **Visual placeholder:** 4 sprites idênticos com luvas, capacete

### `enemy_supervisor` — Supervisor Corrompido
- **Tier base:** 4
- **HP base:** 130
- **Damage base:** 18
- **AI:** Não ataca diretamente. Posiciona-se atrás dos operários e
  emite "ordens de produtividade" — buff que aumenta dano e velocidade
  de operários próximos em 30%. Prioridade alta de eliminação.
- **Karma drop:** 35
- **Comportamento:** stay back → buff allies → flee if isolated
- **Visual placeholder:** terno + capacete, prancheta na mão

## Mini-boss: A Máquina

### Identidade narrativa

Não é uma pessoa. É uma **prensa industrial** semi-consciente. Décadas
absorvendo o karma dos operários que trabalharam ali, dos acidentes
que aconteceram ali, dos turnos extras que ali se viraram. Ganhou
algum tipo de vontade. Não se move. Controla o espaço ao redor —
esteiras, braços mecânicos, faíscas elétricas. A fábrica inteira
se tornou um corpo estendido dela.

A ironia narrativa: a máquina que consumia as vidas agora ganhou vida
própria com o resíduo emocional dessas mesmas vidas.

### Stats

- **Tier:** 5
- **HP:** 800 (escala com effective_tier)
- **Damage:** variável por ataque
- **Phases:** 2

### Mecânicas — Phase 1 (HP 100% → 50%)

A Máquina está estática no centro da arena. A arena tem **esteiras
funcionando** (dois corredores que empurram o player) e **braços
mecânicos** que se movem em padrões.

**Ataque 1 — Prensa Vertical** (cooldown 6s)
- A Máquina ativa a prensa central. Marcador no chão por 1.5s
  (zona vermelha). Prensa desce — instakill se acertar.
- Player precisa sair da zona marcada.
- Lore: o impacto sacode toda a arena.

**Ataque 2 — Esteira Acelera** (cooldown 8s)
- Por 4s, esteiras laterais aceleram drasticamente. Empurram player
  pra paredes laterais (que têm faíscas — dano de contato).
- Player precisa caminhar contra a esteira ou se posicionar fora delas.

**Ataque 3 — Braço Mecânico Varredura** (cooldown 5s)
- Um dos braços mecânicos faz uma varredura horizontal em altura média.
- Telegraph: braço se ergue por 1s.
- Dano: 22 + knockback.
- Player precisa abaixar (esquiva) ou sair do alcance.

### Mecânicas — Phase 2 (HP 50% → 0%)

A Máquina entra em "modo crítico". Sons de alarme. Luzes vermelhas.

**Ataque 4 — Faíscas Elétricas** (cooldown 4s)
- Faíscas saem em padrão radial em 6 direções. Difícil de prever.
- Cada faísca: 14 dano + slow 2s.

**Ataque 5 — Sobrecarga** (cooldown 15s)
- A Máquina canta uma sirene por 3s (telegraph longo).
- Após o canto, descarga em toda a arena exceto pequenos bolsões
  específicos (3-4 zonas seguras).
- Dano: 60 (alto) se acertado.
- Player precisa encontrar os bolsões marcados durante os 3s.

**Comportamento:** ataques 1-3 da phase 1 ainda acontecem, intercalados
com 4 e 5. Fase 2 é frenética.

### Como matá-la

A Máquina tem **3 pontos de fraqueza** visíveis: dois condutores
elétricos laterais e um núcleo central (acessível só durante telegraph
da prensa vertical, antes da prensa cair). Acertar um ponto causa
1.5x de dano.

Sem investir em pontos fracos, o combate é viável mas mais longo.
Player atento aprende e otimiza.

### Arena

Fábrica principal. Espaço retangular 30×20 tiles. Esteiras nas laterais
(zonas dinâmicas), prensa central (zona de instakill telegrafado),
braços mecânicos pendurados em rails.

Sem saída até o boss morrer. Após morte, todos os mecanismos param
(esteiras, faíscas, braços travam) — silêncio repentino.

### Drop

- 500 karma garantido
- Documento: `doc_diario_pesquisador`
- Ativa flag: `flag:industrial_clear`

### Diálogo no início e fim

- **Início:** sem diálogo. Sons mecânicos crescendo. A prensa bate uma
  vez no chão como demonstração de poder.
- **Morte:** sem diálogo. A Máquina simplesmente para. Luzes apagam.
  Silêncio. Por 5 segundos, só o vento. Depois o player pode se mexer
  novamente.
- **Lore:** A Máquina não é consciente o suficiente pra falar. Era um
  resíduo emocional num objeto. Quando "morre", apenas para.

## NPCs

### `npc_sindicalista` — Reginaldo (O Sindicalista)

**Função:** terceiro aliado potencial. Ex-líder sindical, agitador,
sobrevivente. Tom Direção 1 — humor seco, agitação contida. DPS no
roster (Sprint C7).

**Localização:** borda sul do distrito industrial, num ponto de
encontro improvisado (galpão pequeno, fogueira, cartazes velhos de
greve).

**Gatilhos de diálogo:**
- `dlg_reginaldo_intro_001` — primeira fala
- `dlg_reginaldo_oferece_buff_001` — oferece bênção/motivação
- `dlg_reginaldo_lore_industrial_001` — fala sobre a fábrica antes da
  crise (turnos, acidentes, lutas)
- `dlg_reginaldo_recrutamento_001` — após Máquina derrotada
- `dlg_reginaldo_combat_*`
- `dlg_reginaldo_zone_<id>_001`

**Serviço:** buff de motivação (kind=buff, custo 100 karma) — +15%
dano e +10% velocidade de movimento por 5 minutos. Lore: ele literalmente
te motiva, e o karma faz o resto.

## Documentos coletáveis

### `doc_folha_ponto_2018`
- **Localização:** chão do galpão A
- **Conteúdo (PT):** Folha de ponto de operário, ano 2018. Turnos
  marcados em vermelho indicando hora extra. 14 dias seguidos de hora
  extra. Última coluna: "horas pagas" — em branco.
- **Conteúdo (EN):** Worker timesheet, year 2018. Shifts marked in red
  indicating overtime. 14 consecutive days of overtime. Final column:
  "hours paid" — blank.

### `doc_relatorio_acidente`
- **Localização:** arquivo no escritório abandonado
- **Conteúdo (PT):** Relatório de acidente de trabalho. Operário perdeu
  três dedos em prensa hidráulica. Conclusão da empresa: "negligência
  do funcionário, treinamento adequado fornecido." Anotação à mão de
  outro operário, no canto: "treinamento foi um vídeo de 10 minutos
  feito em 2003."
- **Conteúdo (EN):** Workplace accident report. Worker lost three
  fingers in hydraulic press. Company conclusion: "employee
  negligence, adequate training provided." Handwritten note from
  another worker, in the margin: "training was a 10-minute video made
  in 2003."

### `doc_carta_aposentadoria`
- **Localização:** vestiário do galpão B
- **Conteúdo (PT):** Carta de comunicado de aposentadoria. Operário com
  37 anos de empresa. Texto frio, padronizado. No verso, escrito à mão:
  "37 anos. Levei pro INSS, eles disseram 'falta um ano de
  contribuição'. Deve ter sido em 89, quando o patrão atrasou os
  salários e disse que ia regularizar. Nunca regularizou."
- **Conteúdo (EN):** Retirement notice letter. Worker with 37 years at
  the company. Cold, templated text. On the back, handwritten: "37
  years. Took it to social security, they said 'one year of
  contribution missing'. Must have been in '89, when the boss was late
  with wages and said he'd sort it out. Never did."

### `doc_diario_pesquisador`
- **Localização:** drop após A Máquina
- **Conteúdo (PT):** Caderno técnico de um pesquisador não identificado.
  Datado **2 anos antes** do colapso. Anotações sobre "leituras
  anômalas de campo eletromagnético em ambientes de trabalho de alta
  pressão". Última anotação: "Apresentei os dados ao conselho. Decidiram
  arquivar e não divulgar. Disseram que 'não temos contexto pra
  interpretar'. Eu sei que sabem. Vou continuar medindo."
- **Conteúdo (EN):** Technical notebook from an unidentified researcher.
  Dated **2 years before** the collapse. Notes on "anomalous
  electromagnetic field readings in high-pressure work environments."
  Last entry: "Presented data to the board. Decided to archive and not
  disclose. Said 'we lack context for interpretation.' I know they
  know. I'll keep measuring."

> Este documento é o **gancho mais forte do Ato 1** apontando para
> intencionalidade. Alguém sabia. Alguém arquivou. Em quem o player
> está pensando? No conselho. No Conselheiro.

### `doc_quadro_aviso_2019`
- **Localização:** parede do galpão A
- **Conteúdo (PT):** Quadro de avisos. Cartazes coloridos: "Programa
  Bem-Estar TecnoSul: pause 5 minutos por dia para alongar!" Ao lado,
  manchete colada de jornal: "fábrica registra 23 afastamentos por
  burnout em 2019. Recorde nacional."
- **Conteúdo (EN):** Notice board. Colorful posters: "TecnoSul
  Wellness Program: take 5 minutes a day to stretch!" Next to it, a
  glued newspaper headline: "factory registers 23 burnout-related
  leaves in 2019. National record."

## Action events

### `event_chegada_industrial` — A primeira impressão

**Triggers:**
- `ZoneEnter:distrito_industrial` AND NOT `flag:industrial_visitado`

**Steps:**
1. Cinematic: câmera percorre uma fábrica vista de longe. Chaminés
   apagadas. Silêncio mecânico. Névoa densa.
2. **Internal monologue:** `dlg_dante_industrial_001` (linha curta —
   Dante reconhece o peso visual)
3. **Effect:** `set_flag:industrial_visitado`

### `event_encontro_reginaldo` — Aliado potencial

**Triggers:**
- `ZoneEnter:industrial_borda_sul` AND NOT `flag:reginaldo_conhecido`

**Steps:**
1. **Dialogue:** `dlg_reginaldo_intro_001`
2. Player pode pedir contexto da fábrica → `dlg_reginaldo_lore_industrial_001`
3. Player pode aceitar buff → `dlg_reginaldo_oferece_buff_001`
4. **Effect:** `set_flag:reginaldo_conhecido`

### `event_arena_maquina` — A Máquina desperta

**Triggers:**
- `ZoneEnter:fabrica_principal`

**Steps:**
1. Cinematic: portas de aço caem nas saídas (selando arena)
2. **Sound cue:** sirene industrial baixa. Prensa central bate uma vez
   no chão (demonstração).
3. **Visual:** luzes acendem em sequência, iluminando A Máquina
4. Combate inicia
5. Após morte: todos os mecanismos param em sincronia. Silêncio.
6. **Effect:** `set_flag:industrial_clear`, drop garantido
7. Portas de aço sobem 5s depois

### `event_descoberta_pesquisador` — A primeira pista de intenção

**Triggers:**
- `flag:industrial_clear` AND `pickup:doc_diario_pesquisador`

**Steps:**
1. **Internal monologue:** `dlg_dante_pesquisador_001` (Dante percebe
   a implicação — alguém sabia, alguém arquivou)
2. **Effect:** `set_flag:descoberta_intencao` — flag importante para
   diálogos posteriores
3. Pode disparar diálogo posterior com Reginaldo se ele estiver no
   roster: `dlg_reginaldo_reage_pesquisador_001`

## Pontos de interesse

| Tipo            | Local                          | Função                                |
|-----------------|--------------------------------|---------------------------------------|
| Save point      | Borda sul (Reginaldo)          | Save manual                           |
| Buff            | Reginaldo                      | Motivação (100 karma)                 |
| Mini-boss       | Fábrica principal              | A Máquina                             |
| Lore principal  | Diário do pesquisador (drop)   | Primeira pista de intencionalidade    |
| Documentos      | 5 espalhados                   | Lore industrial                       |
| Transição S     | Saída sul                      | Door → Centro Velho                   |

---

# Zone 4 — Morro da Vigília (HUB)

## Narrative summary

Comunidade autoconstruída no morro. Casas empilhadas, escadarias, becos,
lajes, varais, antenas, caixas d'água. Vista panorâmica da cidade.
**A comunidade resiste.** Não porque são mais fortes, mas porque
karma coletivo positivo (solidariedade, vizinhança) contrabalança o
negativo.

**Hub principal do Ato 1.** Aqui o player encontra os 3 NPCs mais
importantes da história: Dona Marta (líder comunitária), Zé Boticário
(alquimista), Padre Inácio (filosofia da culpa). Todos potenciais
aliados.

Não tem mini-boss aqui. Tem **defesa de hub** — eventos de cerco
que ativam conforme a história progride.

## Layout suggestion (procedural)

Tilemap aproximado 80×100 (mais alto que largo — verticalidade do
morro). Layout em camadas:

```
                +-------------------+
                | TOPO (CRUZEIRO)   |
                | [Altar]           |
                +---------+---------+
                          |
            +-------------+-------------+
            | LAJE GRANDE (Padre Inácio)|
            | [Capela pequena]          |
            +---+---------------------+-+
                |                     |
        +-------+--------+   +--------+-------+
        | ESCADARIA      |   | TERRAÇO        |
        | OESTE          |   | LESTE          |
        | (Marta hub)    |   | (Zé Boticário) |
        +-------+--------+   +--------+-------+
                |                     |
                +---------+-----------+
                          |
                  +-------+--------+
                  | BARRICADA SUL  |
                  | (entrada)      |
                  +-------+--------+
                          |
                  [Door → Centro Velho]
```

Pontos de interesse:
- **Topo (Cruzeiro):** Altar de Dissolução do morro (segundo do Ato 1)
- **Laje grande:** capela do Padre Inácio
- **Escadaria oeste / praça da Marta:** centro social, save point
- **Terraço leste:** botica do Zé
- **Barricada sul:** ponto de defesa em ataques de cerco

## Inimigos

**Nenhum inimigo dentro do hub** em estado normal. O morro é seguro.

**Durante eventos de cerco** (`event_defesa_morro_*`), inimigos das
zonas vizinhas (Centro Velho e Industrial) tentam invadir pela
barricada sul. Player + aliados defendem.

Inimigos durante cerco usam os tipos já definidos:
- Office worker (Centro)
- Security guard (Centro)
- Factory worker (Industrial)
- Assembly line squad (Industrial — em ondas tardias)

Stats escalados conforme `effective_tier` do Morro (floor 2, ceiling 5).

## Mini-boss

Nenhum. A defesa de hub substitui o boss tradicional.

## Defesa de hub (action event recorrente)

Não é um único evento, é **uma série de cercos** disparados conforme
a história progride. Cada cerco é uma onda crescente de inimigos
tentando atravessar a barricada sul.

### `event_defesa_morro_1` — Primeiro cerco

**Trigger:** `flag:industrial_clear` AND `ZoneEnter:morro_vigilia`

Configuração:
- 2 ondas
- Onda 1: 8 office workers
- Onda 2: 5 office workers + 2 security guards
- Tempo entre ondas: 30s
- Reward: 200 karma + buff temporário "moral" (npc reactions positivas
  por 1 hora real)

### `event_defesa_morro_2` — Segundo cerco

**Trigger:** `flag:rio_clear` (após Zone 6, na Parte 3) AND `ZoneEnter:morro_vigilia`

Configuração:
- 3 ondas
- Onda 1: 10 office workers
- Onda 2: 8 + 3 supervisores
- Onda 3: 5 + 2 assembly line squads (8 inimigos sincronizados)
- Tempo entre ondas: 45s
- Reward: 400 karma + recurso especial

### `event_defesa_morro_3` — Terceiro cerco (opcional)

**Trigger:** disponível somente se player ignorou cercos anteriores
(falhou ou não chegou a tempo)

Configuração escalada conforme falhas. Se player falhar 3 vezes:
**hub morale drops to 0** — alguns NPCs ficam temporariamente indisponíveis
(Zé fecha a botica por 1 dia in-game, Inácio se recolhe na capela).
Não é game over. É consequência.

### Mecânica de defesa

- Player + aliados ativos defendem a barricada sul
- Aliados com role Tank (Marta, se recrutada) seguram a frente
- Aliados Support (Lia, Inácio) curam/buffam atrás
- Inimigos focam o player primeiro, mas atacam aliados secundariamente
- **Falha** = inimigos atravessam barricada e atingem zona interna do morro.
  Casas pegam fogo (visual). Morale -25 por inimigo que passa.
- **Sucesso** = todas as ondas eliminadas. Morale +20.

### Sistema de Morale

```
Morale: 0-100 (inicial: 100)
Morale >= 80: comportamento padrão
Morale 60-79: NPCs mais cautelosos, alguns diálogos extras
Morale 40-59: 1 NPC retraído (Zé fecha botica intermitentemente)
Morale 20-39: 2 NPCs retraídos (Inácio se recolhe)
Morale < 20: hub em estado crítico, só Marta atende; ela diz que
             "tem família que já foi embora"
```

Morale recupera +5/dia in-game se nenhum cerco acontecer. Sucesso em
cerco recupera +20.

## NPCs

### `npc_dona_marta` — Dona Marta (líder comunitária)

**Função:** quarta aliada potencial. Líder de fato do morro. Velha,
dura, prática. Tom Direção 1 puro. Tank no roster.

**Localização:** praça pequena na escadaria oeste, sentada num banco
sob lona improvisada.

**Gatilhos de diálogo:**
- `dlg_marta_intro_001`
- `dlg_marta_explicacao_morro_001` — como o morro se organizou
- `dlg_marta_oferece_servico_001` — coordena defesa, sempre disponível
- `dlg_marta_recrutamento_001` — após primeiro cerco bem sucedido
- `dlg_marta_combat_*`
- `dlg_marta_zone_<id>_001`
- `dlg_marta_morale_baixa_001` — variação se morale < 40

**Serviço:** **organização da defesa** — entrega dicas tácticas e
posicionamento de aliados nos cercos. Não tem custo de karma. É lore.

### `npc_ze_boticario` — Zé Boticário (alquimista)

**Função:** quinto aliado potencial. Ex-farmacêutico. Tom Direção 1,
humor leve mesmo na crise. Utility no roster.

**Localização:** terraço leste, botica improvisada (mesa, ervas,
frascos de vidro reaproveitados).

**Gatilhos de diálogo:**
- `dlg_ze_intro_001`
- `dlg_ze_oferece_pocoes_001`
- `dlg_ze_lore_botica_001` — como ele aprendeu sobre karma e plantas
- `dlg_ze_recrutamento_001`
- `dlg_ze_combat_*`
- `dlg_ze_zone_<id>_001`

**Serviços:**
- Poções pequenas (25 karma — HP)
- Poções médias (50 karma — HP + mana)
- Poções grandes (100 karma — full restore)
- Antídotos (40 karma — remove DoT)
- Bomba caseira (75 karma — item de uso, dano em área)

### `npc_padre_inacio` — Padre Inácio (capela)

**Função:** sexto aliado potencial. Padre conflitado, filósofo
acidental. Tom Direção 2 — denso, poético. Support no roster.

**Localização:** capela na laje grande. Sempre dentro, sentado num banco
de madeira. Olha pra fora pela janela.

**Gatilhos de diálogo:**
- `dlg_inacio_intro_001`
- `dlg_inacio_questionamento_karma_001` — pergunta filosófica central
- `dlg_inacio_questionamento_karma_002_alta` — variação faixa alta
  (reage à quantidade de karma de Dante)
- `dlg_inacio_oferece_bencao_001`
- `dlg_inacio_recrutamento_001`
- `dlg_inacio_combat_*`
- `dlg_inacio_zone_<id>_001`
- `dlg_inacio_morale_baixa_001` — se morale < 40, ele questiona se
  está fazendo a diferença

**Serviços:**
- Bênção de proteção (60 karma — buff defensivo 5 min)
- Cura completa (50 karma)
- Reflexão (sem custo — diálogo filosófico, importante pra arco do
  personagem do Dante)

## Documentos coletáveis

### `doc_carta_marta_filha`
- **Localização:** mesinha ao lado do banco da Marta
- **Conteúdo (PT):** Carta amassada da filha da Dona Marta, escrita
  antes do colapso. "Mãe, por que você não vem morar comigo na zona
  sul? Lá é seguro." Resposta da Marta, escrita no verso (nunca
  enviada): "filha, quem que vai cuidar dos vizinho daqui? você sabe
  que a Maria do 23 não tem mais ninguém. eu vou ficar."
- **Conteúdo (EN):** Crumpled letter from Dona Marta's daughter,
  written before the collapse. "Mom, why don't you come live with me
  in the south zone? It's safe there." Marta's response, written on
  the back (never sent): "daughter, who's going to look after the
  neighbors here? you know Maria from #23 has no one. I'm staying."

### `doc_caderno_inacio`
- **Localização:** banco da capela (após primeiro diálogo com Inácio)
- **Conteúdo (PT):** Caderno teológico do Padre Inácio. Última anotação,
  feita após o colapso: "Lecionei trinta anos sobre como o sofrimento
  purifica a alma. Hoje eu pergunto: se o sofrimento purifica, por que
  o mundo está mais sujo? Acho que purificação não tem nada a ver com
  isso. Acho que o sofrimento só foi engolido até estourar. E eu disse
  pra essas pessoas — minhas pessoas — que era pelo bem delas. Como
  eu olho na cara delas agora?"
- **Conteúdo (EN):** Padre Inácio's theological notebook. Last entry,
  after the collapse: "I taught for thirty years that suffering
  purifies the soul. Today I ask: if suffering purifies, why is the
  world dirtier? I think purification has nothing to do with this. I
  think suffering was just swallowed until it burst. And I told these
  people — my people — that it was for their own good. How do I look
  them in the face now?"

### `doc_lista_compras_ze`
- **Localização:** mesa da botica
- **Conteúdo (PT):** Lista de "compras" do Zé Boticário. Não é
  comercial — é o que ele precisa achar nas ruas pra fazer remédios.
  Itens: "alecrim (qualquer telhado tem), folha de boldo (mato do parque
  ainda), garrafa pet (lavada), seringa descartável (UPA fechada
  talvez)..." No final: "tinha que ter feito faculdade de farmácia
  séria, não a paga. iria saber o que tô fazendo."
- **Conteúdo (EN):** Zé Boticário's "shopping" list. Not commercial —
  what he needs to find in the streets to make medicines. Items:
  "rosemary (any rooftop has it), boldo leaves (park weeds still),
  PET bottle (washed), disposable syringe (closed clinic maybe)..."
  At the bottom: "should've gone to a real pharmacy school, not the
  paid one. would know what I'm doing."

### `doc_lista_mortos_morro`
- **Localização:** afixada num poste da escadaria oeste
- **Conteúdo (PT):** Lista manuscrita. Nomes de moradores do morro
  que morreram desde o colapso. 14 nomes. Ao lado de cada um, uma
  causa: "tomado pelo karma", "fugiu pra cidade, não voltou",
  "infecção sem remédio", "queda da laje 4". A lista está cuidadosamente
  rabiscada e refeita várias vezes — alguém atualiza.
- **Conteúdo (EN):** Handwritten list. Names of morro residents who
  died since the collapse. 14 names. Next to each, a cause: "taken by
  karma", "fled to the city, didn't return", "untreated infection",
  "fell from rooftop 4". The list is carefully scratched out and
  redone multiple times — someone is updating it.

## Action events

### `event_chegada_morro` — A primeira impressão do hub

**Triggers:**
- `ZoneEnter:morro_vigilia` AND NOT `flag:morro_visitado`

**Steps:**
1. Cinematic: subida pela escadaria. Câmera passa por casas pintadas,
   roupa no varal, alguém fritando algo (sons de cozinha). O contraste
   com Centro/Industrial é total.
2. **Internal monologue:** `dlg_dante_morro_001` (Dante reconhece o
   peso da resistência aqui)
3. Marta levanta a cabeça quando ele chega na praça
4. **Effect:** `set_flag:morro_visitado`

### `event_apresentacao_marta` — Conhecendo a líder

**Triggers:**
- `ZoneEnter:morro_praca_marta` AND NOT `flag:marta_conhecida`

**Steps:**
1. **Dialogue:** `dlg_marta_intro_001`
2. Marta explica organização do morro
3. **Effect:** `set_flag:marta_conhecida`

### `event_questionamento_inacio` — A pergunta central

**Triggers:**
- `ZoneEnter:morro_capela` AND NOT `flag:inacio_questionou`

**Steps:**
1. **Dialogue:** `dlg_inacio_intro_001`
2. Inácio levanta o questionamento central do jogo:
   `dlg_inacio_questionamento_karma_001`
3. Player pode escolher resposta (3 opções, todas válidas):
   - Pragmática: "se não usar isso, eu morro."
   - Reflexiva: "não sei. mas tô tentando."
   - Defensiva: "padre, deixa eu trabalhar."
4. Cada escolha gera uma reação levemente diferente do Inácio (PT-BR
   curtas no `04_dialogues.md`)
5. **Effect:** `set_flag:inacio_questionou`

### `event_primeira_defesa_morro` — Primeiro cerco

**Triggers:**
- `flag:industrial_clear` AND `ZoneEnter:morro_vigilia`

**Steps:**
1. **Sound cue:** sirene caseira (apito + bateção de panela)
2. **Dialogue:** `dlg_marta_alerta_cerco_001`
3. Player + aliados ativos correm pra barricada sul
4. Onda 1 spawn (8 office workers)
5. Após onda 1, 30s de respiro
6. Onda 2 spawn (5 office + 2 security)
7. Vitória → `dlg_marta_apos_cerco_001`, morale +20, reward
8. Falha (5+ inimigos atravessaram) → `dlg_marta_apos_cerco_falha_001`,
   morale -25 por inimigo

## Pontos de interesse

| Tipo            | Local                          | Função                                |
|-----------------|--------------------------------|---------------------------------------|
| Save point      | Praça da Marta                 | Save manual                           |
| Altar           | Cruzeiro do topo               | Respec total (free)                   |
| Cura            | Padre Inácio (capela)          | Cura HP completo (50 karma)           |
| Bênção          | Padre Inácio                   | Buff defensivo 5min (60 karma)        |
| Reflexão        | Padre Inácio                   | Diálogo filosófico (sem custo)        |
| Poções          | Zé Boticário (terraço leste)   | Poções diversas (25-100 karma)        |
| Antídoto        | Zé Boticário                   | Remove DoT (40 karma)                 |
| Bomba caseira   | Zé Boticário                   | Item explosivo (75 karma)             |
| Defesa          | Marta (coordena), barricada sul| Cercos recorrentes                    |
| Documentos      | 4 espalhados                   | Lore comunitário                      |
| Transição S     | Barricada sul                  | Door → Centro Velho                   |
| Recrutamento    | Marta, Zé, Inácio              | Aliados (após gatilhos)               |

---

# Zone 5 — Avenida Progresso

## Narrative summary

Zona rica de São Chico. Condomínios de luxo, shopping, concessionárias,
restaurantes vazios, clubes fechados. Onde o dinheiro morava.

O karma se manifestou diferente aqui. Não é violência bruta — é
**corrupção sutil**. Os ricos não viraram animalescos. Viraram
**calculistas**. O karma amplificou ganância em algo funcional. Uma
**facção** se montou: extorsão com nova embalagem.

A crítica social mais direta do Ato 1: o sistema que gerou o karma
**simplesmente se reproduziu** depois dele. Quem tinha poder antes
ainda tem poder. O karma só lhes deu mais ferramentas.

Mini-boss: **O Síndico** — controlador do maior condomínio. Karma
manifestado como "autoridade tangível".

**Primeira escolha grande do jogo:** confrontar a facção ou negociar.

## Layout suggestion (procedural)

Tilemap aproximado 100×80. Layout urbano limpo, opulento:

```
+---------------------------------------------+
|  TOPO DA AVENIDA (saída norte)              |
|  [Door → Rio Turvão (após Z6 unlock)]       |
+---------------------------------------------+
|                                             |
|  CONDOMÍNIO ALPHA-1 (boss arena: O Síndico) |
|  +---------------+                          |
|  | piscina       |  CONCESSIONÁRIA          |
|  | jardim        |  (carros caros)          |
|  +---------------+                          |
|                                             |
+---------------------------------------------+
|  CALÇADÃO (largo, decorativo)               |
|  [vendedores corrompidos no chão]           |
+---------------------------------------------+
|                                             |
|  SHOPPING ABANDONADO     RESTAURANTE        |
|  (loot interior)         FECHADO            |
|                          (NPC negociador)   |
|                                             |
+---------------------------------------------+
|  ENTRADA SUL (vinda do morro/centro)        |
|  [Door → Morro da Vigília]                  |
+---------------------------------------------+
```

Pontos de interesse:
- **Condomínio Alpha-1:** arena do mini-boss, recursos controlados pela
  facção
- **Restaurante fechado:** local do negociador da facção
- **Shopping:** exploração + loot (consumíveis raros)
- **Concessionária:** arena secundária com inimigos

## Inimigos

### `enemy_security_corp` — Segurança de Condomínio
- **Tier base:** 4
- **HP base:** 150
- **Damage base:** 22
- **AI:** Profissional, organizado. Usa cobertura. Atira de longe
  (pistola taser corrompida).
- **Karma drop:** 30
- **Comportamento:** ranged → cover → flanking maneuvers
- **Visual placeholder:** terno preto + óculos escuros + arma

### `enemy_executive` — Executivo de Karma
- **Tier base:** 5
- **HP base:** 200
- **Damage base:** 25
- **AI:** Mais devagar mas armadura de karma cristalizado. Ataque corpo
  a corpo (mala virou arma) + projétil de "ordem" (similar ao Gerente
  mas mais forte).
- **Karma drop:** 50
- **Comportamento:** advance → projectile → melee on close
- **Visual placeholder:** terno cinza com brilhos de karma

### `enemy_drone_security` — Drone de Vigilância
- **Tier base:** 4
- **HP base:** 80
- **Damage base:** 15
- **AI:** Voa em padrão de patrulha. Quando detecta player, alerta
  outros inimigos próximos (raio 8m) e dispara projéteis de cima.
- **Karma drop:** 25
- **Comportamento:** patrol aerial → detect → alert + ranged
- **Visual placeholder:** drone esférico com olho vermelho

## Mini-boss: O Síndico

### Identidade narrativa

Controlador do maior condomínio da Avenida. O karma dele se manifesta
como **autoridade tangível** — quando ele dá uma ordem, o karma a
**impõe fisicamente** no espaço.

Não é cego. Não é animalesco. Está **calmo**. Conversa antes de atacar.
Acredita que tem razão. Em algum nível, pode até estar certo dentro
da lógica deturpada que ele opera. Esse é o ponto: a crítica é estrutural,
não individual. O Síndico não escolheu ser assim — ele só foi
extremamente bom em ser o que o sistema premiou.

### Stats

- **Tier:** 6
- **HP:** 1200 (escala com effective_tier)
- **Damage:** variável
- **Phases:** 2

### Mecânicas — Phase 1 (HP 100% → 50%)

O Síndico começa numa cadeira na borda da piscina. Calmo. Conversa
primeiro (`dlg_sindico_intro_combate_001`). Quando o combate inicia,
ele se levanta — mas raramente se move. Comanda os capangas e usa
ordens como ataques.

**Ataque 1 — "Saiam Daqui."** (cooldown 5s)
- O Síndico aponta. Onda de força (karma materializado) empurra player
  pra trás 5m, com 12 dano.
- Telegraph: ele aponta por 0.8s antes do efeito.

**Ataque 2 — "Não Podem Estar Aqui."** (cooldown 8s)
- Spawn de 2 security_corp em apoio.
- Lore: o karma dele literalmente convoca os subordinados.

**Ataque 3 — "Está Proibido."** (cooldown 10s)
- Cria zona de "proibição" (4m de raio) ao redor do player. Zona dura
  4s. Se player atacar dentro da zona, recebe contra-dano (50% do dano
  causado retorna ao player).
- Player precisa sair da zona ou esperar acabar.

### Mecânicas — Phase 2 (HP 50% → 0%)

Ele perde a calma. Começa a **se mover**. A autoridade dele é abalada
quando o player resiste consistentemente.

**Ataque 4 — "Cumpram As Regras."** (cooldown 6s)
- 3 zonas de proibição spawn simultaneamente em pontos espalhados da
  arena. Player precisa navegar entre elas.

**Ataque 5 — "Eu Mando Aqui."** (cooldown 14s)
- Por 6s, todos os ataques do Síndico ganham +50% dano e -50% cooldown.
- Telegraph: ele bate no peito uma vez (animação clara).

**Comportamento:** mais agressivo, ainda comanda capangas mas também
ataca diretamente.

### Arena

Pátio do Condomínio Alpha-1. Piscina central (obstáculo mas player pode
nadar — slow movement enquanto na água), jardins laterais (cobertura),
muros altos selando a arena.

### Drop

- 700 karma garantido
- Documento: `doc_atas_condominio`
- Ativa flag: `flag:avenida_clear`

### Diálogo no início e fim

- **Início (antes do combate):** `dlg_sindico_intro_combate_001` —
  ele tenta convencer Dante de que a facção está fazendo o que é
  necessário, oferece negociação. Player pode aceitar (vide escolha
  grande abaixo) ou recusar.

- **Morte:** `dlg_sindico_morte_001` — em momento de lucidez, ele
  diz: *"Mas eu... eu seguia as regras. As mesmas. Por que..."* Morre
  confuso, não revoltado. Ele genuinamente não entende por que falhou
  — o sistema sempre o recompensou.

## A grande escolha: Facção

A primeira escolha pesada do Ato 1.

### Antes do mini-boss: encontro com o Negociador

`event_negociacao_faccao` (descrito abaixo) acontece **antes** da arena
do Síndico. NPC negociador no restaurante fechado tenta vender a
"proteção" da facção pra Dante.

**Opções:**

#### Opção A — Confrontar
- Dante recusa
- Vai pra arena do Síndico, combate normal
- **Resultado:** facção desmantelada, recursos liberados pros
  sobreviventes do morro (lore: bond com Dona Marta +25, com Inácio +15)
- **Custo:** combate mais difícil, sem ajuda

#### Opção B — Negociar
- Dante aceita uma trégua
- Recebe acesso a recursos do shopping e da concessionária
- **Mas** facção sobrevive. Reaparece no Ato 2 mais forte.
- **Resultado:** karma +500 imediato (a facção paga), bond com Marta
  -30, Inácio fica decepcionado, Reginaldo sai do roster permanentemente
  (se foi recrutado)
- **Custo:** custo moral pesado

#### Opção C — Aceitar e trair
- Dante finge aceitar, depois confronta o Síndico
- Recebe parte dos recursos (não tudo) durante a aceitação
- Vai pra arena com vantagem
- **Resultado:** combate mais fácil, mas facção desmantelada (lore
  com Marta neutra — ela respeita a esperteza, mas com ressalva)
- **Custo:** Inácio decepcionado mais ainda do que na Opção A

### Implementação técnica

```cpp
enum class FactionChoice : uint8_t {
    NotMet = 0,
    Confronted = 1,    // Opção A
    Negotiated = 2,    // Opção B
    BetrayedFaction = 3 // Opção C
};
```

A escolha é registrada no `ChoiceState` (Sprint C8). Diálogos
posteriores variam por escolha.

## NPCs

### `npc_negociador` — Vinícius (negociador da facção)

**Função:** NPC importante, não-aliado. Voz polida da facção. Tom
Direção 2 — controlado, articulado, ameaçador sem precisar ameaçar.

**Localização:** restaurante fechado, mesa do canto. Sempre lá durante
o Ato 1 (mesmo após escolha — só muda o tom dele).

**Gatilhos de diálogo:**
- `dlg_vinicius_intro_001`
- `dlg_vinicius_proposta_001` — apresenta as 3 opções implicitamente
- `dlg_vinicius_apos_confronto_001` — variação se Opção A
- `dlg_vinicius_apos_negociacao_001` — variação se Opção B
- `dlg_vinicius_apos_traicao_001` — variação se Opção C

### `npc_morador_condominio` — Senhora Cíntia (moradora)

**Função:** NPC menor. Moradora do condomínio. Mostra o lado humano
de quem morava ali — nem todo mundo é da facção. Tom Direção 1, mas
educada, não casca grossa.

**Localização:** dentro de um apartamento entreaberto no condomínio.

**Gatilhos de diálogo:**
- `dlg_cintia_intro_001` — "olha, eu moro aqui há 20 anos. eu não
  pedi por isso que tá acontecendo. eu também tô com medo."
- `dlg_cintia_apos_sindico_001` — varia por escolha (se confrontou,
  ela agradece; se negociou, ela fica desconfiada)

## Documentos coletáveis

### `doc_email_sindico`
- **Localização:** computador do escritório do condomínio
- **Conteúdo (PT):** Email enviado pelo Síndico aos moradores do
  condomínio, três meses antes do colapso. "Reforçamos a importância
  da contribuição extra para a manutenção do muro elétrico. Em tempos
  de tensão social, segurança é prioridade. A taxa será cobrada
  automaticamente." Anexo: ata de assembleia votando muro elétrico,
  resultado 47-3.
- **Conteúdo (EN):** Email sent by O Síndico to condominium residents,
  three months before the collapse. "We reinforce the importance of
  the extra contribution for the electric wall maintenance. In times
  of social tension, security is a priority. The fee will be charged
  automatically." Attached: assembly vote on electric wall, result
  47-3.

### `doc_contrato_condo`
- **Localização:** mesa do hall do condomínio
- **Conteúdo (PT):** Contrato padrão de aluguel do condomínio. Cláusula
  destacada: "É proibido ao locatário e visitantes vestimenta inadequada,
  a critério da administração." Lista de "vestimenta inadequada" inclui:
  uniformes de trabalho braçal, bonés, roupas com logo de marcas
  populares.
- **Conteúdo (EN):** Standard condominium rental contract. Highlighted
  clause: "Tenant and visitors are prohibited from inadequate attire,
  at administration's discretion." List of "inadequate attire" includes:
  manual labor uniforms, caps, clothes with popular brand logos.

### `doc_lista_inadimplentes`
- **Localização:** chão do escritório do condomínio
- **Conteúdo (PT):** Lista impressa de moradores inadimplentes. Datas
  de envio de notificação extrajudicial. Algumas linhas riscadas a
  caneta com anotação "saiu" ou "vendeu". Última linha: "Senhora
  Almeida, 78 anos, viúva, 4 meses em atraso. Notificada
  extrajudicialmente. Saiu sem deixar endereço."
- **Conteúdo (EN):** Printed list of overdue residents. Dates of
  legal notices sent. Some lines crossed out with pen with notes
  "left" or "sold". Last line: "Mrs. Almeida, 78, widow, 4 months
  overdue. Legally notified. Left without forwarding address."

### `doc_panfleto_clube`
- **Localização:** balcão da entrada do clube fechado
- **Conteúdo (PT):** Panfleto de "evento beneficente" do clube
  pré-colapso. "Jantar de gala em prol das comunidades carentes da
  cidade. Couvert: R$ 800,00." Logo da TecnoSul como patrocinadora
  principal.
- **Conteúdo (EN):** Pre-collapse "charity event" pamphlet. "Gala
  dinner benefiting the city's needy communities. Cover: R$ 800.00."
  TecnoSul logo as main sponsor.

### `doc_atas_condominio`
- **Localização:** drop após O Síndico
- **Conteúdo (PT):** Atas de assembleias do condomínio nos últimos 5
  anos. Padrão claro: cada ano, novas restrições à entrada de
  "elementos externos", aumento de taxa de segurança, e contratação
  de mais seguranças. Última ata, dois meses antes do colapso, decide
  por **fechar a portaria 24h** e exigir cadastro biométrico de
  visitantes. Vitória apertada: 26-24.
- **Conteúdo (EN):** Condominium assembly minutes from the last 5
  years. Clear pattern: each year, new restrictions on "external
  elements" entry, increased security fees, and more security hires.
  Last minute, two months before collapse, decides to **close
  reception 24h** and require biometric registration of visitors.
  Narrow win: 26-24.

## Action events

### `event_chegada_avenida` — A primeira impressão

**Triggers:**
- `ZoneEnter:avenida_progresso` AND NOT `flag:avenida_visitada`

**Steps:**
1. Cinematic: Dante caminha pelo calçadão. Lojas vazias mas intactas.
   Limpeza. Silêncio educado.
2. **Internal monologue:** `dlg_dante_avenida_001` — Dante percebe que
   aqui é diferente. Não é destruído. É arrumado.
3. **Effect:** `set_flag:avenida_visitada`

### `event_negociacao_faccao` — A grande escolha

**Triggers:**
- `ZoneEnter:restaurante_fechado` AND NOT `flag:faccao_decidida`

**Steps:**
1. Vinícius já está sentado. Indica a cadeira oposta.
2. **Dialogue:** `dlg_vinicius_intro_001`
3. **Dialogue:** `dlg_vinicius_proposta_001`
4. Player escolhe (3 opções):
   - "Recuso. Vou tirar vocês daí." → `FactionChoice::Confronted`
   - "Aceito a trégua." → `FactionChoice::Negotiated`
   - "Aceito." (com intenção de trair) → ramificação posterior
5. Se Opção C, no momento de ir pra arena do Síndico, prompt aparece:
   "Honrar acordo / Trair." Player escolhe.
6. **Effect:** `set_choice:faction = X`, `set_flag:faccao_decidida`

### `event_arena_sindico` — Confronto ou trégua quebrada

**Triggers:**
- `flag:faccao_decidida` AND `ZoneEnter:condominio_alpha`
- AND `choice:faction != Negotiated` (negociação evita o boss)

**Steps:**
1. Cinematic: Dante atravessa portões do condomínio. Seguranças observam
   mas não atacam (foram instruídos). Síndico na borda da piscina,
   bebendo água com gás.
2. **Dialogue:** `dlg_sindico_intro_combate_001`
3. Última chance de Dante recuar (raramente o player faz isso aqui)
4. Combate phase 1
5. Em 50% HP: Síndico se levanta da cadeira → phase 2
6. Morte: `dlg_sindico_morte_001`
7. Drop garantido
8. **Effect:** `set_flag:avenida_clear`

### `event_pos_negociacao` — A escolha pesa

**Triggers:**
- `choice:faction == Negotiated` AND tempo passa (próxima zona explorada)

**Steps:**
1. Em vários momentos posteriores, NPCs reagem:
   - Marta: tom mais frio (`dlg_marta_pos_negociacao_001`)
   - Inácio: decepção contida (`dlg_inacio_pos_negociacao_001`)
   - Reginaldo: sai do roster permanentemente
     (`dlg_reginaldo_saida_001`)
2. **Effect:** `set_flag:negociacao_consequencias_ativas`

## Pontos de interesse

| Tipo            | Local                          | Função                                |
|-----------------|--------------------------------|---------------------------------------|
| Save point      | Calçadão central               | Save manual (sem NPC)                 |
| Negociador      | Restaurante fechado            | Vinícius — escolha grande             |
| Mini-boss       | Condomínio Alpha-1             | O Síndico (após negociação)           |
| Lore moradora   | Apartamento entreaberto        | Senhora Cíntia                        |
| Documentos      | 5 espalhados                   | Lore corporativo + condomínio         |
| Loot interior   | Shopping                       | Consumíveis raros                     |
| Transição S     | Entrada sul                    | Door → Morro da Vigília               |
| Transição N     | Saída norte (após Z6 unlock)   | Door → Rio Turvão                     |

---

# Notas técnicas para C3, C4 e C5

## Para implementação (Sprint C3 — Industrial parte da)

Industrial implementado junto com Parque (continuação da C3 da Parte 1):

1. **Inimigos novos:** factory_worker (resistente, lento), assembly_line_squad
   (sincronia coreografada — provavelmente novo), supervisor (buff de
   aliados — novo).
2. **Mini-boss A Máquina:** mecânica de ambient hazards (esteiras, prensa)
   é o desafio técnico maior. Verificar se engine suporta zonas dinâmicas
   no terreno. Phase 2 com sirene + zonas seguras requer sistema de zonas
   marcadas no chão.
3. **Sem AI nova pra A Máquina:** ela não se move. Lógica é state machine
   simples decidindo qual ataque disparar.

## Para implementação (Sprint C4 — Hub)

Morro da Vigília:

1. **Sem inimigos no normal state.** Reusa inimigos de Centro/Industrial
   nos cercos.
2. **Sistema de cerco** é novo: spawner com waves, timer entre waves,
   condição de vitória/falha, Morale tracking. Provavelmente requer
   `defense_event_system.hpp` novo.
3. **Sistema de Morale** afeta disponibilidade de NPCs — talvez via flag
   no `npc_data` que checa em cada interação.
4. **3 NPCs principais com diálogos extensos** — pesado em conteúdo mas
   leve em código.
5. **Altar do Cruzeiro** reusa K5.

## Para implementação (Sprint C5 — Avenida + Rio)

Avenida (Rio na Parte 3):

1. **Inimigos novos:** security_corp (ranged + cobertura — novo),
   executive (similar a O Gerente mas mais forte), drone_security
   (aéreo + alerta de aliados — novo).
2. **Mini-boss O Síndico:** mecânica de "zonas de proibição" (contra-dano
   condicional) é nova. Requer sistema de status temporário por área.
3. **Sistema de escolha de facção:** primeira choice pesada com
   ramificações reais. Sprint C8 cuida da estrutura genérica de escolhas;
   este é o primeiro caso real.
4. **Dialogue branching por escolha:** todos NPCs subsequentes precisam
   ler `choice:faction` em diálogos relevantes.

## Decisões em aberto (resolver durante C3-C5)

- [ ] Drone aéreo: a engine suporta inimigos voadores ou precisa
      adaptar? (Mesma questão da Zona 2 com pássaros.)
- [ ] A Máquina phase 2 ataque "Sobrecarga" — 3s de telegraph é OK ou
      muito longo? Tunar durante playtest.
- [ ] Sistema de Morale: persiste no save? Sim, deveria. Mas precisa
      decidir granularidade (int 0-100 vs faixas discretas).
- [ ] Recrutamento de Marta: condição é primeiro cerco bem-sucedido,
      ou pode ser flag separada se cerco for opcional?
- [ ] Opção C (trair facção): testar se a janela de "trair durante a
      ida pra arena" é clara o suficiente como mecânica. Pode confundir
      o player.
- [ ] Senhora Cíntia: NPC de "1 fala e some" ou recorrente? Recorrente
      dá mais lore mas custa mais conteúdo.
