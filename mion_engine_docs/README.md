# mion_engine — Karma System & Act 1 Design Documents

Pacote completo de documentação para implementação do sistema de karma
e do Ato 1 do jogo na mion_engine.

> Este pacote complementa os arquivos já existentes do projeto
> (`CLAUDE.md`, `README.md`, `end_bug_fix_refactor.md`). Não substitui.

---

## Estrutura do pacote

```
docs/
├── README.md                          ← este arquivo
├── 00_master.md                       ← ÍNDICE + ROADMAP (começa aqui)
│
├── karma_system_design.md             ← design mecânico (foundation)
├── ato1_historia.md                   ← design narrativo (foundation)
│
├── 01_implementation_plan.md          ← plano técnico de 17 sprints
├── 02_skill_tree_definitive.md        ← 57 nós + 12 sinergias
│
├── 03a_zones_content_part1.md         ← Zonas 0-2 (Vila + Centro + Parque)
├── 03b_zones_content_part2.md         ← Zonas 3-5 (Industrial + Morro + Avenida)
├── 03c_zones_content_part3.md         ← Zonas 6-7 (Rio + Torre + epílogo)
│
├── 04a_dialogues_part1.md             ← Família + Vila + Centro Velho
├── 04b_dialogues_part2.md             ← Parque + Industrial
├── 04c_dialogues_part3.md             ← Morro + cercos
├── 04d_dialogues_part4.md             ← Avenida + Rio + escolhas
└── 04e_dialogues_part5.md             ← Torre + epílogo + envelope
```

**Total:** 12 documentos, ~50.000 palavras de design + ~33.000 palavras
de diálogo bilíngue (PT-BR + EN).

---

## Ordem de leitura recomendada

Pra entender o projeto inteiro, lê nesta ordem:

1. **`00_master.md`** — visão geral, roadmap, índice de tudo
2. **`karma_system_design.md`** — como o sistema funciona mecanicamente
3. **`ato1_historia.md`** — quem é Dante, qual é o mundo, a história
4. **`01_implementation_plan.md`** — como implementar, sprint por sprint
5. **Os outros conforme necessidade** — abre quando a sprint pedir

---

## Quando começar uma sprint

Antes de começar **qualquer** sprint, lê na ordem:

1. `CLAUDE.md` (do projeto, não deste pacote) — contrato da engine
2. `README.md` (do projeto) — estado atual
3. `00_master.md` deste pacote — onde tu tá no roadmap
4. O documento específico da sprint:
   - **Sprint K1-K8 (mecânica):** `01_implementation_plan.md` §Sprint K_X
   - **Sprint K2 também:** `02_skill_tree_definitive.md`
   - **Sprint C1-C6 (zonas):** `03a/b/c_zones_content_part_X.md`
   - **Sprint C7-C8 (aliados/eventos):** referência cruzada nas
     `03a/b/c` + `01_implementation_plan.md`
   - **Sprint C9 (diálogos):** `04a/b/c/d/e_dialogues_part_X.md`

---

## Como funciona a hierarquia de autoridade

```
CLAUDE.md                    ← contrato da engine (não-negociável)
  └── README.md              ← estado atual do projeto (humanos)
       └── 00_master.md      ← roadmap deste pacote
            └── outros docs  ← detalhe por área
```

**Regras:**
- Quando código e doc divergem → **código vence**
- Quando docs divergem entre si → **escala pra Dante decidir**
- Quando algo deste pacote conflita com `CLAUDE.md` → **`CLAUDE.md` vence**

---

## Estado atual do projeto

| Item                                    | Status |
|-----------------------------------------|--------|
| Engine fundação (combate, save, mundo)  | ✅ Funcional, 867 testes |
| Sprint 5 (Actor split)                  | 🟡 Em andamento |
| Sprint K1 (Karma foundation)            | ⚪ Próximo |
| Phase A (Karma core systems, K1-K8)     | ⚪ Pendente |
| Phase B (Act 1 content, C1-C9)          | ⚪ Pendente |

Quando uma sprint fechar, atualiza o tracker em `00_master.md`.

---

## Glossário rápido (PT/EN)

| Termo PT      | Termo EN         | Significado                      |
|---------------|------------------|----------------------------------|
| Karma         | karma            | Moeda universal do jogo          |
| Traje         | outfit           | Armadura do player que evolui    |
| Faixa         | band             | Faixa de nível de karma          |
| Altar de dissolução | dissolution altar | Ponto de respec gratuito |
| São Chico     | São Chico        | Cidade do Ato 1 (não traduz)     |
| Dante         | Dante            | Protagonista (não traduz)        |
| O Conselheiro | O Conselheiro    | Boss final do Ato 1 (não traduz) |

---

## O que NÃO está neste pacote

Por escopo decidido: este pacote cobre tudo necessário pra ter o **Ato 1
jogável end-to-end** (gameplay + história + diálogos + eventos). Os
seguintes itens ficam pra fase posterior, **após** o Ato 1 estar redondo:

- Polish de UI / menu principal
- Arte custom (procedural durante implementação)
- Música e SFX custom
- Integração Steam (Steamworks SDK)
- Achievements
- QA de localização
- Marketing, store page, trailer
- Bug fix sweep (bugs encontrados durante implementação são corrigidos
  na hora; sweep formal vem depois)

Esses tópicos serão tratados em documento separado quando o Ato 1 estiver
completo.

---

## Próxima ação

Quando o Sprint 5 (Actor split) fechar:

1. Marca progresso em `00_master.md`
2. Lê `01_implementation_plan.md` §Sprint K1
3. Faz o brief (5-step process do `CLAUDE.md`)
4. Espera aprovação
5. Implementa
6. Diff review

Bom trabalho.
