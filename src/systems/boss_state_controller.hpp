#pragma once

#include <vector>

#include "../core/audio.hpp"
#include "../core/camera.hpp"
#include "../core/dungeon_dialogue_id.hpp"
#include "../entities/actor.hpp"
#include "../entities/enemy_type.hpp"
#include "dialogue_system.hpp"
#include "screen_fx.hpp"

namespace mion {

struct BossState {
    bool  intro_pending    = false;
    float intro_timer      = 0.0f;
    bool  phase2_triggered = false;

    static constexpr float kIntroDuration = 1.8f;

    // Chamado uma vez por fixed_update após EnemyAI.
    // Detecta transição para fase2 do Grimjaw e tick do timer de intro.
    void update(const std::vector<Actor*>& enemies,
                const EnemyDef*             enemy_defs,
                Camera2D&                 camera,
                DialogueSystem&           dialogue,
                ScreenFlashState&         flash,
                float                     dt,
                bool                      stress_mode) {
        if (intro_pending) {
            intro_timer += dt;
            if (intro_timer >= kIntroDuration)
                intro_pending = false;
        }

        if (stress_mode || phase2_triggered) return;

        for (const auto* e : enemies) {
            if (!e) continue;
            if (!enemy_defs[static_cast<int>(e->enemy_type)].is_zone_boss) continue;
            if (e->boss_phase != 2) continue;
            phase2_triggered = true;
            camera.trigger_shake(18.0f, 30);
            dialogue.start(to_string(DungeonDialogueId::BossPhase2));
            flash.trigger({255, 200, 50, 180}, 0.3f);
            break;
        }
    }

    // Chamado por EnemySpawner após spawn do boss — replica o que a cena fazia.
    void apply_spawn_result(bool boss_intro, float boss_intro_t, bool phase2) {
        intro_pending    = boss_intro;
        intro_timer      = boss_intro_t;
        phase2_triggered = phase2;
    }
};

} // namespace mion
