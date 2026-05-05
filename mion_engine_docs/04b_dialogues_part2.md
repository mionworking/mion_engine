# 04b — Dialogues (Part 2: Parque Ibirapitanga + Distrito Industrial)

All Act 1 dialogues for Zones 2 and 3. Second of 5 dialogue parts.
This part covers ~80 lines.

> Companion to `01_implementation_plan.md` § Sprint C9.
> References content from `03a_zones_content_part1.md` (Parque) and
> `03b_zones_content_part2.md` (Industrial).
> Conventions same as 04a — see that doc for key format and faixa
> fallback rules.

---

# Parque Ibirapitanga NPCs

## Dra. Marina (bióloga)

**Tom:** Direção 2. Cientista articulada. Fascínio genuíno mesmo
diante do colapso. Fala com precisão técnica mas com peso humano por
trás.

### Primeira fala

#### `dlg_marina_intro_001`
- **PT:** "Não se mexe muito por uns segundos. Eu tô tentando ver se
  você ativa as ipês quando passa perto delas. Algumas pessoas ativam.
  ...Pronto. Ativa, mas pouco. Curioso. Você é a primeira pessoa que
  entra aqui há semanas. Eu sou a Marina. Bióloga. O parque era meu
  campo de pesquisa, ainda é, só que a metodologia mudou."
- **EN:** "Don't move for a few seconds. I'm trying to see if you
  activate the ipês when you pass near them. Some people do. ...There.
  You activate, but only slightly. Interesting. You're the first
  person to come in here in weeks. I'm Marina. Biologist. The park
  was my research field, still is, only the methodology changed."

### Teoria do karma

#### `dlg_marina_teoria_karma_001`
- **PT:** "O que eu observo é o seguinte. A energia — você chama de
  karma, eu prefiro 'campo anômalo' por enquanto, talvez eu chame de
  outra coisa daqui a um mês — ela é processada de forma neutra pela
  vida não-consciente. Plantas absorvem e crescem. Animais absorvem
  e se adaptam. Apenas humanos quebram. Por quê? Eu tenho uma hipótese.
  O filtro emocional. A natureza não tem culpa, medo, vergonha. Só
  recebe. Nós recebemos e transformamos. E o que transformamos é o
  que volta pra fora, materializado. Você tá entendendo? Quem sofre
  produz. Quem produz, faz isso aqui. Não é justiça. É só consequência.
  Sem juiz."
- **EN:** "Here's what I observe. The energy — you call it karma, I
  prefer 'anomalous field' for now, I'll probably call it something
  else in a month — is processed neutrally by non-conscious life.
  Plants absorb it and grow. Animals absorb it and adapt. Only humans
  break. Why? I have a hypothesis. The emotional filter. Nature has
  no guilt, fear, shame. Just receives. We receive and transform.
  And what we transform is what comes back out, materialized. You
  following? Whoever suffers produces. Whoever produces, does this.
  It's not justice. It's just consequence. No judge."

### Oferece serviço

#### `dlg_marina_oferece_servico_001`
- **PT:** "Eu mapeei a região. Antes do colapso por dever profissional,
  depois por sobrevivência. Posso te mostrar o que tem perto. 60 karma
  — sim, eu também aceito karma agora, não me julga. Quer?"
- **EN:** "I mapped the region. Before the collapse out of professional
  duty, after for survival. I can show you what's nearby. 60 karma —
  yes, I take karma now too, don't judge. Want it?"

### Recrutamento

#### `dlg_marina_recrutamento_001`
- **PT:** "Eu preciso de mais dados. E você é dado. Você processa o
  karma de um jeito que eu não vi em ninguém. Eu vou contigo. Não te
  pergunto, te aviso. Se eu não documentar isso, ninguém vai. E aí
  isso aqui foi tudo em vão."
- **EN:** "I need more data. And you are data. You process karma in
  a way I haven't seen in anyone. I'm coming with you. I'm not
  asking, I'm telling. If I don't document this, no one will. And
  then all of this was for nothing."

### Falas em combate (após recrutada)

#### `dlg_marina_combat_iniciar_001`
- **PT:** "Anotando."
- **EN:** "Taking notes."

#### `dlg_marina_combat_buff_aliado_001`
- **PT:** "Suporte ativado."
- **EN:** "Support active."

