# 03c — Zones Content (Part 3: Zones 6-7 + Act 1 Ending)

Content for the final two zones of Act 1: Margem do Rio Turvão (Z6)
and Torre Horizonte (Z7), plus the Act 1 ending sequence with O
Conselheiro absorption.

> Companion to `01_implementation_plan.md` § Sprints C5 (Rio part),
> C6 (Torre + boss), C8 (final scripted events).
> Continuation of `03a` (Z0-Z2) and `03b` (Z3-Z5).

---

# Zone 6 — Margem do Rio Turvão

## Narrative summary

Faixa ribeirinha. Mata ciliar densa, ponte velha, **estação de
tratamento de água abandonada**, palafitas, trilhas de terra. O Rio
Turvão corta São Chico — e carrega karma dissolvido. A água é o meio
de transporte natural da energia.

Comunidades ribeirinhas foram as primeiras afetadas e as primeiras
esquecidas. Ninguém da imprensa cobriu. Ninguém da prefeitura veio.

Aqui está o **ponto mais antigo** de manifestação de karma na cidade.
E na estação de tratamento, evidências de que a água já carregava
traços de karma **anos antes** da crise. Alguém mediu. Alguém arquivou.
Alguém ignorou.

Mini-boss: **O Engenheiro** — ex-responsável pela estação, único boss
do Ato 1 que **sabe** o que está acontecendo. Suas últimas palavras
direcionam a Torre Horizonte.

Tom: melancólico. A zona mais bonita visualmente (bioluminescência no
rio, lanternas nas palafitas) e a mais triste narrativamente.

## Layout suggestion (procedural)

Tilemap aproximado 110×60 (largo, seguindo o rio):

```
+---------------------------------------------+
|  ENTRADA (norte) [Door → Avenida Progresso] |
+---------------------------------------------+
|  CAIS ABANDONADO   |   PALAFITAS NORTE      |
|  (entrada + npcs)  |   (pescador + lore)    |
+--------------------+------------------------+
|              RIO TURVÃO (largo)             |
|              [bioluminescência]             |
|         +---------------------+             |
|         | PONTE VELHA          |             |
|         | (encontro mid-zone)  |             |
|         +---------------------+             |
+---------------------------------------------+
|  PALAFITAS SUL      |   ESTAÇÃO DE          |
|  (ribeirinhos       |   TRATAMENTO          |
|   corrompidos)      |   (boss arena)        |
+--------------------+------------------------+
|  TRILHAS DE TERRA (oeste, leste)            |
|  [Door W → Morro]  [Door E → Torre Horizonte (após boss)]|
+---------------------------------------------+
```

Pontos de interesse:
- **Cais abandonado:** entrada da zona, NPC pescador
- **Palafitas norte:** comunidade pré-colapso (memorial)
- **Palafitas sul:** ribeirinhos corrompidos (combate)
- **Ponte velha:** encontro narrativo de meio de zona
- **Estação de tratamento:** arena do mini-boss
- **Trilhas de terra:** transições para outras zonas

## Inimigos

### `enemy_river_dweller` — Ribeirinho Corrompido
- **Tier base:** 5
- **HP base:** 140
- **Damage base:** 20
- **AI:** Ataca em emboscada — escondidos atrás de palafitas e árvores.
  Salta na água quando ferido (regen lenta enquanto submerso).
- **Karma drop:** 35
- **Comportamento:** ambush → melee burst → swim retreat
- **Visual placeholder:** roupas molhadas tingidas, partículas de água
  escura

### `enemy_aquatic_creature` — Criatura Aquática
- **Tier base:** 6
- **HP base:** 180
- **Damage base:** 24
- **AI:** Vive submersa. Sai da água atacando em arco. Mais forte na
  beira do rio.
- **Karma drop:** 50
- **Comportamento:** submerged → arc strike from water → submerge again
- **Visual placeholder:** sprite indefinido (peixe-cobra grande?
  algo ambíguo, lembrando que karma criou novas formas de vida)

### `enemy_fog_lurker` — Espreitador da Névoa
- **Tier base:** 5
- **HP base:** 110
- **Damage base:** 18
- **AI:** Translúcido na névoa, fica visível só a 4m. Ataque corpo a
  corpo rápido. Pesadelo em zonas com baixa visibilidade.
- **Karma drop:** 40
- **Comportamento:** stealth → close range strike → fade
- **Visual placeholder:** sprite com transparência variável

## Mini-boss: O Engenheiro

### Identidade narrativa

Ex-responsável pela estação de tratamento. **Ele sabia.** Anos antes
do colapso, mediu contaminantes anômalos na água. Reportou. Foi
ignorado, pressionado, reprimendado. Continuou trabalhando. Os anos
passaram, a contaminação cresceu, ele continuou trabalhando. Quando o
karma estourou visivelmente, ele já era um homem destruído pela culpa.

O karma o consumiu **pela culpa** — único boss do Ato 1 corrompido por
um peso autoinfligido, não pela pressão externa direta. Por isso, é o
único que **mantém parcial consciência durante o combate**. Ele sabe o
que tá fazendo. Sabe que tá errado. Não consegue parar.

Suas últimas palavras dão a pista final que aponta a Torre Horizonte.

### Stats

- **Tier:** 7
- **HP:** 1500 (escala com effective_tier)
- **Damage:** variável
- **Phases:** 2

### Mecânicas — Phase 1 (HP 100% → 60%)

O Engenheiro luta dentro da estação de tratamento. Tanques de água
ainda funcionando — alguns vazando, outros borbulhando com líquido
contaminado por karma. Ele usa o ambiente.

**Ataque 1 — Corrosão** (cooldown 5s)
- Lança projétil líquido contaminado em arco. Quem é atingido recebe
  DoT (8 dano/s por 6s) + redução de armadura -10 enquanto durar.
- Telegraph: ele encosta a mão no tanque vazando, recolhe líquido.

**Ataque 2 — Inundação Localizada** (cooldown 10s)
- Abre uma válvula. Área retangular do chão se enche de líquido
  corrompido (4×4 tiles). Quem fica em pé na área recebe 12 dano/s.
