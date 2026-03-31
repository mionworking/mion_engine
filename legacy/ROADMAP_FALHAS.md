# ROADMAP DE FALHAS — mion_engine_cpp

> Auditoria inicial: 2026-03-31
> Revisão 1 (verificação manual no código): 2026-03-31
> 7 itens corrigidos/reclassificados após verificação

---

## Sumário Executivo

| Categoria | Total | Alta | Média | Baixa |
|-----------|-------|------|-------|-------|
| Falhas de Integração | 9 | 2 | 5 | 2 |
| Type Safety | 5 | 1 | 2 | 2 |
| Implementações Incompletas | 5 | 1 | 2 | 2 |
| Inconsistências de Naming | 3 | 0 | 0 | 3 |
| Problemas Arquiteturais | 3 | 2 | 1 | 0 |
| **TOTAL ATIVO** | **25** | **6** | **10** | **9** |

> **7 itens da auditoria inicial foram removidos após verificação direta no código** — ver seção "Itens Encerrados".

---

## Legenda de Severidade

- `[ALTA]` — Pode causar crash, comportamento incorreto ou perda de dados
- `[MEDIA]` — Inconsistência funcional, balanceamento quebrado, dívida técnica significativa
- `[BAIXA]` — Limpeza, naming, robustez futura

---

## Fase 1 — Correções Críticas (Alta Prioridade)

### F-001 `[ALTA]` — void* para SDL_Texture* em Actor

