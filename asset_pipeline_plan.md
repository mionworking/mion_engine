# Plano: Pipeline Unificada de Assets

> Status: **RASCUNHO para aprovação**
> Data: 2026-04-14

## Motivação

Hoje existem **4 scripts separados** gerando assets com estilos visuais desconectados,
primitivas duplicadas, e sobreposições de output (os 4 NPCs são gerados por 2 scripts
diferentes). O resultado: personagens parecem "retângulos coloridos", monstros são
indistinguíveis entre si, e o áudio é monótono (só onda quadrada).

Este plano unifica tudo num pipeline único com estilo visual coerente e procedural.

---

## 1. Inventário completo de assets

### 1.1 Sprites de personagem (referenciados em `src/`)

| Asset                         | Path no código                                        | Usado por           | Gerado por                    |
|-------------------------------|-------------------------------------------------------|---------------------|-------------------------------|
| Player                        | `assets/sprites/player.png`                           | `player_configurator.hpp` | `gen_animated_sprites.py`     |
| Skeleton / EliteSkeleton      | `assets/Puny-Characters/.../Soldier-Red.png`          | `enemy_type.hpp`    | `gen_animated_sprites.py`     |
| Orc / BossGrimjaw             | `assets/Puny-Characters/.../Orc-Grunt.png`            | `enemy_type.hpp`    | `gen_animated_sprites.py`     |
| Ghost                         | `assets/Puny-Characters/.../Mage-Cyan.png`            | `enemy_type.hpp`    | `gen_animated_sprites.py`     |
| Archer                        | `assets/Puny-Characters/.../Mage-Purple.png`          | `enemy_type.hpp`    | `gen_animated_sprites.py`     |
| PatrolGuard                   | `assets/Puny-Characters/.../Soldier-Blue.png`         | `enemy_type.hpp`    | `gen_animated_sprites.py`     |
| NPC Mira                      | `assets/npcs/npc_mira.png`                            | `town_world_renderer.hpp` | `gen_animated_sprites.py` **E** `gen_environment_sprites.py` |
| NPC Forge                     | `assets/npcs/npc_forge.png`                           | `town_world_renderer.hpp` | `gen_animated_sprites.py` **E** `gen_environment_sprites.py` |
| NPC Villager                  | `assets/npcs/npc_villager.png`                        | `town_world_renderer.hpp` | `gen_animated_sprites.py` **E** `gen_environment_sprites.py` |
| NPC Elder                     | `assets/npcs/npc_elder.png`                           | `town_world_renderer.hpp` | `gen_animated_sprites.py` **E** `gen_environment_sprites.py` |

**Problemas identificados:**
- NPCs duplicados em 2 scripts (sobreposição)
- Skeleton e EliteSkeleton usam o MESMO sprite (Soldier-Red) — visualmente idênticos
- BossGrimjaw usa o MESMO sprite do Orc — deveria ser visualmente distinto
- Ghost usa Mage-Cyan — não parece um fantasma
- Archer usa Mage-Purple — OK para ranged, mas pouco distinto
- Player e Soldier são quase idênticos (mesma silhueta, só cor diferente)
- Manifest JSON diz 256x256 (8x8) mas o script real gera 768x256 (24x8) — inconsistência

### 1.2 Tiles e props (referenciados em `src/`)

