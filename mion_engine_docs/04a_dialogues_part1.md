# 04a — Dialogues (Part 1: Family, Vila Rosário, Centro Velho)

All Act 1 dialogues for Zones 0 and 1 in PT-BR and EN. First of 5
dialogue parts. This part covers ~140 lines.

> Companion to `01_implementation_plan.md` § Sprint C9.
> References content from `03a_zones_content_part1.md`.
> Subsequent parts: 04b (Parque + Industrial), 04c (Morro + cercos),
> 04d (Avenida + Rio + escolhas), 04e (Torre + epílogo).

---

## Localization conventions

### Key format

```
dlg_<character_id>_<context>_<sequence_number>[_<faixa>]
```

Examples:
- `dlg_lia_intro_001` — Lia's first dialogue, sequence 1
- `dlg_inacio_questionamento_002_alta` — Inácio's questioning, variant
  for high karma faixa

### Faixa fallback

When a key has a faixa suffix (`_baixa`, `_media`, `_alta`), the
dialogue system tries faixa-specific first, then falls back to base
key (no suffix). See `karma_faixa.hpp` (Sprint K8) for implementation.

### Tone markers per character

Marked at top of each character section. Direção 1 = dry/practical,
Direção 2 = dense/poetic, mixed = transitions between them.

### Internal monologue (`dlg_dante_*`)

Dante's internal lines are short, observational, dry by default.
Heavier moments shift toward Direção 2.

### Cross-references

When a dialogue branches based on choice or flag, format:
`[FLAG_NAME]` or `[CHOICE:value]`. Implementation reads choice/flag
state and selects appropriate variant.

### Localization file format

Final destination: `data/locale/pt-br.json` and `data/locale/en.json`,
flat key-value structure. Each dialogue line below maps to one entry
in both files.

---

# Família

## Lia (irmã)

**Tom:** Direção 1, com peso emocional. Idealista mas não ingênua.
Intervém quando Dante exagera.

### Telefonema do gatilho

#### `dlg_lia_intro_001`
- **PT:** "Dante, é a Lia. O pai e a mãe... eles não tão bem. O pai
  tá com umas veias pretas no braço e a mãe não me reconhece. Vem pra
  cá, vem agora."
- **EN:** "Dante, it's Lia. Mom and Dad... they're not okay. Dad has
  these black veins in his arm and Mom doesn't recognize me. Come
  here, come now."

### Após o gatilho

#### `dlg_lia_apos_gatilho_001`
- **PT:** "O que... o que você fez? Eles... tão dormindo? Tão
  respirando direito agora. O que foi isso, Dante?"
- **EN:** "What... what did you do? They're... sleeping? They're
  breathing properly now. What was that, Dante?"

#### `dlg_lia_apos_gatilho_002`
- **PT:** "Eu não sei o que tá acontecendo. Você não sabe o que tá
  acontecendo. Mas eles tão melhor. Pelo menos tão melhor. Vai... vai
  ver o que essa cidade tá te mostrando. Eu fico aqui."
- **EN:** "I don't know what's happening. You don't know what's
  happening. But they're better. At least they're better. Go... go
  see what this city is showing you. I'll stay here."

### Visitas posteriores (rotina)

#### `dlg_lia_visita_padrao_001`
- **PT:** "Eles tão estáveis. Sem mudança. Toma um café antes de sair
  de novo, vai."
- **EN:** "They're stable. No change. Have a coffee before you head
  out again."

#### `dlg_lia_visita_padrao_002`
- **PT:** "A mãe falou um pouco hoje. Perguntou onde tu tava. Eu
  disse trabalho. Ela acreditou."
- **EN:** "Mom spoke a bit today. Asked where you were. I said work.
  She believed it."

#### `dlg_lia_visita_padrao_003_media`
- **PT:** "Você tá... diferente. Não sei explicar. Vai com calma,
  Dante."
- **EN:** "You look... different. I can't explain. Take it easy,
  Dante."

#### `dlg_lia_visita_padrao_003_alta`
- **PT:** "Eu sinto teu cheiro de longe agora. Não é ruim, é... não
  sei. É denso. Você tá indo longe demais? Diz pra mim se tá."
