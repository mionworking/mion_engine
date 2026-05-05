# 04d — Dialogues (Part 4: Avenida Progresso + Rio Turvão + Choices)

All Act 1 dialogues for Zones 5 and 6, plus the faction choice
ramifications. Fourth of 5 dialogue parts. This part covers ~95 lines.

> Companion to `01_implementation_plan.md` § Sprint C9, C8 (faction
> choice).
> References content from `03b_zones_content_part2.md` (Avenida) and
> `03c_zones_content_part3.md` (Rio).
> Conventions same as 04a.

---

# Avenida Progresso NPCs

## Vinícius (negociador da facção)

**Tom:** Direção 2. Polido, articulado, ameaçador sem precisar ameaçar.
Voz controlada de quem sempre teve poder e nunca precisou levantar a
voz pra exercê-lo. Sorriso que não chega aos olhos.

### Primeira fala

#### `dlg_vinicius_intro_001`
- **PT:** "Sente-se, por favor. Café? Não? Estamos com algumas
  reservas — fica pra próxima. Meu nome é Vinícius. Você é Dante.
  Tive prazer em receber seu nome através de pessoas que respeito.
  Talvez eu possa te oferecer trinta minutos do meu tempo agora.
  Talvez você se interesse pelo que eu tenho a propor. Em todo caso,
  deve ser uma conversa civilizada."
- **EN:** "Please, sit. Coffee? No? We're a bit short on reserves —
  next time. My name is Vinícius. You're Dante. I've had the
  pleasure of receiving your name from people I respect. Perhaps I
  can offer you thirty minutes of my time now. Perhaps you'll be
  interested in what I have to propose. In any case, it should be a
  civilized conversation."

### A proposta

#### `dlg_vinicius_proposta_001`
- **PT:** "Vou ser direto, com a polidez devida. Esta zona é
  estável. Estável porque algumas pessoas — nós — escolheram
  estabilizá-la. Há recursos. Há água, há remédio, há comida que não
  estragou ainda. Tudo isso passa por nós. Por uma questão de
  organização, claro. Caos seria irresponsável. Os habitantes que
  permanecem contribuem com o que podem — karma, principalmente.
  Você poderia, simplesmente, seguir o seu caminho sem nos atrapalhar.
  Seríamos gratos. Em troca, ofereceríamos acesso a recursos que
  você precisa para subir naquela Torre. E acredite — você precisa.
  Você tem três opções, não duas. Pode aceitar e seguir. Pode aceitar
  e nos trair depois. Pode recusar e ir lutar contra o Síndico.
  Cada escolha tem seu preço. Nenhuma é gratuita. Eu apenas peço
  que você escolha consciente."
- **EN:** "I'll be direct, with due politeness. This zone is stable.
  Stable because some people — us — chose to stabilize it. There are
  resources. Water, medicine, food that hasn't spoiled yet. All of
  it passes through us. As an organizational matter, of course.
  Chaos would be irresponsible. The remaining residents contribute
  what they can — karma, mostly. You could simply go on your way
  without troubling us. We would be grateful. In exchange, we would
  offer access to resources you need to climb that Tower. And
  believe me — you need them. You have three options, not two. You
  can accept and proceed. You can accept and betray us later. You
  can refuse and go fight the Síndico. Each choice has its price.
  None is free. I only ask that you choose consciously."

### Após confronto (se Dante recusou)

#### `dlg_vinicius_apos_confronto_001`
- **PT:** "O Síndico está morto. Imagino que você queira que eu lhe
  diga algo — um discurso de antagonista derrotado, talvez. Não
  tenho um. Você fez sua escolha. Eu farei a minha. Não vou impedir
  você de continuar. Eu lembrarei do seu nome, no entanto. E lembrar
  é, em alguns contextos, um problema."
- **EN:** "The Síndico is dead. I imagine you want me to say
  something — a defeated antagonist's speech, perhaps. I don't have
  one. You made your choice. I'll make mine. I won't stop you from
  continuing. I will remember your name, however. And remembering
  is, in some contexts, a problem."

