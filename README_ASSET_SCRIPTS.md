# README — Asset Scripts (Audio + Texturas)

Este documento descreve os scripts locais usados para gerar assets provisórios
(placeholder) de **áudio** e **texturas**, além dos comandos de validação.

> Objetivo: manter o projeto jogável e visualmente legível durante prototipagem,
> sem depender da arte/som final.

---

## 1) Áudio placeholder (8-bit style)

Script:

- `tools/gen_prototype_8bit_sfx.py`

Gera WAVs de protótipo para:

- SFX de combate/UI/habilidades/footsteps
- loops de ambiente
- faixas de música por estado (`town`, `dungeon calm/combat/boss`, `victory`)

### Uso

```bash
python3 tools/gen_prototype_8bit_sfx.py
```

Opcional (pasta de saída customizada):

```bash
python3 tools/gen_prototype_8bit_sfx.py --output-dir assets/audio
```

---

## 2) Texturas placeholder (runtime obrigatório)

Manifesto (contrato em uso pelo runtime atual):

- `tools/texture_manifest.json`

Gerador:

- `tools/gen_placeholder_textures.py`

Preview:

- `tools/preview_placeholders.py`

### Uso direto (Python)

Gerar texturas do manifesto:

```bash
python3 tools/gen_placeholder_textures.py
```

Forçar sobrescrita:

```bash
python3 tools/gen_placeholder_textures.py --overwrite
```

Gerar preview:

```bash
python3 tools/preview_placeholders.py
```

Saída padrão de preview:

- `assets/placeholders/preview.png`

---

## 3) Texturas placeholder (backlog planejado)

Manifesto de planejamento (ainda não obrigatório no runtime):

- `tools/texture_backlog_contract.json`

### Uso direto (Python)

Gerar backlog:

```bash
python3 tools/gen_placeholder_textures.py --manifest tools/texture_backlog_contract.json
```

Preview backlog:

```bash
python3 tools/preview_placeholders.py \
  --manifest tools/texture_backlog_contract.json \
  --output assets/placeholders/preview_backlog.png
```

---

## 4) Auditoria do contrato de texturas

Script:

- `tools/audit_texture_contract.py`

Ele cruza:

- referências `assets/...png` no código (`src/`)
- manifesto runtime (`texture_manifest.json`)
- backlog (`texture_backlog_contract.json`)

Relatório gerado:

- `tools/texture_contract_inventory.md`

### Uso

```bash
python3 tools/audit_texture_contract.py
```

---

## 5) Comandos via Makefile

Já existem alvos para facilitar o fluxo:

```bash
make gen-placeholders
make preview-placeholders
make verify-placeholders

make gen-placeholders-backlog
make preview-placeholders-backlog
```

`verify-placeholders` roda:

1. geração runtime,
2. preview runtime,
3. `MION_TEXTURE_INTEGRATION_TESTS=1 ./build/mion_tests`

---

## 6) Fluxo recomendado (dia a dia)

1. Ajustar manifesto (`tools/texture_manifest.json` ou backlog).
2. Gerar placeholders.
3. Gerar preview.
4. Rodar verificação.

Exemplo:

```bash
make gen-placeholders
make preview-placeholders
make verify-placeholders
```

---

## 7) Notas importantes

- Os scripts de textura usam `--overwrite` **apenas quando pedido**.
- O contrato runtime deve refletir o que o código realmente referencia hoje.
- O backlog serve para planeamento de produção, sem bloquear execução atual.
- O CI já roda testes com `MION_TEXTURE_INTEGRATION_TESTS=1`.

