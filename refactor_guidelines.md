# Refactor Guidelines

Guia operacional para refactors e implementacoes novas no projeto. Este documento define **como mexer** no codigo sem reforcar os problemas mapeados em `bug_fix_refactor.md`.

Nao e um backlog. Nao lista bugs. Define regras de decisao, escopo, ownership e validacao.

---

## 1. Objetivo

Evitar que refactors e features novas:

- criem mais uma variacao para um problema que ja tem solucao parecida;
- introduzam globais, strings magicas e contratos implicitos;
- misturem UI, gameplay, save, audio e render no mesmo modulo;
- aumentem o acoplamento entre cenas, systems, controllers e dados;
- “colem puxadinhos” em vez de consolidar ownership.

---

## 2. Regras principais

### 2.1 Reutilizar antes de criar

Antes de criar qualquer modulo novo:

1. Procurar equivalentes no `src/`.
2. Comparar explicitamente com o padrao mais proximo.
3. Reutilizar ou estender se a cobertura for >= 70%.
4. So criar estrutura nova se houver justificativa tecnica clara.

Toda tarefa deve responder:

- O que ja existe hoje que parece resolver isso?
- Por que nao posso estender o que existe?
- O que a estrutura nova padroniza, em vez de apenas duplicar?

### 2.2 Um problema por tarefa

Cada refactor deve ter um foco principal.

Permitido:

- corrigir `_all_enemies` e ajustar os chamadores diretos;
- padronizar input de overlay e ajustar os modulos que dependem disso.

Nao permitido:

- refatorar `_all_enemies`, save pipeline e HUD no mesmo trabalho;
- fazer “aproveitando” mudancas de UI, naming e gameplay sem relacao direta.

### 2.3 Ownership explicito

Antes de alterar um fluxo, definir quem e o dono de cada responsabilidade:

- **Scene**: orquestracao de alto nivel e composicao.
- **Controller**: estado local e regras de um fluxo especifico.
- **System**: logica de dominio/gameplay reutilizavel.
- **Renderer/UI helper**: desenho e layout.
- **Save/service layer**: persistencia e IO.
- **State/data**: armazenamento, sem side effects escondidos.

Se um modulo estiver fazendo mais de um papel, isso deve ser tratado como debito tecnico, nao como padrao a copiar.

---

## 3. Anti-duplicacao

### 3.1 E proibido criar uma terceira variacao sem comparar com as duas primeiras

Se ja existem duas abordagens para o mesmo problema, uma terceira so pode nascer com justificativa forte.

Exemplos atuais de zonas de risco:

- controllers de UI com contratos diferentes;
- trackers de input paralelos;
- strings e ids hardcoded espalhados;
- globals mutaveis como fonte de configuracao;
- pipelines de save/restauracao distribuidos entre helper e scene.

### 3.2 Nomes devem refletir contrato

Nao usar nomes como `Controller`, `System`, `Manager`, `Helper`, `Service` de forma generica.

Perguntas obrigatorias:

- Guarda estado?
- Tem ciclo de vida?
- Pode renderizar?
- Pode salvar?
- Pode tocar audio?
- E puro/stateless?

Se o nome nao responder isso com clareza, a API provavelmente esta mal definida.

### 3.3 Nao duplicar dados canonicos

Toda tabela/lista/configuracao fixa deve ter uma fonte unica.

Nao duplicar:

- listas de asset paths;
- defaults de talento/spell/config;
- mapeamentos de zona -> id -> dialogo;
- contratos de input/retorno entre modulos equivalentes.

Se precisar de `reset`, `mirror`, `cache` ou `fallback`, derivar da fonte canonica.

---

## 4. Guardrails arquiteturais

### 4.1 Nao introduzir novos globais mutaveis

Evitar novos `inline` globais, singletons implicitos e ponteiros process-wide.

Se um dado precisa ser compartilhado:

- preferir contexto explicito;
- documentar owner e lifetime;
- passar dependencia por construtor, setter ou contexto controlado.

### 4.2 Nao usar identidade estrutural por string quando o dominio pede id

Strings podem existir na camada de conteudo, debug e serializacao.

Nao devem ser a base de:

- identificacao de boss;
- roteamento de evento de combate para entidade;
- comparacao critica de comportamento;
- fluxo central de cena.

Quando identidade importa, preferir enum, id estavel, handle ou chave centralizada.

### 4.3 Nao esconder side effects em helpers “inocentes”

Se um modulo:

- salva em disco,
- toca audio,
- muda quest,
- dispara transicao,
- altera stats persistentes,

isso deve estar claro no nome, na assinatura ou no `Result`.

Evitar helpers que “parecem pequenos” mas fazem IO ou mudam varios subsistemas.

### 4.4 Nao espalhar contratos de ordem

Se um fluxo depende de ordem exata entre:

- load config,
- apply save,
- recompute stats,
- clamp hp/mana/stamina,
- rebuild actor lists,

essa ordem deve estar encapsulada num pipeline unico ou documentada de forma explicita no modulo dono.

Nao depender de “primeiro chama A, depois B, depois completa manualmente em scene”.

---

## 5. Regras para mudancas em UI, controllers e fluxo

### 5.1 Inputs de UI devem seguir um modelo unico

Nao criar novo edge tracker para cada tela.

Antes de mexer em UI:

1. Verificar se o caso cabe no modelo atual de input compartilhado.
2. Se nao couber, evoluir o modelo comum.
3. Nao adicionar mais uma excecao local sem registrar o motivo.

### 5.2 Controllers devem retornar intencao, nao decidir tudo sozinhos

Sempre que possivel, controller deve devolver algo como:

- `world_paused`
- `should_save`
- `should_close`
- `requested_transition`

