#pragma once

#include <vector>

#include <SDL3/SDL.h>

#include "../core/audio.hpp"
#include "../core/camera.hpp"
#include "../core/ini_loader.hpp"
#include "../core/quest_state.hpp"
#include "../core/run_stats.hpp"
#include "../core/sprite.hpp"
#include "../entities/actor.hpp"
#include "../entities/enemy_type.hpp"
#include "../entities/ground_item.hpp"
#include "../entities/npc.hpp"
#include "../entities/projectile.hpp"
#include "../systems/boss_state_controller.hpp"
#include "../systems/dialogue_system.hpp"
#include "../systems/drop_system.hpp"
#include "../systems/player_action.hpp"
#include "../systems/resource_system.hpp"
#include "../systems/tilemap_renderer.hpp"
#include "../entities/shop.hpp"
#include "world_map.hpp"
#include "zone_manager.hpp"

namespace mion {

struct AreaEntrySystem;

struct WorldContext {
    WorldMap*              world_map        = nullptr;
    ZoneManager*           zone_mgr         = nullptr;
    AreaEntrySystem*       area_entry       = nullptr;
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

    IniData*        rooms_ini         = nullptr;
    EnemyDef*       enemy_defs        = nullptr;
    DropConfig*     drop_config       = nullptr;
    bool*           stress_mode       = nullptr;
    int*            requested_enemies = nullptr;

    TilemapRenderer*    tile_renderer = nullptr;
    ResourceSystem*     resource      = nullptr;
    PlayerActionSystem* player_action = nullptr;
    TextureCache*       tex_cache     = nullptr;
    BossState*          boss_state    = nullptr;
};

} // namespace mion
