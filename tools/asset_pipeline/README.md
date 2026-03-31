# Asset Pipeline

Mini-repo Python para triagem, classificação e conversão segura de assets para o `mion_engine`.

## Status no engine

A pipeline ainda é **offline/manual**: ela gera relatórios e conversões auxiliares, mas não está ligada automaticamente ao boot do jogo nem a um manifest runtime.

## Objetivo

- Inventariar os arquivos em `assets/`
- Interpretar texto útil vindo do nome do arquivo, caminho e OCR opcional
- Relacionar candidatos com os slots que o engine espera
- Serializar o plano em `json`, `csv` e `md`
- Converter apenas o que for seguro e previsível

## Estrutura

- `asset_pipeline/`: código da pipeline
- `configs/engine_asset_contract.json`: contrato atual do engine
- `tests_legacy/` + `tests_v2/`: suítes de teste (oficial V2 + legado opcional)
- `output/`: relatórios gerados

## Uso

```bash
python3 tools/asset_pipeline/main.py plan \
  --source assets \
  --report-dir tools/asset_pipeline/output
```

```bash
python3 tools/asset_pipeline/main.py convert \
  --source assets \
  --report-dir tools/asset_pipeline/output \
  --dest-root tools/asset_pipeline/output/converted
```

## Decisões

- Sem backend de imagem, a pipeline não faz resize/crop "no escuro".
- Sem backend de OCR, a pipeline usa fallback por nome/caminho.
- Conversão de áudio WAV PCM é suportada no ambiente atual.
- Conversão de imagem para o tamanho final fica habilitada automaticamente se `Pillow` estiver disponível.

## Dependências Opcionais

- `Pillow`: conversão e resize de imagem
- `tesseract`: OCR real sobre imagens
