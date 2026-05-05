# 04e — Dialogues (Part 5: Torre Horizonte + Act 1 Ending)

All Act 1 dialogues for Zone 7 (Torre Horizonte) and the final
sequence: O Conselheiro across 3 phases, the absorption flashbacks,
the cure of the parents, and the envelope hook into Act 2. Fifth and
final dialogue part. ~70 lines.

> Companion to `01_implementation_plan.md` § Sprint C9, C8 (final
> scripted events).
> References content from `03c_zones_content_part3.md` (Torre +
> ending sequence).
> Conventions same as 04a.

---

# Torre Horizonte — NPCs no caminho

## Recepcionista corrompida

**Tom:** dissociada. Repete frases de protocolo corporativo em loop,
desencaixadas do contexto. Direção 1 esvaziada. Não combate.

#### `dlg_recepcionista_001`
- **PT:** "Bem-vindo à Torre Horizonte. Em que posso ajudá-lo hoje.
  Tem agendamento. Posso oferecer um café. Em que posso ajudá-lo
  hoje. O elevador principal está à direita. Em que posso ajudá-lo."
- **EN:** "Welcome to Torre Horizonte. How may I assist you today.
  Do you have an appointment. May I offer you coffee. How may I
  assist you today. The main elevator is to the right. How may I
  assist."

> Loop. Mesmo padrão de fala. Os pontos finais são intencionais — a
> entonação se perdeu junto com a consciência. Repete pra qualquer
> aproximação.

---

## Funcionário escondido

**Tom:** Direção 1, paranoia controlada. Sussurra. Direto, prático.

### Encontro

#### `dlg_funcionario_escondido_001`
- **PT:** "Por aqui. Por aqui. Senta. Eu tô aqui há... três meses?
  Quatro? Eu não sei. Eu trabalhava no andar 22, contábil. Quando
  começou, eu desci pelas escadas e me escondi. A copa tem comida
  até hoje porque ninguém usava — todo mundo pedia delivery. Eu
  como salgadinho expirado e bolacha. Tô vivo. Tu tá indo pra cima?
  Não tá certo da cabeça, sabia. Mas se tu tá indo, escuta uma
  coisa: o cara que tu procura — o tal 'Conselheiro' — ele não tem
  cargo formal. Procura na sigla TI, em qualquer documento. Ele
  aparece como 'C.A.' ou 'Conselho.' Nunca o nome real."
- **EN:** "This way. This way. Sit. I've been here... three months?
  Four? I don't know. I worked on the 22nd floor, accounting. When
  it started, I went down the stairs and hid. The break room has
  food still because nobody used it — everyone ordered delivery. I
  eat expired chips and crackers. I'm alive. You going up? You're
  not right in the head, you know. But if you're going, listen to
  one thing: the guy you're looking for — this 'Counselor' — has no
  formal title. Look in IT acronyms, in any document. He shows up
  as 'C.A.' or 'Council.' Never the real name."

### Player oferece escolta de saída

#### `dlg_funcionario_escolta_001`
- **PT:** "Sério? Tu vai me tirar daqui? ...Tá. Tá. Eu confio. Não
  sei por que, mas confio. Vamo. Eu sigo atrás de ti. Se tu cair,
  eu corro. Avisei."
- **EN:** "Seriously? You're getting me out of here? ...Okay. Okay.
  I trust. Don't know why, but I do. Let's go. I'll follow you. If
  you fall, I run. Just warning you."

### Após escolta bem-sucedida (chega na entrada)

#### `dlg_funcionario_escolta_sucesso_001`
- **PT:** "Aqui dá pra eu seguir sozinho. Pra qualquer lugar é
  melhor que ali dentro. Eu... obrigado. Não tenho nada pra te dar.
  Tenho um pendrive com folha de pagamento de oito anos atrás, é o
  que sobra de mim. Toma. Talvez sirva. Vai com Deus, ou com karma,
  com o que tu acreditar."
- **EN:** "From here I can go on my own. Anywhere is better than in
  there. I... thank you. I have nothing to give you. I have a flash
  drive with a payroll file from eight years ago, that's all that's
  left of me. Take it. Maybe it'll be useful. Go with God, or with
  karma, whatever you believe in."

> O pendrive é easter egg — não tem efeito mecânico. Lore:
> nomes de funcionários, alguns reconhecíveis de outras zonas. Player
> com olho atento liga os pontos.

