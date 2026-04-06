#pragma once

#include <string>
#include <vector>

#include "../core/save_system.hpp"
#include "../entities/npc.hpp"
#include "../world/world_context.hpp"
#include "world_save_controller.hpp"

namespace mion {

namespace TownNpcInteractionController {

inline float town_dist_sq(float ax, float ay, float bx, float by) {
    const float dx = ax - bx;
    const float dy = ay - by;
    return dx * dx + dy * dy;
}

inline int find_nearest_npc(const Actor& player, const std::vector<NpcEntity>& npcs) {
    int   best      = -1;
    float best_dist = 1.0e12f;
    for (int i = 0; i < static_cast<int>(npcs.size()); ++i) {
        const NpcEntity& npc = npcs[static_cast<size_t>(i)];
        const float      dist = town_dist_sq(player.transform.x, player.transform.y, npc.x, npc.y);
        if (dist < best_dist) {
            best_dist = dist;
            best      = i;
        }
    }
    return best;
}

inline void handle_npc_interaction(int idx,
                                   WorldContext ctx,
                                   bool& shop_open,
                                   int quest_reward_gold) {
    if (!ctx.npcs || !ctx.dialogue || !ctx.player || !ctx.quest_state)
        return;
    if (idx < 0 || idx >= static_cast<int>(ctx.npcs->size()))
        return;

    NpcEntity& npc = (*ctx.npcs)[static_cast<size_t>(idx)];
    if (npc.type == NpcType::Merchant) {
        shop_open = true;
        if (ctx.shop_forge)
            ctx.shop_forge->selected_index = 0;
        return;
    }

    if (npc.type == NpcType::Generic) {
        if (!npc.dialogue_default.empty())
            ctx.dialogue->start(npc.dialogue_default);
        return;
    }

    if (!ctx.quest_state->is(QuestId::DefeatGrimjaw, QuestStatus::NotStarted)
        && !ctx.quest_state->is(QuestId::DefeatGrimjaw, QuestStatus::InProgress)
        && !ctx.quest_state->is(QuestId::DefeatGrimjaw, QuestStatus::Completed)) {
        ctx.dialogue->start(npc.dialogue_default.empty() ? "mira_default" : npc.dialogue_default);
        return;
    }

    if (ctx.quest_state->is(QuestId::DefeatGrimjaw, QuestStatus::NotStarted)) {
        ctx.dialogue->start("mira_quest_offer", [ctx]() mutable {
            if (!ctx.quest_state)
                return;
            ctx.quest_state->set(QuestId::DefeatGrimjaw, QuestStatus::InProgress);
            SaveData save = WorldSaveController::make_world_save(ctx);
            SaveSystem::save_default(save);
        });
        return;
    }

    if (ctx.quest_state->is(QuestId::DefeatGrimjaw, QuestStatus::InProgress)) {
        ctx.dialogue->start(npc.dialogue_quest_active.empty() ? "mira_quest_active"
                                                               : npc.dialogue_quest_active);
        return;
    }

    if (ctx.quest_state->is(QuestId::DefeatGrimjaw, QuestStatus::Completed)) {
        ctx.dialogue->start(npc.dialogue_quest_done.empty() ? "mira_quest_done"
                                                             : npc.dialogue_quest_done,
                            [ctx, quest_reward_gold]() mutable {
            if (!ctx.quest_state || !ctx.player)
                return;
            ctx.quest_state->set(QuestId::DefeatGrimjaw, QuestStatus::Rewarded);
            ctx.player->gold += quest_reward_gold;
            SaveData save = WorldSaveController::make_world_save(ctx);
            SaveSystem::save_default(save);
        });
        return;
    }

    ctx.dialogue->start(npc.dialogue_default.empty() ? "mira_default" : npc.dialogue_default);
}

} // namespace TownNpcInteractionController

} // namespace mion