#### `dlg_marina_combat_critico_001`
- **PT:** "Padrão hostil identificado!"
- **EN:** "Hostile pattern identified!"

#### `dlg_marina_combat_inimigo_morto_001`
- **PT:** "Espécime neutralizado."
- **EN:** "Specimen neutralized."

#### `dlg_marina_combat_zona_clear_001`
- **PT:** "Estabilizou. Por agora."
- **EN:** "Stable. For now."

### Reação por zona

#### `dlg_marina_zone_industrial_001`
- **PT:** "Concentração de campo aqui é três vezes a do parque. Três
  vezes. E não é fauna. É só sofrimento residual. Anote isso na sua
  cabeça."
- **EN:** "Field concentration here is three times the park's. Three
  times. And it's not fauna. It's just residual suffering. File that
  in your head."

#### `dlg_marina_zone_morro_001`
- **PT:** "Curioso. A leitura aqui é menor do que esperado. Eu acho
  que solidariedade neutraliza o campo. Não tenho como provar ainda."
- **EN:** "Interesting. The readings here are lower than expected. I
  think solidarity neutralizes the field. I can't prove it yet."

#### `dlg_marina_zone_avenida_001`
- **PT:** "A leitura aqui é... organizada. Não é o caos das outras
  zonas. Alguém canalizou. Isso é pior do que aleatório."
- **EN:** "The readings here are... organized. Not the chaos of the
  other zones. Someone channeled it. That's worse than random."

#### `dlg_marina_zone_rio_001`
- **PT:** "A água carrega tudo. Sempre carregou. Quem morava aqui
  bebeu antes de qualquer um perceber. Eu li sobre. Ninguém leu junto."
- **EN:** "The water carries everything. Always did. Whoever lived
  here drank it before anyone noticed. I read about it. No one read
  with me."

#### `dlg_marina_zone_torre_001`
- **PT:** "A leitura aqui... eu nunca vi isso. Não é o campo do
  centro. É um campo que sabe o que tá fazendo."
- **EN:** "The readings here... I've never seen this. It's not the
  centro's field. It's a field that knows what it's doing."

---

## Velho André (banco da praça do parque)

**Tom:** Direção 1 com sabedoria popular. Velho que viu muito. Pouca
fala, observação afiada.

#### `dlg_andre_intro_001`
- **PT:** "Senta um pouco, moço. Não tô te pedindo nada, só
  conversa. Esse banco tá vazio há três meses. Tu é o primeiro que
  passa aqui que ainda enxerga banco como banco e não como obstáculo."
- **EN:** "Sit a bit, son. I'm not asking anything, just conversation.
  This bench has been empty three months. You're the first one
  passing through who still sees a bench as a bench and not an
  obstacle."

#### `dlg_andre_observacao_001`
- **PT:** "Esse parque era horrível, viu. Mato alto, cheio de bicho,
  agulha no chão. Prefeitura nunca cuidou. Agora tá... bonito.
  Esquisito. Acho que natureza tem mais resistência que gente. Vai
  saber."
- **EN:** "This park used to be awful, you know. Tall grass, full of
  bugs, needles on the ground. The city never cared. Now it's...
  beautiful. Weird. I think nature has more resilience than people.
  Go figure."

#### `dlg_andre_observacao_002_alta`
- **PT:** "Tá pesado em ti, moço. Não fala nada, eu vejo. Senta um
  minuto antes de seguir. Só um minuto. O parque não vai a lugar
  nenhum."
- **EN:** "You're heavy, son. Don't say anything, I see. Sit a minute
  before you go on. Just a minute. The park's not going anywhere."

---

# O Jardineiro (mini-boss)

**Tom:** apenas grunhidos vegetais durante o combate. Lucidez na
morte — Direção 2 quebrada, paz triste.

### Combate

Sem diálogo. Sons orgânicos: raízes se movendo, folhas farfalhando,
respiração pesada que parece terra úmida.

### Morte

#### `dlg_jardineiro_morte_001`
- **PT:** "...as árvores. Tão felizes. Nunca tiveram... tanta
  energia. Eu cuidei delas trinta anos. Agora... agora elas cuidam
  de mim. Tá tudo bem. Tá tudo... bem."
- **EN:** "...the trees. They're happy. They've never had... this
  much energy. I cared for them thirty years. Now... now they care
  for me. It's okay. It's all... okay."