- **EN:** "I can feel you from a distance now. It's not bad, it's...
  I don't know. It's heavy. Are you going too far? Tell me if you are."

### Recrutamento

#### `dlg_lia_recrutamento_001`
- **PT:** "Eu vou contigo. Não, não argumenta. Eu também passei a
  vida sentindo essa pressão. Talvez eu sirva pra alguma coisa lá
  fora. Pelo menos pra te lembrar que existe gente que ainda tá
  esperando você voltar."
- **EN:** "I'm coming with you. No, don't argue. I've spent my life
  feeling this pressure too. Maybe I'm good for something out there.
  At least to remind you there are people still waiting for you to
  come back."

### Falas em combate (após recrutada)

#### `dlg_lia_combat_iniciar_001`
- **PT:** "Tô aqui."
- **EN:** "I'm here."

#### `dlg_lia_combat_buff_aliado_001`
- **PT:** "Aguenta, Dante."
- **EN:** "Hold on, Dante."

#### `dlg_lia_combat_critico_001`
- **PT:** "Cuidado!"
- **EN:** "Careful!"

#### `dlg_lia_combat_hp_baixo_001`
- **PT:** "Tô ficando sem fôlego..."
- **EN:** "I'm running out of breath..."

#### `dlg_lia_combat_inimigo_morto_001`
- **PT:** "Era gente."
- **EN:** "That was a person."

#### `dlg_lia_combat_zona_clear_001`
- **PT:** "Pelo menos isso."
- **EN:** "At least there's that."

### Reação por zona

#### `dlg_lia_zone_centro_velho_001`
- **PT:** "Eu fazia estágio nesse prédio aqui do lado. Olha como tá."
- **EN:** "I used to intern in this building right here. Look at it now."

#### `dlg_lia_zone_parque_001`
- **PT:** "É bonito. É esquisito que seja bonito. Tô confusa."
- **EN:** "It's beautiful. It's weird that it's beautiful. I'm confused."

#### `dlg_lia_zone_industrial_001`
- **PT:** "Meu avô trabalhou aqui. Aposentou com a mão tremendo."
- **EN:** "My grandfather worked here. Retired with his hand shaking."

#### `dlg_lia_zone_morro_001`
- **PT:** "Aqui... aqui tem gente que entendeu antes de todo mundo.
  Que tem que se ajudar."
- **EN:** "Here... here are people who understood before everyone
  else. That they have to help each other."

#### `dlg_lia_zone_avenida_001`
- **PT:** "Mesma cidade, outro mundo. Sempre foi."
- **EN:** "Same city, different world. Always was."

#### `dlg_lia_zone_rio_001`
- **PT:** "[silêncio. Lia anda em silêncio.]"
- **EN:** "[silence. Lia walks in silence.]"

#### `dlg_lia_zone_torre_001`
- **PT:** "Você tem certeza que tem que entrar aí?"
- **EN:** "Are you sure you have to go in there?"

### Cena especial: Ponte do Rio

#### `dlg_lia_ponte_001`
- **PT:** "Sabe, eu tinha um trabalho de faculdade sobre populações
  ribeirinhas. Não consegui terminar. A professora não respondeu mais.
  Eu vinha aqui pra ver. Tipo, com meus olhos. Tinha gente. Tinha
  crianças. A gente passa de carro pela ponte e não vê, sabe?"
- **EN:** "You know, I had a college paper on river communities.
  Couldn't finish it. The professor stopped replying. I used to come
  here to see. Like, with my own eyes. There were people. There were
  children. We drive over the bridge and don't see, you know?"

### Final do Ato 1

#### `dlg_lia_final_ato_001`
- **PT:** "Eu fiz café. Entra, vai."
- **EN:** "I made coffee. Come on in."

#### `dlg_lia_envelope_001`
- **PT:** "Tava na porta. Tem teu nome. Não tem remetente."
- **EN:** "It was at the door. It has your name. No return address."

---

## Dona Célia (mãe)

**Tom:** misto. Pré-gatilho: maternal, prática. Pós-gatilho: fragmentos
de lucidez, peso de uma vida engolida.

### Pré-gatilho (apenas no início)

