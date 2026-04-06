# Plano — Migração para Open World
> mion_engine_cpp · 2026-04-04

---

## Premissa de design

O player caminha **livremente** entre town e dungeon no mesmo espaço de coordenadas. Não existe transição de cena — o mundo é contínuo. Inimigos e NPCs coexistem. A câmera segue o player pelo mundo inteiro.

Modelo de referência: **Zelda / Stardew** (não Metroidvania).

---

## O que muda de verdade

| Hoje | Open World |
|---|---|
| 2 cenas separadas com mundos próprios | 1 `WorldScene` com 1 espaço de coordenadas |
| Rebuild do mundo na transição de cena | Mundo construído uma vez no `enter()` |
| Colisão contra 1 `RoomDefinition` por vez | Broadphase — colisão contra N áreas simultâneas |
| Zona implícita (qual cena está ativa) | `ZoneManager` explícito — detecta por posição |
| Save na transição de cena | Save em checkpoint (inn / boss derrotado) |
| `DungeonContext` + `TownContext` separados | `WorldContext` unificado |

## O que **não** muda

Todos os controllers e sistemas refatorados continuam idênticos — eles já são modulares e stateless o suficiente para ser reutilizados diretamente.

`CombatFxController`, `EnemyDeathController`, `BossState`, `ScreenFlashState`,
`PauseMenuController`, `SkillTreeController`, `AttributeLevelUpController`,
`ShopInputController`, `TownNpcInteractionController`, todos os sistemas de simulação.

---

## Visão da arquitetura final

```
WorldScene
│
├── WorldMap                    ← mundo físico: áreas posicionadas em coords globais
│   ├── WorldArea[Town]
│   ├── WorldArea[DungeonRoom0]
│   ├── WorldArea[DungeonRoom1]
│   ├── WorldArea[DungeonRoom2]
│   └── WorldArea[Boss]
│
├── ZoneManager                 ← detecta zona atual por posição do player
│
├── [sistemas sempre ativos]
│   ├── MovementSystem
│   ├── PlayerActionSystem
│   ├── ResourceSystem
│   ├── StatusEffectSystem
│   └── RoomCollisionSystem     ← broadphase: obstacles de N áreas
│
├── [sistemas gateados por zona]
│   ├── EnemyAISystem           (Dungeon*)
│   ├── BossState               (Dungeon*)
│   ├── ProjectileSystem        (Dungeon*)
│   ├── CombatFxController      (Dungeon*)
│   ├── MeleeCombatSystem       (Dungeon*)
│   ├── EnemyDeathController    (Dungeon*)
│   ├── LightingSystem          (Dungeon*)
│   ├── TownNpcWanderSystem     (Town)
│   └── TownNpcInteractionController (Town)
│
├── WorldAudioSystem            ← zone-aware, substitui DungeonAudioSystem + town audio
├── WorldSaveController         ← substitui DungeonSaveController + TownSaveController
└── WorldContext                ← substitui DungeonContext + TownContext
```

---

## Fase 1 — WorldMap + Broadphase Collision

**Objetivo:** Mundo físico unificado. Player pode andar entre áreas. Colisão funciona em qualquer fronteira.

### Arquivos novos

#### `world/world_area.hpp`

```cpp
struct WorldArea {
    ZoneId         zone;
    float          offset_x = 0.f;   // posição global desta área no mundo
    float          offset_y = 0.f;
    RoomDefinition room;              // bounds e obstacles em coords LOCAIS
    Tilemap        tilemap;
};
```

Todos os obstacles são armazenados em coords locais e convertidos para globais
na hora da query. Isso evita duplicar dados ao mover áreas no editor.

#### `world/world_map.hpp`

```cpp
struct WorldMap {
    std::vector<WorldArea> areas;

    // Retorna obstacles de todas as áreas que intersectam query_rect.
    // Obstacles retornados já estão em coordenadas globais (offset aplicado).
    // O rect query deve ser ligeiramente expandido (ex: +64px) para capturar
    // obstacles de bordas adjacentes antes do player chegar neles.
    std::vector<Obstacle> get_obstacles_near(Rect query_rect) const;

    // Rect global que engloba todo o mundo — usado pelo Camera::follow.
    Rect total_bounds() const;

    // Áreas cujo tilemap intersecta o frustum da câmera — para render culling.
    std::vector<const WorldArea*> areas_in_view(const Camera2D& cam) const;

    // Qual área contém o ponto (px, py) — usado pelo ZoneManager.
    const WorldArea* area_at(float px, float py) const;
};
```

### Mudança em `RoomCollisionSystem`

Arquivo: [src/systems/room_collision.hpp](src/systems/room_collision.hpp)

```cpp
// ANTES — único método público
void resolve_all(std::vector<Actor*>& actors, const RoomDefinition& room);
// assinatura interna: resolve(Actor&, const RoomDefinition&)

// ADICIONAR — overload broadphase
void resolve(Actor& actor,
             std::span<const Obstacle> obstacles,
             const WorldBounds& bounds);

void resolve_all(std::vector<Actor*>& actors,
                 std::span<const Obstacle> obstacles,
                 const WorldBounds& world_bounds);
```

O corpo do novo `resolve` é idêntico ao existente: itera `bounds.clamp_box` + push-out por obstacle. Os métodos antigos (`resolve(Actor&, RoomDefinition&)` e `resolve_all(..., RoomDefinition&)`) **ficam** enquanto `DungeonScene`/`TownScene` ainda existirem (Fase 4 os remove).

`resolve_actors` não muda — actor-actor não depende da room.

### Adaptações em builders existentes

**`src/systems/town_builder.hpp`** — `TownBuilder::build_town_world`:
```cpp
// ANTES
static void build_town_world(RoomDefinition& room, Tilemap& tilemap,
                             TilemapRenderer& tile_renderer, ...);

// DEPOIS — coordenadas locais, WorldScene aplica offset ao montar WorldArea
static void build_town_world(RoomDefinition& room, Tilemap& tilemap,
                             TilemapRenderer& tile_renderer, ...,
                             float offset_x = 0.f, float offset_y = 0.f);
```
Todos os `room.add_obstacle` e `room.bounds` continuam em coords locais. `WorldArea::offset_x/y` é aplicado no `WorldMap::get_obstacles_near`.

**`src/systems/room_manager.hpp`** — `RoomManager::build_room`:
```cpp
// Mesma lógica — coords locais, offset passado para WorldArea externamente
static void build_room(RoomDefinition& room, Tilemap& tilemap,
                       int room_index, const IniData& rooms_ini,
                       float offset_x = 0.f, float offset_y = 0.f);
```