---

## Servidor consciente (terminal piscando)

**Tom:** fragmentos. Não é IA — é karma residual de dados processados.
Não tem "tom" coerente. Frases interrompidas, despertencentes.

### Interação

#### `dlg_servidor_consciente_001`
- **PT:** "...processando... 47.829 reportes anômalos arquivados...
  47.829... arquivados... 47.829... os dados querem ser lidos, os
  dados querem ser lidos, os dados querem ser..."
- **EN:** "...processing... 47,829 anomalous reports archived...
  47,829... archived... 47,829... the data wants to be read, the
  data wants to be read, the data wants to..."

#### `dlg_servidor_consciente_002`
- **PT:** "...projeção mantida... ROI ajustado positivo... colapso
  esperado em t-3 anos, t-2, t-1, t, agora, agora, agora... acordem,
  acordem, acordem..."
- **EN:** "...projection maintained... ROI adjusted positive...
  collapse expected in t-3 years, t-2, t-1, t, now, now, now... wake
  up, wake up, wake up..."

> Não é diálogo interativo. Player ouve em sequência se ficar perto
> do terminal. Reforça a tese central: o sistema sabia. Estava
> calibrado. Foi mantido em curso por escolha.

---

# O Conselheiro (boss principal)

**Tom:** Direção 2 puro durante todas as fases. Articulado, calmo
até demais. Conhece Dante. Sabe nome. Sabe onde mora.

## Phase 1 — Escritório privado

### Apresentação (sem combate ainda)

#### `dlg_conselheiro_intro_001`
- **PT:** "Sente-se, por favor. Sim, eu sei que você não vai sentar.
  Mas tinha que oferecer. Eu sou o que vocês chamam de Conselheiro.
  Não é cargo, mas é título funcional. Eu sei seu nome, Dante. Eu
  sei o que você fez nos últimos meses. Eu acompanho de longe — eu
  acompanho todos com leituras anômalas como as suas. Você é o
  vigésimo-sétimo. Os outros vinte e seis estão mortos, recrutados,
  ou quebrados. Você é, por enquanto, indeciso. Eu queria conversar
  antes de você decidir por luta. Você quer escutar, ou prefere
  começar logo?"
- **EN:** "Please, sit. Yes, I know you won't. But I had to offer. I
  am what you call the Counselor. Not a title, but a functional
  one. I know your name, Dante. I know what you've done in the past
  months. I observe from afar — I observe all with anomalous
  readings like yours. You're the twenty-seventh. The other
  twenty-six are dead, recruited, or broken. You are, for now,
  undecided. I wanted to talk before you decide by force. Do you
  want to listen, or would you rather begin?"

### Player choice — escutar ou atacar

#### `dlg_conselheiro_player_escutar_001`
- **PT (Dante):** "Eu escuto."
- **EN (Dante):** "I'll listen."

#### `dlg_conselheiro_player_atacar_001`
- **PT (Dante):** "Não tenho nada pra ouvir de você."
- **EN (Dante):** "I have nothing to hear from you."

### Reação ao escutar (lore extra)

#### `dlg_conselheiro_lore_001`
- **PT:** "Vou ser breve. O programa que produziu o que você chama
  de karma não é desenho original meu. Eu administro a operação local.
  Há outras operações. Há outras cidades. Há um modelo. O modelo é
  preditivo — nós sabíamos que essa concentração ia se manifestar
  visualmente em São Chico há três anos. Decidimos manter porque o
  custo de reverter o programa era alto. Foi uma decisão coletiva.
  Eu votei a favor. Não vou negar. Você merece honestidade nesse
  momento. O que você tá fazendo — caminhando pra cima — você não é
  o primeiro. Os outros vinte e seis tentaram, com mais ou menos a
  mesma trajetória. A maioria foi recrutada antes de chegar aqui.
  Quem chegou aqui — você é o sexto a chegar — esses, eu mato. É
  meu trabalho. Foi uma cortesia te dizer isso antes."
- **EN:** "I'll be brief. The program that produced what you call
  karma is not my original design. I administer the local operation.
  There are other operations. Other cities. There is a model. The
  model is predictive — we knew this concentration would manifest
  visually in São Chico three years ago. We chose to maintain because
  the cost of reversing the program was high. It was a collective
  decision. I voted in favor. I won't deny it. You deserve honesty
  in this moment. What you're doing — walking up here — you're not
  the first. The other twenty-six tried, with more or less the same
  trajectory. Most were recruited before getting here. Those who
  arrived — you're the sixth to arrive — those, I kill. It's my
  job. Telling you this beforehand was a courtesy."

