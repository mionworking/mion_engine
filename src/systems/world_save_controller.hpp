#pragma once

#include <algorithm>

#include "../core/debug_log.hpp"
#include "../core/save_system.hpp"
#include "../world/world_context.hpp"
#include "area_entry_system.hpp"

namespace mion {

namespace WorldSaveController {

namespace detail {

inline constexpr int kMinDifficultyIndex = 0;
inline constexpr int kMaxDifficultyIndex = 2;
inline constexpr float kAutosaveFlashDurationSec = 1.35f;
inline constexpr const char* kAutosaveDebugTag = "world_autosave";

inline DifficultyLevel clamp_difficulty_level(int raw_difficulty) {
    return static_cast<DifficultyLevel>(
        std::clamp(raw_difficulty, kMinDifficultyIndex, kMaxDifficultyIndex)
    );
}

inline bool persistence_disabled_by_stress_mode(const WorldContext& ctx) {
    return ctx.stress_mode && *ctx.stress_mode;
}

inline bool default_save_exists() {
    return SaveSystem::exists_default();
}

inline bool load_default_save_data(SaveData& out_save) {
    return SaveSystem::load_default(out_save);
}

inline bool save_default_save_data(const SaveData& save_data) {
    return SaveSystem::save_default(save_data);
}

inline bool remove_default_save_data() {
    return SaveSystem::remove_default_saves();
}

inline void save_autosave_debug_snapshot(const SaveData& save_data) {
    SaveSystem::save_debug(save_data, kAutosaveDebugTag);
}

inline void merge_victory_flag_from_existing_save(SaveData& in_out_save) {
    SaveData disk{};
    if (detail::load_default_save_data(disk))
        in_out_save.victory_reached = disk.victory_reached;
}

template <typename Mutator>
inline bool mutate_default_save(Mutator&& mutator, bool missing_is_noop, bool seed_empty_on_missing) {
    SaveData save_data{};
    if (!load_default_save_data(save_data)) {
        if (missing_is_noop)
            return true;
        if (!seed_empty_on_missing)
            return false;
        save_data = SaveData{};
    }
    mutator(save_data);
    return save_default_save_data(save_data);
}

} // namespace detail

// NOTE ON RETURN SEMANTICS:
// Methods that return bool follow "operation succeeded" semantics:
// - true: write succeeded OR operation was a legitimate no-op.
// - false: a write/update was attempted and failed.

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
        ctx.player->gold              = sd.gold;
        ctx.player->health.current_hp = sd.player_hp;
        ctx.player->mana              = sd.mana;
        ctx.player->stamina           = sd.stamina;
        ctx.player->transform.x       = sd.player_world_x;
        ctx.player->transform.y = sd.player_world_y;
    }
    if (ctx.quest_state)  *ctx.quest_state = sd.quest_state;
    if (ctx.scene_flags)  *ctx.scene_flags = sd.scene_flags;
    if (ctx.difficulty)
        *ctx.difficulty = detail::clamp_difficulty_level(sd.difficulty);
    if (ctx.area_entry)   ctx.area_entry->from_mask(sd.visited_area_mask);
    if (ctx.projectiles)  ctx.projectiles->clear();
    if (ctx.ground_items) ctx.ground_items->clear();
}

inline bool persist(const WorldContext& ctx, bool show_indicator, float& flash_timer) {
    // Stress mode disables persistence by design; treat as successful no-op.
    if (detail::persistence_disabled_by_stress_mode(ctx))
        return true;

    SaveData d = make_world_save(ctx);
    detail::merge_victory_flag_from_existing_save(d);

    debug_log_progression(d, "world_persist");
    const bool saved = detail::save_default_save_data(d);
    detail::save_autosave_debug_snapshot(d);
    if (show_indicator && saved)
        flash_timer = detail::kAutosaveFlashDurationSec;
    return saved;
}

inline bool snapshot_last_run(const WorldContext& ctx) {
    // No stats or stress mode means there is nothing to snapshot.
    if (!ctx.run_stats || detail::persistence_disabled_by_stress_mode(ctx))
        return true;
    // No baseline save to merge into is also a no-op.
    return detail::mutate_default_save(
        [&](SaveData& d) { d.last_run_stats = *ctx.run_stats; },
        true,
        false
    );
}

inline bool mark_victory_reached(const RunStats& stats) {
    return detail::mutate_default_save(
        [&](SaveData& d) {
            d.victory_reached = true;
            d.last_run_stats  = stats;
        },
        false,
        true
    );
}

inline bool clear_default_saves() {
    // No save present to remove is a successful no-op.
    if (!detail::default_save_exists())
        return true;
    return detail::remove_default_save_data();
}

inline bool load_victory_unlock_flag(bool& out_victory_unlock) {
    SaveData d{};
    if (!detail::load_default_save_data(d)) {
        out_victory_unlock = false;
        return false;
    }
    out_victory_unlock = d.victory_reached;
    return true;
}

inline bool has_default_save() {
    return detail::default_save_exists();
}

inline bool persist_difficulty_if_save_exists(int difficulty_index) {
    // No save present to update is a successful no-op.
    return detail::mutate_default_save(
        [&](SaveData& save_data) { save_data.difficulty = difficulty_index; },
        true,
        false
    );
}

inline bool load_saved_difficulty(DifficultyLevel& out_difficulty) {
    SaveData boot{};
    if (!detail::load_default_save_data(boot))
        return false;
    out_difficulty = detail::clamp_difficulty_level(boot.difficulty);
    return true;
}

inline bool load_default_save(SaveData& out_save) {
    return detail::load_default_save_data(out_save);
}

} // namespace WorldSaveController

} // namespace mion