### Novo: `world/world_map_builder.hpp`

```cpp
// Constrói o WorldMap completo posicionando town + salas side-by-side.
// Layout base:
//   Town (640x480) — corredor 128px — Sala0 — Sala1 — Sala2 — Boss
// Cada área tem offset_x derivado da largura das áreas anteriores.
WorldMap build_world_map(const IniData& rooms_ini,
                         TextureCache* tc,
                         /* out */ std::vector<Tilemap>& tilemaps);
```

### Chamada na WorldScene

```cpp
// Expande o AABB do player para capturar obstacles de áreas vizinhas
const AABB query = expand(player_aabb(), 96.f);
auto obstacles   = _world_map.get_obstacles_near(query);  // coords globais
_col_sys.resolve_all(_actors, obstacles, _world_map.total_bounds());
_col_sys.resolve_actors(_actors, nullptr); // actor-actor sem room bound
```

### Entregável
Player anda livremente. Colisão funciona cruzando fronteiras. Tilemap renderiza áreas no frustum.

---

## Fase 2 — ZoneManager

**Objetivo:** Sistemas sabem em qual zona o player está sem if/else espalhado na cena.

### Arquivo novo: `world/zone_manager.hpp`

```cpp
enum class ZoneId {
    Town,
    DungeonRoom0,
    DungeonRoom1,
    DungeonRoom2,
    Boss,
    Transition   // player a cavalo entre duas áreas — usa zona anterior
};

struct ZoneManager {
    ZoneId current  = ZoneId::Town;
    ZoneId previous = ZoneId::Town;
    bool   just_changed = false;

    void update(float player_x, float player_y, const WorldMap& map) {
        const WorldArea* area = map.area_at(player_x, player_y);
        ZoneId detected = area ? area->zone : current; // mantém se fora de qualquer área

        just_changed = (detected != current);
        if (just_changed) {
            previous = current;
            current  = detected;
        }
    }

    bool in_dungeon() const {
        return current == ZoneId::DungeonRoom0
            || current == ZoneId::DungeonRoom1
            || current == ZoneId::DungeonRoom2
            || current == ZoneId::Boss;
    }

    bool in_town() const { return current == ZoneId::Town; }

    // Converte zona atual para int compatível com dungeon_rules (XP, spawn budget, HUD).
    // Necessário porque EnemyDeathController, EnemySpawner, DungeonHud ainda recebem int.
    int room_index_equiv() const {
        switch (current) {
            case ZoneId::DungeonRoom0: return 0;
            case ZoneId::DungeonRoom1: return 1;
            case ZoneId::DungeonRoom2: return 2;
            case ZoneId::Boss:         return 3;
            default: return 0;
        }
    }
};
```

> **`room_index` nas call sites**: `EnemyDeathController::process_deaths`, `EnemySpawner::spawn_budget`, `DungeonHud::render_dungeon_hud` e `WorldAudioSystem` (boss room = `zone.current == ZoneId::Boss`) continuam com as **mesmas assinaturas**. O `WorldScene` passa `zone_mgr.room_index_equiv()` onde hoje `DungeonScene` passa `_room_index` diretamente. Os corpos das funções não mudam.

### `just_changed` dispara:

| Evento | Direção | Ação |
|---|---|---|
| Town → Dungeon | entrou | Ligar `LightingSystem`, mostrar HUD dungeon, fade de áudio |
| Dungeon → Town | saiu | Desligar `LightingSystem`, esconder HUD dungeon, fade de áudio |
| * → Boss | entrou | `BossState::reset()`, spawnar boss se ainda não spawnou |

### Entregável
Sistemas gateados por zona funcionam. Áudio e lighting transitam suavemente.

---

## Fase 3 — WorldContext + WorldSaveController

**Objetivo:** Contexto unificado. Save/load funciona sem depender de transição de cena.

### `world/world_context.hpp`

Merge de `DungeonContext` + `TownContext`:

```cpp
struct WorldContext {
    WorldMap*              world_map        = nullptr;
    ZoneManager*           zone_mgr         = nullptr;
    AreaEntrySystem*       area_entry       = nullptr;  // substitui RoomFlowSystem
    Actor*                 player           = nullptr;
    std::vector<Actor>*    enemies          = nullptr;
    std::vector<Actor*>*   actors           = nullptr;
    std::vector<NpcEntity>* npcs            = nullptr;
    std::vector<Actor>*    npc_actors       = nullptr;
    ShopInventory*         shop_forge       = nullptr;
    std::vector<Projectile>* projectiles    = nullptr;
    std::vector<GroundItem>* ground_items   = nullptr;
    DialogueSystem*        dialogue         = nullptr;
    Camera2D*              camera           = nullptr;
    AudioSystem*           audio            = nullptr;
    RunStats*              run_stats        = nullptr;
    DifficultyLevel*       difficulty       = nullptr;
    QuestState*            quest_state      = nullptr;
    unsigned int*          scene_flags      = nullptr;

    // Dungeon-specific (usados por EnemyDeathController, EnemySpawner, AreaEntrySystem)
    IniData*        rooms_ini         = nullptr;
    EnemyDef*       enemy_defs        = nullptr; // array[kEnemyTypeCount]
    DropConfig*     drop_config       = nullptr;
    bool*           stress_mode       = nullptr;
    int*            requested_enemies = nullptr;

    // Town-specific (usados por TownWorldRenderer, sistemas de town)
    TilemapRenderer*    tile_renderer = nullptr;
    ResourceSystem*     resource      = nullptr;
    PlayerActionSystem* player_action = nullptr;
};
```

> **Nota**: `room_index` não entra no WorldContext — seu equivalente é `zone_to_room_index(zone_mgr->current)` (ver seção ZoneManager).

### `systems/world_save_controller.hpp`

Substitui `DungeonSaveController` + `TownSaveController`.

```cpp
namespace WorldSaveController {

    // Constrói SaveData a partir do WorldContext completo
    SaveData make_world_save(const WorldContext& ctx);

    // Aplica SaveData ao WorldContext (load)
    void apply_world_save(WorldContext& ctx, const SaveData& sd);

    // Persiste + pisca indicador de autosave
    void persist(const WorldContext& ctx, bool show_indicator, float& flash_timer);
}
```

**Onde o save acontece (substituindo save por transição):**

| Trigger | Ação |
|---|---|
| Boss derrotado | `persist()` automático |
| Player interage com inn / fogueira | `persist()` manual |
| Autosave a cada X salas completadas | `persist()` silencioso |
| Quit no pause menu | `persist()` antes de sair |

### Mudança em `SaveData`

```
kSaveFormatVersion: 5 → 6
```

