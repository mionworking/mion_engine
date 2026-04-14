#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "../core/town_dialogue_ids.hpp"
#include "../entities/npc.hpp"
#include "../world/world_context.hpp"

namespace mion {

namespace TownNpcInteractionController {

inline float town_dist_sq(float ax, float ay, float bx, float by) {
    const float dx = ax - bx;
    const float dy = ay - by;
    return dx * dx + dy * dy;
}

struct NearbyNpcResult {
    int index = -1;
    bool in_interaction_range = false;
};

inline std::optional<NearbyNpcResult> find_nearest_npc_for_interaction(
    const Actor& player,
    const std::vector<NpcEntity>& npcs) {
    int nearest = -1;
    float nearest_dist_sq = 1.0e12f;
    for (int i = 0; i < static_cast<int>(npcs.size()); ++i) {
        const NpcEntity& npc = npcs[static_cast<size_t>(i)];
        const float dist_sq = town_dist_sq(player.transform.x, player.transform.y, npc.x, npc.y);
        if (dist_sq < nearest_dist_sq) {
            nearest_dist_sq = dist_sq;
            nearest = i;
        }
    }
    if (nearest < 0)
        return std::nullopt;

    const NpcEntity& npc = npcs[static_cast<size_t>(nearest)];
    const float dist_sq = town_dist_sq(player.transform.x, player.transform.y, npc.x, npc.y);
    const float radius_sq = npc.interact_radius * npc.interact_radius;
    NearbyNpcResult result;
    result.index = nearest;
    result.in_interaction_range = (dist_sq <= radius_sq);
    return result;
}

inline void handle_npc_interaction(int idx,
                                   WorldContext ctx,
                                   bool& shop_open,
                                   int quest_reward_gold,
                                   const std::function<void()>& request_persist = {}) {
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
        ctx.dialogue->start(npc.dialogue_default.empty()
                                ? std::string(to_string(TownDialogueId::MiraDefault))
                                : npc.dialogue_default);
        return;
    }

    if (ctx.quest_state->is(QuestId::DefeatGrimjaw, QuestStatus::NotStarted)) {
        ctx.dialogue->start(to_string(TownDialogueId::MiraQuestOffer),
                            [ctx, request_persist]() mutable {
            if (!ctx.quest_state)
                return;
            ctx.quest_state->set(QuestId::DefeatGrimjaw, QuestStatus::InProgress);
            if (request_persist)
                request_persist();
        });
        return;
    }

    if (ctx.quest_state->is(QuestId::DefeatGrimjaw, QuestStatus::InProgress)) {
        ctx.dialogue->start(npc.dialogue_quest_active.empty()
                                ? std::string{to_string(TownDialogueId::MiraQuestActive)}
                                : npc.dialogue_quest_active);
        return;
    }

    if (ctx.quest_state->is(QuestId::DefeatGrimjaw, QuestStatus::Completed)) {
        ctx.dialogue->start(npc.dialogue_quest_done.empty()
                                ? std::string{to_string(TownDialogueId::MiraQuestDone)}
                                : npc.dialogue_quest_done,
                            [ctx, quest_reward_gold, request_persist]() mutable {
            if (!ctx.quest_state || !ctx.player)
                return;
            ctx.quest_state->set(QuestId::DefeatGrimjaw, QuestStatus::Rewarded);
            ctx.player->player->gold += quest_reward_gold;
            if (request_persist)
                request_persist();
        });
        return;
    }

    ctx.dialogue->start(npc.dialogue_default.empty()
                            ? std::string(to_string(TownDialogueId::MiraDefault))
                            : npc.dialogue_default);
}

} // namespace TownNpcInteractionController

} // namespace mion
