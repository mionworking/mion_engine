#include "test_common.hpp"

#include "components/progression.hpp"
#include "components/player_config.hpp"
#include "components/talent_tree.hpp"
#include "entities/actor.hpp"
#include "systems/attribute_levelup_controller.hpp"
#include "systems/pause_menu_controller.hpp"
#include "systems/skill_tree_controller.hpp"

using namespace mion;

static void test_attr_controller_spends_point_and_recomputes_stats() {
    reset_player_config_defaults();
    Actor player;
    player.progression.pending_level_ups = 1;
    player.attributes = {};
    player.derived   = {};
    player.talents   = {};
    player.health    = { g_player_config.base_hp, g_player_config.base_hp };
    player.mana.max     = g_player_config.base_mana_max;
    player.stamina.max  = g_player_config.base_stamina_max;
    player.mana.current = player.mana.max;
    player.stamina.current = player.stamina.max;

    AttributeLevelUpController controller;
    controller.sync_open_from_progression(player);

    OverlayInputEdges in{};
    in.confirm = true;

    AttributeLevelUpResult r = controller.update(player, in, /*stress_mode=*/false);
    EXPECT_TRUE(r.should_save);
    EXPECT_EQ(player.progression.pending_level_ups, 0);
    EXPECT_TRUE(player.attributes.vigor == 1
                || player.attributes.forca == 1
                || player.attributes.destreza == 1
                || player.attributes.inteligencia == 1
                || player.attributes.endurance == 1);
    EXPECT_TRUE(player.health.max_hp >= g_player_config.base_hp);
}

static void test_attr_controller_closes_when_no_pending() {
    reset_player_config_defaults();
    Actor player;
    player.progression.pending_level_ups = 1;
    AttributeLevelUpController controller;
    controller.sync_open_from_progression(player);
    player.progression.pending_level_ups = 0;

    OverlayInputEdges in{};
    AttributeLevelUpResult r = controller.update(player, in, /*stress_mode=*/false);
    EXPECT_FALSE(r.screen_open);
    EXPECT_FALSE(r.world_paused);
}

static PauseMenuResult step_pause_menu(PauseMenuController& controller, const InputState& in) {
    PauseMenuResult r = controller.update(in);
    controller.flush_input(in);
    return r;
}

static void test_pause_menu_controller_opens_on_pause() {
    PauseMenuController controller;
    controller.init();

    InputState esc_on{};
    esc_on.pause_pressed = true;
    PauseMenuResult r = step_pause_menu(controller, esc_on);
    EXPECT_TRUE(r.world_paused);
    EXPECT_TRUE(controller.paused());
}

static void test_pause_menu_controller_calls_on_opened() {
    PauseMenuController controller;
    bool opened = false;
    PauseMenuController::InitOptions options;
    options.on_opened = [&opened]{ opened = true; };
    controller.init(options);

    InputState esc_on{};
    esc_on.pause_pressed = true;
    (void)step_pause_menu(controller, esc_on);
    EXPECT_TRUE(opened);
}

static void test_pause_menu_controller_requests_skill_tree() {
    PauseMenuController controller;
    controller.init();

    InputState esc_on{};
    esc_on.pause_pressed = true;
    (void)step_pause_menu(controller, esc_on);
    InputState idle{};
    (void)step_pause_menu(controller, idle);

    InputState down_on{};
    down_on.ui_down_pressed = true;
    (void)step_pause_menu(controller, down_on);
    (void)step_pause_menu(controller, idle);

    InputState confirm_on{};
    confirm_on.confirm_pressed = true;
    PauseMenuResult r = step_pause_menu(controller, confirm_on);
    EXPECT_TRUE(r.should_open_skill_tree);
    EXPECT_FALSE(controller.paused());
}