### Início do combate phase 1

#### `dlg_conselheiro_combate_inicio_001`
- **PT:** "Então é assim. Levanta a guarda."
- **EN:** "So be it. Guard up."

### Falas durante combate phase 1

#### `dlg_conselheiro_combat_p1_001`
- **PT:** "Diretiva."
- **EN:** "Directive."

#### `dlg_conselheiro_combat_p1_002`
- **PT:** "Reestruturação."
- **EN:** "Restructuring."

#### `dlg_conselheiro_combat_p1_003`
- **PT:** "Cláusula restritiva."
- **EN:** "Restrictive clause."

## Phase 2 — Quebra de elemento, terraço

### Cinematic da quebra (50% HP)

#### `dlg_conselheiro_phase2_inicio_001`
- **PT:** "Você é mais resiliente do que o modelo previu. Vamos pro
  ar livre."
- **EN:** "You're more resilient than the model predicted. Let's
  step outside."

> Cinematic 3s: parede do escritório se quebra (suporte estrutural
> cede pelo karma do Conselheiro). Os dois saem pro terraço.

### Falas durante combate phase 2

#### `dlg_conselheiro_combat_p2_001`
- **PT:** "Auditoria."
- **EN:** "Audit."

#### `dlg_conselheiro_combat_p2_002`
- **PT:** "Decisão final."
- **EN:** "Final decision."

#### `dlg_conselheiro_combat_p2_003`
- **PT:** "Pressão sustentada."
- **EN:** "Sustained pressure."

## Phase 3 — Absorção (scriptada)

### Pré-absorção (25% HP)

> Combate para. Cinematic. Conselheiro cambaleia, ajoelha. Não cai.
> Olha pra Dante.

#### `dlg_conselheiro_pre_absorcao_001`
- **PT:** "Você é interessante. Eu vi suas medições. Karma alto,
  pré-frontal intacto. Você é raro. Nós teríamos te recrutado.
  ...Você não vai aceitar a oferta, vai?"
- **EN:** "You're interesting. I saw your measurements. High karma,
  intact prefrontal cortex. You're rare. We would have recruited
  you. ...You're not going to accept the offer, are you?"

### Player choice — 3 respostas

#### `dlg_conselheiro_resposta_nao_001`
- **PT (Dante):** "Não."
- **EN (Dante):** "No."

#### `dlg_conselheiro_resposta_nem_chegou_001`
- **PT (Dante):** "Você nem chegou a oferecer."
- **EN (Dante):** "You didn't even offer."

#### `dlg_conselheiro_resposta_silencio_001`
- **PT (Dante):** "[silêncio]"
- **EN (Dante):** "[silence]"

### Reação a cada escolha (curtas)

#### `dlg_conselheiro_reacao_nao_001`
- **PT (Conselheiro):** "Direto. Eu respeito."
- **EN (Conselheiro):** "Direct. I respect that."

#### `dlg_conselheiro_reacao_nem_chegou_001`
- **PT (Conselheiro):** "Verdade. Eu deveria ter. É culpa minha."
- **EN (Conselheiro):** "True. I should have. That's on me."

#### `dlg_conselheiro_reacao_silencio_001`
- **PT (Conselheiro):** "Mais eloquente do que palavra. Continua."
- **EN (Conselheiro):** "More eloquent than words. Continue."

### Sequência de absorção — flashbacks

> Sequência scriptada compacta, ~15-20s total. Cada flash dura ~5s.
> Player não tem controle durante. Câmera muda.

#### `dlg_flashback_1_voz_feminina_001`
- **PT:** "Reverter o programa custaria 18% da margem."
- **EN:** "Reversing the program would cost 18% of the margin."

#### `dlg_flashback_1_voz_masculina_001`
- **PT:** "Mantenham. E descubram quem reportou pra imprensa."
- **EN:** "Maintain. And find out who reported to the press."

> Flash 1 visual: Conselheiro mais jovem na sala de reunião. Vira
> o retrato da família de cara pra baixo. Continua trabalhando.

#### `dlg_flashback_2_voz_superior_001`
- **PT:** "O modelo previu isso há três anos. Vocês escolheram
  preservar o caixa. Agora vão administrar a consequência. Façam o
  seu trabalho."