| Asset                  | Path                                    | Usado por                        | Gerado por                  |
|------------------------|-----------------------------------------|----------------------------------|-----------------------------|
| Dungeon tileset        | `assets/tiles/dungeon_tileset.png`      | `world_scene.hpp`                | `gen_environment_sprites.py` |
| Town tileset           | `assets/tiles/town_tileset.png`         | `town_builder.hpp`               | `gen_environment_sprites.py` |
| Corridor floor         | `assets/tiles/corridor_floor.png`       | `world_scene.hpp`                | `gen_environment_sprites.py` |
| Door closed            | `assets/props/door_closed.png`          | `dungeon_world_renderer.hpp`     | `gen_environment_sprites.py` |
| Door open              | `assets/props/door_open.png`            | `dungeon_world_renderer.hpp`, `town_world_renderer.hpp` | `gen_environment_sprites.py` |
| Dungeon pillar         | `assets/props/dungeon_pillar.png`       | `room.hpp`, `room_manager.hpp`, `asset_manifest.hpp` | `gen_placeholder_textures.py` |
| Dungeon wall mid       | `assets/props/dungeon_wall_mid.png`     | `room.hpp`, `room_manager.hpp`, `asset_manifest.hpp` | `gen_placeholder_textures.py` |
| Barrel                 | `assets/props/barrel.png`               | `room.hpp`                       | `gen_placeholder_textures.py` |
| Altar                  | `assets/props/altar.png`                | `room.hpp`                       | `gen_placeholder_textures.py` |
| Building tavern        | `assets/props/building_tavern.png`      | `town_world_renderer.hpp`        | `gen_environment_sprites.py` |
| Building forge         | `assets/props/building_forge.png`       | `town_world_renderer.hpp`        | `gen_environment_sprites.py` |
| Building elder         | `assets/props/building_elder.png`       | `town_world_renderer.hpp`        | `gen_environment_sprites.py` |
| Fountain               | `assets/props/fountain.png`             | `town_world_renderer.hpp`        | `gen_environment_sprites.py` |

**Problemas identificados:**
- Pillar, wall_mid, barrel, altar são checkerboards genéricos (`gen_placeholder_textures`)
- Todos os props acima não têm estilo unificado com os tiles do environment
- Backlog planejado mas NÃO gerado: variantes (barrel_broken, altar_corrupted, pillar_cracked)

### 1.3 Áudio (referenciados em `audio_asset_paths.hpp`)

| Categoria | Asset                         | Gerado por                    | Qualidade atual |
|-----------|-------------------------------|-------------------------------|-----------------|
| MUSIC     | `music_town.wav`              | `gen_prototype_8bit_sfx.py`   | Loop 8s, 1 canal, quadrada monótona |
| MUSIC     | `music_dungeon_calm.wav`      | `gen_prototype_8bit_sfx.py`   | Loop 9s, drone oscilante |
| MUSIC     | `music_dungeon_combat.wav`    | `gen_prototype_8bit_sfx.py`   | Loop 8s, pulso rápido sem variação |
| MUSIC     | `music_boss.wav`              | `gen_prototype_8bit_sfx.py`   | Loop 10s, 2 ondas quadradas |
| MUSIC     | `music_victory.wav`           | `gen_prototype_8bit_sfx.py`   | Arpejo curto ~1.5s |
| AMBIENT   | `ambient_dungeon_drips.wav`   | `gen_prototype_8bit_sfx.py`   | OK (gotas espaçadas) |
| AMBIENT   | `ambient_dungeon_torch.wav`   | `gen_prototype_8bit_sfx.py`   | Ruído + quadrada grave |
| AMBIENT   | `ambient_town_wind.wav`       | `gen_prototype_8bit_sfx.py`   | Senoide lenta + ruído |
| AMBIENT   | `ambient_town_birds.wav`      | `gen_prototype_8bit_sfx.py`   | Blips agudos espaçados |
| AMBIENT   | `ambient_transition_wind.wav` | `gen_prototype_8bit_sfx.py`   | Vento grave com eco |
| SFX       | `hit.wav`                     | `gen_prototype_8bit_sfx.py`   | 80ms, ruído + quadrada |
| SFX       | `player_attack.wav`           | `gen_prototype_8bit_sfx.py`   | Sweep 90ms |
| SFX       | `enemy_death.wav`             | `gen_prototype_8bit_sfx.py`   | Arpejo descendente |
| SFX       | `player_death.wav`            | `gen_prototype_8bit_sfx.py`   | Sweep longo 450ms |
| SFX       | `dash.wav`                    | `gen_prototype_8bit_sfx.py`   | Sweep ascendente |
| SFX       | `ranged_attack.wav`           | `gen_prototype_8bit_sfx.py`   | Quadrada curta 60ms |
| SFX       | `spell_bolt.wav`              | `gen_prototype_8bit_sfx.py`   | Sweep ascendente |
| SFX       | `spell_nova.wav`              | `gen_prototype_8bit_sfx.py`   | Acorde + ring |
| SFX       | `sfx_spell_frost.wav`         | `gen_prototype_8bit_sfx.py`   | Sweep descendente |
| SFX       | `sfx_spell_chain.wav`         | `gen_prototype_8bit_sfx.py`   | Arpejo ascendente rápido |
| SFX       | `sfx_skill_cleave.wav`        | `gen_prototype_8bit_sfx.py`   | Sweep descendente |
| SFX       | `sfx_skill_multishot.wav`     | `gen_prototype_8bit_sfx.py`   | 3 notas curtas |
| SFX       | `sfx_skill_poison_arrow.wav`  | `gen_prototype_8bit_sfx.py`   | Vibrato |
| SFX       | `sfx_skill_battle_cry.wav`    | `gen_prototype_8bit_sfx.py`   | Vibrato com attack lento |
| SFX       | `parry.wav`                   | `gen_prototype_8bit_sfx.py`   | 2 notas agudas |
| SFX       | `ui_confirm.wav`              | `gen_prototype_8bit_sfx.py`   | 2 notas ascendentes |
| SFX       | `ui_delete.wav`               | `gen_prototype_8bit_sfx.py`   | 2 notas descendentes |
| SFX       | `sfx_footstep_player.wav`     | `gen_prototype_8bit_sfx.py`   | Click suave 35ms |
| SFX       | `sfx_footstep_enemy.wav`      | `gen_prototype_8bit_sfx.py`   | Click grave 50ms |
| EXTRA     | `dungeon_ambient.wav`         | `gen_prototype_8bit_sfx.py`   | Drone 2.5s (referenciado em `asset_manifest.hpp` para boot check, possivelmente legado) |

