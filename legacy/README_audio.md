# Áudio do Mion Engine

Este diretório contém os ficheiros **WAV** usados pelo jogo. O motor usa **SDL3** diretamente (sem SDL_mixer): SFX com um stream por som, música e ambiente em loop com reabastecimento por frame.

## Formato esperado

- **WAV**, preferencialmente **mono**, **PCM 16-bit**.
- O gerador de protótipo usa **22050 Hz**; o SDL carrega o que o ficheiro declarar — mantém taxa de amostragem consistente para evitar surpresas em memória e qualidade.

## Gerar placeholders (protótipo 8-bit)

Na raiz do repositório:

```bash
python3 tools/gen_prototype_8bit_sfx.py
```

Saída por defeito: esta pasta (`assets/audio`). Para outro destino:

```bash
python3 tools/gen_prototype_8bit_sfx.py --output-dir caminho/para/audio
```

O script **`tools/gen_prototype_8bit_sfx.py`** deve listar **todos** os WAV que o código referencia por caminho fixo (ver secção abaixo). Ao adicionar um som novo ao motor, atualiza o dicionário `specs` no script **ou** passas a fornecer o WAV manualmente (Foley / DAW) com o **mesmo nome** relativo sob `assets/audio/`.

---

## Mapa: ficheiros ↔ código

### SFX (`SoundId`)

Ordem do enum em `src/core/audio.hpp` tem de coincidir com `_sfx_paths[]` em `src/core/audio.cpp` (índice 0 = primeiro ficheiro).

| Ficheiro | `SoundId` | Notas |
|----------|-----------|--------|
| `hit.wav` | `Hit` | Golpes (com atenuação espacial em alguns sítios) |
| `player_attack.wav` | `PlayerAttack` | Ataque corpo a corpo |
| `enemy_death.wav` | `EnemyDeath` | Inimigo morre |
| `player_death.wav` | `PlayerDeath` | Jogador morre |
| `dash.wav` | `Dash` | Dash |
| `ranged_attack.wav` | `RangedAttack` | Tiro único à distância |
| `spell_bolt.wav` | `SpellBolt` | Strafe (spell 3 quando Chain não está disponível) |
| `spell_nova.wav` | `SpellNova` | Nova |
| `sfx_spell_frost.wav` | `SpellFrost` | FrostBolt |
| `sfx_spell_chain.wav` | `SpellChain` | Chain Lightning |
| `sfx_skill_cleave.wav` | `SkillCleave` | Cleave |
| `sfx_skill_multishot.wav` | `SkillMultiShot` | Ataque à distância com talento Multi-Shot (vários projéteis) |
| `sfx_skill_poison_arrow.wav` | `SkillPoisonArrow` | Carregado no arranque; **ainda sem `play_sfx` no gameplay** — reservado |
| `sfx_skill_battle_cry.wav` | `SkillBattleCry` | Battle Cry |
| `parry.wav` | `Parry` | Parry bem-sucedido |
| `ui_confirm.wav` | `UiConfirm` | Confirmar UI / diálogo |
| `ui_delete.wav` | `UiDelete` | Apagar save (title) |
| `sfx_footstep_player.wav` | `FootstepPlayer` | Passos do jogador |
| `sfx_footstep_enemy.wav` | `FootstepEnemy` | Passos de inimigos |

### Música (`MusicState`)

Definida em `src/core/audio.cpp` → `_music_paths[]`. `None` não tem ficheiro.

| Ficheiro | Estado |
|----------|--------|
| `music_town.wav` | `Town` |
| `music_dungeon_calm.wav` | `DungeonCalm` |
| `music_dungeon_combat.wav` | `DungeonCombat` |
| `music_boss.wav` | `DungeonBoss` |
| `music_victory.wav` | `Victory` |

### Ambiente (`AmbientId`)

| Ficheiro | ID |
|----------|-----|
| `ambient_dungeon_drips.wav` | `DungeonDrips` |
| `ambient_dungeon_torch.wav` | `DungeonTorch` |
| `ambient_town_wind.wav` | `TownWind` |
| `ambient_town_birds.wav` | `TownBirds` |

### Manifesto vs motor

| Ficheiro | Uso |
|----------|-----|
| `dungeon_ambient.wav` | Apenas verificação opcional em `src/core/asset_manifest.hpp` (`log_missing_assets_optional`). **Não** é carregado por `AudioSystem`; o dungeon usa música + ambiente listados acima. Mantém-se no gerador para alinhar com o manifest. |

### Música “ad hoc”

Se alguma cena chamar `play_music("assets/audio/...")` com um caminho **novo**, esse ficheiro também precisa de existir e deve ser acrescentado ao manifest opcional e, se quiseres placeholders, ao script de geração. Procura no código por `play_music(` e `assets/audio/`.

---

## Checklist: adicionar um som novo ao jogo

1. **`src/core/audio.hpp`**  
   - Novo valor em `SoundId` **antes** de `COUNT` (ou novo `AmbientId` / entrada em `MusicState`, conforme o caso).

2. **`src/core/audio.cpp`**  
   - Acrescentar `"assets/audio/nome.wav"` no array correspondente **na mesma ordem** que o enum.  
   - Para música/ambiente, atualizar `_music_paths` ou `_ambient_paths`.

3. **`src/core/asset_manifest.hpp`**  
   - Incluir o caminho em `kPaths[]` se quiseres que o arranque registe ficheiros em falta (recomendado para tudo o que o `AudioSystem` carrega).

4. **`tools/gen_prototype_8bit_sfx.py`**  
   - Entrada em `specs`: `"nome.wav": render_...()`, **ou** não adicionar se o áudio for só artístico importado manualmente.

5. **Gameplay**  
   - Chamar `play_sfx`, `play_sfx_pitched`, `play_sfx_at`, `set_music_state`, `play_ambient`, etc., no sítio certo.

6. **Testes / CI**  
   - Se houver testes que assumem lista de assets, atualizar. Correr `mion_tests_v2_core` após mudanças em áudio.

7. **Documentação**  
   - Atualizar as tabelas deste `README.md` para o próximo editor não precisar de caçar enums à mão.

---

## Volumes e API (resumo)

- `play_sfx` / `play_sfx_pitched` / `play_sfx_at`: volume por chamada; `play_sfx_at` aplica queda quadrática com a distância à câmara.
- `set_music_state`: troca de faixa com cross-fade; volumes base por estado estão em `audio.cpp` (`_apply_music_state_after_fade`).
- `set_master_volume`: escala global (inclui música).
- `tick_music()` deve ser chamado **uma vez por frame** para loop e fades.

Ficheiros de referência: `src/core/audio.hpp`, `src/core/audio.cpp`, `src/systems/spell_system.hpp`, `src/systems/dungeon_audio_system.hpp`, `src/systems/footstep_audio_system.hpp`.
