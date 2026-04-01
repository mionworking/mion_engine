#include "../test_common.hpp"
#include "core/ui.hpp"
#include "systems/floating_text.hpp"
#include "core/scene_fader.hpp"

static void test_scene_fader_fade_out_holds_black() {
    mion::SceneFader f;
    f.start_fade_out(0.5f);
    f.tick(0.6f);
    EXPECT_TRUE(f.is_black_hold());
    EXPECT_EQ((int)f.alpha, 255);
}
REGISTER_TEST(test_scene_fader_fade_out_holds_black);

static void test_scene_fader_fade_in_goes_clear() {
    mion::SceneFader f;
    f.start_fade_out(0.1f);
    f.tick(0.2f);
    EXPECT_TRUE(f.is_black_hold());
    f.start_fade_in(0.2f);
    f.tick(0.25f);
    EXPECT_TRUE(f.is_clear());
    EXPECT_EQ((int)f.alpha, 0);
}
REGISTER_TEST(test_scene_fader_fade_in_goes_clear);

static void test_floating_text_tick_erases_dead() {
    mion::FloatingTextSystem sys;
    sys.spawn(0.f, 0.f, "x", {255, 255, 255, 255}, 1, 0.1f, 0.f);
    EXPECT_EQ(sys.texts.size(), (size_t)1);
    sys.tick(0.15f);
    EXPECT_EQ(sys.texts.size(), (size_t)0);
}
REGISTER_TEST(test_floating_text_tick_erases_dead);

static void test_ui_list_nav_skips_disabled() {
    mion::ui::List L;
    L.items     = {"A", "B", "C"};
    L.disabled  = {false, true, false};
    L.selected  = 0;
    L.nav_down();
    EXPECT_EQ(L.selected, 2);
    L.nav_down();
    EXPECT_EQ(L.selected, 0);
    L.nav_up();
    EXPECT_EQ(L.selected, 2);
}
REGISTER_TEST(test_ui_list_nav_skips_disabled);

static void test_ui_list_all_disabled_stays() {
    mion::ui::List L;
    L.items    = {"X", "Y"};
    L.disabled = {true, true};
    L.selected = 0;
    L.nav_down();
    EXPECT_EQ(L.selected, 0);
}
REGISTER_TEST(test_ui_list_all_disabled_stays);

static void test_ui_progress_bar_normalized_fill_clamps() {
    mion::ui::ProgressBar bar;
    bar.value = -0.5f;
    EXPECT_NEAR(bar.normalized_fill(), 0.0f, 0.0001f);
    bar.value = 0.37f;
    EXPECT_NEAR(bar.normalized_fill(), 0.37f, 0.0001f);
    bar.value = 1.7f;
    EXPECT_NEAR(bar.normalized_fill(), 1.0f, 0.0001f);
}
REGISTER_TEST(test_ui_progress_bar_normalized_fill_clamps);

static void test_ui_list_wraps_when_none_disabled() {
    mion::ui::List L;
    L.items    = {"a", "b"};
    L.selected = 0;
    L.nav_up();
    EXPECT_EQ(L.selected, 1);
    L.nav_down();
    EXPECT_EQ(L.selected, 0);
}
REGISTER_TEST(test_ui_list_wraps_when_none_disabled);
