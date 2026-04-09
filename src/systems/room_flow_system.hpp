#pragma once
#include <string>
#include <vector>
#include "../entities/actor.hpp"
#include "../world/room.hpp"

namespace mion {

struct RoomFlowSystem {
    bool        room_cleared         = false;
    bool        transition_requested = false;
    std::string scene_exit_to; // não vazio = pedido de SceneRegistry / next_scene()

    void fixed_update(const std::vector<Actor*>& actors,
                      const Actor& player,
                      const RoomDefinition& room) {
        room_cleared = true;
        for (const auto* a : actors) {
            if (a->team == Team::Enemy && a->is_alive) {
                room_cleared = false;
                break;
            }
        }

        transition_requested = false;
        scene_exit_to.clear();
        if (!player.is_alive) return;

        AABB pb = player.collision.bounds_at(player.transform.x, player.transform.y);
        for (const auto& door : room.doors) {
            if (!pb.intersects(door.bounds)) continue;
            if (door.requires_room_cleared && !room_cleared) continue;
            if (!door.exit_to_zone.empty()) {
                scene_exit_to = door.exit_to_zone;
                return;
            }
            if (!door.exit_to_scene.empty()) {
                scene_exit_to = door.exit_to_scene;
                return;
            }
            transition_requested = true;
            return;
        }
    }
};

} // namespace mion