### Após negociação (se Dante aceitou)

#### `dlg_vinicius_apos_negociacao_001`
- **PT:** "Excelente decisão. Os recursos prometidos estão liberados.
  Você notará que algumas portas se abrem mais facilmente agora.
  Algumas vozes silenciam. Não é mágica — é cooperação. Eu mantenho
  meu lado. Espero que você mantenha o seu. E, Dante — eu sei que
  algumas pessoas no morro estão decepcionadas. É natural. Pessoas
  decepcionadas, eventualmente, se acalmam. Sempre se acalmam.
  Boa sorte na sua escalada."
- **EN:** "Excellent decision. The promised resources are released.
  You'll notice some doors open more easily now. Some voices go
  silent. It's not magic — it's cooperation. I'll hold up my side.
  I trust you'll hold up yours. And, Dante — I know some people on
  the morro are disappointed. It's natural. Disappointed people,
  eventually, calm down. They always calm down. Good luck on your
  climb."

### Após traição (se Dante aceitou e depois traiu)

#### `dlg_vinicius_apos_traicao_001`
- **PT:** "Inteligente. Sinceramente. Você aceitou os recursos, usou
  o que precisava, e nos eliminou no momento de menor exposição. Isso
  é uma habilidade que admiramos, em geral. Em qualquer outra
  circunstância, eu estaria considerando uma oferta de emprego. Em
  esta — eu estou considerando outra coisa. Não vou te ameaçar
  agora. Seria deselegante. Mas você acabou de entrar em um conjunto
  específico de pessoas que eu não esqueço. Considere isso uma
  cortesia: você foi avisado."
- **EN:** "Clever. Sincerely. You accepted the resources, used what
  you needed, and eliminated us at the moment of least exposure.
  That's a skill we admire, in general. In any other circumstance,
  I'd be considering a job offer. In this one — I'm considering
  something else. I won't threaten you now. It would be inelegant.
  But you've just entered a specific set of people I don't forget.
  Consider this a courtesy: you've been warned."

---

## Senhora Cíntia (moradora do condomínio)

**Tom:** Direção 1, mas educada — não casca grossa. Mulher de classe
média alta, 60 anos, viúva, sozinha no condomínio. Ainda tenta manter
forma e modos.

### Primeira fala

#### `dlg_cintia_intro_001`
- **PT:** "Não atira. Por favor. Eu... eu moro aqui. Eu moro aqui há
  vinte anos. O meu marido morreu em 2019, antes disso tudo. Eu
  fiquei. Eu não pedi por isso que tá acontecendo. Eu também tô com
  medo. Eu sei que é fácil você me odiar — eu vivo num condomínio,
  isso é uma posição. Mas eu não fiz parte das decisões. Eu só
  votava como eu era ensinada a votar. Eu só morava onde meu marido
  comprou. Você entende? Talvez não. Mas eu queria pelo menos te
  pedir pra entender."
- **EN:** "Don't shoot. Please. I... I live here. I've lived here
  for twenty years. My husband died in 2019, before all of this. I
  stayed. I didn't ask for what's happening. I'm scared too. I know
  it's easy to hate me — I live in a condo, that's a position. But
  I wasn't part of the decisions. I just voted the way I was taught
  to vote. I just lived where my husband bought. Do you understand?
  Maybe not. But I'd like at least to ask you to understand."

### Após confronto

#### `dlg_cintia_apos_confronto_001`
- **PT:** "Você matou o Síndico. Obrigada. Sim, obrigada. Ele me
  fez assinar uma renúncia ao meu apartamento outro dia. Eu assinei
  porque tinha uma arma na minha mesa de jantar. Eu sei o que essa
  facção fazia. A maioria de nós sabia. Ninguém falava. Agora a
  gente vai ter que aprender a falar. Eu acho. Eu não sei se eu
  consigo, mas eu vou tentar."
- **EN:** "You killed the Síndico. Thank you. Yes, thank you. He
  made me sign over my apartment the other day. I signed because
  there was a gun on my dining table. I know what that faction did.
  Most of us knew. Nobody talked. Now we'll have to learn to talk.
  I think. I don't know if I can, but I'll try."

