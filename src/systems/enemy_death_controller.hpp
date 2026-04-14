#pragma once

#include <algorithm>
#include <optional>
#include <random>
#include <vector>

#include "../core/audio.hpp"
#include "../core/dungeon_dialogue_id.hpp"
#include "../core/quest_state.hpp"
#include "../core/run_stats.hpp"
#include "../entities/actor.hpp"
#include "../entities/enemy_type.hpp"
#include "../entities/ground_item.hpp"
#include "../world/dungeon_rules.hpp"
#include "drop_system.hpp"
#include "simple_particles.hpp"

namespace mion {

namespace EnemyDeathController {

struct DeathResult {
    std::optional<DungeonDialogueId> post_mortem_dialogue; // set se boss morreu
    bool                             boss_defeated   = false;
    bool                             quest_completed = false; // DefeatGrimjaw passou para Completed
    int                              xp_gained       = 0;    // total XP ganho no frame (para debug_log na cena)
};

// Processa todos os atores que morreram neste frame (was_alive && !is_alive).
// Retorna DeathResult — a cena decide o que fazer com quest/save.
inline DeathResult process_deaths(const std::vector<Actor*>& actors,
                                  Actor&                      player,
                                  const EnemyDef*             enemy_defs,
                                  const DropConfig&           drop_config,
                                  std::vector<GroundItem>&    ground_items,
                                  RunStats*                   run_stats,
                                  QuestState&                 quest_state,
                                  SimpleParticleSystem&       particles,
                                  AudioSystem*                audio,
                                  int                         room_index,
                                  bool                        stress_mode,
                                  std::mt19937*               rng) {
    DeathResult result;

    for (auto* a : actors) {
        if (!a->was_alive || a->is_alive) continue;

        if (a->team == Team::Enemy) {
            if (rng)
                particles.spawn_burst(a->transform.x, a->transform.y,
                                      22, 200, 60, 80, 40.f, 140.f, *rng);

            const EnemyDef& def = enemy_defs[static_cast<int>(a->enemy_type)];
            if (rng)
                DropSystem::on_enemy_died(ground_items, a->transform.x, a->transform.y,
                                          def, drop_config, *rng, -1, -1);

            const int gained = player.progression.add_xp(
                dungeon_rules::xp_per_enemy_kill(room_index));
            player.talents.pending_points += gained;
            result.xp_gained += gained;

            if (run_stats) {
                run_stats->enemies_killed++;
                run_stats->max_level_reached =
                    std::max(run_stats->max_level_reached, player.progression.level);
            }

            if (!stress_mode && def.is_zone_boss) {
                result.boss_defeated = true;
                result.post_mortem_dialogue = DungeonDialogueId::MinibossDeath;
                if (run_stats) run_stats->boss_defeated = true;
                if (quest_state.is(QuestId::DefeatGrimjaw, QuestStatus::InProgress)) {
                    quest_state.set(QuestId::DefeatGrimjaw, QuestStatus::Completed);
                    result.quest_completed = true;
                }
            }

            if (audio) audio->play_sfx(SoundId::EnemyDeath);
        } else {
            if (audio) audio->play_sfx(SoundId::PlayerDeath);
        }
    }

    return result;
}

} // namespace EnemyDeathController

} // namespace mion