static void test_pause_menu_controller_requests_quit_to_title() {
    PauseMenuController controller;
    controller.init();

    InputState esc_on{};
    esc_on.pause_pressed = true;
    (void)step_pause_menu(controller, esc_on);
    InputState idle{};
    (void)step_pause_menu(controller, idle);

    InputState down_on{};
    down_on.ui_down_pressed = true;
    for (int i = 0; i < 3; ++i) {
        (void)step_pause_menu(controller, down_on);
        (void)step_pause_menu(controller, idle);
    }

    InputState confirm_on{};
    confirm_on.confirm_pressed = true;
    PauseMenuResult r = step_pause_menu(controller, confirm_on);
    EXPECT_TRUE(r.should_quit_to_title);
    EXPECT_FALSE(controller.paused());
}

static void test_skill_tree_controller_auto_opens_when_pending() {
    reset_talent_tree_defaults();
    Actor player;
    player.talents.pending_points = 1;
    SkillTreeController controller;
    controller.rebuild_columns();

    OverlayInputEdges in{};
    SkillTreeResult r = controller.update(player, in, /*stress_mode=*/false);
    EXPECT_TRUE(r.screen_open);
    EXPECT_TRUE(r.world_paused);
    EXPECT_EQ(controller.selected_col(), 0);
    EXPECT_EQ(controller.selected_row(), 0);
}

static void test_skill_tree_controller_spends_arcane_reservoir() {
    reset_talent_tree_defaults();
    Actor player;
    player.talents.pending_points = 1;
    player.mana.max = 100.0f;
    player.mana.current = 80.0f;
    SkillTreeController controller;
    controller.rebuild_columns();
    controller.open();

    OverlayInputEdges move_right{};
    move_right.right = true;
    (void)controller.update(player, move_right, /*stress_mode=*/false);
    (void)controller.update(player, move_right, /*stress_mode=*/false);

    OverlayInputEdges confirm{};
    confirm.confirm = true;
    SkillTreeResult r = controller.update(player, confirm, /*stress_mode=*/false);
    EXPECT_TRUE(r.should_save);
    EXPECT_EQ(player.talents.level_of(TalentId::ArcaneReservoir), 1);
    EXPECT_EQ(player.talents.pending_points, 0);
    EXPECT_NEAR(player.mana.max, 130.0f, 0.001f);
    EXPECT_NEAR(player.mana.current, 110.0f, 0.001f);
}

static void test_skill_tree_controller_tab_toggles_manual_open() {
    reset_talent_tree_defaults();
    Actor player;
    SkillTreeController controller;
    controller.rebuild_columns();

    OverlayInputEdges open{};
    open.tab = true;
    SkillTreeResult opened = controller.update(player, open, /*stress_mode=*/false);
    EXPECT_TRUE(opened.screen_open);
    EXPECT_TRUE(opened.world_paused);

    OverlayInputEdges close{};
    close.tab = true;
    SkillTreeResult closed = controller.update(player, close, /*stress_mode=*/false);
    EXPECT_FALSE(closed.screen_open);
    EXPECT_TRUE(closed.world_paused);
}

void run_dungeon_controller_tests() {
    run("AttrController.SpendPoint", test_attr_controller_spends_point_and_recomputes_stats);
    run("AttrController.CloseWhenNoPending", test_attr_controller_closes_when_no_pending);
    run("PauseMenuController.OpenOnPause", test_pause_menu_controller_opens_on_pause);
    run("PauseMenuController.OnOpened", test_pause_menu_controller_calls_on_opened);
    run("PauseMenuController.RequestSkillTree", test_pause_menu_controller_requests_skill_tree);
    run("PauseMenuController.RequestQuitTitle", test_pause_menu_controller_requests_quit_to_title);
    run("SkillTreeController.AutoOpenPending", test_skill_tree_controller_auto_opens_when_pending);
    run("SkillTreeController.SpendArcaneReservoir", test_skill_tree_controller_spends_arcane_reservoir);
    run("SkillTreeController.TabToggle", test_skill_tree_controller_tab_toggles_manual_open);
}