Novos campos:
- `float player_world_x`, `player_world_y` — posição global; substitui `place_player_intro_spawn`
- `uint8_t visited_area_mask` — bitmask das áreas visitadas (bit 0=DungeonRoom0, 1=Room1, 2=Room2, 3=Boss); restaura `AreaEntrySystem::visited_areas` no load para evitar re-spawn de inimigos

`room_index` permanece no struct por compatibilidade de leitura de saves antigos, mas **não é mais escrito em saves v6** — torna-se campo legado.

Migração `migrate_v5_to_v6` em `save_migration.hpp`:
```cpp
inline SaveData migrate_v5_to_v6(SaveData data) {
    data.version          = kSaveFormatVersion; // 6
    data.player_world_x   = 0.f;  // default: spawn inicial da town
    data.player_world_y   = 0.f;  // (coordenada global definida pelo WorldMap)
    data.visited_area_mask = 0;   // nenhuma área visitada — re-spawn conservador
    // room_index do save antigo é ignorado; player começa na town
    return data;
}
```

### Implementação: arquivos modificados

**`src/core/save_data.hpp`** — após `unsigned int scene_flags`:
```cpp
// v6: open world
float        player_world_x   = 0.f;
float        player_world_y   = 0.f;
uint8_t      visited_area_mask = 0; // bit0=Room0, bit1=Room1, bit2=Room2, bit3=Boss
```

**`src/core/save_system.hpp`** — seção de escrita (linha ~246) e leitura (linha ~125):
```cpp
// escrita (versão 6)
f << "player_world_x=" << d.player_world_x << "\n";
f << "player_world_y=" << d.player_world_y << "\n";
f << "visited_area_mask=" << (int)d.visited_area_mask << "\n";

// leitura
d.player_world_x    = get_float(kv, "player_world_x", 0.f);
d.player_world_y    = get_float(kv, "player_world_y", 0.f);
d.visited_area_mask = (uint8_t)get_int(kv, "visited_area_mask", 0);
```

**`src/core/save_migration.hpp`** — adicionar no final:
```cpp
inline SaveData migrate_v5_to_v6(SaveData data) {
    data.version           = kSaveFormatVersion; // 6
    data.player_world_x    = 0.f; // posição = spawn inicial da town (offset 0,0)
    data.player_world_y    = 0.f;
    data.visited_area_mask = 0;   // conservador: re-spawn em todas as salas
    // data.room_index permanece legado; não usado em v6
    return data;
}
```
Registrar `migrate_v5_to_v6` na cadeia em `SaveSystem::load_default`.

**`src/systems/world_save_controller.hpp`** (novo) — baseado em `dungeon_save_controller.hpp` + `town_save_controller.hpp`:
```cpp
namespace WorldSaveController {

inline SaveData make_world_save(const WorldContext& ctx) {
    SaveData d;
    d.version              = kSaveFormatVersion;
    d.room_index           = 0; // legado: sempre 0 em v6
    d.player_hp            = ctx.player->health.current_hp;
    d.gold                 = ctx.player->gold;
    d.quest_state          = *ctx.quest_state;
    d.progression          = ctx.player->progression;
    d.talents              = ctx.player->talents;
    d.mana                 = ctx.player->mana;
    d.stamina              = ctx.player->stamina;
    d.difficulty           = ctx.difficulty ? (int)*ctx.difficulty : 1;
    d.attributes           = ctx.player->attributes;
    d.attr_points_available= ctx.player->progression.attr_points_available;
    d.scene_flags          = ctx.scene_flags ? *ctx.scene_flags : 0;
    d.player_world_x       = ctx.player->transform.x;
    d.player_world_y       = ctx.player->transform.y;
    d.visited_area_mask    = ctx.area_entry ? ctx.area_entry->to_mask() : 0;
    return d;
}

inline void apply_world_save(WorldContext& ctx, const SaveData& sd) {
    if (ctx.player) {
        ctx.player->progression = sd.progression;
        ctx.player->talents     = sd.talents;
        ctx.player->attributes  = sd.attributes;
        ctx.player->gold        = sd.gold;
        ctx.player->mana        = sd.mana;
        ctx.player->stamina     = sd.stamina;
        ctx.player->transform.x = sd.player_world_x;
        ctx.player->transform.y = sd.player_world_y;
    }
    if (ctx.quest_state)  *ctx.quest_state  = sd.quest_state;
    if (ctx.scene_flags)  *ctx.scene_flags  = sd.scene_flags;
    if (ctx.difficulty)   *ctx.difficulty   = static_cast<DifficultyLevel>(
                                                  std::clamp(sd.difficulty, 0, 2));
    if (ctx.area_entry)   ctx.area_entry->from_mask(sd.visited_area_mask);
    if (ctx.projectiles)  ctx.projectiles->clear();
    if (ctx.ground_items) ctx.ground_items->clear();
}

inline void persist(const WorldContext& ctx, bool show_indicator, float& flash_timer) {
    SaveData d = make_world_save(ctx);
    SaveSystem::save_default(d);
    if (show_indicator) flash_timer = 1.35f;
}

} // namespace WorldSaveController
```

### Entregável
Save/load funciona em qualquer ponto do mundo. Player é restaurado na posição salva.

---

## Fase 4 — WorldScene

**Objetivo:** Cena única substituindo `TownScene` + `DungeonScene`.

### `scenes/world_scene.hpp`

#### `enter()`
```
Carregar configs: DungeonConfigLoader + TownConfigLoader
Registrar dialogues (dungeon + town)
Construir WorldMap completo (town + dungeon rooms posicionadas)
Load save → apply_world_save → posicionar player
Iniciar ZoneManager
Iniciar áudio baseado na zona inicial
```

#### `fixed_update()`
```
_screen_flash.tick(dt)
_zone_mgr.update(player_pos, _world_map)

if (_zone_mgr.just_changed):
    _handle_zone_transition()    ← fade áudio, toggle lighting/HUD

[ui guard: hitstop / dialogue / pause]
[ui] AttributeLevelUpController
[ui] SkillTreeController

MovementSystem
PlayerActionSystem
ResourceSystem
StatusEffectSystem

if (zone_mgr.in_dungeon()):
    EnemyAISystem             (com activation radius)
    BossState::update(...)
    ProjectileSystem
    CombatFxController::apply_projectile_hit_fx
    MeleeCombatSystem
    CombatFxController::apply_combat_feedback
    CombatFxController::apply_melee_hit_fx
    EnemyDeathController::process_deaths

if (zone_mgr.in_town()):
    TownNpcWanderSystem
    TownNpcInteractionController

// Colisão — sempre, broadphase
auto obstacles = _world_map.get_obstacles_near(expand(player_aabb(), 96.f))
_col_sys.resolve_all(_actors, obstacles, _world_map.total_bounds())

AreaEntrySystem::update(...)    ← detecta entrada em área nova → spawna inimigos
WorldAudioSystem::update(...)
AnimationDriver
FootstepAudioSystem
FloatingTextSystem
Camera::follow(player, world_map.total_bounds())
```