#### `dlg_celia_pre_gatilho_001`
- **PT:** "Dante, vai com Deus. Almoço tá na geladeira pro caso de
  voltar com fome."
- **EN:** "Dante, go with God. Lunch is in the fridge in case you
  come back hungry."

### Gatilho (durante a manifestação)

#### `dlg_celia_gatilho_001`
- **PT:** "...você é quem... quem é você... onde tá meu menino..."
- **EN:** "...who are you... who are you... where is my boy..."

### Lucidez breve (pós-gatilho)

#### `dlg_celia_lucida_001`
- **PT:** "Dante, meu filho. Bonito. Cuida da Lia. Cuida do pai. Eu
  preciso descansar mais um pouco."
- **EN:** "Dante, my boy. Handsome. Take care of Lia. Take care of
  your father. I need to rest a bit more."

#### `dlg_celia_lucida_002_alta`
- **PT:** "Eu sinto teu cansaço daqui. Tá levando o peso de gente
  demais. Não esquece de descansar. Não esquece."
- **EN:** "I can feel your tiredness from here. You're carrying too
  many people's weight. Don't forget to rest. Don't forget."

### Epílogo (após cura)

#### `dlg_celia_epilogo_001`
- **PT:** "Tô leve. Faz tempo que não tô leve. O que você fez,
  filho?"
- **EN:** "I feel light. I haven't felt light in so long. What did
  you do, son?"

#### `dlg_celia_epilogo_002`
- **PT:** "Senta aqui um pouco. Só um pouco. O sol tá entrando
  bonito hoje."
- **EN:** "Sit here a little. Just a little. The sun's coming in nice
  today."

---

## Seu Hélio (pai)

**Tom:** Direção 2 condensada. Fala pouquíssimo. Cada frase é peso.
Homem do trabalho silencioso.

### Pré-gatilho

#### `dlg_helio_pre_gatilho_001`
- **PT:** "Tá indo? Vai com cuidado. Volta cedo."
- **EN:** "Heading out? Be careful. Come back early."

### Gatilho

#### `dlg_helio_gatilho_001`
- **PT:** "Cuida delas."
- **EN:** "Take care of them."

> Esta é a fala mais importante do início do Ato 1. Duas palavras.
> Toda uma vida.

### Lucidez rara (3-4 momentos no jogo)

#### `dlg_helio_lucido_001`
- **PT:** "Filho. Senta. ...Não tem nada pra dizer. Só queria te
  ver."
- **EN:** "Son. Sit. ...Nothing to say. Just wanted to see you."

#### `dlg_helio_lucido_002`
- **PT:** "Tua mãe. Cuida dela bem. Ela aguentou muito por nós."
- **EN:** "Your mother. Take care of her well. She put up with a lot
  for us."

#### `dlg_helio_lucido_003_alta`
- **PT:** "Tu tá igual eu. No final. Cansado. Não fica como eu fiquei.
  Para de vez em quando."
- **EN:** "You look like me. At the end. Tired. Don't end up like I
  did. Stop once in a while."

### Epílogo (após cura)

#### `dlg_helio_epilogo_001`
- **PT:** "Tá bom. Tá bom agora."
- **EN:** "It's good. It's good now."

---

# Vila Rosário NPCs

## Seu Quinho (mercadinho)

**Tom:** Direção 1 puro. Humor seco. Dono de mercadinho de bairro
clássico. Sabe de fofoca.

### Primeira interação (pré-gatilho, rotina)

#### `dlg_quinho_intro_001`
- **PT:** "Dante, e aí. Tu vai querer pão hoje? Acabou às sete da
  manhã, tem o de farinha integral que ninguém quer mas eu garanto
  que tá bom."
- **EN:** "Dante, hey. You want bread today? Sold out at seven in
  the morning, but I've got the whole-grain one nobody wants and I
  swear it's good."

### Após gatilho (Quinho percebe que algo mudou)

#### `dlg_quinho_apos_gatilho_001`
- **PT:** "Cara, tu tá com uma cara. Pior que aquele dia que tu veio
  buscar leite às três da manhã na adolescência. O que aconteceu? Não
  precisa contar."
- **EN:** "Man, you look terrible. Worse than that night when you
  came in for milk at three a.m. as a teenager. What happened? You
  don't have to say."