- Player precisa sair da zona. A zona dura 8s.

**Ataque 3 — Tubo Quebrado** (cooldown 7s)
- Uma das tubulações do teto explode acima do player. Choque mecânico
  + dano: 25 + stun 1s.
- Telegraph: rangido metálico audível 1s antes.

### Mecânicas — Phase 2 (HP 60% → 0%)

Ele entra em **modo de luto ativo**. Por momentos, para. Lágrimas.
Sussurra desculpas. Player sente o conflito interno dele.

**Ataque 4 — Mea Culpa** (cooldown 12s)
- Por 3s, O Engenheiro fica imóvel, ajoelhado. Imune a dano. Murmura.
  Após os 3s, libera onda em todas as direções (8 projéteis radiais)
  com 30 dano cada.
- Lore: ele tenta se conter, falha, libera tudo de uma vez.
- Player pode usar o tempo de imunidade pra reposicionar — incentivo
  de design.

**Ataque 5 — Confissão Tóxica** (cooldown 15s)
- Cria zona ao seu redor (raio 6m) que dura 10s. Dentro da zona,
  ataques do player causam **metade do dano**. Lore: ele "confessa" e
  o ambiente absorve a violência.
- Player precisa sair pra atacar com efetividade.

**Comportamento phase 2:** sequência mais lenta entre ataques. Pausas
significantes. Ele dá ao player espaço pra repensar — e ainda assim,
quando ataca, ataca pesado.

### Arena

Estação de tratamento de água. Tanques abertos (4 grandes, espalhados),
tubulações no teto, válvulas no chão. Espaço de combate em forma de "U"
ao redor dos tanques. Iluminação fraca e azulada.

### Drop

- 1000 karma garantido
- Documento: `doc_emails_engenheiro`
- Ativa flag: `flag:rio_clear`, `unlock_zone:torre_horizonte`

### Diálogo no início e fim

#### Início (antes do combate)

Único boss do Ato 1 que conversa **antes** da luta com lucidez.
`dlg_engenheiro_intro_combate_001`:

PT (extrato): *"Eu sabia. Anos antes. Mediram. Eu também medi. Eu mostrei
pros caras lá em cima. Sabe o que eles fizeram? Arquivaram. 'Não temos
contexto'. E eu... eu continuei vindo trabalhar. Todos os dias. Sabendo.
Você não devia ter vindo aqui. Vai embora. Por favor."*

Player não tem opção de "ir embora". Ele insiste, atacar é o único
caminho. O Engenheiro aceita, com tristeza.

#### Morte

`dlg_engenheiro_morte_001`:

PT (extrato): *"Não eram só medidas. Eram... pessoas. As crianças do
Bairro Norte. As idosas que tinham insuficiência. Eu sabia que a água
tava... que ia... [pausa] Você quer saber de onde vem? A Torre. O
escritório do conselho. Eles têm tudo. Tudo arquivado. Tudo. Vai lá.
Pelo menos isso. Vai lá... e olha... no meu nome..."*

Morre. Última fala carrega o gancho narrativo direto pra Torre
Horizonte (Z7).

## NPCs

### `npc_pescador_seu_zequinha` — Seu Zequinha (pescador ribeirinho)

**Função:** NPC menor mas memorável. Pescador velho que sobreviveu por
costume — passou a vida toda no rio, conhece cada curva, cada pedra,
cada sinal de mudança. Tom Direção 1 com sabedoria popular.

**Localização:** cais abandonado, na ponta de um trapiche velho. Linha
na água, mesmo sem peixe.

**Gatilhos de diálogo:**
- `dlg_zequinha_intro_001`
- `dlg_zequinha_lore_rio_001` — sobre como o rio mudou ao longo dos anos
- `dlg_zequinha_oferece_servico_001`

**Serviço:** revelar mapa local + indicação dos pontos seguros nas
trilhas (50 karma). Lore: ele conhece as margens.

### `npc_lia_no_rio` — Lia (visita memorial)

**Função:** se Lia foi recrutada e está no roster, **scripted event**
no encontro com a ponte velha. Não é NPC fixo da zona; é uma cena.

Vide `event_ponte_lia` abaixo.

### `npc_pescador_fantasma_lore` — Memorial do Pescador (NPC ausente)

**Função:** **NPC que não está mais lá.** Apenas um memorial — uma
fotografia presa em poste, flores murchas, oferendas. Quem vivia ali
era pescador. Quem o matou foi a corrupção do rio. NPCs próximos
(Zequinha) o mencionam.

**Interação:** ler o memorial dispara documento `doc_memorial_pescador`.

## Documentos coletáveis

### `doc_recibos_estacao_2017`
- **Localização:** sala de controle da estação (antes do boss)
- **Conteúdo (PT):** Recibos de equipamentos solicitados pela estação
  em 2017. Lista incluindo "kit de medição de contaminantes orgânicos
  classe IV." Carimbo: "PEDIDO NEGADO — ALOCAÇÃO INSUFICIENTE." Anotação
  à mão: "pedi de novo. e de novo. e de novo. cinco vezes em quatro
  anos. nunca compraram."
- **Conteúdo (EN):** Equipment requests from the station in 2017. List
  including "class IV organic contaminant measurement kit." Stamp:
  "REQUEST DENIED — INSUFFICIENT ALLOCATION." Handwritten note: "asked
  again. and again. and again. five times in four years. they never
  bought it."

### `doc_email_engenheiro_supervisao`
- **Localização:** computador na sala de controle
- **Conteúdo (PT):** Cadeia de emails de 4 anos. Engenheiro reportando
  contaminantes anômalos. Respostas:
  - 2020: "agradecemos o reporte, vamos avaliar."
  - 2021: "questão sob análise."
  - 2022: "decidimos arquivar — falta contexto interpretativo."
  - 2023: "Engenheiro [Nome], pedimos discrição. Esses reportes têm
    causado preocupação desnecessária. Foque no operacional."
  - 2024 (último): "[Nome], esta será sua advertência formal. Nova
    insistência será tratada como insubordinação."