### Após negociação

#### `dlg_cintia_apos_negociacao_001`
- **PT:** "Você... entrou em acordo com eles? Eu vi o Vinícius
  sorrindo essa manhã. Ele só sorri quando ganha. Eu não te julgo,
  eu não tenho esse direito — eu vivi anos do silêncio que você
  acabou de comprar com algumas horas de conversa. Mas eu te peço
  uma coisa. Quando você subir lá na Torre e descobrir o que
  descobrir — não esquece de mim. Não esquece da gente que ficou
  aqui esperando que alguém viesse. A gente continua esperando."
- **EN:** "You... made a deal with them? I saw Vinícius smiling this
  morning. He only smiles when he wins. I don't judge you, I don't
  have that right — I lived years of the silence you just bought
  with a few hours of conversation. But I ask you one thing. When
  you climb up that Tower and find what you find — don't forget me.
  Don't forget the people who stayed here waiting for someone to
  come. We're still waiting."

---

# O Síndico (mini-boss)

**Tom:** Direção 2. Frio, calmo, certo de si. Não acredita ser
vilão — acredita estar fazendo o necessário.

### Início do combate

#### `dlg_sindico_intro_combate_001`
- **PT:** "Sente-se, se quiser. Não? Tudo bem. Vou continuar minha
  água. Você é o Dante. O Vinícius me falou. Vou economizar nosso
  tempo. Eu não sou o que você pensa que eu sou. Eu sou apenas a
  pessoa que ficou em pé quando todo mundo saiu correndo. Alguém
  precisava organizar. Eu organizei. As pessoas que continuam aqui
  estão vivas. Isso não é vitória pequena. Você acredita que não
  poderia haver outro caminho. Eu acredito que houve, e ele teria
  matado mais gente. A diferença entre nós dois é que eu tomei
  decisões e você só vai tomar agora. Vamos? Eu termino esse copo."
- **EN:** "Sit, if you want. No? That's fine. I'll continue my
  water. You're Dante. Vinícius told me. Let me save us time. I'm
  not what you think I am. I'm just the person who stayed standing
  when everyone else ran. Someone had to organize. I organized.
  The people still here are alive. That's no small victory. You
  believe there could have been another way. I believe there was,
  and it would have killed more people. The difference between us
  is that I made decisions and you're only about to. Shall we? I'll
  finish this glass."

### Falas durante combate (Phase 1)

#### `dlg_sindico_combat_001`
- **PT:** "Saiam daqui."
- **EN:** "Leave."

#### `dlg_sindico_combat_002`
- **PT:** "Não podem estar aqui."
- **EN:** "You cannot be here."

#### `dlg_sindico_combat_003`
- **PT:** "Está proibido."
- **EN:** "This is prohibited."

### Falas durante combate (Phase 2)

#### `dlg_sindico_combat_phase2_001`
- **PT:** "Cumpram as regras."
- **EN:** "Follow the rules."

#### `dlg_sindico_combat_phase2_002`
- **PT:** "Eu mando aqui."
- **EN:** "I'm in charge here."

### Morte

#### `dlg_sindico_morte_001`
- **PT:** "Mas eu... eu seguia as regras. As mesmas. Por que..."
- **EN:** "But I... I followed the rules. The same ones. Why..."

> Última fala. Morre confuso, não revoltado. Genuinamente não entende
> por que falhou. O sistema sempre o recompensou.

---

# Rio Turvão NPCs

## Seu Zequinha (pescador ribeirinho)

**Tom:** Direção 1 com sabedoria popular. Velho de rio. Pouco mas
preciso.

### Primeira fala

#### `dlg_zequinha_intro_001`
- **PT:** "Sou eu, sim, o Zequinha. O resto que tinha aqui, foi.
  Pesco do mesmo jeito. Não pega mais peixe, mas a linha precisa
  ficar na água, isso me obriga a sentar todo dia, e sentar todo
  dia me obriga a continuar. Tu veio do norte. Veio buscar o quê?"