**Problemas identificados:**
- Tudo usa apenas onda quadrada + noise — sem triangle, saw, sine
- SR=22050 (baixo, mascara detalhes)
- Sem ADSR real (envelope é `exp(-t*K)`)
- Músicas são monofônicas — 1 canal, sem harmonia
- `dungeon_ambient.wav` possivelmente legado (não está em `kAmbientPaths`)

### 1.4 Backlog planejado (não existem ainda no disco)

| Asset                              | Definido em                    | Categoria  |
|------------------------------------|--------------------------------|------------|
| `assets/tiles/dungeon_floor_base.png`  | `texture_backlog_contract.json` | Tile       |
| `assets/tiles/dungeon_floor_crack.png` | `texture_backlog_contract.json` | Tile       |
| `assets/tiles/dungeon_floor_moss.png`  | `texture_backlog_contract.json` | Tile       |
| `assets/tiles/door_locked.png`         | `texture_backlog_contract.json` | Tile       |
| `assets/fx/spell_frost_sheet.png`      | `texture_backlog_contract.json` | FX sheet   |
| `assets/fx/spell_chain_sheet.png`      | `texture_backlog_contract.json` | FX sheet   |
| `assets/fx/spell_nova_sheet.png`       | `texture_backlog_contract.json` | FX sheet   |
| `assets/fx/arrow_sheet.png`            | `texture_backlog_contract.json` | FX sheet   |
| `assets/fx/hit_flash_sheet.png`        | `texture_backlog_contract.json` | FX sheet   |
| `assets/ui/upgrade_icons.png`          | `texture_backlog_contract.json` | UI         |
| `assets/props/barrel_broken.png`       | `texture_backlog_contract.json` | Prop       |
| `assets/props/altar_corrupted.png`     | `texture_backlog_contract.json` | Prop       |
| `assets/props/pillar_cracked.png`      | `texture_backlog_contract.json` | Prop       |

---

## 2. Sobreposições entre scripts atuais

| Path de output                    | Scripts que geram               |
|-----------------------------------|---------------------------------|
| `assets/npcs/npc_mira.png`       | `gen_animated_sprites`, `gen_environment_sprites`, `gen_placeholder_textures` |
| `assets/npcs/npc_forge.png`      | (mesmos 3)                      |
| `assets/npcs/npc_villager.png`   | (mesmos 3)                      |
| `assets/npcs/npc_elder.png`      | (mesmos 3)                      |
| Todos os Puny-Characters (5)     | `gen_animated_sprites`, `gen_placeholder_textures` |
| `assets/sprites/player.png`      | `gen_animated_sprites`, `gen_placeholder_textures` |
| Tiles (3) + props door/building/fountain (6) | `gen_environment_sprites`, `gen_placeholder_textures` |

**Resultado:** dependendo da ordem de execução, quem "vence" muda. Sem `--overwrite`, o primeiro script
executado é o que vale. Com `--overwrite`, o último sobrescreve. Isso é uma bagunça.