#### `render()`
```
for (area in world_map.areas_in_view(camera)):
    TilemapRenderer::render(area.tilemap)
    if area.zone == Town:    TownWorldRenderer::render(area, npcs)
    if area.zone == Dungeon: DungeonWorldRenderer::render(area, enemies, projectiles, items)

Player sprite

if (zone_mgr.in_dungeon()):
    LightingSystem::render()
    DungeonHUD::render()

ScreenFx::render_screen_flash
ScreenFx::render_death_fade
if (boss_intro_active): ScreenFx::render_boss_intro

[overlays UI: attr, skill tree, dialogue, pause]
```

### `AreaEntrySystem` — substitui `RoomFlowSystem` para dungeon

```cpp
// Detecta quando player entra em uma WorldArea pela primeira vez nesta run.
// Spawna inimigos, dispara blurb de diálogo, persiste autosave.
// Não faz rebuild — a área já existe no WorldMap.
struct AreaEntrySystem {
    std::set<ZoneId> visited_areas;

    void update(const ZoneManager& zone, WorldContext& ctx);
    // se zone.just_changed && !visited_areas.count(zone.current):
    //     spawnar inimigos na área nova
    //     disparar diálogo se aplicável
    //     visited_areas.insert(zone.current)

    // Converte para/de bitmask para persistir em SaveData.
    uint8_t to_mask() const;
    void from_mask(uint8_t mask);
};
```

> **Persistência**: `WorldSaveController::make_world_save` chama `area_entry->to_mask()` e salva em `SaveData::visited_area_mask`. `apply_world_save` restaura via `area_entry->from_mask(sd.visited_area_mask)`. Sem isso, inimigos de salas já limpas ressurgem após load.

### `WorldAudioSystem` — substitui DungeonAudioSystem + town audio

```cpp
namespace WorldAudioSystem {
    void update(AudioSystem& audio,
                const ZoneManager& zone,
                const Actor& player,
                const std::vector<Actor>& enemies);
    // Gerencia fade de música e ambience baseado em zona + estado de combate.
    // boss room detectado via zone.current == ZoneId::Boss — não precisa de int.
}
```

### Implementação: `scenes/world_scene.hpp`

#### Members — copiar de `DungeonScene` + `TownScene`, remover duplicatas

```cpp
// === World ===
WorldMap            _world_map;
ZoneManager         _zone_mgr;
AreaEntrySystem     _area_entry;

// === Actors ===
Actor               _player;
std::vector<Actor>  _enemies;
std::vector<Actor*> _actors;           // ptrs: player + enemies
std::vector<NpcEntity>  _npcs;
std::vector<Actor>  _npc_actors;       // actors de NPCs para colisão
ShopInventory       _shop_forge;
QuestState          _quest_state;
unsigned int        _scene_flags = 0;

// === Sistemas sempre ativos ===
MovementSystem      _movement_sys;
PlayerActionSystem  _player_action;
ResourceSystem      _resource_sys;
StatusEffectSystem  _status_sys;
RoomCollisionSystem _col_sys;
DialogueSystem      _dialogue;
Camera2D            _camera;
TilemapRenderer     _tile_renderer;    // para tilemap da área atual em view
AnimationDriver     _anim;             // stateless, namespace — não precisa de member

// === Sistemas dungeon ===
MeleeCombatSystem   _combat_sys;
EnemyAISystem       _enemy_ai;
ProjectileSystem    _projectile_sys;
std::vector<Projectile>  _projectiles;
std::vector<GroundItem>  _ground_items;
LightingSystem      _lighting;
BossState           _boss_state;
SimpleParticleSystem _particles;
FloatingTextSystem  _floating_texts;
ScreenFlashState    _screen_flash;

// === Dados de configuração dungeon ===
EnemyDef            _enemy_defs[kEnemyTypeCount]{};
DropConfig          _drop_config{};
IniData             _rooms_ini{};

// === UI / overlay ===
PauseMenuController        _pause_controller;
AttributeLevelUpController _attr_controller;
SkillTreeController        _skill_tree_controller;
OverlayInputTracker        _overlay_input;

// === Estado UI / flags ===
bool    _shop_open = false;
bool    _interacting_npc_hint = false;
bool    _stress_mode = false;
int     _requested_enemy_count = 3;
float   _player_cast_timer = 0.f;
bool    _prev_upgrade_1 = false, _prev_upgrade_2 = false, _prev_upgrade_3 = false;
bool    _prev_talent_1  = false, _prev_talent_2  = false, _prev_talent_3  = false;
bool    _death_snapshot_done = false;
float   _death_fade_remaining = 0.f;
float   _autosave_flash = 0.f;
int     _hitstop = 0;
std::string _pending_next_scene;
std::string _post_mortem_dialogue_id;

// === Non-owning ===
SDL_Renderer*    _renderer   = nullptr;
AudioSystem*     _audio      = nullptr;
RunStats*        _run_stats  = nullptr;
DifficultyLevel* _difficulty = nullptr;
std::mt19937*    _rng        = nullptr;
std::optional<TextureCache> _tex_cache;
```

#### `enter()` — sequência concreta