> Sem skip. Cena dura ~5s após dano final. O corpo se dissolve em
> pétalas e raízes que voltam à terra.

---

# Distrito Industrial NPCs

## Reginaldo (Sindicalista)

**Tom:** Direção 1 puro. Agitador, bem-humorado mesmo no fim do
mundo. Voz alta, gesticula. Veterano de greve.

### Primeira fala

#### `dlg_reginaldo_intro_001`
- **PT:** "E aí, parceiro! Senta aqui, fogueira tá quente. Não tem
  café porque acabou na semana passada e ninguém entrega mais nada,
  mas tem um chimarrão que tá circulando há três dias e ainda dá
  pra mais. Eu sou o Reginaldo. Reginaldo do sindicato — ex,
  agora. Sindicato não existe mais. Mas eu existo, e tô aqui."
- **EN:** "Hey, friend! Sit here, fire's warm. No coffee 'cause it
  ran out last week and nobody delivers anything anymore, but
  there's a chimarrão going around for three days and there's still
  some left. I'm Reginaldo. Reginaldo from the union — ex, now.
  Union doesn't exist anymore. But I exist, and I'm here."

### Lore industrial

#### `dlg_reginaldo_lore_industrial_001`
- **PT:** "Quer saber o que aconteceu nessa fábrica aqui? Tudo. Tudo
  aconteceu. Trinta anos de hora extra não paga. Quinze anos pelejando
  pra mudar isso. Acidente todo mês — sempre 'culpa do funcionário',
  sempre 'falha individual'. A gente teve um colega — Aparício se
  chamava — que perdeu o braço em prensa hidráulica. Empresa pagou
  três salários e mandou embora dizendo que ele tava 'distraído'.
  Distraído. Trinta e dois anos de empresa. Distraído. Isso aqui não
  ficou desse jeito por acaso. A energia não criou isso. A energia só
  fez ficar visível."
- **EN:** "You want to know what happened at this factory? Everything.
  Everything happened. Thirty years of unpaid overtime. Fifteen
  years fighting to change it. An accident every month — always
  'employee fault', always 'individual failure'. We had a coworker —
  Aparício was his name — who lost his arm in a hydraulic press.
  Company paid three months' salary and fired him saying he was
  'distracted'. Distracted. Thirty-two years at the company.
  Distracted. This place didn't end up this way by accident. The
  energy didn't create this. The energy just made it visible."

### Oferece buff

#### `dlg_reginaldo_oferece_buff_001`
- **PT:** "Tu tá indo entrar lá? Olha, deixa eu te dar uma motivação.
  100 karma, sim. É o que circula agora. Mas eu te garanto: se a
  gente aguentou turno de 12 horas sem banheiro, tu aguenta isso aí
  com mais força. Aceita?"
- **EN:** "You're going in there? Look, let me give you some
  motivation. 100 karma, yeah. That's what circulates now. But I
  guarantee you: if we put up with 12-hour shifts without a bathroom
  break, you can handle that with more strength. Deal?"

### Reage ao documento do pesquisador

#### `dlg_reginaldo_reage_pesquisador_001`
- **PT:** "Caralho. Caralho. Eu lembro desse nome. Faz dez anos. O
  cara veio na sede, queria conversar com a gente. Ninguém deu bola
  porque ele falava complicado, falava de 'campo' não sei o quê. A
  gente tava tentando garantir que pagassem nossa hora extra, sabe?
  Não era ele. Era a empresa. E ele tava tentando avisar a gente. E
  a gente não escutou. Ninguém escutou ninguém nessa cidade, parceiro.
  Por isso tá assim."
- **EN:** "Damn. Damn. I remember that name. Ten years ago. The guy
  came to our office, wanted to talk to us. Nobody paid attention
  because he talked complicated, talked about 'field' something. We
  were trying to make sure they paid our overtime, you know? Wasn't
  him. It was the company. And he was trying to warn us. And we
  didn't listen. Nobody in this city listened to anybody, friend.
  That's why it's like this."

### Recrutamento

#### `dlg_reginaldo_recrutamento_001`
- **PT:** "Tu vai mais longe que eu, parceiro. Eu tô velho pra subir
  torre, mas eu não tô velho pra te bater na lateral. Me leva. Quem
  sabe assim a gente fecha algum capítulo dessa porra toda."
