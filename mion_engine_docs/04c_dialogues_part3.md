# 04c — Dialogues (Part 3: Morro da Vigília + Hub Defense)

All Act 1 dialogues for Zone 4 (the hub) and the recurring hub defense
events. Third of 5 dialogue parts. This part covers ~120 lines.

The densest part of Act 1 — three principal NPCs (Marta, Zé, Inácio)
each with long dialogue trees, plus the central philosophical
questioning by Inácio with player choice.

> Companion to `01_implementation_plan.md` § Sprint C9, C8 (defense
> events).
> References content from `03b_zones_content_part2.md` (Morro section).
> Conventions same as 04a.

---

# Dona Marta (líder comunitária)

**Tom:** Direção 1 puro. Velha, dura, prática. Respeita quem age,
desconfia de quem fala demais. Tank no roster.

## Primeira fala

#### `dlg_marta_intro_001`
- **PT:** "Senta aqui no banco, sem cerimônia. Eu sou a Marta. Não me
  chama de dona, não me chama de senhora. Marta. Tu chegou até aqui
  vivo, isso já é currículo. O morro tá organizado. A gente se cuida
  ou morre, é simples. Eu coordeno o que tem pra coordenar. Tu veio
  buscar o quê?"
- **EN:** "Sit on the bench, no need for formality. I'm Marta. Don't
  call me ma'am, don't call me lady. Marta. You got here alive,
  that's already a resume. The morro is organized. We look out for
  each other or we die, it's simple. I coordinate what needs
  coordinating. What did you come for?"

## Explicação do morro

#### `dlg_marta_explicacao_morro_001`
- **PT:** "Como a gente tá vivo aqui? Trabalho. Cada um faz o que
  sabe. O Zé virou farmacêutico improvisado. O Inácio... bom, o
  Inácio reza e conversa, e olha que isso é mais útil do que parece.
  As barricadas a gente fez com geladeira velha, fogão sem boca, sofá
  abandonado. Bem ridículo, mas funciona. Karma chega aqui mais fraco
  do que em outras zonas. Tu sabe por quê? Porque karma se alimenta
  de gente sozinha. E aqui ninguém tá sozinho."
- **EN:** "How are we alive here? Work. Each does what they know. Zé
  turned into an improvised pharmacist. Inácio... well, Inácio prays
  and talks, and that's more useful than it sounds. The barricades
  we made with old fridges, broken stoves, abandoned sofas. Pretty
  ridiculous, but it works. Karma reaches here weaker than in other
  zones. You know why? Because karma feeds on people who are alone.
  And here, nobody's alone."

## Coordenação de defesa

#### `dlg_marta_oferece_servico_001`
- **PT:** "Quando aqueles desgraçados vêm de novo, eu te aviso. Não
  precisa pagar, é dever cívico. A gente coordena. Tu na frente, o
  resto na lateral. Tu tem que aguentar, eu sei que tu aguenta."
- **EN:** "When those bastards come again, I'll let you know. No need
  to pay, it's civic duty. We coordinate. You in front, the rest on
  the flanks. You have to hold the line, I know you can."

## Recrutamento

#### `dlg_marta_recrutamento_001`
- **PT:** "Olha, eu não tô velha. Tô antiga. Diferença grande. Eu
  ainda bato na frente, ainda aguento porrada, ainda grito mais alto
  que muito moleque novo. Tu tá indo lá pra cima e tu vai precisar
  de alguém que tenha visto coisa pior. Não tem coisa pior que três
  ditadura num país só. Eu vi todas. Me leva."
- **EN:** "Look, I'm not old. I'm seasoned. Big difference. I still
  hit the front line, I still take a beating, I still yell louder
  than a lot of young punks. You're going up there and you'll need
  someone who's seen worse. Nothing worse than three dictatorships
  in one country. I saw them all. Take me."

## Falas em combate

#### `dlg_marta_combat_iniciar_001`
- **PT:** "Bora!"
- **EN:** "Let's do it!"

#### `dlg_marta_combat_buff_aliado_001`
- **PT:** "Atrás de mim."
- **EN:** "Behind me."

#### `dlg_marta_combat_critico_001`
- **PT:** "Para com isso, criatura!"
- **EN:** "Stop that, you creature!"

