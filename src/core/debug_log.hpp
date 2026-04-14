 #pragma once

#include <stdarg.h>
#include <stdio.h>

#include "save_data.hpp"
#include "../entities/actor.hpp"

namespace mion {

// Lightweight debug logging for development.
// Active in debug builds (NDEBUG not defined) or when MION_ENABLE_DEBUG_LOG is set.
#if !defined(NDEBUG) || defined(MION_ENABLE_DEBUG_LOG)

inline void debug_log(const char* fmt, ...) {
    FILE* fp = fopen("mion_debug.log", "a");
    if (!fp) return;
    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    fputc('\n', fp);
    fflush(fp);
    va_end(args);
    fclose(fp);
}

 inline void debug_log_progression(const SaveData& d, const char* tag = "progression") {
     debug_log(
         "[%s] room=%d level=%d xp=%d xp_to_next=%d pending_level_ups=%d "
         "gold=%d attr_pts=%d hp=%d",
         tag,
         d.room_index,
         d.progression.level,
         d.progression.xp,
         d.progression.xp_to_next,
         d.progression.pending_level_ups,
         d.gold,
         d.attr_points_available,
         d.player_hp);
 }

 inline void debug_log_player_state(const Actor& player, const char* tag = "player") {
     debug_log(
         "[%s] pos=(%.1f,%.1f) hp=%d/%d level=%d xp=%d xp_to_next=%d pending_level_ups=%d "
         "vigor=%d forca=%d des=%d int=%d end=%d",
         tag,
         player.transform.x,
         player.transform.y,
         player.health.current_hp,
         player.health.max_hp,
         player.player->progression.level,
         player.player->progression.xp,
         player.player->progression.xp_to_next,
         player.player->progression.pending_level_ups,
         player.player->attributes.vigor,
         player.player->attributes.forca,
         player.player->attributes.destreza,
         player.player->attributes.inteligencia,
         player.player->attributes.endurance);
 }

 #else

 inline void debug_log(const char* /*fmt*/, ...) {}
 inline void debug_log_progression(const SaveData& /*d*/, const char* /*tag*/ = "progression") {}
 inline void debug_log_player_state(const Actor& /*player*/, const char* /*tag*/ = "player") {}

 #endif // !NDEBUG || MION_ENABLE_DEBUG_LOG

 } // namespace mion