- **Conteúdo (EN):** 4-year email chain. Engineer reporting anomalous
  contaminants. Responses:
  - 2020: "thank you for the report, we will evaluate."
  - 2021: "matter under analysis."
  - 2022: "decided to archive — interpretive context lacking."
  - 2023: "Engineer [Name], we ask for discretion. These reports have
    caused unnecessary concern. Focus on operational duties."
  - 2024 (last): "[Name], this is your formal warning. Further
    insistence will be treated as insubordination."

### `doc_lista_mortos_bairro_norte`
- **Localização:** poste no cais (preso com prego enferrujado)
- **Conteúdo (PT):** Lista escrita à mão. 47 nomes. Crianças e idosos.
  Datas: dois anos antes do colapso visível. Causa anotada em quase
  todos: "intoxicação não-identificada, falência renal." No topo da
  folha: "imprensa ignorou. SUS arquivou. eu lembro."
- **Conteúdo (EN):** Handwritten list. 47 names. Children and elderly.
  Dates: two years before the visible collapse. Cause noted on almost
  all: "unidentified intoxication, renal failure." At the top of the
  page: "press ignored. SUS archived. I remember."

### `doc_memorial_pescador`
- **Localização:** memorial no cais
- **Conteúdo (PT):** Texto numa placa improvisada de madeira, escrito
  a giz: "Aqui pescava o Seu Manoel. 60 anos no rio. Bebeu da água
  porque sempre bebeu. Morreu sem saber por quê. Sentimos falta. — os
  vizinhos, 2024."
- **Conteúdo (EN):** Text on an improvised wooden plaque, written in
  chalk: "Here fished Seu Manoel. 60 years on the river. Drank the
  water because he always did. Died without knowing why. We miss him.
  — the neighbors, 2024."

### `doc_emails_engenheiro`
- **Localização:** drop após O Engenheiro (cofre privado dele)
- **Conteúdo (PT):** Email pessoal do Engenheiro, salvo num pendrive
  escondido no cofre da estação. Nunca foi enviado. Endereçado a um
  jornalista cujo email está rabiscado. Texto:

  *"Se você tá lendo isso, eu finalmente desisti — ou morri tentando.
  Faz dois anos que tento expor o que sei. As leituras vêm da Torre.
  O sistema deles bloqueou meus reportes oficiais, monitora meu email
  corporativo, e me ameaçou três vezes este ano com demissão. O que eu
  posso te dizer: tudo passa pelo Conselho. Tudo. Eles sabiam antes
  da gente — não sei como. Talvez tenham causado. Talvez tenham só
  deixado acontecer pra ver. Os arquivos completos estão no andar 38
  da Torre Horizonte, sala dos servidores principais. Acesso restrito
  ao 'Conselheiro' (não é o cargo formal, é como o pessoal chama).
  Boa sorte. Eu desejo de coração que você consiga onde eu falhei."*