### Fofoca / direcionamento

#### `dlg_quinho_rumores_centro_001`
- **PT:** "Tu sabe o que tá rolando lá no centro, né? Não, claro que
  tu não sabe, ninguém sabe direito. O Mariano que mora ali na esquina
  foi pro centro semana passada e voltou esquisito. Falando coisas
  sem sentido. Aí ele... bom, ele não é mais ele. A mulher dele tá
  cuidando. Tá tudo errado, Dante. Tá tudo errado."
- **EN:** "You know what's going down at the centro, right? No, of
  course you don't, no one really does. Mariano who lives at the
  corner went to the centro last week and came back weird. Saying
  nonsense. Then he... well, he's not him anymore. His wife's looking
  after him. Everything's wrong, Dante. Everything's wrong."

### Comentários por progresso de zona

#### `dlg_quinho_zone_progress_centro_001`
- **PT:** "Te vi voltando do centro hoje. Tá vivo, isso já é bom.
  Aceita um café?"
- **EN:** "Saw you coming back from centro today. You're alive,
  that's already something. Want a coffee?"

#### `dlg_quinho_zone_progress_industrial_001`
- **PT:** "Distrito industrial? Cara, tu tá doido. Meu cunhado
  trabalhou lá vinte anos e parou de falar do trabalho aos quinze.
  Não sei como tu tá entrando lá."
- **EN:** "Industrial district? Man, you're nuts. My brother-in-law
  worked there twenty years and stopped talking about work after
  fifteen. I don't know how you're getting in there."

#### `dlg_quinho_zone_progress_morro_001`
- **PT:** "Subiu no morro? A Marta é gente boa, viu. Velha briguenta
  mas gente boa. Se ela gostar de tu, te ajuda. Se não gostar, te
  diz na cara."
- **EN:** "Went up to the morro? Marta's good people, you know. Old
  and ornery but good people. If she likes you, she'll help you. If
  she doesn't, she'll tell you to your face."

#### `dlg_quinho_zone_progress_torre_001`
- **PT:** "A Torre? Dante, escuta — tu não precisa fazer isso. Eu
  sei que tu sente que precisa. Mas tu não precisa. Diz pra mim que
  pelo menos pensou em parar."
- **EN:** "The Tower? Dante, listen — you don't have to do this. I
  know you feel like you do. But you don't have to. Tell me you at
  least thought about stopping."

### Compra de poções

#### `dlg_quinho_oferece_pocoes_001`
- **PT:** "Tem umas garrafinhas que a Eunice me deixou. Diz que cura
  uns cortes. 25 karma — sim, eu também aprendi a aceitar essa moeda
  esquisita, e daí. Quer levar?"
- **EN:** "I've got some little bottles Eunice left with me. Says
  they heal cuts. 25 karma — yeah, I also learned to take this weird
  currency, so what. Want some?"

---

## Padre Lucas (igreja da Vila)

**Tom:** misto. Direção 1 na maior parte, com momentos de Direção 2.
Padre simples, sem grandes elaborações teológicas. Quer ajudar.

#### `dlg_lucas_intro_001`
- **PT:** "Dante, meu filho. Vem cá. Vem cá. Tu sabe que essa
  igreja sempre foi pequena, mas o que eu tiver, é teu. Tem água
  benta, tem oração, tem cura se Deus quiser e tu também. Que karma
  tu trouxe?"
- **EN:** "Dante, my son. Come here. Come here. You know this church
  has always been small, but whatever I have is yours. There's holy
  water, there's prayer, there's healing if God wants and you do too.
  How much karma do you have?"

#### `dlg_lucas_oferece_cura_001`
- **PT:** "50 karma. Isso paga o pão dos pobres da semana. Tu não me
  paga, paga eles. Aceita?"
- **EN:** "50 karma. That'll feed this week's poor. You don't pay me,
  you pay them. Deal?"

#### `dlg_lucas_apos_cura_001`
- **PT:** "Pronto. Vai com Deus. E volta vivo."
- **EN:** "There. Go with God. And come back alive."

---

# Centro Velho NPCs

## Olavo (ex-professor de filosofia)