---

## 3. Scripts atuais → destino

| Script atual                       | Gera hoje               | Destino proposto           |
|------------------------------------|--------------------------|----------------------------|
| `tools/gen_animated_sprites.py`    | Player, monstros, NPCs  | → `legacy/tools/`          |
| `tools/gen_environment_sprites.py` | Tiles, props, buildings, NPCs (duplicado) | → `legacy/tools/` |
| `tools/gen_placeholder_textures.py`| Checkerboards via manifest | → `legacy/tools/`         |
| `tools/gen_prototype_8bit_sfx.py`  | Todos os WAVs           | → `legacy/tools/`          |
| `tools/texture_manifest.json`      | Descrição de texturas    | → Migrar para formato do novo pipeline ou manter como referência |
| `tools/texture_backlog_contract.json` | Backlog planejado     | → Absorver no novo pipeline |
| `tools/audit_texture_contract.py`  | Validação de contrato    | → Atualizar ou reescrever para novo pipeline |

---

## 4. Pipeline proposta: `tools/gen_assets.py`

### 4.1 Estrutura

```
tools/gen_assets.py                  Entry point único
tools/mion_assets/                   Módulo Python
    __init__.py
    style.py                         Paleta global, regras de outline/sombra/dithering
    primitives.py                    Primitivas de desenho unificadas (fill, circle, outline, dither, etc.)
    png_writer.py                    Escrita de PNG RGBA (um único, reutilizado por todos)
    wav_writer.py                    Escrita de WAV (um único)
    characters.py                    Rig paramétrico de personagens (archetypes + animações)
    environment.py                   Tiles, props, buildings
    audio.py                         SFX, música, ambientes (com pyfxr ou melhorado)
    manifest.py                      Inventário declarativo de todos os assets
```

### 4.2 Estilo visual unificado (`style.py`)

Regras que valem para TUDO:

- **Outline preto de 1px** ao redor de toda silhueta visível
- **Paleta restrita por bioma**: dungeon (frios: azul-cinza-roxo), town (quentes: verde-marrom-dourado)
- **Paleta por personagem**: 5 cores fixas (base, sombra, highlight, accent, skin/bone)
- **Dithering 2-cor** nas transições de sombra (estilo NES/SNES)
- **Direção de luz consistente**: top-left (sombra no canto inferior-direito de tudo)
- **Grid base 32x32** para personagens; props/tiles respeitam múltiplos de 32

### 4.3 Rig de personagens (`characters.py`)

Um **archetype** define:
- `build`: proporção corporal (`thin` / `medium` / `heavy` / `ethereal`)
- `head_type`: `helmet` / `skull` / `hood` / `tusked` / `crown` / `hair`
- `torso_type`: `armored` / `ribcage` / `robe` / `leather` / `dress`
- `leg_type`: `boots` / `bone` / `sandals` / `bare` / `skirt`
- `weapon_type`: `sword` / `axe` / `staff` / `bow` / `none`
- `extra`: `shield` / `cape` / `shoulder_pad` / `glow` / `none`
- `palette`: 5 cores do estilo

**Archetypes necessários (10):**

| Archetype       | Usado para                   | Build     | Diferencial visual              |
|-----------------|------------------------------|-----------|---------------------------------|
| `player`        | Player                       | medium    | Armadura azul, escudo, cape     |
| `skeleton`      | Skeleton                     | thin      | Ossos visíveis, crânio, costelas |
| `elite_skeleton`| EliteSkeleton                | thin      | Skeleton + coroa/glow vermelho  |
| `orc`           | Orc                          | heavy     | Verde, presas, ombros grandes   |
| `boss_grimjaw`  | BossGrimjaw                  | heavy+    | Orc maior, armadura, glow       |
| `ghost`         | Ghost                        | ethereal  | Translúcido, sem pernas, hover  |
| `archer`        | Archer                       | medium    | Capuz, arco, aljava             |
| `patrol_guard`  | PatrolGuard                  | medium    | Armadura azul escuro, lança     |
| `npc_civil`     | NPCs (Mira, Forge, Villager) | medium    | Roupas civis, sem arma          |
| `npc_elder`     | NPC Elder                    | medium    | Manto, cajado, barba            |