```cpp
void enter() override {
    if (_renderer && !_tex_cache.has_value())
        _tex_cache.emplace(_renderer);

    // 1. Carregar configs
    DungeonConfigLoader::load_enemy_defs(_enemy_defs, _drop_config);
    DungeonConfigLoader::load_rooms_ini(_rooms_ini);
    TownConfigLoader::load_town_player_and_progression_config();

    // 2. Registrar diálogos
    _register_dungeon_dialogue(_dialogue);   // de dungeon_dialogue.hpp
    TownConfigLoader::load_town_dialogues(_dialogue);
    _dialogue.set_audio(_audio);

    // 3. Construir WorldMap
    _world_map = WorldMapBuilder::build(_rooms_ini, _tex_cache.has_value() ? &*_tex_cache : nullptr);

    // 4. Load save → posicionar player
    SaveData sd;
    if (SaveSystem::load_default(sd))
        WorldSaveController::apply_world_save(_make_context(), sd);
    else
        _fresh_run();

    // 5. Configurar player (textures, stats)
    configure_player(_player, _tex_cache.has_value() ? &*_tex_cache : nullptr);

    // 6. Construir lista de actors
    _actors.clear();
    _actors.push_back(&_player);
    NpcActorFactory::rebuild_npc_actors(_npcs, _player, _npc_actors, _actors);

    // 7. ZoneManager inicial
    _zone_mgr.update(_player.transform.x, _player.transform.y, _world_map);

    // 8. Áudio inicial baseado na zona
    if (_audio) {
        _audio->stop_all_ambient();
        if (_zone_mgr.in_town()) {
            _audio->play_ambient(AmbientId::TownWind, 0.18f);
            _audio->play_ambient(AmbientId::TownBirds, 0.11f);
            _audio->set_music_state(MusicState::Town);
        } else {
            DungeonAudioSystem::init_dungeon_audio(*_audio);
        }
    }

    // 9. Lighting + UI
    if (_renderer) _lighting.init(_renderer, viewport_w, viewport_h);
    _pause_controller.init();
    _skill_tree_controller.rebuild_columns();
    _camera.viewport_w = (float)viewport_w;
    _camera.viewport_h = (float)viewport_h;
}
```

#### `fixed_update()` — loop concreto com referências de código

```cpp
void fixed_update(float dt, const InputState& input) override {
    _screen_flash.tick(dt);  // de screen_fx.hpp
    _zone_mgr.update(_player.transform.x, _player.transform.y, _world_map);

    if (_zone_mgr.just_changed)
        _handle_zone_transition();  // fade áudio, toggle lighting/HUD

    // [UI guards — mesma lógica de DungeonScene::fixed_update linhas ~250-305]
    // AttributeLevelUpController, SkillTreeController, PauseMenuController
    // ... (copiar bloco de dungeon_scene.hpp:250-305, trocando _persist_save por
    //      WorldSaveController::persist(_make_context(), ...))

    const int hp0   = _player.health.current_hp;
    const int gold0 = _player.gold;

    // 1. Movimento
    _movement_sys.fixed_update(_actors, dt);

    // 2. Player input — idêntico a dungeon_scene.hpp:341
    _player_action.fixed_update(_player, input, dt, _audio, &_projectiles, &_actors);

    // 3. Resources + status
    _resource_sys.fixed_update(_actors, dt);
    _status_sys.fixed_update(_actors, dt);
    for (auto* a : _actors) if (a->is_alive) a->combat.advance_time(dt);

    // 4. IA + combate (só dungeon)
    if (_zone_mgr.in_dungeon()) {
        _enemy_ai.fixed_update(_actors, dt, _current_area_pathfinder(), &_projectiles);
        _boss_state.update(_enemies, _camera, _dialogue, _screen_flash, dt, _stress_mode);

        _projectile_sys.fixed_update(_projectiles, _actors, _current_area_room(), dt);
        CombatFxController::apply_projectile_hit_fx(
            _projectile_sys, _actors, _particles, _floating_texts,
            _screen_flash, _audio, _cam_aud_x(), _cam_aud_y(), _rng);

        _combat_sys.fixed_update(_actors, dt);
        CombatFxController::apply_combat_feedback(_combat_sys, _camera, _audio, _hitstop);
        CombatFxController::apply_melee_hit_fx(
            _combat_sys, _actors, _particles, _floating_texts,
            _screen_flash, _audio, _cam_aud_x(), _cam_aud_y(), _rng);

        auto dr = EnemyDeathController::process_deaths(
            _actors, _player, _enemy_defs, _drop_config, _ground_items,
            _run_stats, _quest_state, _particles, _audio,
            _zone_mgr.room_index_equiv(), _stress_mode, _rng);
        if (dr.boss_defeated)
            WorldSaveController::persist(_make_context(), _show_autosave_indicator, _autosave_flash);
    }

    // 5. NPCs (só town)
    if (_zone_mgr.in_town()) {
        TownNpcWanderSystem::update_town_npcs(_npcs, dt, _npc_rng_state);
        // interação NPC: handle_npc_interaction agora recebe WorldContext
        if (confirm_edge && near_npc >= 0 && _interacting_npc_hint)
            TownNpcInteractionController::handle_npc_interaction(
                near_npc, _make_context(), _shop_open, kQuestRewardGold);
    }

    // 6. Colisão broadphase — sempre
    {
        const AABB query = expand(_player_aabb(), 96.f);
        auto obs = _world_map.get_obstacles_near(query);
        _col_sys.resolve_all(_actors, obs, _world_map.total_bounds());
        _col_sys.resolve_actors(_actors, nullptr);
    }

    // 7. AreaEntrySystem — detecta entradas em área nova, spawna inimigos
    _area_entry.update(_zone_mgr, _make_context());

    // 8. Áudio + animações + câmera
    WorldAudioSystem::update(*_audio, _zone_mgr, _player, _enemies);
    AnimationDriver::update_player_dungeon_anim(_player, _player_cast_timer, dt);
    for (auto& e : _enemies) AnimationDriver::update_basic_actor_anim(e, dt);
    AnimationDriver::update_player_town_anim(_player, dt);  // só aplica se in_town
    _particles.update(dt);
    _floating_texts.tick(dt);
    _camera.follow(_player.transform.x, _player.transform.y, _world_map.total_bounds());
    if (_rng) _camera.step_shake(*_rng);
    if (_audio) _audio->tick_music();
    FootstepAudioSystem::update_footsteps(*_audio, _player, _enemies, _camera, ...);
}
```

> **`_current_area_pathfinder()`** — helper privado que retorna `_world_map.area_at(player_x, player_y)->nav_mesh` (ou pathfinder global se Opção B for adotada).
>
> **`_current_area_room()`** — retorna `_world_map.area_at(player_x, player_y)->room` para `ProjectileSystem` (que ainda recebe `RoomDefinition` para bounds).

#### `_make_context()` — WorldScene

```cpp
WorldContext _make_context() {
    WorldContext ctx;
    ctx.world_map    = &_world_map;
    ctx.zone_mgr     = &_zone_mgr;
    ctx.area_entry   = &_area_entry;
    ctx.player       = &_player;
    ctx.enemies      = &_enemies;
    ctx.actors       = &_actors;
    ctx.npcs         = &_npcs;
    ctx.npc_actors   = &_npc_actors;
    ctx.shop_forge   = &_shop_forge;
    ctx.projectiles  = &_projectiles;
    ctx.ground_items = &_ground_items;
    ctx.dialogue     = &_dialogue;
    ctx.camera       = &_camera;
    ctx.audio        = _audio;
    ctx.run_stats    = _run_stats;
    ctx.difficulty   = _difficulty;
    ctx.quest_state  = &_quest_state;
    ctx.scene_flags  = &_scene_flags;
    ctx.rooms_ini    = &_rooms_ini;
    ctx.enemy_defs   = _enemy_defs;
    ctx.drop_config  = &_drop_config;
    ctx.stress_mode  = &_stress_mode;
    ctx.requested_enemies = &_requested_enemy_count;
    ctx.tile_renderer = &_tile_renderer;
    ctx.resource      = &_resource_sys;
    ctx.player_action = &_player_action;
    return ctx;
}
```

