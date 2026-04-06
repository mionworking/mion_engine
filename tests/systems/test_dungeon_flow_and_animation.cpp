#include "../test_common.hpp"

#include <SDL3/SDL.h>

#include "../../src/systems/animation_driver.hpp"
#include "../../src/systems/combat_fx_controller.hpp"

using namespace mion;

static void animation_driver_selects_expected_player_anims() {
    Actor player;
    player.sprite_sheet = reinterpret_cast<SDL_Texture*>(1);
    player.anim.build_puny_clips(0, 8.0f);

    player.is_moving = true;
    AnimationDriver::update_player_town_anim(player, 0.1f);
    EXPECT_EQ(player.anim.current_anim, ActorAnim::Walk);

    player.is_moving = false;
    player.combat.attack_phase = AttackPhase::Startup;
    AnimationDriver::update_player_town_anim(player, 0.1f);
    EXPECT_EQ(player.anim.current_anim, ActorAnim::Attack);

    player.combat.attack_phase = AttackPhase::Idle;
    AnimationDriver::update_player_dungeon_anim(player, 0.3f, 0.1f);
    EXPECT_EQ(player.anim.current_anim, ActorAnim::Cast);
}

static void combat_fx_controller_applies_hitstop_and_death_fade() {
    MeleeCombatSystem combat;
    combat.pending_hitstop = 5;
    combat.player_hit_landed = true;

    Camera2D camera;
    int hitstop = 0;
    CombatFxController::apply_combat_feedback(combat, camera, nullptr, hitstop);
    EXPECT_EQ(hitstop, 5);
    EXPECT_GT(camera.shake_remaining, 0);

    bool death_snapshot_done = false;
    float death_fade_remaining = 0.0f;
    std::string pending_next_scene;
    int snapshot_calls = 0;
    CombatFxController::update_death_flow(
        false,
        2.0f,
        death_snapshot_done,
        death_fade_remaining,
        1.5f,
        pending_next_scene,
        [&]() { ++snapshot_calls; });
    EXPECT_TRUE(death_snapshot_done);
    EXPECT_EQ(snapshot_calls, 1);
    EXPECT_EQ(pending_next_scene, "game_over");
}

REGISTER_TEST(animation_driver_selects_expected_player_anims);
REGISTER_TEST(combat_fx_controller_applies_hitstop_and_death_fade);