- **EN:** "You're going further than me, friend. I'm too old to climb
  towers, but I'm not too old to hit the flanks. Take me. Maybe like
  this we close some chapter of this whole shit."

### Saída do roster (se Dante negocia com facção)

#### `dlg_reginaldo_saida_001`
- **PT:** "Tu negociou com aqueles caras. Eu vi. Não, não me explica
  — eu sei que tu tem motivo. Eu sei que tu tem cabeça. Mas eu não
  posso ficar contigo. Eu passei trinta anos brigando contra exatamente
  isso. Se eu ficar agora, eu fui pra essas trinta anos. Tu entende.
  Eu vou ficar aqui. Volta quando quiser conversar. Mas não em
  combate. Não nesse, pelo menos."
- **EN:** "You negotiated with those guys. I saw. No, don't explain
  — I know you have a reason. I know you have a head on your
  shoulders. But I can't stay with you. I spent thirty years fighting
  against exactly this. If I stay now, I betrayed those thirty
  years. You understand. I'll stay here. Come back when you want to
  talk. But not in combat. Not this one, at least."

### Falas em combate

#### `dlg_reginaldo_combat_iniciar_001`
- **PT:** "Vamo!"
- **EN:** "Let's go!"

#### `dlg_reginaldo_combat_buff_aliado_001`
- **PT:** "Aguenta firme!"
- **EN:** "Hold the line!"

#### `dlg_reginaldo_combat_critico_001`
- **PT:** "Cuidado aí!"
- **EN:** "Watch out!"

#### `dlg_reginaldo_combat_inimigo_morto_001`
- **PT:** "Mais um."
- **EN:** "Another one down."

#### `dlg_reginaldo_combat_zona_clear_001`
- **PT:** "Vitória da classe trabalhadora, parceiro."
- **EN:** "Working class victory, friend."

### Reação por zona

#### `dlg_reginaldo_zone_morro_001`
- **PT:** "A Marta. Velha amiga. Não, ela vai dizer que nunca foi
  amiga. Ela tá certa. Mas a gente lutou junto em 2008. Manda lembrança."
- **EN:** "Marta. Old friend. No, she'll say she was never my friend.
  She's right. But we fought together in 2008. Send her my regards."

#### `dlg_reginaldo_zone_avenida_001`
- **PT:** "Avenida Progresso. Que nome irônico, hein. Progresso de
  quem? Sempre foi pergunta sem resposta nessa cidade."
- **EN:** "Avenida Progresso. What an ironic name, huh. Progress for
  whom? Always was a question without an answer in this city."

#### `dlg_reginaldo_zone_torre_001`
- **PT:** "A Torre. Parceiro. Tu vai entrar ali pelo elevador de
  visitante ou pelo de serviço? Pergunta pessoal. A diferença é
  enorme."
- **EN:** "The Tower. Friend. You going in through the visitor
  elevator or the service one? Personal question. Big difference."

---

# A Máquina (mini-boss)

**Tom:** sem diálogo. Sons mecânicos exclusivamente.

### Combate

Sons:
- Sirene industrial baixa
- Prensa batendo no chão (impacto rítmico)
- Esteiras zumbindo
- Faíscas crepitando
- Em phase 2: alarme estridente, descarga elétrica

> A Máquina não fala porque nunca foi consciente o bastante pra falar.
> É resíduo emocional num objeto. Quando "morre", apenas para. Sons
> param em sequência: alarme → faíscas → esteiras → prensa → silêncio.

### Pós-morte

**Sem diálogo.** 5 segundos de silêncio absoluto antes de Dante poder
se mover novamente.

---

# Inimigos do Industrial — falas curtas durante combate

## Operário Corrompido

**Tom:** murmúrios fragmentados. Resíduo de fala no estado corrompido.

#### `dlg_factory_worker_combat_001`
- **PT:** "...meta..."
- **EN:** "...target..."

#### `dlg_factory_worker_combat_002`
- **PT:** "...turno..."
- **EN:** "...shift..."

#### `dlg_factory_worker_combat_003`
- **PT:** "...mais cinco..."
- **EN:** "...five more..."

## Supervisor Corrompido

**Tom:** ordens secas, gritadas.

#### `dlg_supervisor_combat_001`
- **PT:** "PRODUZ!"
- **EN:** "PRODUCE!"

#### `dlg_supervisor_combat_002`
- **PT:** "MAIS RÁPIDO!"
- **EN:** "FASTER!"