### Implementação: `systems/town_npc_interaction.hpp`

Mudar a assinatura de `handle_npc_interaction`:

```cpp
// ANTES
inline void handle_npc_interaction(int idx, TownContext ctx, bool& shop_open, int quest_reward_gold);

// DEPOIS — WorldContext tem todos os campos que TownContext tinha
inline void handle_npc_interaction(int idx, WorldContext ctx, bool& shop_open, int quest_reward_gold);
```

O corpo não muda — todos os campos (`ctx.npcs`, `ctx.dialogue`, `ctx.player`, `ctx.quest_state`, `ctx.shop_forge`) existem no `WorldContext`.

### Implementação: `systems/area_entry_system.hpp` (novo)

```cpp
struct AreaEntrySystem {
    std::set<ZoneId> visited_areas;

    void update(const ZoneManager& zone, WorldContext& ctx) {
        if (!zone.just_changed) return;
        if (visited_areas.count(zone.current)) return;

        // Spawnar inimigos para a nova área dungeon
        if (zone.current != ZoneId::Town && zone.current != ZoneId::Transition) {
            const int budget = EnemySpawner::spawn_budget(
                zone.room_index_equiv(), *ctx.stress_mode,
                *ctx.requested_enemies, ctx.difficulty);
            // Obter room e nav mesh da WorldArea
            const WorldArea* area = ctx.world_map->area_at(
                ctx.player->transform.x, ctx.player->transform.y);
            if (area) {
                EnemySpawner::spawn_for_budget(
                    *ctx.enemies, area->room, area->tilemap, area->nav_mesh,
                    *ctx.player, ctx.enemy_defs,
                    ctx.tex_cache, zone.room_index_equiv(), budget,
                    *ctx.stress_mode, ctx.difficulty);
                // Rebuild actor pointers após spawn
                ctx.actors->clear();
                ctx.actors->push_back(ctx.player);
                for (auto& e : *ctx.enemies) ctx.actors->push_back(&e);
            }
        }

        visited_areas.insert(zone.current);
    }

    uint8_t to_mask() const {
        uint8_t m = 0;
        if (visited_areas.count(ZoneId::DungeonRoom0)) m |= 0x01;
        if (visited_areas.count(ZoneId::DungeonRoom1)) m |= 0x02;
        if (visited_areas.count(ZoneId::DungeonRoom2)) m |= 0x04;
        if (visited_areas.count(ZoneId::Boss))         m |= 0x08;
        return m;
    }

    void from_mask(uint8_t mask) {
        visited_areas.clear();
        if (mask & 0x01) visited_areas.insert(ZoneId::DungeonRoom0);
        if (mask & 0x02) visited_areas.insert(ZoneId::DungeonRoom1);
        if (mask & 0x04) visited_areas.insert(ZoneId::DungeonRoom2);
        if (mask & 0x08) visited_areas.insert(ZoneId::Boss);
    }
};
```

### Implementação: `systems/world_audio_system.hpp` (novo)

Baseado em `DungeonAudioSystem::update_dungeon_music` + lógica de town:

```cpp
namespace WorldAudioSystem {

inline void init_for_zone(AudioSystem& audio, ZoneId zone) {
    audio.stop_all_ambient();
    if (zone == ZoneId::Town || zone == ZoneId::Transition) {
        audio.play_ambient(AmbientId::TownWind,  0.18f);
        audio.play_ambient(AmbientId::TownBirds, 0.11f);
        if (zone == ZoneId::Transition)
            audio.play_ambient(AmbientId::TransitionWind, 0.10f); // novo
        audio.set_music_state(MusicState::Town);
    } else {
        audio.play_ambient(AmbientId::DungeonDrips, 0.22f);
        audio.play_ambient(AmbientId::DungeonTorch, 0.14f);
        audio.set_music_state(MusicState::DungeonCalm);
    }
}

inline void update(AudioSystem& audio, const ZoneManager& zone,
                   const Actor& player, const std::vector<Actor>& enemies) {
    if (!zone.in_dungeon()) return;

    MusicState target = MusicState::DungeonCalm;

    if (zone.current == ZoneId::Boss) {
        for (const auto& e : enemies)
            if (e.is_alive && e.name == "Grimjaw") { target = MusicState::DungeonBoss; break; }
    } else {
        for (const auto& e : enemies) {
            if (!e.is_alive) continue;
            const float dx = player.transform.x - e.transform.x;
            const float dy = player.transform.y - e.transform.y;
            if (dx*dx + dy*dy < e.aggro_range * e.aggro_range)
                { target = MusicState::DungeonCombat; break; }
        }
    }

    if (target != audio.current_music_state())
        audio.set_music_state(target);
}

} // namespace WorldAudioSystem
```

`_handle_zone_transition()` no `WorldScene` chama `WorldAudioSystem::init_for_zone(*_audio, _zone_mgr.current)`.

### Entregável
Jogo roda completamente em cena única. TownScene e DungeonScene não são mais usadas.

---

## Fase 5 — Limpeza

### Arquivos a remover

| Arquivo | Motivo |
|---|---|
| `src/scenes/dungeon_scene.hpp` | substituído por `WorldScene` |
| `src/scenes/town_scene.hpp` | substituído por `WorldScene` |
| `src/systems/dungeon_save_controller.hpp` | substituído por `WorldSaveController` |
| `src/systems/town_save_controller.hpp` | substituído por `WorldSaveController` |

### `src/systems/dungeon_flow_controller.hpp` — partes a remover

- `apply_save_and_build_world(...)` — substituído por `WorldSaveController::apply_world_save` + `WorldMapBuilder::build`
- `start_full_run(...)` — substituído por `WorldScene::_fresh_run()`
- `advance_room(...)` — substituído por `AreaEntrySystem::update`
- `update_room_flow(...)` — substituído por `AreaEntrySystem::update` + `WorldAudioSystem::update`

O que **pode ficar** no arquivo ou ser movido para utilitário: nenhuma função — o arquivo inteiro fica obsoleto.

### `src/core/scene_context.hpp` — o que muda