- **EN:** "Yeah, it's me, Zequinha. The rest that was here, gone. I
  fish the same way. No fish anymore, but the line needs to be in
  the water, that obliges me to sit every day, and sitting every day
  obliges me to continue. You came from the north. What are you
  here for?"

### Lore do rio

#### `dlg_zequinha_lore_rio_001`
- **PT:** "O rio mudou. Sempre mudou, mas isso é diferente. Quando
  eu era moleque, dava pra pescar lambari, traíra, até cascudo. Aí
  veio fábrica. Aí veio cano de esgoto. Aí o rio ficou marrom, mas
  ainda dava peixe pequeno. Aí — uns três, quatro anos atrás, eu
  acho — começou a aparecer um brilho na água, à noite. Bonito. Eu
  achava bonito. Os peixes começaram a sumir. Mas as pessoas
  continuaram bebendo. As crianças continuaram nadando. Eu falava
  pra prefeitura, falava na rádio comunitária. Ninguém ouviu. Aí
  morreu gente. Aí morreu mais. Aí o rio ficou desse jeito. Brilho
  bonito, peixe nenhum, gente nenhuma."
- **EN:** "The river changed. Always changed, but this is different.
  When I was a boy, you could catch lambari, traíra, even cascudo.
  Then factories came. Then sewer pipes. Then the river turned
  brown, but you could still catch small fish. Then — three, four
  years ago, I think — a glow started showing up in the water, at
  night. Pretty. I thought it was pretty. The fish started
  disappearing. But people kept drinking. Children kept swimming.
  I told the city council, told the community radio. Nobody
  listened. Then people died. Then more died. Then the river
  became this. Pretty glow, no fish, no people."

### Oferece serviço

#### `dlg_zequinha_oferece_servico_001`
- **PT:** "Posso te mostrar onde dá pra pisar com segurança nessas
  trilhas. Onde tem gente corrompida, onde tem armadilha de trincheira,
  onde tem buraco no chão. Cinquenta karma. Eu uso pra pagar o
  remédio que o Zé do morro me manda — sim, o Zé. A gente é parente
  distante, primo de primo, mas a gente se vira. Pega ou não?"
- **EN:** "I can show you where it's safe to step on these trails.
  Where there's corrupted folk, where there's pit trap, where the
  ground gives out. Fifty karma. I use it to pay for medicine that
  Zé from the morro sends me — yes, Zé. We're distant family, cousin
  of cousin, but we work it out. Yes or no?"

### Sobre o Engenheiro

#### `dlg_zequinha_sobre_engenheiro_001`
- **PT:** "Tu vai entrar na estação de tratamento? Tem um cara lá
  dentro. Eu o conhecia. Ele vinha aqui no cais às vezes, tarde da
  noite. Quase nunca falava. Uma vez, só uma vez, ele me disse 'Seu
  Zequinha, eu sei sobre o rio'. Eu perguntei o que ele sabia. Ele
  só balançou a cabeça. Tomou cerveja de garrafa direto, foi embora.
  Eu acho que ele tava tentando dizer alguma coisa e não conseguia.
  Tu vai encontrar ele lá. Tenta lembrar — ele tava tentando."
- **EN:** "You're going into the treatment station? There's a guy
  in there. I knew him. He used to come to the dock sometimes, late
  at night. Hardly spoke. Once, just once, he told me 'Seu
  Zequinha, I know about the river'. I asked what he knew. He just
  shook his head. Drank beer straight from the bottle, left. I
  think he was trying to say something and couldn't. You'll find
  him in there. Try to remember — he was trying."

### Sobre o pescador morto (memorial)

#### `dlg_zequinha_sobre_manoel_001`
- **PT:** "O Manoel? O Manoel pescou aqui antes de mim. Sessenta
  anos no rio. Eu aprendi com ele. Ele bebia da água porque sempre
  bebeu. Morreu sem saber por quê. Esse povo todo dessa lista — o
  povo do bairro norte — eles morreram sem saber. Tu vai descobrir
  o porquê, não vai? Faz isso. Por todos eles. E por mim, que ainda
  tô aqui."