- **EN:** "The model predicted this three years ago. You chose to
  preserve cash flow. Now you'll administer the consequence. Do
  your job."

> Flash 2 visual: Conselheiro mais velho, em sala com várias telas.
> Numa: dados de "manifestações de campo anômalo" (karma) crescendo.
> Noutra: emails do Engenheiro sendo arquivados em série. Conselheiro
> abaixa a cabeça. Não responde.

### Pós-absorção

> Volta ao presente. Dante recolhe a mão. Conselheiro sentado no chão,
> esvaziado. Velho. Humano.

#### `dlg_conselheiro_pos_absorcao_001`
- **PT:** "Agora você sabe. Não, não exatamente. Você viu fragmentos.
  Mas sabe o que eu sabia. E sabe que eu não podia fazer nada com
  isso. [pausa] Eu nem sou o último andar. Eu nem sou o primeiro
  andar dos que decidem. Você ainda tem uma escalada longa, Dante.
  Eles vão te medir. Vão te recrutar. Ou vão te desligar. Eu... eu
  vou ficar aqui. Pode ir."
- **EN:** "Now you know. No, not exactly. You saw fragments. But
  you know what I knew. And you know I couldn't do anything about
  it. [pause] I'm not even the top floor. I'm not even the first
  floor of those who decide. You still have a long climb, Dante.
  They'll measure you. Recruit you. Or shut you down. I... I'll
  stay here. You can go."

### Pós-Ato 1, Conselheiro como NPC quebrado

> Acessível no escritório se player retornar. Apenas uma fala curta.
> Sem interação significativa.

#### `dlg_conselheiro_quebrado_001`
- **PT:** "Você voltou. Eu não tenho mais nada pra te dizer. Eu
  já não lembro a maior parte. Sem o karma, eu sou só um velho.
  Vai. Por favor."
- **EN:** "You came back. I have nothing more to say. I already
  don't remember most of it. Without karma, I'm just an old man.
  Go. Please."

---

# Narrador interno (Dante) — Torre

#### `dlg_dante_torre_chegada_001`
- **PT:** "Eu entrei nesse prédio cento e sete vezes durante meu
  contrato. Eu nunca passei do andar 22. Hoje eu vou pro 40."
- **EN:** "I entered this building one hundred and seven times during
  my contract. I never went past the 22nd floor. Today I'm going
  to the 40th."

#### `dlg_dante_subindo_001`
- **PT:** "Cada andar tem o eco de uma reunião. Eu sinto. Reuniões
  decidiram a vida da minha mãe e da minha cidade. Em salas com
  café. Em mesas redondas. Anotando metas em quadro branco."
- **EN:** "Each floor has the echo of a meeting. I feel it. Meetings
  that decided the life of my mother and my city. In rooms with
  coffee. At round tables. Writing targets on whiteboards."

#### `dlg_dante_pre_conselheiro_001`
- **PT:** "Eu cheguei até aqui. Eu não sou o primeiro, ele disse. Eu
  vou ser o que termina."
- **EN:** "I made it here. I'm not the first, he said. I'll be the
  one who finishes."

#### `dlg_dante_pos_conselheiro_001`
- **PT:** "Eu sei como curar eles agora. Eu absorvi também isso. Como
  reverter. Não totalmente, mas o suficiente. Tô voltando pra Vila."
- **EN:** "I know how to cure them now. I absorbed that too. How to
  reverse it. Not fully, but enough. I'm going back to Vila."

---

# Epílogo — Vila Rosário

## Cena na porta de casa

#### `dlg_lia_final_ato_001`
- **PT:** "Eu fiz café. Entra, vai."
- **EN:** "I made coffee. Come on in."

> Beat curto. Lia não pergunta o que aconteceu. Ela só sabe que
> aconteceu. Os dois entram em silêncio.

## Cena de cura — Quarto dos pais

> Sem diálogo durante a cura. Apenas o ritual. Dante senta na beira
> da cama de Dona Célia primeiro. Toca a mão dela. Karma sai
> visivelmente — partículas escuras que evaporam. Ela respira fundo.
> Não acorda imediatamente.

> Dante vai pro lado da cama de Seu Hélio. Mesmo ritual. Karma sai
> mais lentamente — havia mais. As mãos dele param de tremer.

> Dante fica parado por um momento entre os dois.

