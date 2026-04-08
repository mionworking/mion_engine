#include "register_scenes.hpp"
#include "scene_context.hpp"
#include "scene_ids.hpp"

#include "../scenes/title_scene.hpp"
#include "../scenes/world_scene.hpp"
#include "../scenes/game_over_scene.hpp"
#include "../scenes/victory_scene.hpp"
#include "../scenes/credits_scene.hpp"

namespace mion {

void register_default_scenes(SceneRegistry& registry) {
    registry.register_scene(SceneId::kTitle, [](const SceneCreateContext& ctx) {
        auto s = std::make_unique<TitleScene>();
        s->viewport_w = ctx.viewport_w;
        s->viewport_h = ctx.viewport_h;
        s->set_audio(ctx.audio);
        s->set_difficulty_target(ctx.difficulty);
        s->set_locale(ctx.locale);
        return s;
    });

    registry.register_scene(SceneId::kWorld, [](const SceneCreateContext& ctx) {
        auto s = std::make_unique<WorldScene>();
        s->viewport_w = ctx.viewport_w;
        s->viewport_h = ctx.viewport_h;
        s->set_renderer(ctx.renderer);
        s->set_audio(ctx.audio);
        s->set_show_autosave_indicator(ctx.show_autosave_indicator);
        s->set_run_stats(ctx.run_stats);
        s->set_difficulty(ctx.difficulty);
        s->set_locale(ctx.locale);
        s->set_rng(ctx.rng);
        const int n = ctx.stress_enemy_count;
        if (n > 3) {
            s->enable_stress_mode(n);
        } else if (n > 0) {
            s->set_enemy_spawn_count(n);
        }
        return s;
    });

    registry.register_scene(SceneId::kGameOver, [](const SceneCreateContext& ctx) {
        auto s = std::make_unique<GameOverScene>();
        s->viewport_w = ctx.viewport_w;
        s->viewport_h = ctx.viewport_h;
        s->set_audio(ctx.audio);
        s->set_run_stats_source(ctx.run_stats);
        s->set_locale(ctx.locale);
        return s;
    });

    registry.register_scene(SceneId::kVictory, [](const SceneCreateContext& ctx) {
        auto s = std::make_unique<VictoryScene>();
        s->viewport_w = ctx.viewport_w;
        s->viewport_h = ctx.viewport_h;
        s->set_audio(ctx.audio);
        s->set_run_stats_source(ctx.run_stats);
        s->set_locale(ctx.locale);
        return s;
    });

    registry.register_scene(SceneId::kCredits, [](const SceneCreateContext& ctx) {
        auto s = std::make_unique<CreditsScene>();
        s->viewport_w = ctx.viewport_w;
        s->viewport_h = ctx.viewport_h;
        s->set_audio(ctx.audio);
        s->set_locale(ctx.locale);
        return s;
    });
}

} // namespace mion