**Tom:** Direção 2 puro. Articulado, denso, ligeiramente irônico.
Acende um cigarro com calma enquanto o mundo desaba.

### Primeira fala

#### `dlg_olavo_intro_001`
- **PT:** "Posso te oferecer um cigarro? Não, tu não fuma. Bom pra ti.
  Eu fumo mais agora do que fumei nos trinta anos de docência. Curioso
  que isso é o que sobrou da minha disciplina. Senta aí. Tu veio do
  bairro, eu sei, dá pra ver. Você ainda tem a expressão de quem
  acredita que tem volta."
- **EN:** "Can I offer you a cigarette? No, you don't smoke. Good for
  you. I smoke more now than in thirty years of teaching. Curious
  that this is what's left of my discipline. Sit. You came from the
  neighborhood, I can tell. You still have the look of someone who
  believes there's a way back."

### Pergunta sobre karma

#### `dlg_olavo_explicacao_karma_001`
- **PT:** "Karma. A palavra que ouço dez vezes por dia agora. Engraçado
  — o conceito original não tem nada a ver com punição. É só a
  consequência natural. Causa e efeito sem juiz. O que tu vê aqui na
  cidade, isso, isso é o peso do mundo se tornando real. Décadas de
  pressão, gente sendo amassada por planilhas, mães sem dormir
  trabalhando turno duplo, pais sumindo porque quebrar é mais fácil
  que ficar. Tudo isso virou energia. E a energia não some — ela se
  acumula. Foi se acumulando. Acumulou tanto que materializou."
- **EN:** "Karma. The word I hear ten times a day now. Funny — the
  original concept has nothing to do with punishment. It's just
  natural consequence. Cause and effect without a judge. What you see
  here in the city, this, this is the weight of the world becoming
  real. Decades of pressure, people crushed by spreadsheets, mothers
  not sleeping working double shifts, fathers vanishing because
  breaking is easier than staying. All of that became energy. And
  energy doesn't disappear — it accumulates. It kept accumulating. It
  accumulated so much it materialized."

#### `dlg_olavo_explicacao_karma_002_alta`
- **PT:** "Você tá começando a sentir, né? Não me diz, eu vejo. A
  gente se torna o que processa. Você tá processando muito. Toma
  cuidado com o que você se torna no caminho. Sócrates dizia que a
  vida não examinada não vale a pena ser vivida. Eu acrescento — a
  vida não pausada também não."
- **EN:** "You're starting to feel it, aren't you? Don't tell me, I
  can see. We become what we process. You're processing a lot. Be
  careful what you become along the way. Socrates said the unexamined
  life is not worth living. I'd add — the unpaused life isn't either."

### Direcionamento

#### `dlg_olavo_recomenda_centro_001`
- **PT:** "Tem uma praça aqui no centro com um chafariz seco. Algo
  está acontecendo lá. Um homem — chamavam ele de O Gerente — virou
  algo que não dá pra explicar com palavras antigas. Talvez seja onde
  você precisa ir. Talvez não. Eu só observo."
- **EN:** "There's a square here in the centro with a dry fountain.
  Something is happening there. A man — they called him O Gerente —
  has become something old words can't describe. Maybe that's where
  you need to go. Maybe not. I just observe."

#### `dlg_olavo_recomenda_parque_001`
- **PT:** "Tem uma bióloga lá no parque. Marina alguma coisa. Ela tava
  fazendo pesquisa de campo quando isso tudo estourou. Se alguém tá
  entendendo isso de uma forma diferente da minha, é ela. Vale a
  pena."
- **EN:** "There's a biologist at the park. Marina something. She was
  doing fieldwork when all this broke loose. If anyone is
  understanding this differently from me, it's her. Worth a visit."

### Após O Gerente

#### `dlg_olavo_apos_gerente_001`
- **PT:** "Ele morreu, então. Você sente diferente quando mata alguém
  que ainda tinha um fragmento de pessoa? Eu sentiria. Mas você não
  matou um homem — você desfez um nó. Um nó que estava apertando todo
  mundo aqui em volta. É outra coisa. Não sei se é melhor ou pior. É
  outra coisa."