#### `dlg_dante_cura_001`
- **PT:** "Pronto. Tá feito."
- **EN:** "There. It's done."

## Cena no terraço da Vila — amanhecer

> Dante sentado na borda do terraço. São Chico ao longe. Névoa de
> karma diminuída nas zonas que ele explorou. Torre Horizonte
> apagada.

#### `dlg_dante_fim_ato_001`
- **PT:** "Eles disseram que eu não sou o último andar. E eu não
  sei o que isso significa. Mas a minha mãe respira. Meu pai respira.
  Hoje é só isso. Hoje, hoje basta."
- **EN:** "They said I'm not the top floor. And I don't know what
  that means. But my mother breathes. My father breathes. Today is
  only that. Today, today is enough."

## Cena do envelope

> Lia desce até a porta. Abre. Há um envelope no degrau. Sem
> remetente. Pega. Olha em volta. Ninguém. Volta pra dentro. Sobe
> ao terraço.

#### `dlg_lia_envelope_001`
- **PT:** "Tava na porta. Tem teu nome. Não tem remetente."
- **EN:** "It was at the door. It has your name. No return address."

> Dante abre. Conteúdo (close-up):

#### `dlg_envelope_conteudo_001`
- **PT:** "Sr. Dante, Acompanhamos seu trabalho recente com interesse.
  Suas medições estão notáveis. Gostaríamos de marcar uma conversa
  em ambiente mais adequado, em data e local de sua escolha — ou
  nossa, se preferir. Esta proposta é cortesia, não exigência. Sua
  família está bem. Esperamos que continue assim. — Conselho de
  Diretores."
- **EN:** "Mr. Dante, We have followed your recent work with interest.
  Your measurements are remarkable. We would like to arrange a
  conversation in a more appropriate setting, at a date and place
  of your choosing — or ours, if you prefer. This offer is courtesy,
  not requirement. Your family is well. We hope it stays that way.
  — Board of Directors."

> Dante segura o papel. Olha pra Lia. Lia olha pra ele. Silêncio.

#### `dlg_dante_envelope_001`
- **PT:** "Eles sabem onde a gente mora."
- **EN:** "They know where we live."

> Fade to black.

## Title cards

#### `dlg_titlecard_fim_ato_001`
- **PT:** "FIM DO ATO 1"
- **EN:** "END OF ACT 1"

---

# Pós-Ato 1 — Falas adicionais

## Dona Célia (pós-cura)

#### `dlg_celia_epilogo_001`
- **PT:** "Tô leve. Faz tempo que não tô leve. O que você fez,
  filho?"
- **EN:** "I feel light. I haven't felt light in so long. What did
  you do, son?"

> Já existe em 04a. Reativa após cura.

## Seu Hélio (pós-cura)

#### `dlg_helio_epilogo_001`
- **PT:** "Tá bom. Tá bom agora."
- **EN:** "It's good. It's good now."

> Já existe em 04a. Reativa após cura.

## Lia pós-Ato 1

#### `dlg_lia_pos_ato_001`
- **PT:** "Eles tão bem. A mãe quer fazer bolo. Ela disse que tu
  precisa engordar dois quilos. Eu disse que tu precisa dormir uma
  semana. Ela disse 'as duas coisas, Lia.' Acho que ela tá voltando
  mesmo."
- **EN:** "They're well. Mom wants to make cake. She said you need
  to gain two kilos. I said you need to sleep for a week. She said
  'both, Lia.' I think she's really coming back."

## Lia sobre o envelope

#### `dlg_lia_pos_envelope_001`
- **PT:** "Você vai responder? Não responde. Por enquanto não responde.
  A mãe acabou de voltar. A gente precisa de tempo. Eles que esperem."
- **EN:** "Are you going to answer? Don't. For now, don't. Mom just
  came back. We need time. Let them wait."

---

# Dialogue counts (this part)

| Character / context             | Lines | Notes |
|---------------------------------|-------|-------|
| Recepcionista corrompida        | 1     | Loop, single line |
| Funcionário escondido           | 3     | Lore + escolta |
| Servidor consciente             | 2     | Karma residual |
| O Conselheiro                   | 14    | All phases + post-Ato 1 |
| Flashback voices                | 3     | Sequence of absorption |
| Dante (internal narrator)       | 7     | Tower + epilogue |
| Lia (epilogue + envelope)       | 4     | Bridge to Act 2 |
| Dona Célia (post-cura)          | 1     | Reactivation |
| Seu Hélio (post-cura)           | 1     | Reactivation |
| Envelope content                | 1     | The Act 2 hook |
| Title card                      | 1     | "FIM DO ATO 1" |
| **Total**                       | **38**| **Part 04e** |