- **Conteúdo (EN):** Personal email from O Engenheiro, saved on a
  hidden flash drive in the station safe. Never sent. Addressed to a
  journalist whose email is scratched out. Text:

  *"If you're reading this, I finally gave up — or died trying. For
  two years I've been trying to expose what I know. The readings come
  from the Tower. Their system blocked my official reports, monitors
  my corporate email, and has threatened me three times this year
  with dismissal. What I can tell you: everything passes through the
  Council. Everything. They knew before us — I don't know how. Maybe
  they caused it. Maybe they just let it happen to see. The full
  archives are on the 38th floor of Torre Horizonte, main server
  room. Restricted access to the 'Counselor' (not a formal title,
  it's what the staff calls him). Good luck. I sincerely hope you
  succeed where I failed."*

> Este documento é o **gancho final do Ato 1.** Direciona explicitamente
> Dante (e o player) à Torre Horizonte.

## Action events

### `event_chegada_rio` — A primeira impressão

**Triggers:**
- `ZoneEnter:rio_turvao` AND NOT `flag:rio_visitado`

**Steps:**
1. Cinematic: câmera segue o rio. Bioluminescência. Lanternas penduradas
   nas palafitas. Memorial à esquerda. Silêncio quebrado por um peixe
   saltando.
2. **Internal monologue:** `dlg_dante_rio_001` (Dante percebe que aqui
   é triste de um jeito diferente)
3. **Effect:** `set_flag:rio_visitado`

### `event_encontro_zequinha` — Sabedoria popular

**Triggers:**
- `ZoneEnter:cais_abandonado` AND NOT `flag:zequinha_conhecido`

**Steps:**
1. **Dialogue:** `dlg_zequinha_intro_001`
2. Player pode perguntar sobre o rio → `dlg_zequinha_lore_rio_001`
3. **Effect:** `set_flag:zequinha_conhecido`

### `event_ponte_lia` — Cena memorial (se Lia recrutada)

**Triggers:**
- `flag:zequinha_conhecido` AND `ally_active:lia` AND
  `ZoneEnter:ponte_velha`

**Steps:**
1. Cinematic curta: Lia para na ponte. Olha o rio.
2. **Dialogue:** `dlg_lia_ponte_001` (Direção 1, mas com peso)

   PT: *"Sabe, eu tinha um trabalho de faculdade sobre populações
   ribeirinhas. Não consegui terminar. A professora não respondeu mais.
   Eu vinha aqui pra ver. Tipo, com meus olhos. Tinha gente. Tinha
   crianças. A gente passa de carro pela ponte e não vê, sabe?"*

3. Dante e Lia ficam parados por 4s. Câmera lenta. Nenhum diálogo,
   apenas o som do rio.
4. Lia continua andando. Player retoma controle.

> Esta cena é uma das mais sutis do Ato 1. Não tem reward mecânico.
> Tem peso emocional. Marcador de afeto pra futuras escolhas.

### `event_arena_engenheiro` — O confronto melancólico

**Triggers:**
- `ZoneEnter:estacao_tratamento_interior`

**Steps:**
1. Sala escura. O Engenheiro sentado num banco metálico, de costas pra
   porta. Vira lentamente quando Dante entra.
2. **Dialogue:** `dlg_engenheiro_intro_combate_001` (sem opção de evitar
   combate — Engenheiro pede que Dante vá embora, mas não cede)
3. Combate phase 1
4. Em 60% HP: pausa de 2s. Engenheiro respira pesado. Phase 2 começa.
5. Morte: ele cai de joelhos. **Dialogue:** `dlg_engenheiro_morte_001`
6. Drop garantido (incluindo `doc_emails_engenheiro` — o pendrive)
7. **Effect:** `set_flag:rio_clear`, `unlock_zone:torre_horizonte`

### `event_descoberta_torre_link` — A pista final

**Triggers:**
- `pickup:doc_emails_engenheiro`

**Steps:**
1. **Internal monologue:** `dlg_dante_emails_engenheiro_001` (Dante
   conecta os pontos: industrial → conselho → torre)
2. **Effect:** `set_flag:torre_target`
3. **Effect:** mapa atualiza com marcador na Torre Horizonte

## Pontos de interesse

| Tipo            | Local                          | Função                                |
|-----------------|--------------------------------|---------------------------------------|
| Save point      | Cais abandonado (Zequinha)     | Save manual                           |
| Mapa local      | Zequinha                       | Revelar pontos seguros (50 karma)     |
| Mini-boss       | Estação de tratamento          | O Engenheiro                          |
| Cena memorial   | Ponte velha (com Lia)          | Sem reward mecânico                   |
| Memorial        | Cais (placa Seu Manoel)        | Lore                                  |
| Documentos      | 5 espalhados                   | Negligência institucional             |
| Transição N     | Saída norte                    | Door → Avenida Progresso              |
| Transição W     | Saída oeste                    | Door → Morro da Vigília               |
| Transição E     | Saída leste (após boss)        | Door → Torre Horizonte                |

---

# Zone 7 — Torre Horizonte (clímax do Ato 1)

## Narrative summary

O prédio mais alto de São Chico. Sede de um conglomerado empresarial.
**40 andares.** Dante prestava serviço de TI nesse prédio — instalou
servidores, configurou redes, corrigiu sistemas. Conhece a infraestrutura
digital e física do lugar.

A ascensão pela Torre é a **dungeon final** do Ato 1. Não é mapa aberto
— é progressão vertical estruturada por blocos de andares. Cada bloco
é um mini-estágio com identidade e mecânicas próprias.

No topo: **O Conselheiro**. O cara que ninguém vê. O que tomava as
decisões reais. Não está corrompido como os outros — está calmo. Usa
karma conscientemente.

E quando Dante derrota e absorve o karma dele, descobre a verdade que
fecha o Ato 1: **O Conselheiro não era o topo. Era um gerente regional.**
A Torre é uma franquia. Alguém acima decidia tudo. O "vilão" era mais
uma peça da máquina.

## Estrutura da dungeon

A Torre é dividida em **5 blocos** de andares. Cada bloco tem:
- Inimigos com tema próprio
- Documentos coletáveis específicos
- Um mini-encontro ou puzzle
- Save point no fim do bloco
- Elevador para o próximo bloco

**Blocos:**

```
Andar 40 — TERRAÇO (boss arena: O Conselheiro, Phase 2)
Andar 38-39 — SALA DOS SERVIDORES + ESCRITÓRIO PRIVADO (boss Phase 1)
Andar 30-37 — DIRETORIA (executivos, drones, segurança alta)
Andar 20-29 — OPERAÇÕES (cubículos distorcidos, employees)
Andar 10-19 — ÁREA TÉCNICA (servidores, áreas que Dante conhecia)
Andar 1-9  — RECEPÇÃO E HALL (segurança no térreo)
```

## Layout suggestion (procedural)

Cada bloco é um tilemap próprio (~40×40 tiles). Estrutura linear com
exploração lateral em cada bloco:

```
[Bloco 5: Terraço]              ← boss phase 2
       ↑ elevador
[Bloco 4: Servidores + Escritório]  ← boss phase 1
       ↑ elevador
[Bloco 3: Diretoria]            ← executivos, drones
       ↑ elevador
[Bloco 2: Operações]            ← cubículos
       ↑ elevador
[Bloco 1: Área Técnica]         ← servidores que Dante conhecia
       ↑ escada de serviço
[Térreo: Recepção e Hall]       ← entrada
```

## Inimigos

### Bloco térreo + 1 (Recepção, Hall, Área Técnica)

**`enemy_corp_security` — Segurança Corporativa**
- Tier 6, HP 200, Damage 25
- AI: profissional, organizada. Cobertura. Ranged + melee híbrido.
- Karma drop: 60

**`enemy_drone_corp` — Drone Corporativo Avançado**
- Tier 6, HP 100, Damage 22
- AI: aéreo, dispara raios laser, alerta outros
- Karma drop: 50

### Bloco 2 (Operações)

**`enemy_office_drone` — Office Drone (employee corrompido)**
- Tier 5, HP 120, Damage 18
- AI: vagam pelos cubículos. Atacam quando o player entra no campo de
  visão do cubículo.
- Karma drop: 30
- Visual: roupas casuais corporativas + névoa

**`enemy_zombie_executive` — Executivo Zumbi**
- Tier 6, HP 250, Damage 28
- AI: lento, resistente. Ataque corpo a corpo pesado (laptop como arma).
- Karma drop: 60

### Bloco 3 (Diretoria)

**`enemy_executive_armored` — Executivo Blindado**
- Tier 7, HP 350, Damage 32
- AI: armadura de karma cristalizada. Reduz dano físico recebido em 30%.
  Vulnerável a magia.
- Karma drop: 90

**`enemy_security_elite` — Segurança Elite**
- Tier 7, HP 300, Damage 35
- AI: time de 2-3. Coordenação real (um chama atenção, outro flanqueia).
- Karma drop: 80

### Bloco 4 (Servidores + Escritório)

**`enemy_data_specter` — Espectro de Dados**
- Tier 7, HP 150, Damage 30
- AI: incorpóreo. Aparece e desaparece. Ataca passando através de
  obstáculos. Difícil de mirar.
- Karma drop: 70
- Lore: karma residual da informação processada na sala dos servidores

## Mini-bosses dos blocos

Cada bloco tem **1 mini-encontro pesado** (não chega a ser mini-boss
formal mas é um encontro coreografado):

### Bloco 1: "O Hall Bloqueado"
- Encontro com 4 segurança_corp + 1 drone_corp
- Em arena fechada (lobby do bloco). Saída se desbloqueia matando todos.

### Bloco 2: "Reunião Permanente"
- Sala de reunião com 6 office_drone + 1 zombie_executive ao redor de
  uma mesa. Se ativam quando Dante entra.

### Bloco 3: "Andar do Diretor"
- 2 executive_armored + 2 security_elite. Combate em corredor com pouca
  cobertura. O encontro mais difícil pré-boss.

### Bloco 4: "Sala dos Servidores"
- 4 data_specter aparecendo e desaparecendo. Player precisa proteger um
  terminal por 90 segundos enquanto baixa os arquivos do Engenheiro.
  Falhar = baixar parcial e perder lore.

## Boss principal: O Conselheiro

### Identidade narrativa

Aparenta uns 60 anos. Terno claro, postura de quem nunca perdeu uma
discussão. Cabelo cuidado. Está sentado na ponta da mesa de reunião do
escritório privado quando Dante entra. **Calmo.** Não parece surpreso.

Ele não está corrompido como os outros. Está **lúcido**. Karma nele é
cristalizado, organizado, funcional. Ele usa o karma. Comanda. Nunca
foi consumido por ele — porque foi sempre quem o mediu, processou,
decidiu o que fazer com ele.

A diferença entre ele e os outros bosses: ele **escolhe** lutar. Não
foi pego por uma onda. É um processo deliberado.

Acredita genuinamente que o que fez (e faz) é o melhor possível dadas
as circunstâncias. "Alguém precisava administrar isso." Pode até estar
certo, dentro da lógica deturpada que opera. Esse é o ponto: ele não
é caricatura. É o que acontece quando inteligência genuína serve um
sistema que premia destruição calibrada.

### Stats

- **Tier:** 9
- **HP:** 3500 (escala com effective_tier)
- **Damage:** variável, mas alto e calculado
- **Phases:** 3 (incluindo a fase de absorção, scriptada)

### Phase 1 (HP 100% → 50%) — Escritório Privado

Combate dentro do escritório. Móveis caros, mesa grande, retratos nas
paredes. Pouco espaço de manobra.

**Ataque 1 — Diretiva** (cooldown 4s)
- Projétil de karma preciso. Trajetória curva (busca o player levemente).
- Dano: 30. Telegraph: ele aponta com calma, sem urgência.

**Ataque 2 — Reestruturação** (cooldown 8s)
- 3 executive_armored spawnam pelas portas laterais. Lore: chamadas em
  reuniões emergenciais que ele convoca pelo karma.

**Ataque 3 — Cláusula Restritiva** (cooldown 12s)
- Marca o player com símbolo de karma. Por 8s, todos os ataques do
  player consomem 50% mais mana/stamina.
- Counter: difícil de removê-lo. Player pode passar pela altar de
  dissolução portátil (item especial dropável de inimigos da Torre)
  ou aguentar.

### Phase 2 (HP 50% → 25%) — Quebra de elemento

Em 50% HP, **paredes desabam**. O escritório se conecta com o terraço
através de uma janela enorme que se abre/quebra. O Conselheiro caminha
pra fora, em direção ao terraço. Combate continua.

A mudança é narrativa: ele perde a casca de "burocrata calmo". Começa
a usar karma com mais força, menos retórica.

**Ataque 4 — Auditoria** (cooldown 6s)
- Onda de karma que percorre o terraço. Player precisa pular ou se
  esconder atrás de cobertura (mobília derrubada). Dano: 45.

**Ataque 5 — Decisão Final** (cooldown 18s)
- Para por 4s. Estende a mão pra cima. Karma se concentra acima do
  terraço. 5 raios verticais descem em pontos espalhados.
- Dano cada: 50. Telegraph claro (marcadores no chão).
- Player precisa rotear entre marcadores.

**Ataque 6 — Pressão Sustentada** (cooldown 10s)
- Por 5s, o terraço inteiro tem partículas pesadas que aplicam slow 30%
  e dano leve 5/s. Lore: a "pressão" que ele exerce no espaço.

### Phase 3 (HP 25% → 0%) — Absorção (scriptada)

Em 25% HP, combate **para**. Cinematic de 8 segundos:
- O Conselheiro cambaleia. Pela primeira vez, parece exausto.
- Ele se ajoelha. Não cai. Apenas se ajoelha. Olha pra Dante.
- **Dialogue:** `dlg_conselheiro_pre_absorcao_001`

> PT: *"Você é interessante. Eu vi suas medições. Karma alto, pré-frontal
> intacto. Você é raro. Nós teríamos te recrutado. ...Você não vai
> aceitar a oferta, vai?"*

Player escolhe responder (3 opções, todas levam à absorção):
- "Não."
- "Você nem chegou a oferecer."
- "[silêncio]"

Cada escolha gera linha curta de reação dele.

Dante avança. O Conselheiro **não resiste**. Apenas observa. Dante
toca nele. **Inicia a sequência de absorção.**

## Sequência de absorção (scriptada)

Sequência scriptada compacta. Roda como cinematic com flashbacks
gameplay-leves. Duração total ~15-20s.

### Etapa 1: O contato

**Fade to white.** Som abafado. Por 1.5s.

### Etapa 2: O fluxo invertido

Dante recebe karma do Conselheiro. Mas é **diferente**. Não é dor
bruta como os pais. É **estrutura**. Planilhas. Reuniões. Voltagens
medidas.

Visualmente, isso aparece como **2 flashes intercalados** — não toma
controle do player, mas a câmera mostra:

#### Flash 1 — Sala de reunião (5s)

Mesa longa. Pessoas em ternos discutindo um gráfico. Um deles é o
Conselheiro mais jovem. O gráfico mostra "Índice de Estresse
Operacional" crescente.

Voz feminina (off): *"Reverter o programa custaria 18% da margem."*
Voz masculina (off): *"Mantenham. E descubram quem reportou pra
imprensa."*

Câmera vai do gráfico ao retrato da família na mesa do Conselheiro
mais jovem. Ele vira o retrato de cara pra baixo. Continua.

#### Flash 2 — Audiência superior (5s)

Sala com várias telas. O Conselheiro nosso assiste, **mais velho**.
Numa tela: dados de "manifestações de campo anômalo" — o karma.
Crescimento exponencial. Numa outra: emails do Engenheiro sendo
arquivados em série.

Voz off-screen, com tom de quem comanda mesmo o Conselheiro:
*"O modelo previu isso há três anos. Vocês escolheram preservar o
caixa. Agora vão administrar a consequência. Façam o seu trabalho."*

O Conselheiro abaixa a cabeça. Não responde.

> Os dois flashes condensam: **(1)** ele decidiu. **(2)** ele também
> recebia ordens. A revelação dupla — vilão e vítima ao mesmo tempo —
> em 10 segundos de cinemática.

### Etapa 3: A revelação

**Flash final, presente:** Dante recolhe a mão. O Conselheiro está
sentado no chão. Sem o karma cristalizado, é apenas um homem velho.
Olha pra Dante.

**Dialogue:** `dlg_conselheiro_pos_absorcao_001`

PT: *"Agora você sabe. Não, não exatamente. Você viu fragmentos. Mas
sabe o que eu sabia. E sabe que eu não podia fazer nada com isso.
[pausa] Eu nem sou o último andar. Eu nem sou o primeiro andar dos
que decidem. Você ainda tem uma escalada longa, Dante. Eles vão te
medir. Vão te recrutar. Ou vão te desligar. Eu... eu vou ficar aqui.
Pode ir."*

Dante hesita. O Conselheiro acena com a mão. Sem energia.

### Etapa 4: A cura

**Internal monologue:** `dlg_dante_pos_conselheiro_001` (Dante
reconhece que pode curar os pais agora — o conhecimento absorvido
inclui o método).

### Etapa 5: O fim do Ato 1

**Cut to:** Vila Rosário. Dante voltando à casa. Lia na porta. Cena
silenciosa.

**Dialogue:** `dlg_lia_final_ato_001` — curta, peso emocional.

Dante entra. Vai ao quarto dos pais. Cena de cura — toca em cada um.
Karma sai deles. Eles dormem. Mas estão **estabilizados de verdade**
pela primeira vez.

**Cut to:** terraço da Vila, ao amanhecer. Dante sentado na borda.
Olhando São Chico de longe. A névoa de karma diminuiu nas zonas que
ele explorou. A Torre Horizonte está apagada. Por enquanto.

**Internal monologue:** `dlg_dante_fim_ato_001` — última fala do Ato.

PT: *"Eles disseram que eu não sou o último andar. E eu não sei o
que isso significa. Mas a minha mãe respira. Meu pai respira. Hoje é
só isso. Hoje, hoje basta."*

### Etapa 6: A oferta (gancho do Ato 2)

Antes do fade final, beat narrativo curto:

**Cut to:** porta da casa, ainda amanhecer. Lia abre a porta —
**alguém deixou um envelope no degrau**. Pega. Olha em volta.
Ninguém.

Volta pra dentro. Dante ainda no terraço. Lia hesita, sobe pra
mostrar.

**Dialogue:** `dlg_lia_envelope_001` — *"Tava na porta. Tem teu
nome."*

Dante abre. Conteúdo do envelope (close-up):

PT: *"Sr. Dante,*

*Acompanhamos seu trabalho recente com interesse. Suas medições
estão notáveis. Gostaríamos de marcar uma conversa em ambiente mais
adequado, em data e local de sua escolha — ou nossa, se preferir.
Esta proposta é cortesia, não exigência.*

*Sua família está bem. Esperamos que continue assim.*

*— Conselho de Diretores."*

EN: *"Mr. Dante,*

*We have followed your recent work with interest. Your measurements
are remarkable. We would like to arrange a conversation in a more
appropriate setting, at a date and place of your choosing — or ours,
if you prefer. This offer is courtesy, not requirement.*

*Your family is well. We hope it stays that way.*

*— Board of Directors."*

Dante segura o papel. Olha pra Lia. Lia olha pra ele. Silêncio.

**Internal monologue:** `dlg_dante_envelope_001` — uma única linha,
que reconhece a ameaça por trás da cortesia.

PT: *"Eles sabem onde a gente mora."*

**Fade to black.**

**Title card (PT):** "FIM DO ATO 1"
**Title card (EN):** "END OF ACT 1"

**Effect:** `set_flag:ato1_completo`, `set_flag:oferta_recebida`. Save
automático em ponto especial.

> Após o fim do Ato 1, player pode continuar explorando São Chico
> livremente (todas as zonas acessíveis, encontros opcionais respawning,
> aliados disponíveis). Ato 2 será desenvolvido posteriormente.

## NPCs (Torre)

### `npc_recepcionista_corrompida` — Recepcionista (interação opcional)

**Localização:** balcão de recepção do térreo. Sentada com sorriso
pintado. Não ataca. Apenas oferece "agendamento" — diálogo absurdo,
karma falando burocracia.

**Função:** atmosfera + lore.

**Diálogo:** `dlg_recepcionista_001` — repete frases de protocolo,
dissociada.

### `npc_servidor_consciente` — Servidor Consciente (interação rara)

**Localização:** sala dos servidores (Bloco 4). Um terminal pisca em
padrão estranho. Se Dante interage, **algo no servidor responde** — não
é IA, é karma residual de dados processados.

**Diálogo:** `dlg_servidor_consciente_001` — fragmentos. Dá ao player
contexto histórico do que a Torre processava.

### `npc_funcionario_escondido` — Funcionário Escondido

**Localização:** debaixo de uma mesa no Bloco 2. Aterrorizado mas
**lúcido**. Não foi corrompido — se escondeu desde o colapso e
sobreviveu comendo lanches da copa.

**Diálogo:** `dlg_funcionario_escondido_001` — informação prática sobre
a Torre. Confirma que o "Conselheiro" não é cargo formal — é como
chamavam o cara que decidia tudo. Player pode escoltá-lo até a saída
(opcional, side quest leve).

## Documentos coletáveis (Torre)

### Bloco 1: Recepção e Hall

**`doc_organograma`**
- Organograma de TI da Torre. Mostra Dante terceirizado no nível mais
  baixo. Acima dele, várias camadas de "coordenadores" e "gerentes" e
  só então diretoria. O Conselheiro nem aparece — é figura informal.
- (PT) "Organograma oficial. Versão 4.2."
- (EN) "Official org chart. Version 4.2."

**`doc_aviso_ti_terceirizada`**
- Aviso de redução de contrato com TI terceirizada (a empresa de Dante).
  "Otimização de custos." Datado uma semana antes do colapso.

### Bloco 2: Área Técnica

**`doc_anotacao_dante_passada`**
- Folhinha de bloco de notas do próprio Dante, esquecida quando ele
  trabalhava lá. Apenas anotações técnicas — IPs, senhas (já trocadas),
  número do cabo X. Lore: ele realmente conhecia este lugar.

### Bloco 3: Operações

**`doc_meta_produtividade`**
- Tabela de metas de produtividade impossíveis. Cada andar tem nome
  + número. Notas vermelhas em quase todos: "abaixo da meta." Em alguns:
  "afastamento por saúde." Em três: "demissão por desempenho."

**`doc_email_pressao`**
- Email circular: "lembrem-se: não há reuniões marcadas após as 19h.
  Toda interação fora do horário será considerada voluntária e não
  computada como hora extra."

### Bloco 4: Diretoria

**`doc_relatorio_consultoria`**
- Relatório de consultoria externa, contratada pelo Conselho. Conclusão:
  "o modelo atual gera externalidades sociais não-precificadas estimadas
  em 4.7 bilhões/ano em saúde mental e fragmentação comunitária. ROI
  ajustado: positivo."
- Marcado: "ARQUIVAR — informação sensível."

### Bloco 4: Servidores e Escritório

**`doc_arquivos_engenheiro`**
- Os arquivos completos que o Engenheiro mencionou. Backup em formato
  legível: 4 anos de medições anômalas, todos os reportes ignorados,
  comunicações internas autorizando supressão.
- **Reward:** desbloqueia diálogo extra com aliados após o Ato 1.

### Bloco 5: Terraço (drop após boss)

**`doc_modelo_preditivo`**
- Documento técnico do modelo preditivo que o Conselheiro mencionou
  na Phase 3 absorção. Mostra que **o colapso foi previsto há 3 anos**.
  Decisões registradas: "manter operação. provisão de contingência:
  R$ 2 bi. ROI ajustado mantido." Assinatura do Conselheiro + outros
  diretores não-identificados.

> Hooks pro Ato 2 vêm da sequência de absorção e do epílogo, não deste
> documento. Ele apenas confirma que a operação foi deliberada.

## Action events específicos da Torre

### `event_chegada_torre` — Reconhecimento

**Triggers:**
- `flag:rio_clear` AND `ZoneEnter:torre_horizonte`

**Steps:**
1. Cinematic curta: Dante na frente da Torre. Olha pra cima. A altura
   é desproporcional a tudo que ele explorou. Recepção apagada. Porta
   automática que ainda funciona — abre quando ele se aproxima.
2. **Internal monologue:** `dlg_dante_torre_chegada_001`
3. **Effect:** `set_flag:torre_visitada`

### `event_baixar_arquivos` — Bloco 4

**Triggers:**
- `ZoneEnter:torre_servidores`

**Steps:**
1. Terminal pisca. Tela mostra: "Arquivos detectados — 4.2GB.
   Iniciando download..."
2. Spawn de 4 data_specter ao redor (eles são o "firewall residual"
   do sistema reagindo).
3. Player elimina os 4 specters (combate normal).
4. Após o último specter morrer, terminal completa o download.
5. **Effect:** `pickup:doc_arquivos_engenheiro`

### `event_subida_final` — Antecipação

**Triggers:**
- `flag:bloco4_clear` AND `ZoneEnter:torre_elevador_final`

**Steps:**
1. Elevador. Música muda para silêncio só com batimento cardíaco.
2. **Internal monologue:** `dlg_dante_subindo_001`
3. Elevador chega no andar 39 (escritório do Conselheiro)
4. Porta abre

### `event_arena_conselheiro` — A confrontação

**Triggers:**
- `ZoneEnter:torre_escritorio_conselheiro`

**Steps:**
1. Conselheiro sentado à mesa. **Não levanta.** Indica cadeira oposta.
2. **Dialogue:** `dlg_conselheiro_intro_001`
3. Player tem opção de "sentar e escutar" ou "ignorar e atacar".
   Ambos levam ao combate, mas a primeira opção dá lore extra
   (Conselheiro fala mais antes do combate).
4. Combate phase 1
5. Em 50% HP: cinematic da quebra de parede + saída pro terraço
6. Phase 2 no terraço
7. Em 25% HP: phase 3 (sequência scriptada de absorção — vide acima)
8. **Effect:** `set_flag:conselheiro_absorvido`,
   `set_flag:conselheiro_understanding`

### `event_fim_ato_1` — Closure

**Triggers:**
- `flag:conselheiro_absorvido`

**Steps:**
1. Cut to Vila Rosário (cena scriptada acima)
2. Cura dos pais
3. Internal monologue final
4. Title card "FIM DO ATO 1"
5. Save automático
6. Mundo persiste em estado pós-ato 1: Conselheiro como NPC quebrado
   no escritório (acessível mas não interativo além de uma fala curta),
   pais estabilizados na Vila, todas as zonas exploráveis.

## Pontos de interesse (Torre)

| Tipo            | Local                          | Função                                |
|-----------------|--------------------------------|---------------------------------------|
| Save points     | Final de cada bloco (5 totais) | Save manual                           |
| Mini-encontros  | 1 por bloco                    | Combates coreografados                |
| Lore principal  | Bloco 4 (servidores)           | Arquivos completos do Engenheiro      |
| Boss arena 1    | Andar 39 (escritório)          | Conselheiro Phase 1                   |
| Boss arena 2    | Andar 40 (terraço)             | Conselheiro Phase 2 + Phase 3 absorção|
| Documentos      | 8+ espalhados                  | Lore corporativo, modelo preditivo    |
| Transição S     | Saída térrea                   | Door → Rio Turvão                     |
| NPCs vivos      | Recepcionista + Funcionário escondido + Servidor | Atmosfera + lore  |

---

# Estado pós-Ato 1

Após `flag:ato1_completo`:

## Mundo persistente

- **Conselheiro NPC quebrado:** acessível no escritório, fala curta
  mas sem interação significativa
- **Pais estabilizados:** Dona Célia e Seu Hélio acordados, lúcidos.
  Diálogos de epílogo (curtos, peso emocional)
- **Aliados ativos:** todos os recrutados ainda no roster
- **Zonas exploráveis:** todas. Inimigos respawnam (em densidade
  reduzida) para grinding de karma se desejado
- **Faixa de karma:** independente da progressão (player pode chegar a
  faixa alta sem completar o Ato 1, ou vice-versa)
- **Choices feitas:** persistem como flags pra Ato 2 ler

## Hooks pro Ato 2

- **A oferta:** envelope deixado na porta da Vila no epílogo. "Conselho
  de Diretores" convida Dante pra "uma conversa". Ameaça envolta em
  cortesia. Eles sabem o endereço dele. Sabem da família.
- **"Eu nem sou o último andar"** — diálogo do Conselheiro. Confirma
  que a Torre Horizonte é uma franquia regional, não a sede.
- **Conselheiro vivo mas quebrado** — pode ser interrogado mais tarde.
  Pode dar nomes, pode lembrar mais detalhes da audiência superior dos
  flashbacks.
- **A fonte real** — não está em São Chico. O Ato 2 sai da cidade.
- **Manifestações em outras regiões** — pistas em arquivos que sugerem
  que o programa não acontecia só aqui.

## Achievements opcionais (placeholder, não implementar agora)

- "Cidadão Pleno" — explorar todas as 8 zonas
- "Arquivista" — coletar todos os documentos
- "Equipe" — recrutar todos os 6 aliados
- "Difícil de Comprar" — escolha A na facção
- "Mestre" — completar uma árvore de skill por inteiro

> Lista placeholder. Achievements ficam pra depois (`05_publish.md` quando for criado).

---

# Notas técnicas para C5 e C6

## Para implementação (Sprint C5 — Rio parte da)

Rio Turvão é a parte 2 da Sprint C5 (junto com Avenida da Parte 2):

1. **Inimigos novos:** river_dweller (ambush + swim retreat — novo
   comportamento de "submergir"), aquatic_creature (ataque em arco),
   fog_lurker (stealth — novo).
2. **Mini-boss O Engenheiro:** mecânica de phase 2 com auto-imunidade
   por 3s + onda radial é straightforward. Phase 2 ataque "Confissão
   Tóxica" (zona de redução de dano) requer sistema de status por área.
3. **Cena memorial Lia (`event_ponte_lia`):** scripted event com pausa
   de 4s sem input. Implementação simples mas requer testar feel.

## Para implementação (Sprint C6 — Torre)

Torre Horizonte é a Sprint C6 inteira:

1. **5 blocos = 5 tilemaps** com transições por elevador. Reusa sistema
   de zonas/transições já existente.
2. **Inimigos novos:** corp_security, drone_corp_advanced, office_drone,
   zombie_executive, executive_armored, security_elite, data_specter.
   **8 inimigos novos** — bloco mais pesado de novos enemy types do
   Ato 1.
3. **Mini-encontros coreografados:** spawn timed + arena fechada +
   recompensa ao limpar. Sistema simples de "encontro" requer um
   `encounter_system.hpp` novo.
4. **Defesa do terminal (Bloco 4):** 90s protegendo objeto com HP.
   Sistema novo (`defense_target.hpp`) ou adaptação do sistema de
   defesa do hub.
5. **Boss O Conselheiro:**
   - 3 phases (combate, terraço, absorção scriptada)
   - Phase 3 é cinematic + scripted, requer sistema de "boss cutscene"
     ou hard-code do evento
   - Mecânica "Cláusula Restritiva" (debuff de mana/stamina) requer
     sistema de status no player
6. **Sequência de absorção (~30s scripted):** 4 flashes intercalados,
   cada um é uma cena curta. Isso pode ser feito com sistema de
   "scripted scene" usando overlay, fade, dialogue trigger.
7. **Mundo pós-ato 1:** flags persistem, NPCs mudam de estado, mundo
   continua jogável.

## Decisões em aberto (resolver durante C5/C6)

- [ ] River_dweller "swim retreat" — adapta engine pra suporte a
      submersão temporária (invulnerável + regen)?
- [ ] Fog_lurker stealth — implementação por transparência alpha
      gradativa ou por sistema de "detect range" com sprite hidden?
- [ ] Cena memorial Lia: scripted lockout vs window de skip? Sugestão:
      sem skip (peso narrativo), 4s é curto.
- [ ] Servidor consciente (NPC raro): vale a pena implementar ou cortar
      como side lore? Decisão durante implementação.
- [ ] Phase 3 absorção: timing de 15-20s total é apropriado? Permitir
      skip? Sugestão: sem skip na primeira vez, replay do save permite.
- [ ] Funcionário escondido escolta: side quest com reward (karma) ou
      apenas lore? Sugestão: karma + diálogo extra com Inácio se
      Dante o salva.