- `DungeonContext` e `TownContext` podem ser removidos após Fase 5 (nada mais as usa)
- `SceneCreateContext` não muda estruturalmente — continua sendo passado para `WorldScene::set_*` via `SceneRegistry`

### `src/core/scene_registry` — mudança concreta

Remover registro de `"dungeon"` e `"town"`, registrar `"world"`:
```cpp
// ANTES
registry.add("dungeon", [](...) -> std::unique_ptr<IScene> { return std::make_unique<DungeonScene>(); });
registry.add("town",    [](...) -> std::unique_ptr<IScene> { return std::make_unique<TownScene>(); });

// DEPOIS
registry.add("world",   [](...) -> std::unique_ptr<IScene> { return std::make_unique<WorldScene>(); });
```

`TitleScene::next_scene()` retorna `"world"` em vez de `"town"` ou `"dungeon"`.

### Testes a atualizar

| Arquivo de teste | O que muda |
|---|---|
| `tests/systems/test_dungeon_flow_and_animation.cpp` | instancia `DungeonScene` → instanciar `WorldScene` ou testar `AreaEntrySystem` diretamente |
| `tests/systems/test_town_scene_modules.cpp` | idem para `TownScene` → `WorldScene` |
| `tests/core/test_dungeon_config_loader.cpp` | sem mudança — testa loader independente |
| `tests/core/test_town_config_loader.cpp` | sem mudança |
| `tests/core/test_dungeon_dialogue_registry.cpp` | sem mudança |

---

## Assinaturas que realmente mudam

O plano afirma que "controllers continuam idênticos" — os **corpos** sim, mas alguns têm o tipo do contexto na assinatura:

| Arquivo | Mudança |
|---|---|
| `TownNpcInteractionController::handle_npc_interaction` | `TownContext ctx` → `WorldContext ctx` |
| `DungeonSaveController` | removido; substituído por `WorldSaveController` |
| `TownSaveController` | removido; substituído por `WorldSaveController` |

Funções que recebem `int room_index` diretamente (`EnemyDeathController::process_deaths`, `EnemySpawner::spawn_budget`, `DungeonHud::render_dungeon_hud`) **não mudam** — o `WorldScene` passa `zone_mgr.room_index_equiv()`.

---

## Ponto de atenção — Pathfinder por área

`DungeonFlowController` chama `pathfinder.build_nav(tilemap, room)` após cada `build_room`. No open world as áreas são construídas uma vez no `enter()`. Opções:

- **Opção A** (simples): cada `WorldArea` carrega seu `NavMesh` no build; `EnemyAISystem` recebe `area.nav_mesh` baseado na zona atual.
- **Opção B** (mais trabalho): `Pathfinder` global reconstruído quando `ZoneManager::just_changed` para a zona de dungeon ativa.

Opção A evita acoplamento e é consistente com "área como dado auto-contido".

---

## Ponto de atenção — Inimigos visíveis entre salas

No open world, o player pode ver inimigos de salas adjacentes simultaneamente.

**Solução: activation radius no `EnemyAISystem`**

```cpp
// Só processa AI de inimigos dentro do raio de ativação.
// Inimigos fora do raio ficam parados (idle), mas são renderizados.
static constexpr float kAIActivationRadius = 640.f; // ~2 salas de distância

for (auto& e : enemies) {
    if (dist(e, player) > kAIActivationRadius) continue;
    _enemy_ai.update_single(e, ...);
}
```

Inimigos fora do raio: renderizados, animação idle, sem pathfinding. Custo praticamente zero.

---

## Risco principal — Layout das áreas

A disposição física das áreas no mundo (qual offset cada uma tem) precisa ser cuidadosa:

- Áreas não podem se **sobrepor** (causaria colisão duplicada e render incorreto)
- Deve haver passagens físicas entre town e dungeon (corredor, entrada de caverna)
- A dungeon pode ser disposta horizontalmente (sala 0 → 1 → 2 → boss) ou em L/U

Sugestão de layout inicial:

```
[Town  640x480] ──── corredor 128px ──── [Sala0 640x480] ── [Sala1] ── [Sala2] ── [Boss]
```

O corredor entre town e dungeon é ele mesmo uma `WorldArea` com `ZoneId::Transition` — sem inimigos, sem NPCs, iluminação gradual.

---

## Assets: impacto na pipeline de placeholders

O projeto tem quatro scripts de geração em `tools/` que produzem assets temporários (WAV e PNG) até o áudio/arte final chegar. O open world adiciona novos caminhos de arquivo que precisam ser registrados nesses scripts e na documentação existente.

### Novos assets necessários

#### Áudio — `tools/gen_prototype_8bit_sfx.py`

O dicionário `specs` do script precisa de uma entrada para cada WAV novo:

| Arquivo | Uso | Gerador sugerido |
|---|---|---|
| `ambient_transition_wind.wav` | Ambiente do corredor town↔dungeon | `render_noise` (vento suave) |

> Não há necessidade de novo `MusicState` — a transição usa crossfade entre `Town` e `DungeonCalm` existentes, gerenciado pelo `WorldAudioSystem`.

Checklist obrigatório (de `assets/audio/README.md`):
1. Novo `AmbientId` em `audio.hpp` antes de `COUNT`
2. Caminho em `_ambient_paths[]` em `audio.cpp` na mesma ordem
3. Caminho em `kPaths[]` em `asset_manifest.hpp`
4. Entrada em `specs` no `gen_prototype_8bit_sfx.py`
5. Chamada a `play_ambient(...)` no `WorldAudioSystem` para a zona `Transition`
6. Atualizar tabela de ambient em `assets/audio/README.md`

#### Textures — `tools/gen_environment_sprites.py`

O script atualmente gera tiles de town e dungeon separados. Para o corredor de transição (`ZoneId::Transition`) é preciso:

| Arquivo | Dimensão | Descrição |
|---|---|---|
| `assets/tiles/corridor_floor.png` | 64×32 (2 tiles 32×32) | Piso de pedra + terra — transição gradual entre town e dungeon |

Adicionar ao bloco de geração em `gen_environment_sprites.py`. O gradiente visual (pedra → terra → grama) pode ser feito com a mesma palette `dungeon_stone` fundindo para `dungeon_prop`.

#### Texture contract

Após adicionar `corridor_floor.png` ao código do `WorldArea` de Transition, rodar:
```bash
python3 tools/asset_pipeline/main.py  # atualiza texture_contract_inventory.md
```
O relatório em `tools/texture_contract_inventory.md` deve refletir o novo caminho no backlog até o código referenciar diretamente.

### O que **não** muda na pipeline

Todos os outros assets existentes continuam válidos e servidos pelos mesmos scripts:

