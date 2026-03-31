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

    // --- Campos de posição base e wander ---
    float spawn_x         = 0.0f;  // posição original (centro do wander)
    float spawn_y         = 0.0f;
    float wander_radius   = 80.0f; // raio máximo de afastamento do spawn
    float wander_speed    = 35.0f; // pixels por segundo
    float wander_timer    = 0.0f;  // tempo até próximo reposicionamento
    float wander_dir_x    = 0.0f;  // direção atual de movimento
    float wander_dir_y    = 0.0f;
    float collision_half  = 14.0f; // meia-largura/altura do AABB NPC

    // Actor associado na cena (Town) para integrar com MovementSystem/RoomCollisionSystem.
    Actor*      actor           = nullptr;
};

} // namespace mion