- **EN:** "Manoel? Manoel fished here before me. Sixty years on the
  river. I learned from him. He drank the water because he always
  did. Died without knowing why. All these people on this list —
  the north neighborhood people — they died without knowing. You'll
  find out why, won't you? Do that. For all of them. And for me,
  still here."

---

# O Engenheiro (mini-boss)

**Tom:** Direção 2 puro. Lúcido durante o combate inteiro. Único
boss do Ato 1 que conversa com clareza enquanto luta. Tristeza
profunda em cada palavra.

### Início do combate

#### `dlg_engenheiro_intro_combate_001`
- **PT:** "Eu sabia que alguém ia chegar até aqui um dia. Achei que
  fosse repórter. Não, mentira — eu nunca achei isso. Eu sabia que
  ia ser alguém igual a você. Alguém com peso. Alguém com motivo.
  Eu não vou te receber bem. Eu queria poder. Mas eu não controlo
  isso aqui. Eu controlo, mas não totalmente. O karma faz a maior
  parte. Eu... eu sabia. Anos antes. Mediram. Eu também medi. Eu
  mostrei pros caras lá em cima. Sabe o que eles fizeram? Arquivaram.
  'Não temos contexto'. E eu... eu continuei vindo trabalhar. Todos
  os dias. Sabendo. Você não devia ter vindo aqui. Vai embora. Por
  favor."
- **EN:** "I knew someone would get here one day. Thought it'd be a
  reporter. No, lie — I never thought that. I knew it would be
  someone like you. Someone with weight. Someone with a reason. I'm
  not going to receive you well. I wish I could. But I don't
  control this. I control it, but not entirely. The karma does most
  of it. I... I knew. Years before. They measured. I measured too.
  I showed the guys upstairs. You know what they did? Archived it.
  'No context'. And I... I kept coming to work. Every day. Knowing.
  You shouldn't have come here. Go away. Please."

### Falas durante combate (Phase 1)

#### `dlg_engenheiro_combat_001`
- **PT:** "Por favor."
- **EN:** "Please."

#### `dlg_engenheiro_combat_002`
- **PT:** "Eu não consigo parar."
- **EN:** "I can't stop."

#### `dlg_engenheiro_combat_003`
- **PT:** "Não é meu corpo decidindo."
- **EN:** "It's not my body deciding."

### Falas durante combate (Phase 2 — Mea Culpa)

#### `dlg_engenheiro_combat_meaculpa_001`
- **PT:** "Mea culpa..."
- **EN:** "Mea culpa..."

#### `dlg_engenheiro_combat_meaculpa_002`
- **PT:** "...mea maxima culpa..."
- **EN:** "...mea maxima culpa..."

#### `dlg_engenheiro_combat_phase2_001`
- **PT:** "As crianças do bairro norte."
- **EN:** "The children of the north neighborhood."

#### `dlg_engenheiro_combat_phase2_002`
- **PT:** "A senhora Aparecida. O senhor Manoel. As gêmeas que..."
- **EN:** "Mrs. Aparecida. Mr. Manoel. The twins who..."

### Morte

#### `dlg_engenheiro_morte_001`
- **PT:** "Não eram só medidas. Eram... pessoas. As crianças do
  Bairro Norte. As idosas que tinham insuficiência. Eu sabia que a
  água tava... que ia... [pausa] Você quer saber de onde vem? A
  Torre. O escritório do conselho. Eles têm tudo. Tudo arquivado.
  Tudo. Vai lá. Pelo menos isso. Vai lá... e olha... no meu nome..."
- **EN:** "They weren't just measurements. They were... people. The
  children of the Bairro Norte. The elderly with kidney failure. I
  knew the water was... was going to... [pause] You want to know
  where it comes from? The Tower. The council's office. They have
  everything. All archived. Everything. Go there. At least that.
  Go... and look... under my name..."

> Morre. Cabeça baixa. Não cai — apenas para de respirar sentado.

