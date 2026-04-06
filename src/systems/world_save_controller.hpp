#pragma once

#include <algorithm>

#include "../core/debug_log.hpp"
#include "../core/save_system.hpp"
#include "../world/world_context.hpp"
#include "area_entry_system.hpp"

namespace mion {

namespace WorldSaveController {

inline SaveData make_world_save(const WorldContext& ctx) {
    SaveData d;
    d.version     = kSaveFormatVersion;
    d.room_index  = 0; // legacy: always 0 in v6
    if (ctx.player) {
        d.player_hp            = ctx.player->health.current_hp;
        d.gold                 = ctx.player->gold;
        d.progression          = ctx.player->progression;
        d.talents              = ctx.player->talents;
        d.mana                 = ctx.player->mana;
        d.stamina              = ctx.player->stamina;
        d.attributes           = ctx.player->attributes;
        d.attr_points_available = ctx.player->progression.pending_level_ups;
        d.player_world_x       = ctx.player->transform.x;
        d.player_world_y       = ctx.player->transform.y;
    }
    if (ctx.quest_state)
        d.quest_state = *ctx.quest_state;
    if (ctx.difficulty)
        d.difficulty = static_cast<int>(*ctx.difficulty);
    if (ctx.run_stats)
        d.last_run_stats = *ctx.run_stats;
    if (ctx.scene_flags)
        d.scene_flags = *ctx.scene_flags;
    if (ctx.area_entry)
        d.visited_area_mask = ctx.area_entry->to_mask();
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
    if (ctx.quest_state)  *ctx.quest_state = sd.quest_state;
    if (ctx.scene_flags)  *ctx.scene_flags = sd.scene_flags;
    if (ctx.difficulty)
        *ctx.difficulty = static_cast<DifficultyLevel>(std::clamp(sd.difficulty, 0, 2));
    if (ctx.area_entry)   ctx.area_entry->from_mask(sd.visited_area_mask);
    if (ctx.projectiles)  ctx.projectiles->clear();
    if (ctx.ground_items) ctx.ground_items->clear();
}

inline void persist(const WorldContext& ctx, bool show_indicator, float& flash_timer) {
    if (ctx.stress_mode && *ctx.stress_mode)
        return;

    SaveData d = make_world_save(ctx);
    SaveData disk{};
    if (SaveSystem::load_default(disk))
        d.victory_reached = disk.victory_reached;
    if (ctx.difficulty)
        d.difficulty = static_cast<int>(*ctx.difficulty);

    debug_log_progression(d, "world_persist");
    SaveSystem::save_default(d);
    SaveSystem::save_debug(d, "world_autosave");
    if (show_indicator)
        flash_timer = 1.35f;
}

inline void snapshot_last_run(const WorldContext& ctx) {
    if (!ctx.run_stats || (ctx.stress_mode && *ctx.stress_mode))
        return;
    SaveData d{};
    if (!SaveSystem::load_default(d))
        return;
    d.last_run_stats = *ctx.run_stats;
    SaveSystem::save_default(d);
}

} // namespace WorldSaveController

} // namespace mion