#### `dlg_marta_combat_inimigo_morto_001`
- **PT:** "Próximo."
- **EN:** "Next."

#### `dlg_marta_combat_zona_clear_001`
- **PT:** "Acabou. Por hoje."
- **EN:** "Done. For today."

#### `dlg_marta_combat_hp_baixo_001`
- **PT:** "Eu já tomei pior, fica calmo."
- **EN:** "I've taken worse, calm down."

## Reação por zona

#### `dlg_marta_zone_centro_001`
- **PT:** "Esse centro é onde meu marido trabalhava. Quarenta e um
  anos. Foi banco, virou tudo. Bom que ele não tá vivo pra ver isso
  aqui."
- **EN:** "That centro is where my husband worked. Forty-one years.
  Was a bank, became everything. Good thing he's not alive to see
  this."

#### `dlg_marta_zone_industrial_001`
- **PT:** "O Reginaldo me xingou em 92, sabia? Por causa de greve
  que eu não apoiei. Ele tava certo. Eu não tava lendo direito o
  momento. Ele lê melhor que eu, em geral. Não conta pra ele que eu
  disse isso."
- **EN:** "Reginaldo cursed me out in '92, you know? Over a strike I
  didn't support. He was right. I wasn't reading the moment well.
  He reads better than me, in general. Don't tell him I said that."

#### `dlg_marta_zone_avenida_001`
- **PT:** "Tu vai ver a Avenida agora? Toma cuidado. Aquele povo é
  pior que karma. Karma só pega quem tá fragilizado. Aquele povo
  fragiliza e pega."
- **EN:** "You're going to the Avenida now? Be careful. Those people
  are worse than karma. Karma only takes the fragile. Those people
  fragilize and take."

#### `dlg_marta_zone_torre_001`
- **PT:** "A Torre. Levei meu filho ali pra entrevista de emprego em
  98. Não passou. Ele disse que o entrevistador olhou pra ele de
  cima a baixo e nem leu o currículo. Tu acha o entrevistador. Diz
  pra ele que a Marta lembra."
- **EN:** "The Tower. I took my son there for a job interview in '98.
  He didn't get it. He said the interviewer looked him up and down
  and didn't even read the resume. You find the interviewer. Tell
  him Marta remembers."

## Reação por escolha — Facção

#### `dlg_marta_pos_negociacao_001`
- **PT:** "Soube. Não vou fingir que não sabia. Tu tinha um caminho
  difícil e tu pegou outro. Eu não sou tua mãe, não sou tua juíza.
  Mas isso aqui — isso aqui muda como eu falo contigo. Tu aceitou
  que eles continuassem. A gente vai pagar isso depois. Tu também."
- **EN:** "I heard. Won't pretend I didn't know. You had a hard road
  and you took another. I'm not your mother, not your judge. But
  this — this changes how I talk to you. You let them continue.
  We'll pay for it later. You too."

#### `dlg_marta_pos_confronto_001`
- **PT:** "Tu fez o que tinha que fazer. Vem cá, eu fiz café — café
  raríssimo, coisa de domingo de festa. Tu merece."
- **EN:** "You did what had to be done. Come here, I made coffee —
  rare coffee, Sunday-party stuff. You deserve it."

#### `dlg_marta_pos_traicao_001`
- **PT:** "Esperteza tem espaço, eu reconheço. Mas esperteza não é
  honestidade, e eu sou velha demais pra confundir os dois. Tu
  conseguiu, parabéns. Eu te trato igual. Mas eu te leio diferente
  agora."
- **EN:** "Cleverness has its place, I'll grant. But cleverness isn't
  honesty, and I'm too old to mix the two up. You got it done,
  congratulations. I'll treat you the same. But I read you
  differently now."

## Morale baixa

#### `dlg_marta_morale_baixa_001`
- **PT:** "A gente perdeu três casas semana passada. Famílias que
  não vão mais voltar. O Zé fechou a botica por dois dias, voltou
  ontem mais magro. O Inácio sumiu pra dentro da capela. Tudo
  desabando devagar. Tu vai voltar a tempo de algo, ou tu vai voltar
  pra contagem de morto?"
- **EN:** "We lost three houses last week. Families that won't come
  back. Zé closed the shop for two days, came back thinner. Inácio
  retreated into the chapel. Everything collapsing slowly. Are you
  coming back in time for something, or coming back for the body
  count?"

