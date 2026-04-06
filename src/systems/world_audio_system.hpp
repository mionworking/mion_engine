#pragma once

#include "../core/audio.hpp"
#include "../world/world_area.hpp"
#include "../world/zone_manager.hpp"
#include "../entities/actor.hpp"

namespace mion {

namespace WorldAudioSystem {

inline void init_for_zone(AudioSystem& audio, ZoneId zone) {
    audio.stop_all_ambient();
    if (zone == ZoneId::Town) {
        audio.play_ambient(AmbientId::TownWind,  0.18f);
        audio.play_ambient(AmbientId::TownBirds, 0.11f);
        audio.set_music_state(MusicState::Town);
    } else if (zone == ZoneId::Transition) {
        audio.play_ambient(AmbientId::TransitionWind, 0.24f);
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
            if (dx * dx + dy * dy < e.aggro_range * e.aggro_range)
                { target = MusicState::DungeonCombat; break; }
        }
    }

    if (target != audio.current_music_state())
        audio.set_music_state(target);
}

} // namespace WorldAudioSystem

} // namespace mion
