# Roadmap — Placeholder Textures / Art Stub
> Gerar texturas simples automaticamente para manter o jogo visualmente legível até a arte final ficar pronta.

---

## Objetivo

- Evitar “quadrados vazios” e falhas de load enquanto os sprites finais não existem
- Padronizar placeholders por categoria (player, inimigo, props, tiles, UI)
- Permitir regenerar tudo em 1 comando quando o inventário mudar
- Ter uma pré-visualização rápida para validar proporções/legibilidade

---

## O que já existe

- `TextureCache` carrega PNGs e já faz fallback quando asset não existe
- Referências explícitas a sprites/props no código (`enemy_type.hpp`, `room.hpp`, `dungeon_scene.hpp`)
- Script de áudio procedural (`tools/gen_prototype_8bit_sfx.py`) como padrão de automação de “asset provisório”
- `asset_manifest.hpp` já lista assets críticos para detectar faltas no boot
- `tools/texture_manifest.json` (contrato obrigatório runtime) implementado
- `tools/texture_backlog_contract.json` (contrato planejado) implementado
- `tools/gen_placeholder_textures.py` implementado (manifesto runtime + backlog)
- `tools/preview_placeholders.py` implementado (`preview.png` + `preview_backlog.png`)
- `tools/audit_texture_contract.py` + `tools/texture_contract_inventory.md` implementados
- alvo `Makefile`: `gen-placeholders`, `preview-placeholders`, `verify-placeholders`,
  `gen-placeholders-backlog`, `preview-placeholders-backlog`

---

## O que falta

- Integrar validação de contrato no CI (env `MION_TEXTURE_INTEGRATION_TESTS`)
- Conectar backlog planejado ao runtime quando essas texturas passarem a ser usadas no código
- Melhorar preview com legenda embutida por célula e comparação dimensão esperada vs real
- Expandir backlog para UI/FX adicionais conforme novas features entrarem

---

## Ficheiros a criar / tocar

| Ficheiro | Tipo |
|---|---|
| `tools/gen_placeholder_textures.py` | NOVO — gera PNGs placeholder |
| `tools/texture_manifest.json` | NOVO — lista declarativa de texturas e metadados |
| `tools/preview_placeholders.py` | NOVO — gera uma sheet/preview com legendas |
| `assets/placeholders/preview.png` | NOVO — saída visual gerada |
| `tests/test_plan.cpp` | +teste de cobertura do manifest (opcional via env) |
| `src/core/asset_manifest.hpp` | +entradas mínimas para placeholders críticos |
| `README.md` | +secção “Gerar placeholders” |

---

## Estratégia técnica

### 1) Manifesto declarativo (`tools/texture_manifest.json`)

Definir cada textura com:

- `path`: caminho final esperado (`assets/props/barrel.png`)
- `kind`: `sprite_sheet`, `prop`, `tile`, `ui_icon`
- `width`/`height` em pixels
- `palette`: nome de paleta (`dungeon`, `enemy_red`, `enemy_mage`, etc.)
- `label`: texto curto desenhado no placeholder (`BARREL`, `ORC`, `WALL`)
- `frames` (opcional): para spritesheet (ex.: 4x8)

Vantagem: inventário fica centralizado e versionável.

---

### 2) Gerador procedural (`tools/gen_placeholder_textures.py`)

Gerar PNG com Pillow (ou fallback puro Python, se quiser manter zero dependência).

Estilo recomendado para protótipo:

- fundo com padrão simples (xadrez/diagonal)
- borda forte por categoria
- label central legível
- variação de cor por `kind` e `palette`
- para spritesheet: desenhar grid de frames e pequenas variações por célula

CLI sugerida:

```bash
python3 tools/gen_placeholder_textures.py
python3 tools/gen_placeholder_textures.py --only assets/props
python3 tools/gen_placeholder_textures.py --dry-run
```

---

### 3) Preview visual (`tools/preview_placeholders.py`)