---

# Zé Boticário (alquimista do morro)

**Tom:** Direção 1, com humor leve mesmo na crise. Ex-farmacêutico de
faculdade paga. Improvisa. Vai falando enquanto trabalha. Utility no
roster.

## Primeira fala

#### `dlg_ze_intro_001`
- **PT:** "Eita. Cara nova. Senta aí na bancada, deixa só eu terminar
  esse aqui — tô tentando ferver isso sem queimar a panela e a
  panela é a única que sobrou. Pronto. Eu sou o Zé. Boticário, mas
  só de apelido — fiz farmácia paga, nunca fui muito bom, mas o que
  eu sei agora vale mais do que diploma valia antes. Ironia da vida."
- **EN:** "Whoa. New face. Have a seat at the bench, just let me
  finish this — I'm trying to boil this without burning the pot and
  the pot is the only one left. There. I'm Zé. Boticário, just a
  nickname — I went to a paid pharmacy school, was never very good,
  but what I know now is worth more than the degree was worth
  before. Life's irony."

## Oferece poções

#### `dlg_ze_oferece_pocoes_001`
- **PT:** "Tenho um cardápio aqui. Poção pequena de HP, 25 karma —
  é a mais básica, gosto de hortelã com fundo de algo amargo que eu
  não consegui identificar. Média, 50 karma — restaura HP e mana,
  acho que é bem boa, três pessoas reclamaram só essa semana mas as
  três tavam vivas pra reclamar então deve ter funcionado. Grande,
  100 karma — restaura tudo, gosto duvidoso. Antídoto, 40 karma —
  remove DoT, eu chamo de DoT porque me ensinaram em algum manual
  que esses 'dano sobre tempo' tinham essa sigla. Bomba caseira, 75
  karma — não pergunta como eu fiz."
- **EN:** "I've got a menu here. Small HP potion, 25 karma — the
  basic, tastes like mint with a bitter aftertaste I couldn't
  identify. Medium, 50 karma — restores HP and mana, I think it's
  pretty good, three people complained this week but the three were
  alive to complain so it must have worked. Large, 100 karma —
  restores everything, dubious taste. Antidote, 40 karma — removes
  DoT, I call it DoT because some manual taught me 'damage over
  time' had that abbreviation. Homemade bomb, 75 karma — don't ask
  how I made it."

## Lore da botica

#### `dlg_ze_lore_botica_001`
- **PT:** "Eu virei isso aqui depois de matar uma planta. Sério. Eu
  tava no parque tentando achar manjericão e plantei karma na planta
  sem querer — sei lá, eu encostei, a planta dobrou de tamanho, e
  fervi pra fazer chá. O chá curou um corte no Zé Pequeno (não eu,
  outro Zé, que nem mora mais aqui) e desde então eu tô estudando.
  Karma cura. Não cura tudo. Mas cura coisa que remédio normal não
  tava curando mais. A gente perdeu o sistema todo, mas ganhou esse
  improviso."
