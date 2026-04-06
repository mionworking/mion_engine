#pragma once

#include <vector>

#include "../entities/actor.hpp"
#include "../entities/npc.hpp"

namespace mion {

namespace NpcActorFactory {

inline void rebuild_npc_actors(std::vector<NpcEntity>& npcs,
                               Actor& player,
                               std::vector<Actor>& npc_actors,
                               std::vector<Actor*>& actors) {
    npc_actors.clear();
    npc_actors.reserve(npcs.size());

    for (auto& npc : npcs) {
        Actor actor;
        actor.name               = npc.name;
        actor.team               = Team::Enemy; // neutro na town; sem combate real.
        actor.transform.set_position(npc.x, npc.y);
        actor.collision.half_w   = npc.collision_half;
        actor.collision.half_h   = npc.collision_half;
        actor.is_alive           = true;
        actor.was_alive          = true;
        actor.move_speed         = npc.wander_speed;
        actor.sprite_sheet       = nullptr;
        actor.sprite_scale       = player.sprite_scale > 0.0f ? player.sprite_scale : 2.0f;

        npc_actors.push_back(actor);
        npc.actor = &npc_actors.back();
        actors.push_back(npc.actor);
    }
}

} // namespace NpcActorFactory

} // namespace mion
