#pragma once
#include <string>
#include <SDL3/SDL.h>

namespace mion {

struct Actor;

enum class NpcType { QuestGiver, Merchant, Generic };

struct NpcEntity {
    float       x = 0.0f;
    float       y = 0.0f;
    std::string name;
    NpcType     type            = NpcType::Generic;
    float       interact_radius = 48.0f;
    SDL_Color   portrait_color  = {180, 200, 160, 255};

    std::string dialogue_default;
    std::string dialogue_quest_active;
    std::string dialogue_quest_done;

    // --- Base position and wander fields ---
    float spawn_x         = 0.0f;  // original position (wander center)
    float spawn_y         = 0.0f;
    float wander_radius   = 80.0f; // maximum wander distance from spawn
    float wander_speed    = 35.0f; // pixels per second
    float wander_timer    = 0.0f;  // time until next repositioning
    float wander_dir_x    = 0.0f;  // current movement direction
    float wander_dir_y    = 0.0f;
    float collision_half  = 14.0f; // NPC AABB half-extents

    // Linked Actor in the Town scene, used by MovementSystem and RoomCollisionSystem.
    Actor*      actor           = nullptr;
};

} // namespace mion