---

# Lia (cena memorial da ponte)

**Cena especial.** Já apresentada em 04a (`dlg_lia_ponte_001`),
referenciada aqui pra completude. Trigger: `flag:zequinha_conhecido`
AND `ally_active:lia` AND `ZoneEnter:ponte_velha`.

Sem novo diálogo nesta parte — usa o existente. Implementação
referência 04a.

---

# Narrador interno (Dante)

### Avenida — primeira impressão

#### `dlg_dante_avenida_001`
- **PT:** "Aqui não tem o caos das outras zonas. Tudo arrumado.
  Pintura nova. É pior assim."
- **EN:** "There's no chaos of the other zones here. Everything
  organized. Fresh paint. It's worse this way."

### Após escolha — confronto

#### `dlg_dante_pos_sindico_001`
- **PT:** "Ele disse que organizou. Eu acabei a organização dele.
  Não sei se sou melhor."
- **EN:** "He said he organized. I ended his organizing. I don't
  know if I'm better."

### Após escolha — negociação

#### `dlg_dante_pos_negociacao_001`
- **PT:** "Eu fiz a conta. A conta fechou. Mas a Marta não vai me
  olhar do mesmo jeito de novo. Eu não sei se a conta fecha mesmo."
- **EN:** "I did the math. The math worked. But Marta won't look at
  me the same way again. I don't know if the math really works."

### Após escolha — traição

#### `dlg_dante_pos_traicao_001`
- **PT:** "Eu enganei eles. Eles mereceram. Mas eu também enganei
  a mim mesmo, em algum lugar do caminho. Tô aprendendo."
- **EN:** "I tricked them. They deserved it. But I also tricked
  myself, somewhere along the way. I'm learning."

### Rio — primeira impressão

#### `dlg_dante_rio_001`
- **PT:** "Aqui é triste de um jeito diferente. As outras zonas
  reclamam. Aqui só ficou silêncio."
- **EN:** "It's sad here in a different way. The other zones
  complain. Here only silence remained."

### Após O Engenheiro

#### `dlg_dante_pos_engenheiro_001`
- **PT:** "Ele me disse o caminho. Olhar no nome dele. A Torre tem
  o que ele sofreu. E eu tô indo buscar."
- **EN:** "He told me the path. Look under his name. The Tower has
  what he suffered. And I'm going to get it."

### Após emails do Engenheiro

#### `dlg_dante_emails_engenheiro_001`
- **PT:** "Andar 38. Sala dos servidores. O 'Conselheiro' — não é
  cargo, é como chamavam. Eu trabalhei nesse prédio por anos. Eu
  conheço aquela sala. Eu conheço por dentro. Tô indo."
- **EN:** "38th floor. Server room. The 'Counselor' — not a title,
  it's what they called him. I worked in that building for years.
  I know that room. I know it inside. I'm going."

---

# Reactions: outros aliados reagem à escolha de facção

## Lia reage à negociação

#### `dlg_lia_reacao_negociacao_001`
- **PT:** "Por que você fez isso? Não, não me responde. Eu sei o
  motivo. O motivo é sempre o mesmo — sobrevivência. Eu vou ficar
  contigo. Eu não vou desistir da gente. Mas eu... eu tô triste,
  Dante. Tu poderia ter conversado comigo antes."
- **EN:** "Why did you do that? No, don't answer. I know the reason.
  The reason is always the same — survival. I'll stay with you. I
  won't give up on us. But I... I'm sad, Dante. You could have
  talked to me first."

## Marina reage à negociação

#### `dlg_marina_reacao_negociacao_001`
- **PT:** "Decisão pragmática. Eu reconheço. Cientificamente, é a
  estratégia de risco mínimo. Humanamente — eu não sei. Eu não
  estudo isso. Eu continuo contigo, é claro. Os dados são mais
  importantes que minha opinião sobre os teus métodos."
- **EN:** "Pragmatic decision. I recognize it. Scientifically, it's
  the minimum-risk strategy. Humanly — I don't know. I don't study
  that. I'll keep going with you, of course. The data is more
  important than my opinion of your methods."