**Arquivo:** [src/entities/actor.hpp:62](src/entities/actor.hpp#L62)

```cpp
// PROBLEMA:
void* sprite_sheet = nullptr;  // SDL_Texture*

// TODO: Alterar para:
SDL_Texture* sprite_sheet = nullptr;
// E remover todos os static_cast<SDL_Texture*>(a.sprite_sheet) no código
```

**Impacto:** Cast incorreto resulta em Undefined Behavior silencioso. Todos os sistemas de renderização que acessam `sprite_sheet` estão vulneráveis.

**Correção:** `struct SDL_Texture;` (forward declaration) em `actor.hpp` e tipar diretamente.

---

### F-002 `[ALTA]` — Actor como God Object

**Arquivo:** [src/entities/actor.hpp:1](src/entities/actor.hpp)

```cpp
// PROBLEMA: Actor mistura em 174 linhas:
// - Física / posição (x, y, vx, vy)
// - CombatState combat;
// - AnimPlayer anim;
// - EquipmentState equipment;
// - ProgressionState progression;
// - SpellBookState spell_book;
// - TalentState talent_state;
// - DerivedStats derived;
// - StatusEffectState status_effects;
// Tudo em um único struct sem separação
```

**Impacto:** Qualquer mudança em qualquer componente recompila tudo que inclui `actor.hpp`. Torna teste unitário impossível.

**Correção (longo prazo):** Separar em componentes opcionais ou usar handle/ID approach.

---

### F-003 `[ALTA]` — AudioSystem* sem proteção contra null em toda a base

**Arquivo:** [src/core/scene_context.hpp:12](src/core/scene_context.hpp#L12)

```cpp
// CONTEXTO:
// player_action.hpp já protege todos os 9 call-sites:
if (audio) audio->play_sfx_pitched(SoundId::Dash);  // OK

// enemy_ai.hpp não chama audio diretamente — não é risco atual.

// RISCO REAL: padrão de proteção não é enforçado pelo tipo.
// Qualquer novo sistema pode receber o ctx e chamar audio-> sem guard,
// porque o tipo não comunica que pode ser null.
```

**Impacto:** Futuro código novo pode crashar em ambientes headless/CI sem aviso.

**Correção (boa prática — Effective C++ Item 18):** Usar `std::reference_wrapper<AudioSystem>` ou implementar `NullAudioSystem` para eliminar a possibilidade de null do contrato do tipo.

---

### F-004 `[ALTA]` — Equipment não conectado ao DerivedStats após compra

**Arquivo:** [src/systems/shop_system.hpp:34](src/systems/shop_system.hpp#L34)

```cpp
// PROBLEMA: ShopSystem aplica upgrade direto em campos de Actor:
player.mana.max += 15;

// Mas DerivedStats nunca é recalculado após equipar item.
// recompute_player_derived_stats() não é chamado em ShopSystem.
```

**Impacto:** Equipamentos de armadura/ataque são decorativos — não afetam gameplay.

**Correção:** Chamar `recompute_player_derived_stats(player)` ao final de cada transação no ShopSystem.

---

### F-005 `[ALTA]` — Migração de save v2→v3 ausente

**Arquivo:** [src/core/save_system.hpp:230](src/core/save_system.hpp#L230)

```cpp
// PROBLEMA: Chain de migração tem gap:
// v1 → v2: OK (linha ~189)
// v2 → v3: AUSENTE
// v3 → v4: OK
// Saves na versão 2 não conseguem chegar à v4
```

**Impacto:** Players com saves antigos (v2) terão dados corrompidos ou jogo crasha ao carregar.

**Correção:** Implementar bloco `migrate_v2_to_v3()` antes da migração v3→v4.

---

## Fase 2 — Falhas de Integração (Média Prioridade)

### F-006 `[MEDIA]` — kEnemyProjSpeed hardcoded

**Arquivo:** [src/systems/enemy_ai.hpp:19](src/systems/enemy_ai.hpp#L19)

```cpp
// PROBLEMA:
static constexpr float kEnemyProjSpeed = 340.0f;

// Padrão do engine usa config.ini para balanceamento (ex: player configs)
// mas velocidade de projétil de inimigo não é configurável
```

**Correção:** Expor via `data/enemies.ini` ou seção `[combat]` no `config.ini`.

---

### F-007 `[MEDIA]` — g_attribute_scales nunca carregado do INI

**Arquivo:** [src/components/attributes.hpp:34](src/components/attributes.hpp#L34)

```cpp
// PROBLEMA:
inline AttributeScales g_attribute_scales;  // valores default hardcoded

// O padrão apply_*_ini() existe no engine mas
// g_attribute_scales nunca é populado a partir do config.ini
// Vigor/Forca/Destreza têm scaling fixo no código
```

**Correção:** Chamar `apply_attributes_ini(ini)` em `main.cpp` junto com os outros `apply_*_ini()` calls.

---

### F-008 `[MEDIA]` — sync_from_talents com thresholds hardcoded

**Arquivo:** [src/components/spell_book.hpp:68](src/components/spell_book.hpp#L68)

```cpp
// PROBLEMA:
void sync_from_talents(const TalentState& ts) {
    fireball_rank = ts.ge(TalentId::SpellPower, 1) ? 1 : 0;
    // threshold "1" hardcoded em vez de ler TalentNode.parent_min_level
}
```

**Impacto:** Rebalancear custo de talentos de magia requer mudança no código, não nos dados.

**Correção:** Ler threshold a partir de `g_talent_nodes[TalentId::SpellPower].unlock_level`.

---

### F-009 `[MEDIA]` — Dano de melee assimétrico entre player e inimigo

**Arquivo:** [src/systems/melee_combat.hpp:82](src/systems/melee_combat.hpp#L82)

```cpp
// Player usa:
float dmg = attacker.derived.melee_damage_final;  // aplica modificadores

// Inimigo usa:
float dmg = attacker.attack_damage;  // campo bruto, ignora DerivedStats
```

**Impacto:** Inimigos ignoram qualquer modificador de dano em DerivedStats — balanceamento inconsistente.

**Correção:** Normalizar para `derived.melee_damage_final` em todos os atacantes, ou garantir que inimigos tenham derived stats calculado antes do combate.

---

### F-010 `[MEDIA]` — Cobertura incompleta de Status Effects nos sistemas de ação

**Arquivo:** [src/systems/melee_combat.hpp:122](src/systems/melee_combat.hpp#L122)

```cpp
// CHECADO:
// player_action.hpp:75 — verifica Stun antes de mover   ✓
// enemy_ai.hpp:57      — verifica Stun antes de atacar  ✓

// NAO CHECADO:
// spell_effects.hpp — não verifica Silence antes de conjurar
// melee_combat.hpp  — aplica status effects Enemy→Player mas não Player→Enemy
```

**Correção:** Varredura sistemática — adicionar verificação de status no início de cada sistema que processa ações.

---

### F-011 `[MEDIA]` — SceneCreateContext não injeta Locale, Save e Input

**Arquivo:** [src/core/scene_context.hpp:11](src/core/scene_context.hpp#L11)

```cpp
// PROBLEMA:
struct SceneCreateContext {
    SDL_Renderer* renderer;
    AudioSystem*  audio;     // injetado
    SDL_FRect     viewport;
    // FALTANDO:
    // LocaleSystem*  locale;  // acessado via g_locale global
    // SaveSystem*    save;    // acessado via global
    // InputSystem*   input;   // acessado via global
};
```

**Impacto:** Quebra consistência com o padrão de injeção já adotado para AudioSystem. Dificulta testes isolados de cenas.

**Correção:** Adicionar os 3 campos ao contexto e remover acesso a globals nas cenas.

---

### F-012 `[MEDIA]` — Caminho dash.wav ausente do asset_manifest

**Arquivo:** [src/core/audio.cpp:47](src/core/audio.cpp#L47)

```cpp
// audio.cpp registra:
{ SoundId::Dash, "assets/audio/dash.wav" }

// Mas asset_manifest.hpp não lista este arquivo.
// log_missing_assets_optional() nunca alertará sobre ausência dele.
```

**Correção:** Adicionar `"assets/audio/dash.wav"` ao manifest.

---

### F-013 `[BAIXA]` — reset_talent_tree_defaults() duplica inicialização

**Arquivo:** [src/components/talent_tree.hpp:88](src/components/talent_tree.hpp#L88)

```cpp
// PROBLEMA:
// g_talent_nodes[] é inicializado em uma função
// reset_talent_tree_defaults() replica manualmente os mesmos dados
// Adicionar novo talento requer atualizar DOIS lugares
```

**Correção:** `reset_talent_tree_defaults()` deve apenas copiar de um `constexpr` array de defaults.

---

### F-014 `[BAIXA]` — Migração v1→v2 perde level de talento

**Arquivo:** [src/core/save_system.hpp:189](src/core/save_system.hpp#L189)

```cpp
// Durante migração v1 → v2:
new_talent_lv = (old_lv != 0) ? 1 : 0;
// Perde o level real (ex: talento no nível 3 vira nível 1)
```

**Impacto:** Players que sobem de save v1 perdem progresso de talents.

**Correção:** Preservar `old_lv` diretamente na migração.

---

## Fase 3 — Type Safety

### F-015 `[MEDIA]` — Índice de array sem bounds check em TilemapRenderer

**Arquivo:** [src/systems/tilemap_renderer.hpp:70](src/systems/tilemap_renderer.hpp#L70)

```cpp
// PROBLEMA:
SDL_Color c = TILE_COLOR[(int)type];
// TileType inválido → acesso fora do array → UB
```

**Correção:**
```cpp
assert((int)type < (int)std::size(TILE_COLOR));
```

---

### F-016 `[MEDIA]` — std::rand() sem seed garantida em DropSystem

**Arquivo:** [src/systems/drop_system.hpp:27](src/systems/drop_system.hpp#L27)

```cpp
// PROBLEMA:
if (std::rand() % 100 < cfg.drop_chance_pct)
// std::rand() global não é thread-safe e tem distribuição fraca
```

**Correção:** Substituir por `std::mt19937` com seed controlada (passada via contexto ou inicializada uma vez em main).

---

### F-017 `[ALTA]` — void* em actor.hpp já catalogado em F-001

*(ver F-001)*

---

### F-018 `[BAIXA]` — C-style cast em main.cpp

**Arquivo:** [src/main.cpp:37](src/main.cpp#L37)

```cpp
// PROBLEMA:
int val = (int)parsed;

// CORRETO (C++ Core Guidelines ES.49):
int val = static_cast<int>(parsed);
```

---

### F-019 `[BAIXA]` — Callbacks to_sx/to_sy sem validação de null

**Arquivo:** [src/systems/floating_text.hpp:66](src/systems/floating_text.hpp#L66)

```cpp
float sx = to_sx(ft.x);  // crash se to_sx == nullptr
```

**Correção:** Assert na inicialização do FloatingText ou validação antes do render.

---

## Fase 4 — Implementações Incompletas

### F-020 `[MEDIA]` — Pitch shifting com fallback silencioso

**Arquivo:** [src/core/audio.cpp:166](src/core/audio.cpp#L166)

```cpp
if (spec.format != SDL_AUDIO_S16 || spec.channels != 1) {
    play_sfx(id);  // fallback silencioso — perde variação de pitch
    return;
}
```

**Impacto:** Sons em formato diferente de S16 mono perdem variação de pitch sem qualquer aviso.

**Correção:** Adicionar `SDL_Log("WARN: play_sfx_pitched: formato não suportado para '%s'", ...)` antes do fallback.

---

### F-021 `[MEDIA]` — apply_frost_slow com fórmula hardcoded

**Arquivo:** [src/systems/spell_effects.hpp:114](src/systems/spell_effects.hpp#L114)

```cpp
float slow = 0.55f - 0.05f * (rank - 1);
// Rebalancear requer recompilação
```

**Correção:** Ler de `g_spell_config.frost_slow_base` e `g_spell_config.frost_slow_per_rank`.

---

### F-022 `[BAIXA]` — AnimPlayer Cast/Dash: fallback explícito, sem log de diagnóstico

**Arquivo:** [src/core/animation.hpp:101](src/core/animation.hpp#L101)

```cpp
// Comportamento ATUAL (por design):
// Cast/Dash: fallback para Idle se não há sheet dedicada  ← comentado no código
make(ActorAnim::Cast, 0, idle_n, false);
make(ActorAnim::Dash, 1, walk_n, true);

// AUSENTE: nenhum SDL_Log de diagnóstico quando o fallback é ativado.
// Em debug, difícil saber quais atores estão no caminho de fallback.
```

**Observação:** O fallback é design intencional (documentado). A sugestão é apenas adicionar log em modo debug para facilitar diagnóstico durante desenvolvimento.

---

### F-023 `[BAIXA]` — StatusEffect sobrescreve slots[0] sem política

**Arquivo:** [src/components/status_effect.hpp:45](src/components/status_effect.hpp#L45)

```cpp
void apply_full(...) {
    // Quando todos os slots estão ocupados, sobrescreve slots[0]
    // Sem LRU, sem prioridade por tipo (Stun deve ter precedência sobre Poison)
}
```

**Correção:** Implementar política de substituição — ex: substituir o efeito com menor tempo restante, ou por prioridade de tipo.

---

## Fase 5 — Naming e Estilo

### F-024 `[BAIXA]` — Magic number PI em player_action.hpp

**Arquivo:** [src/systems/player_action.hpp:186](src/systems/player_action.hpp#L186)

```cpp
// PROBLEMA:
float dtheta = 24.0f * (3.14159265f / 180.0f);

// CORRETO (C++20, disponível com -std=c++20):
#include <numbers>
constexpr float kDegToRad = std::numbers::pi_v<float> / 180.0f;
float dtheta = 24.0f * kDegToRad;
```

---

### F-025 `[BAIXA]` — Phase::Black em SceneFader com nome ambíguo

**Arquivo:** [src/core/scene_fader.hpp:49](src/core/scene_fader.hpp#L49)

```cpp
enum class Phase { FadeOut, Black, FadeIn, Done };
// "Black" é um estado de hold com duração, não uma transição instantânea.
// Sugestão: renomear para "HoldBlack" para refletir a semântica.
```

---

### F-026 `[BAIXA]` — Sufixo _ptr inconsistente com o restante do engine

**Arquivo:** [src/scenes/title_scene.hpp:328](src/scenes/title_scene.hpp#L328)

```cpp
DifficultyConfig* _difficulty_ptr;  // usa sufixo _ptr

// Restante do engine:
SDL_Renderer* _renderer;   // sem sufixo
AudioSystem*  _audio;      // sem sufixo
```

**Correção:** Renomear para `_difficulty` seguindo o padrão estabelecido.

---

## Checklist de Progresso

### Fase 1 — Crítico
- [ ] F-001 — void* para SDL_Texture*
- [ ] F-002 — Actor God Object (refactor longo)
- [ ] F-003 — AudioSystem null safety por tipo (NullObject pattern)
- [ ] F-004 — Equipment → DerivedStats wiring no ShopSystem
- [ ] F-005 — Migração save v2→v3

### Fase 2 — Integração
- [ ] F-006 — kEnemyProjSpeed via INI
- [ ] F-007 — g_attribute_scales via INI
- [ ] F-008 — sync_from_talents thresholds data-driven
- [ ] F-009 — Melee damage assimétrico
- [ ] F-010 — Status effects: cobertura em spell/drop systems
- [ ] F-011 — SceneCreateContext: injetar Locale, Save, Input
- [ ] F-012 — dash.wav no asset_manifest
- [ ] F-013 — reset_talent_tree_defaults duplicação
- [ ] F-014 — Migração v1→v2 preservar talent level

### Fase 3 — Type Safety
- [ ] F-015 — Bounds check em TilemapRenderer
- [ ] F-016 — std::rand → std::mt19937
- [ ] F-018 — C-style cast em main.cpp
- [ ] F-019 — Callbacks floating_text null guard

### Fase 4 — Incompleto
- [ ] F-020 — Pitch fallback: adicionar SDL_Log warning
- [ ] F-021 — Frost slow via config
- [ ] F-022 — AnimPlayer fallback: adicionar log de diagnóstico (debug)
- [ ] F-023 — StatusEffect: política de substituição de slot

### Fase 5 — Naming
- [ ] F-024 — Magic number PI
- [ ] F-025 — Phase::Black → Phase::HoldBlack
- [ ] F-026 — Sufixo _ptr → bare name

---

## Itens Encerrados (verificados no código em 2026-03-31)

Estes itens foram identificados na auditoria inicial mas **não correspondem ao estado atual do código**.

| Item original | Status real | Evidência |
|---------------|-------------|-----------|
| F-005 original — `boss_defeated` nunca setado (era `int`) | **Resolvido** | `run_stats.hpp:17` (`bool`); `dungeon_scene.hpp:634` (set); `save_system.hpp:220,285` (serializado) |
| F-011 original — DungeonScene sem TextureCache | **Resolvido** | `dungeon_scene.hpp:1073` (`std::optional<TextureCache> _tex_cache`), usado em 6+ call-sites |
| F-016 original — SceneRegistry::create sem null check | **Resolvido** | `main.cpp:101–108` (initial scene) e `main.cpp:167` (`if (next_scene)`) — ambos protegidos |
| F-023 original — `pending_level_ups` nunca consumido | **Resolvido** | `progression.hpp:72,76,80` (3 métodos consomem); `dungeon_scene.hpp:438,655,958` (verificado ativamente) |
| F-027 original — `g_player_config` não reseta no DungeonScene | **Resolvido** | `dungeon_scene.hpp:1603` (`reset_player_config_defaults()`) |
| F-029 original — `requires_room_cleared = false` (invertido) | **Inválido** | `room.hpp:35` (`= true`) — default já correto |
| F-003 original — player_action/enemy_ai sem null guard | **Parcialmente inválido** | `player_action.hpp` protege todos os 9 call-sites; `enemy_ai.hpp` não usa áudio diretamente |

---

## Notas de Sprint

**Sprint 1 (impacto imediato, baixo risco):**
1. F-012 (dash.wav manifest) — 5 min
2. F-018 (C-style cast) — 5 min
3. F-004 (equipment → derived stats) — 30 min, impacto direto em gameplay
4. F-009 (melee damage assimétrico) — 20 min, impacto em balanceamento
5. F-005 (migração v2→v3) — 1h, crítico para saves existentes

**Sprint 2 (data-driven configs):**
- F-007 + F-006 — padrão `apply_*_ini()` já existe, fácil replicar
- F-008 — sync_from_talents data-driven
- F-010 — varredura de status effects

**Sprint 3 (arquitetura):**
- F-003 — NullAudioSystem / reference_wrapper
- F-011 — SceneCreateContext injeção completa
- F-001 — typed SDL_Texture* (requer verificar todos os cast-sites)