**Animações por archetype (layout Puny Characters 768x256, 24 colunas x 8 linhas):**

| Colunas  | Animação | Frames |
|----------|----------|--------|
| 0-5      | Walk     | 6      |
| 6-11     | Attack   | 6      |
| 12-15    | Idle     | 4      |
| 16-18    | Cast     | 3      |
| 19       | Hurt     | 1      |
| 20-23    | Death    | 4      |

| Linhas   | Direção        |
|----------|----------------|
| 0, 1     | Sul (frente)   |
| 2, 5     | Norte (costas) |
| 3, 4, 6, 7 | Leste (Oeste = flip no renderer) |

### 4.4 Melhorias visuais concretas nos personagens

**De → Para:**

| Aspecto              | Hoje (gen_animated_sprites)       | Proposta                                  |
|----------------------|-----------------------------------|-------------------------------------------|
| Silhueta             | Retângulos coloridos sem contorno | Outline preto 1px + silhueta anatômica    |
| Cabeça               | Círculo sólido                    | Formato variado por head_type + olhos     |
| Torso                | Retângulo de uma cor              | 2-3 tons + detalhes (cinto, peito, ombros) |
| Pernas               | 2 retângulos oscilantes           | Botas/pés distintos, stride mais natural  |
| Arma                 | Linha de 1-2 pixels               | Silhueta reconhecível (espada, machado, cajado, arco) |
| Sombra               | Elipse blend manual               | Elipse consistente com alpha fixo         |
| Diferenciação        | Só cor muda                       | Silhueta + acessórios + proporções mudam  |
| Olhos                | 1-2 pixels opcionais              | Sempre presentes (exceto skull de costas) |
| Dithering            | Nenhum                            | Nas transições de sombra/highlight        |
| Skeleton             | Soldier mais fino                 | Costelas visíveis, membros finos, crânio com mandíbula |
| Ghost                | Mage recolorido                   | Semi-transparente, sem pernas, flutuante  |
| Boss                 | Orc idêntico (só escala muda)     | Ornamentos, glow, tamanho exagerado       |

### 4.5 Environment (`environment.py`)

Mantém a qualidade atual dos tiles/buildings mas com estilo unificado:

**Tiles (já funcionais, ajustes de coerência):**
- `dungeon_tileset.png` (64x32) — manter base, adicionar variação de crack/moss na nova versão
- `town_tileset.png` (64x32) — manter base, uniformizar paleta com personagens
- `corridor_floor.png` (192x32) — idem

**Props existentes (manter e melhorar):**
- `door_closed.png`, `door_open.png` — OK, uniformizar outline
- `building_tavern.png`, `building_forge.png`, `building_elder.png` — OK, são bons
- `fountain.png` — OK

**Props placeholder (hoje são checkerboards, gerar de verdade):**
- `dungeon_pillar.png` (96x128) — pilar de pedra com textura e sombra
- `dungeon_wall_mid.png` (128x128) — parede de pedra com juntas
- `barrel.png` (64x64) — barril de madeira com aros de metal
- `altar.png` (80x80) — altar de pedra com runas

**Backlog (gerar quando integrar no runtime):**
- Variantes de props (barrel_broken, altar_corrupted, pillar_cracked)
- Tiles variantes dungeon (floor_base, floor_crack, floor_moss)
- Door locked
- FX sheets (spell_frost, spell_chain, spell_nova, arrow, hit_flash)
- UI (upgrade_icons)

### 4.6 Áudio (`audio.py`)

**Dependência nova: `pyfxr`** (pip install pyfxr) para SFX estilo sfxr.

**De → Para:**

| Aspecto         | Hoje                              | Proposta                                  |
|-----------------|-----------------------------------|-------------------------------------------|
| Waveforms       | Só quadrada + noise               | Quadrada, triângulo, saw, sine, noise     |
| Sample rate     | 22050 Hz                          | 44100 Hz                                  |
| Envelope        | `exp(-t*K)` simples               | ADSR real (attack/decay/sustain/release)  |
| Filtros         | Nenhum                            | Passa-baixa simples + resonância          |
| SFX             | Sweeps/blips genéricos            | sfxr-style com parâmetros por tipo de som |
| Músicas         | 1 canal, 1 nota por vez           | 2-3 canais (melodia + baixo + percussão)  |
| Progressão      | Nota fixa ou step repetitivo      | Progressão de acordes (I-IV-V-vi etc.)    |
| Efeito          | Nenhum                            | Delay/eco simples nas músicas             |