#### `dlg_supervisor_combat_003`
- **PT:** "NÃO PARA!"
- **EN:** "DON'T STOP!"

---

# Narrador interno (Dante)

### Primeira impressão do parque

#### `dlg_dante_natureza_001`
- **PT:** "Tudo aqui tá... vivo. Vivo demais. Eu não esperava bonito."
- **EN:** "Everything here is... alive. Too alive. I wasn't expecting
  beauty."

### Pós-Jardineiro

#### `dlg_dante_pos_jardineiro_001`
- **PT:** "Ele foi embora em paz. Acho que ele foi o único."
- **EN:** "He went in peace. I think he was the only one."

### Primeira impressão do industrial

#### `dlg_dante_industrial_001`
- **PT:** "O peso aqui dentro é diferente. Lembrei do meu pai chegando
  em casa de obra. Sabia o cheiro antes da porta abrir."
- **EN:** "The weight in here is different. Reminded me of my father
  coming home from a job. I knew the smell before the door opened."

### Pós-A Máquina

#### `dlg_dante_pos_maquina_001`
- **PT:** "Ela parou. Ela só parou. Como se tivesse esperando alguém
  apertar o botão certo."
- **EN:** "It stopped. It just stopped. Like it was waiting for
  someone to press the right button."

### Descoberta do diário do pesquisador

#### `dlg_dante_pesquisador_001`
- **PT:** "Dois anos. Dois anos antes. Alguém mediu, mostrou, e os
  caras lá em cima arquivaram. Eles sabiam. Eles sempre souberam."
- **EN:** "Two years. Two years before. Someone measured, showed it,
  and the guys up there archived it. They knew. They always knew."

---

# Action event dialogues

## Mariano lateral (easter egg da Parte 1, ativa aqui)

#### `dlg_dante_mariano_001`
- **PT:** "É o cara que o Quinho falou. 'Foi pro centro e voltou
  esquisito.' Era ele."
- **EN:** "It's the guy Quinho mentioned. 'Went to centro and came
  back weird.' That was him."

> Triggered when player encounters Mariano (Centro Velho lateral) AND
> already heard `dlg_quinho_rumores_centro_001`. Internal monologue.

---

# Dialogue counts (this part)

| Character / context             | Lines | Notes |
|---------------------------------|-------|-------|
| Marina (biologist)              | 14    | Theory + ally + zone reactions |
| Velho André (park bench)        | 3     | Lore + faixa variation |
| O Jardineiro (mini-boss)        | 1     | Death only (combat is silent) |
| Reginaldo (Sindicalista)        | 17    | Most dialogue of any ally |
| A Máquina (mini-boss)           | 0     | Sound only |
| Factory enemies (combat lines)  | 6     | Worker + Supervisor |
| Dante (internal narrator)       | 5     | Park + Industrial reactions |
| Mariano cross-reference         | 1     | Easter egg internal line |
| **Total**                       | **47**| **Part 04b** |

> Estimated final word count for this part:
> - PT-BR: ~2,700 words
> - EN: ~2,600 words

---

# Implementation notes

## For Sprint C9 integration

- Reginaldo has the most lines because he's the most vocal ally
  (Direção 1, agitador). Lia and Marina are quieter by personality.
- O Jardineiro and A Máquina have minimal/no dialogue intentionally
  — narrative weight comes from action and environment.
- Combat lines for enemies (factory worker, supervisor) are short and
  trigger on attack animation. Implementation: pool of lines per
  enemy type, random selection on attack with cooldown to avoid spam.

## Reginaldo's "saída" (departure on faction negotiation)

`dlg_reginaldo_saida_001` triggers when:
- Reginaldo is in the active roster (or recruited but inactive)
- AND `choice:faction == Negotiated` (set in 04d via Avenida choice)

After this dialogue:
- `recruit_state:reginaldo = LEFT_PERMANENTLY`
- He's no longer selectable
- Returning to the Industrial border, he's not there anymore (or sitting
  silently with no interaction)

## Word "chimarrão"

Kept untranslated in EN. It's a Brazilian/Southern beverage. Adds
local color. Player who doesn't know learns from context.

## "Caralho"

Reginaldo is the only character who uses strong cursing in 04b. Per
the user preferences, language has natural orality but isn't gratuitous
— it lands when it lands.