- **EN:** "So he's dead. Do you feel different when you kill someone
  who still had a fragment of person in them? I would. But you didn't
  kill a man — you undid a knot. A knot tightening everyone around
  here. It's a different thing. I don't know if better or worse. It's
  different."

---

## Eunice (alquimista do terminal)

**Tom:** Direção 1. Ex-enfermeira de UPA. Pragmática, cansada, doce
quando consegue.

#### `dlg_eunice_intro_001`
- **PT:** "Olha, eu não sou farmacêutica. Eu fui enfermeira de UPA
  vinte e três anos. Sei o que cura, sei o que não cura, sei o que
  pode matar. Pra mim chega. Tu precisa de quê, hoje?"
- **EN:** "Look, I'm not a pharmacist. I was an emergency nurse for
  twenty-three years. I know what heals, what doesn't, what can kill.
  That's enough for me. What do you need today?"

#### `dlg_eunice_oferece_servicos_001`
- **PT:** "Cura completa, 60 karma — paga o esforço. Poção de HP
  média, 40 karma — vou-te dizer, é meio amarga. Buff de resistência,
  80 karma — não me pergunta como funciona, pergunta pro karma. Quer?"
- **EN:** "Full heal, 60 karma — pays for the effort. Mid HP potion,
  40 karma — I'll tell you, kinda bitter. Resistance buff, 80 karma —
  don't ask me how it works, ask the karma. Want it?"

#### `dlg_eunice_apos_cura_001`
- **PT:** "Pronto. Tá fechado. Bebe água depois."
- **EN:** "Done. Closed up. Drink water afterwards."

#### `dlg_eunice_lore_001`
- **PT:** "Sabe o que é mais engraçado? Quando o caos começou, eu
  pensei 'agora vão precisar de mim'. E é sério, precisam. Mais agora
  do que quando eu tava na UPA. Lá, eu tinha um patrão. Aqui, eu sou
  o patrão. Diferença grande."
- **EN:** "You know what's funny? When the chaos started, I thought
  'now they'll need me'. And really, they do. More now than when I
  was at the clinic. There, I had a boss. Here, I'm the boss. Big
  difference."

---

## Mariano (vizinho corrompido — easter egg)

**Tom:** uma única fala, muito quebrada. Citado pelo Quinho, encontrado
em rua lateral do Centro Velho. Não combatível mas próximo de inimigos.

#### `dlg_mariano_easter_001`
- **PT:** "...os arquivos... os arquivos não fecham... eu falei... eu
  falei pra ela... arquivos..."
- **EN:** "...the files... the files don't close... I said... I told
  her... files..."