**SFX com pyfxr:**
- `hit.wav` → impacto metálico com decay curto
- `player_attack.wav` → whoosh de espada (sweep descendente + noise)
- `enemy_death.wav` → arpejo descendente com decay
- `dash.wav` → whoosh rápido ascendente
- `spell_bolt.wav` → zap elétrico ascendente
- `spell_nova.wav` → expansão com reverb
- `parry.wav` → clank metálico agudo
- Etc. (mesma lista de 19 SFX + 5 músicas + 5 ambients)

**Músicas procedurais melhoradas:**
- `music_town.wav` → Melodia em triângulo + baixo em quadrada, progressão I-IV-V-vi, 16s
- `music_dungeon_calm.wav` → Drone grave + melodia minor key esparsa, 20s
- `music_dungeon_combat.wav` → Pulso rápido de baixo + arpejo tenso, 16s
- `music_boss.wav` → Graves pesados + dissonância crescente, 20s
- `music_victory.wav` → Fanfare maior com harmonia, 4s

### 4.7 Asset `dungeon_ambient.wav`

Referenciado apenas em `asset_manifest.hpp:12` para log de "missing asset" no boot.
**Não** está em `kAmbientPaths` (o sistema de áudio não o carrega em runtime).

**Decisão necessária:** remover a referência de `asset_manifest.hpp` ou integrar em `AmbientId`?

---

## 5. Pipeline de migração

### Fase 0 — Preparação (antes de qualquer código novo)

1. Criar `legacy/tools/` (se não existir)
2. Mover scripts atuais:
   - `tools/gen_animated_sprites.py` → `legacy/tools/gen_animated_sprites.py`
   - `tools/gen_environment_sprites.py` → `legacy/tools/gen_environment_sprites.py`
   - `tools/gen_placeholder_textures.py` → `legacy/tools/gen_placeholder_textures.py`
   - `tools/gen_prototype_8bit_sfx.py` → `legacy/tools/gen_prototype_8bit_sfx.py`
3. Mover metadados associados:
   - `tools/texture_manifest.json` → `legacy/tools/texture_manifest.json`
   - `tools/texture_backlog_contract.json` → `legacy/tools/texture_backlog_contract.json`
   - `tools/audit_texture_contract.py` → `legacy/tools/audit_texture_contract.py`
4. Criar `assets/_backup_pre_pipeline/` com cópia dos assets atuais (safety net)
5. Atualizar `Makefile` (targets `gen-placeholders`, `preview-placeholders`, etc.)
6. Atualizar README seção "Assets"

### Fase 1 — Infraestrutura do novo pipeline

1. Criar `tools/mion_assets/` com módulos base:
   - `png_writer.py` — writer PNG RGBA (extrair do existente, single copy)
   - `wav_writer.py` — writer WAV (extrair do existente)
   - `primitives.py` — todas as primitivas de desenho unificadas
   - `style.py` — paleta global, regras de outline/sombra
   - `manifest.py` — inventário declarativo de todos os assets
2. Criar `tools/gen_assets.py` — entry point:
   - `python3 tools/gen_assets.py` → gera tudo
   - `python3 tools/gen_assets.py --only sprites` → só personagens
   - `python3 tools/gen_assets.py --only environment` → só tiles/props/buildings
   - `python3 tools/gen_assets.py --only audio` → só WAVs
   - `python3 tools/gen_assets.py --overwrite` → sobrescreve existentes
   - `python3 tools/gen_assets.py --list` → lista o que seria gerado sem gerar
3. Validar: `gen_assets.py --list` produz exatamente os mesmos paths que os 4 scripts antigos combinados

### Fase 2 — Sprites de personagem (prioridade máxima)

1. Implementar `characters.py` com rig paramétrico
2. Definir os 10 archetypes (player, 6 inimigos, 3 NPCs + elder)
3. Gerar os 10 sprite sheets no layout 768x256
4. Rodar `make run` — validar visualmente que personagens aparecem corretos
5. Atualizar `texture_manifest.json` (ou equivalente no novo pipeline) para 768x256 24x8

### Fase 3 — Props de dungeon (substituir checkerboards)

