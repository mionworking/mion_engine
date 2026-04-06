#include "../test_common.hpp"

#include "../../src/systems/animation_driver.hpp"
#include "../../src/systems/combat_fx_controller.hpp"
#include "../../src/core/dungeon_dialogue.hpp"
#include "../../src/systems/dungeon_flow_controller.hpp"

using namespace mion;

static void dungeon_flow_controller_starts_full_run_from_room_zero() {
    RoomDefinition room;
    Tilemap tilemap;
    Actor player;
    std::vector<Projectile> projectiles;
    std::vector<GroundItem> ground_items;
    int room_index = 99;
    QuestState quest_state;
    RunStats run_stats;
    IniData rooms_ini;
    Pathfinder pathfinder;
    DialogueSystem dialogue;
    bool first_door_dialogue_done = true;
    std::string post_mortem_dialogue_id = "x";
    bool pending_room_blurb = true;
    int load_room_visuals_calls = 0;
    int configure_player_calls = 0;
    int spawn_calls = 0;

    register_dungeon_dialogue(dialogue);

    DungeonFlowController::start_full_run(
        false,
        room,
        tilemap,
        player,
        projectiles,
        ground_items,
        room_index,
        quest_state,
        &run_stats,
        rooms_ini,
        pathfinder,
        false,
        dialogue,
        first_door_dialogue_done,
        post_mortem_dialogue_id,
        pending_room_blurb,
        [&]() { ++load_room_visuals_calls; },
        [&](bool reset_health_to_max) {
            EXPECT_TRUE(reset_health_to_max);
            ++configure_player_calls;
        },
        [&]() { ++spawn_calls; });

    EXPECT_EQ(room_index, 0);
    EXPECT_EQ(load_room_visuals_calls, 1);
    EXPECT_EQ(configure_player_calls, 1);
    EXPECT_EQ(spawn_calls, 1);
    EXPECT_TRUE(dialogue.is_active());
    EXPECT_FALSE(first_door_dialogue_done);
    EXPECT_TRUE(post_mortem_dialogue_id.empty());
    EXPECT_FALSE(pending_room_blurb);
}

static void dungeon_flow_controller_advances_room_and_persists() {
    RoomDefinition room;
    Tilemap tilemap;
    Actor player;
    std::vector<Projectile> projectiles;
    std::vector<GroundItem> ground_items;
    int room_index = 0;
    RunStats run_stats;
    IniData rooms_ini;
    Pathfinder pathfinder;
    DialogueSystem dialogue;
    bool pending_room_blurb = false;
    int load_room_visuals_calls = 0;
    int spawn_calls = 0;
    int persist_calls = 0;

    DungeonFlowController::advance_room(
        room,
        tilemap,
        player,
        projectiles,
        ground_items,
        room_index,
        &run_stats,
        rooms_ini,
        pathfinder,
        false,
        dialogue,
        pending_room_blurb,
        [&]() { ++load_room_visuals_calls; },
        [&]() { ++spawn_calls; },
        [&]() { ++persist_calls; });

    EXPECT_EQ(room_index, 1);
    EXPECT_EQ(run_stats.rooms_cleared, 1);
    EXPECT_EQ(load_room_visuals_calls, 1);
    EXPECT_EQ(spawn_calls, 1);
    EXPECT_EQ(persist_calls, 1);
}

static void dungeon_flow_controller_transitions_to_town_exit() {
    RoomFlowSystem room_flow_sys;
    RoomDefinition room;
    room.bounds = {0.0f, 200.0f, 0.0f, 200.0f};
    room.add_door(40.0f, 40.0f, 80.0f, 80.0f, false, "town");

    Actor player;
    player.team = Team::Player;
    player.transform.set_position(60.0f, 60.0f);
    std::vector<Actor*> actors = {&player};

    bool first_door_dialogue_done = false;
    bool pending_room_blurb = false;
    std::string post_mortem_dialogue_id;
    DialogueSystem dialogue;
    bool scene_exit_pending = false;
    float scene_exit_timer = 0.0f;
    std::string scene_exit_target;
    std::string pending_next_scene;
    QuestState quest_state;
    int persist_calls = 0;
    int advance_calls = 0;
    int snapshot_calls = 0;

    DungeonFlowController::update_room_flow(
        room_flow_sys,
        actors,
        player,
        room,
        0.5f,
        false,
        1,
        first_door_dialogue_done,
        pending_room_blurb,
        post_mortem_dialogue_id,
        dialogue,
        nullptr,
        scene_exit_pending,
        scene_exit_timer,
        scene_exit_target,
        pending_next_scene,
        nullptr,
        quest_state,
        [&]() { ++persist_calls; },
        [&]() { ++advance_calls; },
        [&]() { ++snapshot_calls; });

    EXPECT_EQ(persist_calls, 1);
    EXPECT_EQ(advance_calls, 0);
    EXPECT_EQ(snapshot_calls, 0);
    EXPECT_EQ(pending_next_scene, "town");
}

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

REGISTER_TEST(dungeon_flow_controller_starts_full_run_from_room_zero);
REGISTER_TEST(dungeon_flow_controller_advances_room_and_persists);
REGISTER_TEST(dungeon_flow_controller_transitions_to_town_exit);
REGISTER_TEST(animation_driver_selects_expected_player_anims);
REGISTER_TEST(combat_fx_controller_applies_hitstop_and_death_fade);