> Estimated final word count for this part:
> - PT-BR: ~2,800 words
> - EN: ~2,700 words

---

# Final implementation notes

## Conselheiro phases — gating

Phase transitions are HP-based and trigger scripted events:

- 100% → 50% HP: standard combat phase 1 (escritório)
- At 50% HP: cinematic 3s (parede quebra) → phase 2 (terraço)
- 50% → 25% HP: standard combat phase 2 (terraço)
- At 25% HP: combat halts, scripted absorption begins
- Absorption is one-way; no skip on first run
- Player input locked during absorption (~15-20s)
- Save replay can skip if player has already completed once

## Player choice during pre-absorção

Player picks from 3 responses to "Você não vai aceitar a oferta, vai?"
Each generates a different short reaction from Conselheiro. Choice
doesn't gate the absorption — it always happens. But it sets a flag
for replay value: `conselheiro_response = no | nem_chegou | silencio`.

This flag has no immediate Act 1 effect but reserves space for Act 2
to pick up.

## Sequência de absorção — implementation

Cinematic engine for absorption needs:
- Camera control (3 distinct angles per flash)
- Audio fade (dialogue overlap with ambient)
- Visual style (slight color shift to indicate flashback)
- Text-only display for the voice lines (no character on screen for
  voices)

If full cinematic system is too heavy for Sprint C8, MVP version:
- Black screen + white text (PT-BR + EN, language-aware)
- Voice lines shown sequentially with timing
- 5s per flash (~15s total flashbacks + ~5s wrap)
- Functional but minimal. Visual polish later.

## Envelope as save anchor

After `dlg_dante_envelope_001` and fade to black, **mandatory save
checkpoint**. Player can't lose this state. Save flags:

```
flag:ato1_completo = true
flag:envelope_recebido = true
flag:conselheiro_quebrado_alive = true
choice:conselheiro_response = (the chosen value)
```

The save loads back into the Vila terrace at sunrise, with player
free to roam São Chico (now in pós-Ato 1 state).

## Funcionário escondido escort

Side quest, optional. Reward:
- 200 karma
- Pendrive item (lore-only, easter egg)
- Special line from Inácio if Dante saved someone:
  `dlg_inacio_reacao_funcionario_001` — "Você salvou alguém. Sem
  precisar. Isso conta. Pra mim conta. Pra ele conta. Vai com isso."
  (already covered in 04c implicitly)

If player skips the escort, no penalty. Funcionário stays hidden,
Dante moves on.

## Title card spec

`dlg_titlecard_fim_ato_001` displays as full-screen text, white on
black, centered, ~3 seconds, language-aware. After title card,
brief silence (~2s), then save automático prompt.

## "Mãe quer fazer bolo"

Dona Célia's first post-cura action: wanting to bake. Callback to
`doc_receita_dona_celia` in Vila Rosário (the cornmeal cake recipe
"fazer no domingo quando o Dante vem"). Player who read the document
recognizes the closure. Pure narrative payoff, no mechanic.

## Hook to Act 2 — single line

The entire Act 2 setup distills into one sentence: *"Eles sabem onde
a gente mora."* Spoken by Dante. Heard by Lia. Read by player.

That's the hook. Personal. Ameaçador. Specific. The next Act
proceeds from there.

---

# All five parts — total dialogue counts

| Part | Title | Lines | Words PT (est.) | Words EN (est.) |
|------|-------|-------|-----------------|-----------------|
| 04a  | Family + Vila + Centro | 67 | 3,400 | 3,200 |
| 04b  | Parque + Industrial | 47 | 2,700 | 2,600 |
| 04c  | Morro hub + cerco | 66 | 4,200 | 4,000 |
| 04d  | Avenida + Rio + choices | 38 | 3,800 | 3,600 |
| 04e  | Torre + epilogue | 38 | 2,800 | 2,700 |
| **TOTAL** | **Act 1 complete** | **256** | **~16,900** | **~16,100** |

Roughly 33,000 words of bilingual dialogue across the Act. Within the
600-800 line target estimated at the start.