## Marta reage à confrontação

#### `dlg_marta_reacao_confronto_002`
- **PT:** "Tu fez bonito, ouviu? A Senhora Cíntia me mandou recado.
  Disse que dormiu inteiro pela primeira vez em meses. É isso que
  conta no fim do dia."
- **EN:** "You did well, you hear? Mrs. Cíntia sent word. Said she
  slept the whole night for the first time in months. That's what
  counts at the end of the day."

## Inácio reage à confrontação

#### `dlg_inacio_reacao_confronto_001`
- **PT:** "Eu rezei por aquele homem hoje. Pelo Síndico. Ele era um
  homem ruim. Mas eu reza por homens ruins quando eles morrem
  porque é o que eu sei fazer. Tu fez o que precisava. Eu rezo. A
  gente faz o que sabe."
- **EN:** "I prayed for that man today. For the Síndico. He was a
  bad man. But I pray for bad men when they die because it's what
  I know how to do. You did what was necessary. I pray. We do what
  we know."

---

# Dialogue counts (this part)

| Character / context             | Lines | Notes |
|---------------------------------|-------|-------|
| Vinícius (negotiator)           | 5     | Polished menace, all 3 outcomes |
| Senhora Cíntia (resident)       | 3     | Confronto + negociação variants |
| O Síndico (mini-boss)           | 7     | Intro + combat + death |
| Seu Zequinha (fisherman)        | 4     | Lore + service + memorial |
| O Engenheiro (mini-boss)        | 8     | Lucid throughout combat |
| Dante (internal narrator)       | 7     | Avenida + 3 choice variants + Rio |
| Ally reactions to faction       | 4     | Lia, Marina, Marta, Inácio |
| **Total**                       | **38**| **Part 04d** |

> Estimated final word count for this part:
> - PT-BR: ~3,800 words
> - EN: ~3,600 words

---

# Implementation notes

## Faction choice — branch implementation

The choice flag `choice:faction` from Sprint C8 affects dialogue
selection across many characters in 04d and 04e. Implementation
priority:

1. Vinícius dialogues are gated by `choice:faction`
2. Cíntia dialogues are gated by `choice:faction`
3. Ally reactions trigger after the choice is made and Dante returns
   to a zone where they're present
4. Internal monologue (`dlg_dante_pos_*`) triggers on the same event

## O Engenheiro lucidity

Unlike other bosses, O Engenheiro speaks coherently throughout combat.
This is intentional — narrative weight. Implementation:

- Combat lines pool (3-4 lines) cycles randomly during phase 1
- Phase 2 ("Mea Culpa") plays specific Latin lines + names of
  victims at scripted moments (during the 3s telegraph of his radial
  attack)
- Death dialogue is scripted (not pool) — locked sequence

## Vinícius across ranges

Vinícius doesn't reappear after the faction choice (he stays in his
restaurant, but with state-locked dialogue). He's not an aliado, not
a service NPC. He's narrative only.

If player goes back to the restaurant after Avenida is clear, he's
still there but only the post-choice variant fires. Then silence.

## Zequinha's mention of Zé

Cross-character link: Zequinha mentions Zé Boticário ("primo de
primo"). Player who notices learns the morro and the river have
quiet ties. Reinforces the world feeling alive. No mechanic, just
texture.

## "Mea culpa" in EN

Kept in Latin. Same in PT and EN. Religious phrase, recognized in
both languages. The Engenheiro's Catholic guilt translates without
adaptation.

## Cíntia's specificity

She mentions signing over her apartment "with a gun on her dining
table." Specific enough to be visceral, vague enough to imply the
faction's broader pattern. She's a small NPC with one of the heaviest
single lines in 04d.

## Final hook to Torre

`dlg_dante_emails_engenheiro_001` is the bridge to Zone 7. After
this internal monologue, the Torre is unlocked and the narrative
arrow points there. Player has freedom to delay (return to morro,
explore other zones, finish side content) but the path forward is
explicit.