| Script | Cobre |
|---|---|
| `gen_animated_sprites.py` | Player, enemies, NPCs — layout Puny Characters inalterado |
| `gen_placeholder_textures.py` | Props, UI, FX — manifesto inalterado |
| `gen_environment_sprites.py` | `town_floor.png`, `dungeon_tileset.png`, NPC sprites, portas — só adicionar corredor |
| `gen_prototype_8bit_sfx.py` | Todos os SFX/música/ambient existentes — só adicionar `ambient_transition_wind.wav` |

---

## Ordem de execução

```
Fase 1  WorldMap + Broadphase     ← base de tudo, sem isso nada funciona
Fase 2  ZoneManager               ← sistemas gateados, áudio e lighting
Fase 3  WorldContext + WorldSave  ← save/load funcional
Fase 4  WorldScene                ← cena unificada, tudo junto
Fase 5  Limpeza                   ← remove código morto
```

Cada fase entrega algo jogável e buildável. Não existe estado intermediário que quebra o jogo — as cenas antigas continuam funcionando até a Fase 4 ser concluída.

---

## Correções de implementação (verificação final)

Quatro discrepâncias encontradas ao comparar o plano com o código existente:

### 1. `DungeonConfigLoader` — função real tem assinatura diferente

O plano chama `load_enemy_defs` e `load_rooms_ini` — essas funções **não existem**.
A função real é ([dungeon_config_loader.hpp:19](src/systems/dungeon_config_loader.hpp#L19)):

```cpp
DungeonConfigLoader::load_dungeon_static_data(
    _enemy_defs, _drop_config, _rooms_ini, _enemy_sprite_paths);
```

O `WorldScene` precisa do member adicional ausente do plano:
```cpp
std::array<std::string, kEnemyTypeCount> _enemy_sprite_paths{};
```

O `enter()` deve chamar `load_dungeon_static_data` com os quatro parâmetros.

### 2. `EnemySpawner::spawn_for_budget` chama `enemies.clear()` — destrói inimigos de outras áreas

([enemy_spawner.hpp:47](src/systems/enemy_spawner.hpp#L47)) O spawner limpa o vetor inteiro antes de spawnar. No open world, `_enemies` contém inimigos de **todas as áreas** — chamar `spawn_for_budget` para a Sala 1 apagaria inimigos da Sala 0 ainda vivos.

**Solução: cada `WorldArea` tem sua própria lista de inimigos.**

```cpp
struct WorldArea {
    ZoneId         zone;
    float          offset_x = 0.f;
    float          offset_y = 0.f;
    RoomDefinition room;
    Tilemap        tilemap;
    Pathfinder     pathfinder;             // nav mesh desta área
    std::vector<Actor> enemies;            // NOVO: inimigos próprios desta área
};
```

`AreaEntrySystem::update` chama `EnemySpawner::spawn_for_budget(area->enemies, ...)` em vez de `ctx.enemies`. O `WorldScene` acumula os ponteiros de todos inimigos vivos de todas as áreas em `_actors` a cada frame (iterar `_world_map.areas`, push_back).

### 3. `WorldArea::nav_mesh` vs `Pathfinder`

O plano usa o nome `NavMesh` mas `EnemySpawner::spawn_for_budget` recebe `const Pathfinder& pathfinder` ([enemy_spawner.hpp:39](src/systems/enemy_spawner.hpp#L39)) e `EnemyAISystem::fixed_update` também. O tipo correto em `WorldArea` é `Pathfinder` (não `NavMesh`):

```cpp
// WorldArea — corrigido
Pathfinder pathfinder;  // construído via pathfinder.build_nav(tilemap, room) no enter()
```

Trocar todas as referências a `area->nav_mesh` por `area->pathfinder` no plano.

### 4. O arquivo real é `register_scenes.cpp`, não `scene_registry`

O plano menciona "Atualizar `src/core/scene_registry`" mas o arquivo com as factories é [src/core/register_scenes.cpp](src/core/register_scenes.cpp). As mudanças concretas:

```cpp
// register_scenes.cpp — remover:
registry.register_scene("town",    ...);
registry.register_scene("dungeon", ...);

// Adicionar:
registry.register_scene("world", [](const SceneCreateContext& ctx) {
    auto s = std::make_unique<WorldScene>();
    s->viewport_w = ctx.viewport_w;
    s->viewport_h = ctx.viewport_h;
    s->set_renderer(ctx.renderer);
    s->set_audio(ctx.audio);
    s->set_show_autosave_indicator(ctx.show_autosave_indicator);
    s->set_run_stats(ctx.run_stats);
    s->set_difficulty(ctx.difficulty);
    s->set_rng(ctx.rng);
    if (ctx.stress_enemy_count > 3)     s->enable_stress_mode(ctx.stress_enemy_count);
    else if (ctx.stress_enemy_count > 0) s->set_enemy_spawn_count(ctx.stress_enemy_count);
    return s;
});
```

Também atualizar o include no topo: remover `dungeon_scene.hpp` e `town_scene.hpp`, adicionar `world_scene.hpp`. Declaração de `register_default_scenes` em [register_scenes.hpp](src/core/register_scenes.hpp) não muda.

---

## Arquivos criados / modificados por fase

| Fase | Novos | Modificados | Removidos |
|---|---|---|---|
| 1 | `world/world_area.hpp`, `world/world_map.hpp`, `world/world_map_builder.hpp` | `room_collision.hpp` (overload broadphase), `town_builder.hpp` (offset params), `room_manager.hpp` (offset params) | — |
| 2 | `world/zone_manager.hpp` (inclui `room_index_equiv()`) | — | — |
| 3 | `world/world_context.hpp`, `systems/world_save_controller.hpp` | `core/save_data.hpp` (v6: `player_world_x/y`, `visited_area_mask`), `core/save_migration.hpp` (`migrate_v5_to_v6`), `core/save_system.hpp` (ler/escrever campos v6) | `dungeon_save_controller.hpp`, `town_save_controller.hpp` |
| 4 | `scenes/world_scene.hpp`, `systems/area_entry_system.hpp`, `systems/world_audio_system.hpp` | `core/register_scenes.cpp` (registrar `"world"`, remover `"town"`/`"dungeon"`), `core/scene_context.hpp` (`DungeonContext`/`TownContext` obsoletos), `systems/town_npc_interaction.hpp` (`TownContext`→`WorldContext`) | — |
| 5 | — | testes | `scenes/dungeon_scene.hpp`, `scenes/town_scene.hpp`, `systems/dungeon_flow_controller.hpp` (inteiro), `core/register_scenes.cpp` (includes) |