1. Implementar no `environment.py` os 4 props que hoje são checkerboard:
   - `dungeon_pillar.png` — pilar de pedra
   - `dungeon_wall_mid.png` — parede de pedra
   - `barrel.png` — barril de madeira
   - `altar.png` — altar de pedra
2. Regenerar tiles e buildings (mesmo estilo, melhorias marginais)
3. Validar visualmente

### Fase 4 — Áudio

1. Instalar `pyfxr` como dependência do pipeline: `pip install pyfxr`
2. Implementar `audio.py` com SFX via pyfxr
3. Implementar músicas procedurais melhoradas (multi-canal, ADSR, progressão de acordes)
4. Gerar os 30 WAVs
5. Rodar `make run` — validar que áudio funciona e soa melhor
6. Decidir sobre `dungeon_ambient.wav` (remover referência ou integrar)

### Fase 5 — Limpeza

1. Remover `assets/_backup_placeholders/` (backup antigo dos placeholders)
2. Remover `assets/placeholders/` (previews dos antigos)
3. Confirmar que `legacy/tools/` tem os scripts antigos
4. Atualizar README
5. Atualizar `Makefile` com novo target:
   - `make gen-assets` → `python3 tools/gen_assets.py --overwrite`
   - Remover targets antigos (`gen-placeholders`, `preview-placeholders`, etc.)

---

## 6. Dependências

| Dependência     | Já disponível? | Necessária para |
|-----------------|----------------|-----------------|
| Python 3.8+     | Sim            | Tudo            |
| Pillow (PIL)    | Sim (10.2.0)   | Sprites melhorados (dithering, composição) |
| pyfxr           | **Não**        | SFX melhorados (Fase 4) |
| numpy           | **Não**        | Opcional (não obrigatório se evitarmos) |

**Nota:** O pipeline atual usa ZERO dependências externas para sprites (PNG manual via zlib/struct).
A proposta de usar Pillow é opcional — podemos continuar com o writer manual se preferir
zero dependência, ao custo de implementar dithering/composição na mão.

---

## 7. Riscos

| Risco | Impacto | Mitigação |
|-------|---------|-----------|
| Layout 768x256 vs 256x256 no manifest | Engine espera 768x256 (24 cols) — manifest diz 256x256 | Corrigir manifest; engine já funciona com 768x256 |
| Estilo novo "pior" que o atual | Regressão visual | Backup em `_backup_pre_pipeline/`; old scripts em `legacy/tools/` |
| pyfxr não disponível em CI/produção | Build quebra | Audio é gerado offline; WAVs commitados; pyfxr é dev-only |
| Props de dungeon mudam de tamanho | Colisão/layout quebra | Manter exatamente os mesmos tamanhos do manifest |

---

## 8. Critério de DONE

- [ ] `python3 tools/gen_assets.py` gera TODOS os assets listados na seção 1 (sprites + tiles + props + áudio)
- [ ] Nenhum script antigo em `tools/` (todos em `legacy/tools/`)
- [ ] `make run` funciona com os assets novos sem erro
- [ ] Personagens são visualmente distinguíveis entre si (silhueta, não só cor)
- [ ] Props de dungeon não são mais checkerboards
- [ ] Áudio usa pelo menos 2 waveforms diferentes e tem envelope ADSR
- [ ] Músicas têm pelo menos 2 canais (melodia + baixo)
- [ ] Manifest/contrato atualizado reflete os tamanhos reais (768x256 para personagens)
- [ ] README seção "Assets" atualizado com novo pipeline

---

## 9. Decisões pendentes (para o usuário)

1. **Pillow vs manual?** Usar Pillow para composição/dithering (mais fácil, já instalado)
   ou continuar com buffer RGBA manual (zero dependência)?

2. **pyfxr para SFX?** Instalar dependência nova ou melhorar o synth manual?

3. **`dungeon_ambient.wav`** — remover referência de `asset_manifest.hpp` ou
   integrar em `AmbientId`/`kAmbientPaths`?

4. **Backlog (Fase futura):** gerar os FX sheets / variantes de props nesta pipeline ou
   deixar para quando o runtime integrá-los?

5. **Prioridade de fases:** a ordem proposta (sprites → props → áudio → limpeza) está OK
   ou prefere outra sequência?