Gerar um `assets/placeholders/preview.png` com:

- miniaturas em grid
- nome do ficheiro por baixo
- destaque em vermelho para qualquer ficheiro ausente/ilegível

Isto acelera revisão de escala e identidade visual sem abrir ficheiro a ficheiro.

---

### 4) Cobertura de inventário (teste)

Adicionar teste opcional (ex.: `MION_TEXTURE_INTEGRATION_TESTS`) que:

- lê `tools/texture_manifest.json`
- verifica se todos os `path` existem após geração
- opcionalmente valida dimensões com `stbi_info`

Assim, o CI consegue garantir que o “pacote de placeholders” está completo.

---

## Inventário inicial mínimo (fase 1)

Baseado nas referências atuais de código:

- Props:
  - `assets/props/barrel.png`
  - `assets/props/altar.png`
  - `assets/props/dungeon_pillar.png`
  - `assets/props/dungeon_wall_mid.png`
- Enemy sheets:
  - `assets/Puny-Characters/Puny-Characters/Soldier-Red.png`
  - `assets/Puny-Characters/Puny-Characters/Orc-Grunt.png`
  - `assets/Puny-Characters/Puny-Characters/Mage-Cyan.png`
  - `assets/Puny-Characters/Puny-Characters/Mage-Purple.png`
  - `assets/Puny-Characters/Puny-Characters/Soldier-Blue.png`

> Observação: se os ficheiros já existirem (arte provisória/real), o script deve respeitar `--overwrite` para não destruir trabalho manual sem intenção.

---

## Fases de implementação

### Fase A — Infra

1. Criar `texture_manifest.json` com inventário inicial ✅
2. Criar `gen_placeholder_textures.py` (geração básica por cor+label) ✅
3. Gerar placeholders e validar boot sem “asset missing” ✅

### Fase B — Qualidade visual mínima

1. Melhorar legibilidade (grid, borda, label consistente) ✅
2. Adicionar variação para spritesheets (frame hints) ✅
3. Criar `preview_placeholders.py` e `preview.png` ✅

### Fase C — Robustez

1. Teste opcional de cobertura no `test_plan.cpp` ✅
2. Atualizar documentação de comandos ✅ (roadmap + targets no Makefile)
3. Integrar alvos no `Makefile` ✅ (`gen/preview/verify` + backlog)
4. Auditoria automática runtime x backlog x código ✅ (`audit_texture_contract.py`)

---

## Critérios de pronto

- `python3 tools/gen_placeholder_textures.py` gera 100% do manifesto sem erro
- `preview.png` mostra todos os placeholders com labels legíveis
- jogo abre cenas principais sem logs de missing texture dos itens do manifesto
- teste opcional de inventário passa localmente/CI

---

## Comandos de verificação

```bash
# Gerar placeholders (não sobrescreve existentes)
make gen-placeholders

# Gerar contact-sheet visual
make preview-placeholders

# Gerar backlog planejado (não-runtime)
make gen-placeholders-backlog
make preview-placeholders-backlog

# Validar inventário (env-gated) via mion_tests
make verify-placeholders
```

> Observação: `verify-placeholders` requer `build/mion_tests` já existente.

---

## Riscos e mitigação

- **Risco:** sobrescrever arte manual sem querer  
  **Mitigação:** default `--no-overwrite`; sobrescrever só com flag explícita

- **Risco:** dimensões erradas quebram alinhamento  
  **Mitigação:** manifesto conter `width/height` e teste validar

- **Risco:** manifesto ficar desatualizado  
  **Mitigação:** tarefa recorrente: varrer paths referenciadas no código e comparar

---

## Próximo passo recomendado

1. Adicionar legenda textual no `preview_placeholders.py` (nome curto por célula).
2. Validar dimensão real vs dimensão contratada no relatório de auditoria.
3. Ligar `MION_TEXTURE_INTEGRATION_TESTS=1` em job de CI para travar regressão de inventário.