> Lore: este é o Mariano que o Quinho mencionou ("foi pro centro e
> voltou esquisito"). Player que prestou atenção liga os dois NPCs.
> Mecanicamente: NPC ambiente, não combatível, derrubá-lo é opcional
> (se confrontado). Se Dante chegar perto, ele recua.

---

# O Gerente (mini-boss)

**Tom:** ininteligível durante o combate. Lucidez única na morte —
Direção 2 quebrada.

### Início do combate

Sem diálogo. Apenas grunhidos articulando palavras corporativas
distorcidas: "RELATÓRIO!", "PRAZO!", "META!". Som ambiente.

### Falas-projétil (durante "Ordem Direta")

#### `dlg_gerente_combat_001`
- **PT:** "RELATÓRIO!"
- **EN:** "REPORT!"

#### `dlg_gerente_combat_002`
- **PT:** "PRAZO!"
- **EN:** "DEADLINE!"

#### `dlg_gerente_combat_003`
- **PT:** "META!"
- **EN:** "TARGET!"

#### `dlg_gerente_combat_004`
- **PT:** "URGENTE!"
- **EN:** "URGENT!"

#### `dlg_gerente_combat_005`
- **PT:** "REUNIÃO!"
- **EN:** "MEETING!"

### Morte

#### `dlg_gerente_morte_001`
- **PT:** "...mas... o trimestre... eu não posso... a meta... ainda
  faltam... ainda faltam três pontos... eu prometi..."
- **EN:** "...but... the quarter... I can't... the target... still
  missing... still missing three points... I promised..."

> Última fala. Morre acreditando que falhou em entregar o trimestre.

---

# Narrador interno (Dante)

**Tom:** Direção 1 dominante, ocasionalmente Direção 2 em momentos
pesados. Curto. Sem floreio.

### Tutorial e gatilho

#### `dlg_dante_intro_001`
- **PT:** "Sexta-feira. Domingo eu visito eles. Hoje é trabalho."
- **EN:** "Friday. Sunday I visit them. Today is work."

#### `dlg_dante_apos_gatilho_001`
- **PT:** "Eles estão respirando. Estão respirando. Eu não sei o que
  fiz. Mas estão respirando."
- **EN:** "They're breathing. They're breathing. I don't know what I
  did. But they're breathing."

### Figueira

#### `dlg_dante_figueira_001`
- **PT:** "A árvore tá pulsando. Ou eu tô vendo coisa."
- **EN:** "The tree is pulsing. Or I'm seeing things."

### Centro Velho

#### `dlg_dante_centro_chegada_001`
- **PT:** "Eu trabalhava aqui. Esse hall, eu sei como rodava o
  servidor de e-mail dele. Hoje tem alguém ali que não é mais alguém."
- **EN:** "I used to work here. This hall, I know how their email
  server ran. Today there's someone there who's not someone anymore."

#### `dlg_dante_pos_gerente_001`
- **PT:** "Ele morreu pensando em cota."
- **EN:** "He died thinking about quota."

---

# Action event dialogues

## Tutorial primeiro combate

#### `dlg_tutorial_primeiro_combate_001`
- **PT:** "Você absorveu karma do inimigo derrotado. Karma se acumula
  como total (seu nível) e disponível (pra gastar)."
- **EN:** "You absorbed karma from the defeated enemy. Karma
  accumulates as total (your level) and available (to spend)."

## Tutorial Altar de Dissolução

#### `dlg_tutorial_altar_001`
- **PT:** "Altares de Dissolução permitem reorganizar karma investido.
  Sem custo. O karma volta pro saldo disponível."
- **EN:** "Dissolution Altars let you reorganize invested karma. No
  cost. Karma returns to your available balance."

---

# Dialogue counts (this part)

| Character / context             | Lines | Notes |
|---------------------------------|-------|-------|
| Lia (sister)                    | 22    | Family + ally + zone reactions |
| Dona Célia (mother)             | 5     | Pre/post gatilho + epilogue |
| Seu Hélio (father)              | 6     | Sparse but heavy |
| Seu Quinho (neighbor)           | 8     | Direção 1, gossip + zone reactions |
| Padre Lucas (chapel)            | 3     | Service offers |
| Olavo (philosopher)             | 5     | Karma exposition + faixa variation |
| Eunice (alchemist)              | 4     | Service NPC |
| Mariano (easter egg)            | 1     | Lore connection |
| O Gerente (mini-boss)           | 6     | 5 combat lines + death |
| Dante (internal narrator)       | 5     | Mixed tone |
| Tutorial system                 | 2     | First combat + altar |
| **Total**                       | **67**| **Part 04a** |

> Estimated final word count for this part:
> - PT-BR: ~3,400 words
> - EN: ~3,200 words

---

# Implementation notes

## For Sprint C9 integration

- All keys above map to entries in `data/locale/pt-br.json` and
  `data/locale/en.json`
- Faixa-suffixed keys (e.g. `_alta`) require `karma_faixa` (Sprint K8)
  resolved correctly
- Some lines reference flags or choices that aren't yet implemented
  (e.g. `[CHOICE:faction]` in later parts) — implement faixa fallback
  first (K8), choice branching later (C8)

## Direction-of-translation note

PT-BR is the primary voice. EN translations preserve meaning but soften
some Brazilian idioms (e.g. "tá com uma cara" → "you look terrible")
to read naturally. Tone markers (Direção 1 vs 2) hold in both languages
but the *texture* of the language shifts — PT-BR carries more orality,
EN reads slightly cleaner. This is intentional. Localization QA later
(post-implementation) can polish.

## Easter egg cross-reference

Mariano (centro side street) connects to Quinho's mention. Dante
pickups + sees + remembers chain triggers `dlg_dante_mariano_001` (in
04b — Centro Velho lateral exploration; cross-reference future part).