Persistencia, troca de cena e efeitos globais devem ficar no owner superior, salvo excecao muito bem justificada.

### 5.3 Render e layout nao devem carregar regra de dominio

UI helper / renderer:

- desenha;
- formata visual;
- usa estado preparado.

Nao deve:

- decidir quest;
- alterar save;
- recalcular gameplay;
- embutir fluxo de negocio.

---

## 6. Processo obrigatorio por tarefa

Cada tarefa de refactor ou implementacao deve ter:

- **Objetivo**
- **Escopo**
- **Fora de escopo**
- **Arquivos permitidos**
- **Dependencias**
- **Riscos**
- **Invariantes**
- **Plano curto de implementacao**
- **Plano de validacao**
- **Definition of Done**

Template base:

```md
### Tarefa: <id + titulo>

- Objetivo:
- Escopo:
- Fora de escopo:
- Arquivos permitidos:
- Dependencias:
- Risco:
- Invariantes:
- Passos:
  1.
  2.
  3.
- Validacao:
  - Build:
  - Testes:
  - Manual:
- DoD:
  - [ ]
  - [ ]
```

---

## 7. Checklist anti-puxadinho

Antes de concluir uma mudanca, verificar:

- A logica ficou no modulo certo?
- Estou criando mais um padrao para algo que ja existe?
- O novo fluxo depende de ordem escondida?
- O modulo novo faz mais do que o nome sugere?
- Ha side effects escondidos na implementacao?
- O chamador sabe claramente o que o modulo pode alterar?
- Estou usando string/global porque e facil ou porque e realmente a camada correta?

Se duas ou mais respostas forem “sim, mas depois limpamos”, parar e reavaliar.

---

## 8. Validacao minima

Toda mudanca deve validar o que toca.

### 8.1 Sempre

- build do alvo principal;
- testes automatizados relevantes;
- leitura rapida dos arquivos afetados para garantir consistencia de naming/ownership.

### 8.2 Quando aplicavel

- smoke test de dungeon/town;
- pause/dialogue/shop;
- save/load;
- musica/FX;
- transicao de cena;
- pickup/combate/boss.

### 8.3 Greps de seguranca

Usar conforme a tarefa:

- `rg '\brand\(|\bsrand\(' src/`
- `rg 'detail::active_locale|locale_bind\(|\bL\(' src/`
- `rg '"Grimjaw"' src/`
- `rg 'SaveSystem::save_default|SaveSystem::load_default|SaveSystem::remove_default_saves' src/`

---

## 9. Quando parar e abrir outra tarefa

Parar imediatamente se durante a implementacao surgir:

- necessidade de mexer em outro subsistema sem dependencia direta;
- descoberta de outro bug estrutural nao previsto;
- mudanca de contrato publico maior do que o planejado;
- necessidade de introduzir novo global, novo tracker paralelo ou nova taxonomia local;
- conflito com ownership ainda nao definido.

Nesse caso:

1. documentar a descoberta;
2. nao resolver “de carona”;
3. abrir tarefa nova.

---

## 10. Relacao com outros documentos

- `bug_fix_refactor.md`
  - inventario de problemas atuais, hotspots e planos de correcao.

- `refactor_guidelines.md`
  - regras permanentes de trabalho para nao repetir os mesmos erros.

- `ARCHITECTURE_GUIDE.md`
  - arquitetura consolidada da engine: camadas, taxonomia de modulos, contratos de ownership, padroes canonicos e anti-padroes conhecidos. Consultar antes de criar qualquer modulo novo.

---

## 11. Cobertura de testes — quando parar

Testes sao valiosos, mas um arquivo de testes que cresce sem limite gera ruido, tempo de manutencao e falsa sensacao de seguranca.

### 11.1 Criterio de suficiencia por modulo

Um modulo esta coberto o suficiente quando existem testes para:

1. **Caminho feliz principal** — a operacao funciona com entradas validas.
2. **Caminho de erro/ausencia** — a operacao retorna corretamente quando dependencias estao ausentes (ex.: save nao existe, ponteiro nulo).
3. **No-op legitimo** — quando a operacao nao deve fazer nada (ex.: stress mode, ausencia de trabalho), retorna sucesso sem efeito colateral.
4. **Invariante critico** — a regra mais importante do modulo que, se quebrar, nao aparece em build (ex.: clamp de dificuldade, preservacao de flag de vitoria).

Nao e necessario testar:

- Cada combinacao de campos ortogonais ja cobertos individualmente.
- Propriedades do filesystem ou comportamento do OS (ex.: mtime, atomicidade).
- Paths de erro que dependem de falha de hardware/SO.

### 11.2 Sinal de que ha testes demais para o mesmo contrato

- Dois testes descrevem "o mesmo contrato" com entradas diferentes sem cobrir um novo invariante.
- Um teste replica manualmente o que uma funcao publica ja faz (ex.: testar o fluxo interno de `persist` na mao quando `persist` ja e testado diretamente).
- O numero de testes de um modulo supera 3x o numero de APIs publicas sem justificativa de invariante adicional.

Quando isso ocorrer: marcar o teste redundante com comentario `// REDUNDANTE: coberto por <nome_do_outro_teste>` e remover na proxima tarefa que tocar o arquivo.

### 11.3 Ordem de prioridade ao escrever testes novos

1. Invariante que, se quebrar, nao aparece em build.
2. No-op e caminhos de erro (comportamento defensivo).
3. Roundtrip de dados (make + apply, persist + load).
4. Casos de borda de dominio (clamp, overflow, estado vazio).
5. Integracao entre dois modulos que dependem um do outro.

---

## 13. Regra final

Se uma mudanca melhora o caso local, mas piora a previsibilidade global do projeto, ela nao esta pronta.