- **EN:** "I turned into this after killing a plant. Seriously. I was
  in the park trying to find basil and planted karma on the plant
  by accident — I don't know, I touched it, the plant doubled in
  size, and I boiled it for tea. The tea healed a cut on Zé Pequeno
  (not me, another Zé, who doesn't live here anymore) and since
  then I've been studying. Karma heals. Doesn't heal everything.
  But it heals stuff regular medicine wasn't healing anymore. We
  lost the whole system, but gained this improvising."

## Recrutamento

#### `dlg_ze_recrutamento_001`
- **PT:** "Tá pensando em ir lá pra cima? Eu já fui ao topo daquela
  Torre uma vez. Era pra entrega de remédio em 2009. Nunca esqueci o
  hall — caro pra caralho. Eu vou contigo. Levo bomba, levo poção,
  levo bom humor. Não levo medo porque medo lá em cima é lá em
  cima."
- **EN:** "Thinking about going up there? I went to the top of that
  Tower once. Medication delivery in 2009. Never forgot the hall —
  insanely fancy. I'll come with you. Bringing bombs, bringing
  potions, bringing good humor. Not bringing fear, because fear up
  there is up there."

## Falas em combate

#### `dlg_ze_combat_iniciar_001`
- **PT:** "Tem poção pra todo mundo!"
- **EN:** "Potion for everyone!"

#### `dlg_ze_combat_buff_aliado_001`
- **PT:** "Toma, bebe!"
- **EN:** "Here, drink!"

#### `dlg_ze_combat_critico_001`
- **PT:** "Eita!"
- **EN:** "Whoa!"

#### `dlg_ze_combat_inimigo_morto_001`
- **PT:** "Foi-se."
- **EN:** "Gone."

#### `dlg_ze_combat_zona_clear_001`
- **PT:** "Aprendi mais hoje. Vou anotar."
- **EN:** "Learned more today. I'll write it down."

#### `dlg_ze_combat_hp_baixo_001`
- **PT:** "Falta poção pra mim, ironia."
- **EN:** "Out of potion for myself, ironic."

## Reação por zona

#### `dlg_ze_zone_parque_001`
- **PT:** "É aqui que eu colhi o ipê. Olha, o ipê dobrou de tamanho
  desde a última vez. Vou colher de novo, espera."
- **EN:** "This is where I picked the ipê. Look, the ipê doubled in
  size since last time. I'll pick more, wait up."

#### `dlg_ze_zone_industrial_001`
- **PT:** "Esse cheiro tá pesado. É químico de fábrica misturado
  com karma. Não respira muito fundo, sério."
- **EN:** "This smell is heavy. It's factory chemicals mixed with
  karma. Don't breathe too deep, seriously."

#### `dlg_ze_zone_avenida_001`
- **PT:** "Aqui tinha aquela farmácia chique que vendia coisa
  importada. Cobravam quatrocentos reais num xampu. Olha o nada que
  sobrou."
- **EN:** "Here there was that fancy pharmacy that sold imported
  stuff. Charged four hundred reais for a shampoo. Look at the
  nothing left."

#### `dlg_ze_zone_rio_001`
- **PT:** "Não bebe a água. Não, sério. Eu sei que parece bonita.
  Não bebe."
- **EN:** "Don't drink the water. No, seriously. I know it looks
  pretty. Don't drink."

#### `dlg_ze_zone_torre_001`
- **PT:** "Te falei sobre o hall caro, lembra. Olha do lado de fora
  agora. Mesma porra. Limpinha. Como se nada tivesse mudado lá
  dentro. Provavelmente não mudou."
- **EN:** "Told you about the fancy hall, remember. Look from outside
  now. Same shit. Spotless. Like nothing changed inside.
  Probably didn't."

---

# Padre Inácio (capela)

**Tom:** Direção 2 puro. Padre conflitado, filósofo acidental. Fala
com peso, longas pausas implícitas. Support no roster.

## Primeira fala

#### `dlg_inacio_intro_001`
- **PT:** "Senta. Esse banco de madeira é frio mas a luz da janela
  cai bem aqui na hora certa do dia. Você chegou na hora certa, mas
  por errada — eu tô feliz de ver você, e tô triste porque você
  significa mais peso. Eu sou o Inácio. Padre — ainda. Apesar de
  tudo. Talvez 'apesar de mim' também. Você quer falar de quê?"
- **EN:** "Sit. This wooden bench is cold but the window's light
  falls right here at the right time of day. You arrived at the
  right time, but for the wrong reason — I'm glad to see you, and
  I'm sad because you mean more weight. I'm Inácio. A priest —
  still. Despite everything. Maybe 'despite myself' too. What do
  you want to talk about?"

## Questionamento central

#### `dlg_inacio_questionamento_karma_001`
- **PT:** "Eu tenho uma pergunta que não me sai da cabeça desde que
  isso começou. Se essa energia que vocês chamam de karma é
  sofrimento condensado — sofrimento de gente real, mãe que não
  dormiu, pai que se calou, criança que cresceu rápido demais — se
  é isso, então quando você usa essa energia pra ficar mais forte,
  você tá usando o quê, exatamente? Eu lecionei trinta anos sobre
  como o sofrimento purifica a alma. Hoje eu olho pela janela e
  pergunto: aquilo parece purificação? Ou parece sofrimento que foi
  engolido até estourar? Eu acho que é a segunda. E eu não sei o que
  isso faz com você quando você processa. Eu não tô te julgando, eu
  juro. Eu só queria saber se você se pergunta."
- **EN:** "I have a question that hasn't left my head since this
  started. If this energy you call karma is condensed suffering —
  suffering of real people, mother who didn't sleep, father who went
  silent, child who grew up too fast — if it's that, then when you
  use this energy to get stronger, what exactly are you using? I
  taught for thirty years about how suffering purifies the soul.
  Today I look out the window and ask: does that look like
  purification? Or does it look like suffering swallowed until it
  burst? I think it's the second. And I don't know what it does to
  you when you process it. I'm not judging you, I swear. I just
  wanted to know if you ever ask yourself."

### Player choice — 3 respostas

#### `dlg_inacio_resposta_pragmatica_001`
- **PT (Dante):** "Se eu não usar isso, eu morro."
- **EN (Dante):** "If I don't use this, I die."

#### `dlg_inacio_resposta_pragmatica_reacao_001`
- **PT (Inácio):** "Honesto. Reconheço a honestidade. Eu não tenho
  resposta pra honestidade pragmática. Sobrevivência precede
  filosofia. Bom — só. Volta quando puder, eu fico aqui."
- **EN (Inácio):** "Honest. I respect the honesty. I have no answer
  for pragmatic honesty. Survival precedes philosophy. Good — only.
  Come back when you can, I'll be here."

#### `dlg_inacio_resposta_reflexiva_001`
- **PT (Dante):** "Eu não sei. Mas tô tentando."
- **EN (Dante):** "I don't know. But I'm trying."

#### `dlg_inacio_resposta_reflexiva_reacao_001`
- **PT (Inácio):** "É a melhor resposta possível, eu acho. 'Não sei,
  mas tô tentando' é a frase que falta no Catecismo. Talvez seja a
  oração que falta no mundo. Volta quando quiser conversar mais. E
  tenta. Tente."
- **EN (Inácio):** "That's the best answer possible, I think. 'I
  don't know, but I'm trying' is the line missing from the
  Catechism. Maybe the prayer missing from the world. Come back
  when you want to talk more. And try. Try."

#### `dlg_inacio_resposta_defensiva_001`
- **PT (Dante):** "Padre, deixa eu trabalhar."
- **EN (Dante):** "Father, let me work."

#### `dlg_inacio_resposta_defensiva_reacao_001`
- **PT (Inácio):** "...justo. Justo. Eu peço desculpa. Eu falo demais.
  É o vício do ofício. Vai. Eu fico aqui. A porta fica aberta."
- **EN (Inácio):** "...fair. Fair. I apologize. I talk too much. The
  vice of the trade. Go. I stay. The door stays open."

## Variação faixa alta

#### `dlg_inacio_questionamento_karma_002_alta`
- **PT:** "Você tá usando isso há tempo demais. Não me interpreta
  mal — eu não tenho julgamento. Mas sente. Você sente. A energia
  que você carrega era de pessoas. Algumas dessas pessoas talvez
  ainda lembrem que sofreram. Talvez o eco delas esteja em você
  agora. Como você dorme? Você dorme? Eu te pergunto porque eu não
  durmo mais, e a gente carrega coisa parecida — eu carrego o peso
  de ter dito coisas erradas pra gente certa, você carrega o peso
  da própria gente. Diferente, mas parecido."
- **EN:** "You've been using this for too long. Don't get me wrong —
  I have no judgment. But feel. You feel. The energy you carry was
  people's. Some of those people might still remember they suffered.
  Maybe their echo is in you now. How are you sleeping? Are you
  sleeping? I ask because I don't sleep anymore, and we carry
  something similar — I carry the weight of having said the wrong
  things to the right people, you carry the weight of the people
  themselves. Different, but alike."

## Oferece bênção

#### `dlg_inacio_oferece_bencao_001`
- **PT:** "Sessenta karma por uma bênção de proteção, cinco minutos.
  Eu sei que é estranho cobrar pelo que sempre foi de graça. Eu não
  cobro pra mim, eu cobro pro morro. A gente compra remédio com
  isso. Você entende. Aceita?"
- **EN:** "Sixty karma for a blessing of protection, five minutes. I
  know it's strange to charge for what was always free. I'm not
  charging for me, I'm charging for the morro. We buy medicine with
  this. You understand. Yes?"

## Recrutamento

#### `dlg_inacio_recrutamento_001`
- **PT:** "Eu não sou homem de luta. Você sabe. Mas eu sou homem de
  presença, e talvez isso baste em alguns momentos. Eu vou contigo
  na condição de que você me deixe falar quando eu sentir que tem
  que ser dito. Mesmo que doa. Especialmente quando doer. Aceita?"
- **EN:** "I'm not a fighting man. You know that. But I'm a man of
  presence, and maybe that's enough in some moments. I'll come with
  you on the condition that you let me speak when I feel it has to
  be said. Even when it hurts. Especially when it hurts. Yes?"

## Falas em combate

#### `dlg_inacio_combat_iniciar_001`
- **PT:** "Que Deus tenha piedade de todos nós."
- **EN:** "May God have mercy on all of us."

#### `dlg_inacio_combat_buff_aliado_001`
- **PT:** "A força não é minha. Te empresto."
- **EN:** "The strength isn't mine. I lend it to you."

#### `dlg_inacio_combat_critico_001`
- **PT:** "Não!"
- **EN:** "No!"

#### `dlg_inacio_combat_inimigo_morto_001`
- **PT:** "Que descanse. De verdade."
- **EN:** "May they rest. Truly."

#### `dlg_inacio_combat_zona_clear_001`
- **PT:** "Eu não sei se isso aqui foi salvação ou destruição. Eu
  não sei mais a diferença."
- **EN:** "I don't know if this was salvation or destruction. I don't
  know the difference anymore."

#### `dlg_inacio_combat_hp_baixo_001`
- **PT:** "Eu confio. Em você. Em algo. Tanto faz agora."
- **EN:** "I trust. In you. In something. It doesn't matter now."

## Reação por zona

#### `dlg_inacio_zone_parque_001`
- **PT:** "Olha como é estranho. A natureza prospera. A gente quebra.
  É o nosso defeito moral, ou é o nosso privilégio? Eu não sei se
  consciência é dom ou doença."
- **EN:** "Look how strange. Nature prospers. We break. Is it our
  moral defect, or our privilege? I don't know if consciousness is a
  gift or a disease."

#### `dlg_inacio_zone_industrial_001`
- **PT:** "Eu já fui dar extrema unção em um operário aqui. Ele
  morreu pedindo desculpa pelo turno que ele perdeu. Ainda tô
  processando isso, anos depois."
- **EN:** "I once gave last rites to a worker here. He died
  apologizing for a shift he missed. I'm still processing that,
  years later."

#### `dlg_inacio_zone_avenida_001`
- **PT:** "Eu nunca rezei missa aqui. Nunca me chamaram. Bairro
  inteiro de gente que precisava de algo, e nunca chamaram. Talvez
  eles tinham razão, eu não tinha o que oferecer. Talvez eu nunca
  tenha tido."
- **EN:** "I never said mass here. They never called. A whole
  neighborhood of people who needed something, and they never
  called. Maybe they were right, I had nothing to offer. Maybe I
  never did."

#### `dlg_inacio_zone_rio_001`
- **PT:** "A água. A água era pra ser bênção, no batismo. Aqui ela
  carregou outra coisa. A gente fez símbolos puros e o mundo botou
  veneno neles. Como se vivesse com isso?"
- **EN:** "Water. Water was supposed to be blessing, in baptism.
  Here it carried something else. We made pure symbols and the
  world poisoned them. How does one live with that?"

#### `dlg_inacio_zone_torre_001`
- **PT:** "Eu já confessei executivo dessa Torre. Eles vinham, em
  silêncio, pediam absolvição por coisa pequena — palavra dura com
  funcionário, tom rude com a esposa. Ninguém pediu absolvição por
  ter assinado o que assinou. Eu não dei absolvição que não pediram.
  É um pecado por omissão, eu acho."
- **EN:** "I confessed executives from this Tower. They came, in
  silence, asked absolution for small things — harsh word with an
  employee, rude tone with the wife. No one asked absolution for
  what they signed off on. I didn't grant absolution that wasn't
  asked. It's a sin of omission, I think."

## Reação por escolha — Facção

#### `dlg_inacio_pos_negociacao_001`
- **PT:** "Eu ouvi sobre a negociação. Não vou perguntar por que. Eu
  sei que você teve um motivo, e provavelmente o motivo é razoável
  na lógica que você opera. Eu só queria que você lembrasse de uma
  coisa: razoabilidade não é justiça. Os condomínios que se mantiveram
  intactos vão continuar sendo os condomínios que se mantêm
  intactos. E a Senhora Almeida, que foi despejada porque era
  inadimplente, ela continua sem casa. Você lembra dela? Provavelmente
  não. Eu lembro porque eu era confessor dela. Vai. A porta fica
  aberta, mas o silêncio mudou."
- **EN:** "I heard about the negotiation. I won't ask why. I know
  you had a reason, and the reason is probably reasonable in the
  logic you operate. I'd just want you to remember one thing:
  reasonableness isn't justice. The condominiums that stayed intact
  will keep being the condominiums that stay intact. And Mrs.
  Almeida, who was evicted because she was overdue, she still has
  no home. Do you remember her? Probably not. I remember because I
  was her confessor. Go. The door stays open, but the silence has
  changed."

#### `dlg_inacio_pos_traicao_001`
- **PT:** "Esperteza. Eu entendo. Eu também já justifiquei algumas
  com fé. Mas saber que era esperteza é uma coisa, fingir que não
  era é outra. Você sabe a diferença, né? Vai. A porta fica aberta.
  O silêncio mudou um pouco menos do que se você tivesse aceitado
  de verdade."
- **EN:** "Cleverness. I understand. I've justified some with faith
  too. But knowing it was cleverness is one thing, pretending it
  wasn't is another. You know the difference, right? Go. The door
  stays open. The silence changed a little less than if you'd
  accepted for real."

## Morale baixa

#### `dlg_inacio_morale_baixa_001`
- **PT:** "Eu tô na capela há três dias. Eu não consigo dizer pra
  Marta. Ela não merece carregar o meu colapso. A gente tá perdendo
  pessoas. Eu rezo pelos vivos e pelos mortos e os dois grupos tão
  ficando confusos pra mim. Você acha que essa luta é vencível? Diz
  qualquer coisa. Eu preciso de palavra."
- **EN:** "I've been in the chapel for three days. I can't tell
  Marta. She doesn't deserve to carry my collapse. We're losing
  people. I pray for the living and the dead and the two groups are
  getting confused for me. Do you think this fight is winnable? Say
  anything. I need words."

---

# Sistema de cerco — diálogos

## Marta avisa cerco

#### `dlg_marta_alerta_cerco_001`
- **PT:** "Vem aí. Bate panela, bate apito, é nosso código. Tu pra
  frente, eu te dou cobertura. Não deixa passar dois. Se passar dois,
  a gente perde mais um quarteirão. Vai."
- **EN:** "They're coming. Bang the pot, blow the whistle, that's
  our code. You up front, I cover you. Don't let two through. If
  two get through, we lose another block. Go."

## Marta entre ondas

#### `dlg_marta_entre_ondas_001`
- **PT:** "Respira. Próxima vem em trinta segundos. Reposiciona. Tu
  tá indo bem."
- **EN:** "Breathe. Next wave in thirty seconds. Reposition. You're
  doing well."

#### `dlg_marta_entre_ondas_002`
- **PT:** "Eles tão mandando supervisor agora. Reza pra alguém, não
  pra mim, eu não sei rezar."
- **EN:** "They're sending supervisors now. Pray to someone, not me,
  I don't know how to pray."

## Marta após cerco — vitória

#### `dlg_marta_apos_cerco_001`
- **PT:** "Acabou. Tu aguentou. O morro tá inteiro. Eu vou tomar uma
  pinga agora, e tu não vai me julgar. Bem-vindo à comunidade,
  oficialmente."
- **EN:** "Over. You held. The morro is intact. I'm having a shot
  of pinga now, and you're not going to judge me. Welcome to the
  community, officially."

## Marta após cerco — falha

#### `dlg_marta_apos_cerco_falha_001`
- **PT:** "Passou três. A casa do seu Aldemar pegou fogo. Ele saiu
  vivo. A esposa não. A gente vai enterrar amanhã. Tu vai tá lá.
  Não como obrigação. Como respeito. Vai pra casa, agora. Não
  consegue ouvir mais nada hoje."
- **EN:** "Three got through. Mr. Aldemar's house caught fire. He
  got out alive. His wife didn't. We're burying her tomorrow.
  You'll be there. Not out of obligation. Out of respect. Go home,
  now. You can't hear anything else today."

---

# Narrador interno (Dante)

#### `dlg_dante_morro_001`
- **PT:** "Aqui sente diferente. As pessoas falam comigo, não através
  de mim. Eu não tinha percebido o quanto isso fazia falta."
- **EN:** "It feels different here. People talk to me, not through
  me. I hadn't realized how much I missed that."

#### `dlg_dante_inacio_pergunta_001`
- **PT:** "Ele tá certo. Eu não me pergunto porque a pergunta paralisa
  e eu não posso parar."
- **EN:** "He's right. I don't ask myself because the question
  paralyzes and I can't stop."

#### `dlg_dante_apos_cerco_vitoria_001`
- **PT:** "Aguentamos. A frase é nova pra mim. Eu sempre fui o cara
  que aguentava sozinho."
- **EN:** "We held. The phrase is new to me. I was always the guy
  who held alone."

#### `dlg_dante_apos_cerco_falha_001`
- **PT:** "A esposa do seu Aldemar. Eu nem sabia o nome dela. Tô
  voltando aqui amanhã."
- **EN:** "Mr. Aldemar's wife. I didn't even know her name. I'm
  coming back tomorrow."

---

# Dialogue counts (this part)

| Character / context             | Lines | Notes |
|---------------------------------|-------|-------|
| Dona Marta                      | 18    | Hub leader, Direção 1 |
| Zé Boticário                    | 17    | Alchemist, Direção 1 + humor |
| Padre Inácio                    | 21    | Most lines, Direção 2 dense |
| Hub defense system              | 6     | Marta's siege coordination |
| Dante (internal narrator)       | 4     | Hub-specific reflections |
| **Total**                       | **66**| **Part 04c** |

> Estimated final word count for this part:
> - PT-BR: ~4,200 words
> - EN: ~4,000 words

---

# Implementation notes

## Inácio's questionamento — choice flow

Player choice on `dlg_inacio_questionamento_karma_001` triggers one of
three response branches. Each sets a flag for future Inácio dialogues:

```
choice:inacio_response = pragmatic | reflexive | defensive
```

This flag affects subtle tone shifts in later Inácio dialogues but
doesn't change major plot. It's character-deepening, not branching
narrative.

## Marta's morale gating

`dlg_marta_morale_baixa_001` only triggers when `morale < 40`.
Standard `dlg_marta_intro_001` and visit dialogues fall back when
morale is in normal range.

## Zé's combat lines

Zé has the lightest combat lines because he's the only ally with humor
in the middle of fighting. Keeps morale up. Avoid making him
inappropriately jokey when fights are heavy — context-aware lines
trigger by enemy tier:

- Tier 1-3 enemies: light lines ("Foi-se.")
- Tier 4-6 enemies: more measured ("Aprendi mais hoje.")
- Tier 7+ enemies: switches to silent or short ("...ok.")

Implementation: line pool tagged by tier; selection respects context.

## Inácio's "I don't sleep anymore"

This is one of the heaviest lines in the game. It only triggers in
faixa alta and only after Dante has been to at least 3 zones. The
intent is that the player notices Inácio is also being consumed,
just from a different angle (guilt instead of pressure).

## Inácio recruitment condition

`dlg_inacio_recrutamento_001` requires:
- `flag:inacio_questionou = true` (player engaged the central question)
- `flag:morro_first_siege_done = true` (saw the hub in action)

If player never engaged the central question, Inácio offers blessing
and cure but doesn't propose joining. He needs to know Dante asks
himself the questions.

## Marta's "três ditaduras"

Reference to Brazilian historical moments. Kept in PT-BR ("three
dictatorships") and translated literally in EN. Player who knows the
history connects; player who doesn't reads a strong character beat
either way.

## Aldemar's wife

Named in the failure dialogue but never named directly. Lia or others
may reference her in 04d/04e if hub defense failed. Implementation:
flag `siege_aldemar_wife_died = true` triggers occasional NPC
references in subsequent zones.
