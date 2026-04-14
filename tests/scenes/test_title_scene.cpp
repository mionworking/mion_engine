#include "../test_common.hpp"
#include <cstdio>
#include "core/input.hpp"
#include "core/save_data.hpp"
#include "core/save_system.hpp"
#include "core/scene_ids.hpp"
#include "core/scripted_input.hpp"
#include "scenes/title_scene.hpp"

static void test_title_scene_attack_requests_town() {
    mion::DifficultyLevel diff = mion::DifficultyLevel::Normal;
    mion::TitleScene      scene;
    scene.set_difficulty_target(&diff);
    scene.enter();

    mion::InputState in;
    EXPECT_EQ(scene.next_scene(), mion::SceneId::kNone);

    in.attack_pressed = true;
    scene.fixed_update(0.016f, in);
    EXPECT_EQ(scene.next_scene(), mion::SceneId::kNone);

    in.attack_pressed = false;
    mion::InputState confirm;
    confirm.confirm_pressed = true;
    scene.fixed_update(0.016f, confirm);
    EXPECT_EQ(scene.next_scene(), mion::SceneId::kWorld);
    EXPECT_TRUE(diff == mion::DifficultyLevel::Normal);

    scene.clear_next_scene_request();
    EXPECT_EQ(scene.next_scene(), mion::SceneId::kNone);
}
REGISTER_TEST(test_title_scene_attack_requests_town);

static void test_title_erase_feedback_timer() {
    mion::TitleScene scene;
    scene.enter();
    EXPECT_FALSE(scene.has_erase_feedback());

    // Pressionar N ativa o timer de feedback
    mion::InputState in;
    in.erase_save_pressed = true;
    scene.fixed_update(0.016f, in);
    EXPECT_TRUE(scene.has_erase_feedback());

    // Soltar N — timer deve continuar decaindo
    in.erase_save_pressed = false;
    // 70 ticks * 0.016s = ~1.12s > 1.0s de duração do timer
    for (int i = 0; i < 70; ++i)
        scene.fixed_update(0.016f, in);
    EXPECT_FALSE(scene.has_erase_feedback());
}
REGISTER_TEST(test_title_erase_feedback_timer);

static void test_title_scene_can_open_credits() {
    mion::SaveSystem::remove_default_saves();
    std::remove("mion_save.txt");
    mion::TitleScene scene;
    scene.enter();

    mion::InputState down_press;
    down_press.ui_down_pressed = true;
    mion::InputState down_release;
    down_release.ui_down_pressed = false;
    // Move to EXTRAS via nav_up from index 0 (0 -> 4 -> 3), independent of CONTINUE state.
    mion::InputState up_press_2;
    up_press_2.ui_up_pressed = true;
    mion::InputState up_release_2;
    up_release_2.ui_up_pressed = false;
    scene.fixed_update(0.016f, up_press_2);
    scene.fixed_update(0.016f, up_release_2);
    scene.fixed_update(0.016f, up_press_2);
    scene.fixed_update(0.016f, up_release_2);

    mion::InputState pick_press;
    pick_press.confirm_pressed = true;
    mion::InputState pick_release;
    pick_release.confirm_pressed = false;
    // Enter EXTRAS submenu
    scene.fixed_update(0.016f, pick_press);
    scene.fixed_update(0.016f, pick_release);
    // Pick CREDITS in EXTRAS
    scene.fixed_update(0.016f, pick_release);
    scene.fixed_update(0.016f, pick_press);
    EXPECT_EQ(scene.next_scene(), mion::SceneId::kCredits);

    mion::SaveSystem::remove_default_saves();
    std::remove("mion_save.txt");
}
REGISTER_TEST(test_title_scene_can_open_credits);

static void test_title_scene_continue_with_save_requests_town() {
    mion::SaveSystem::remove_default_saves();
    std::remove("mion_save.txt");
    mion::SaveData sd{};
    sd.version = 3;
    sd.player_hp = 90;
    sd.room_index = 0;
    EXPECT_TRUE(mion::SaveSystem::save("mion_save.txt", sd));

    mion::TitleScene scene;
    scene.enter();

    mion::InputState down_press;
    down_press.ui_down_pressed = true;
    mion::InputState down_release;
    down_release.ui_down_pressed = false;
    scene.fixed_update(0.016f, down_press);   // NEW GAME -> CONTINUE
    scene.fixed_update(0.016f, down_release);

    mion::InputState pick_press;
    pick_press.confirm_pressed = true;
    scene.fixed_update(0.016f, pick_press);
    EXPECT_EQ(scene.next_scene(), mion::SceneId::kWorld);

    mion::SaveSystem::remove_default_saves();
    std::remove("mion_save.txt");
}
REGISTER_TEST(test_title_scene_continue_with_save_requests_town);

static void test_title_scene_main_menu_quit_requests_quit() {
    mion::SaveSystem::remove_default_saves();
    std::remove("mion_save.txt");
    mion::TitleScene scene;
    scene.enter();

    mion::InputState up_press;
    up_press.ui_up_pressed = true;
    mion::InputState up_release;
    up_release.ui_up_pressed = false;
    // From index 0, nav_up always wraps to QUIT (index 4), independent of disabled CONTINUE.
    scene.fixed_update(0.016f, up_press);
    scene.fixed_update(0.016f, up_release);

    mion::InputState pick_press;
    pick_press.confirm_pressed = true;
    scene.fixed_update(0.016f, pick_press);
    EXPECT_EQ(scene.next_scene(), mion::SceneId::kQuit);

    mion::SaveSystem::remove_default_saves();
    std::remove("mion_save.txt");
}
REGISTER_TEST(test_title_scene_main_menu_quit_requests_quit);

static void test_title_scene_settings_backspace_returns_to_main() {
    mion::SaveSystem::remove_default_saves();
    std::remove("mion_save.txt");
    mion::TitleScene scene;
    scene.enter();

    // Reach SETTINGS robustly: index 0 -> up(quit=4) -> up(extras=3) -> up(settings=2)
    mion::InputState up_press;
    up_press.ui_up_pressed = true;
    mion::InputState up_release;
    up_release.ui_up_pressed = false;
    scene.fixed_update(0.016f, up_press);
    scene.fixed_update(0.016f, up_release);
    scene.fixed_update(0.016f, up_press);
    scene.fixed_update(0.016f, up_release);
    scene.fixed_update(0.016f, up_press);
    scene.fixed_update(0.016f, up_release);

    mion::InputState pick_press;
    pick_press.confirm_pressed = true;
    mion::InputState pick_release;
    pick_release.confirm_pressed = false;
    scene.fixed_update(0.016f, pick_press);   // Enter settings
    scene.fixed_update(0.016f, pick_release);

    mion::InputState back_press;
    back_press.ui_cancel_pressed = true;
    mion::InputState back_release;
    back_release.ui_cancel_pressed = false;
    scene.fixed_update(0.016f, back_press);
    scene.fixed_update(0.016f, back_release);

    // Back in settings should not trigger any scene transition request.
    EXPECT_EQ(scene.next_scene(), mion::SceneId::kNone);

    mion::SaveSystem::remove_default_saves();
    std::remove("mion_save.txt");
}
REGISTER_TEST(test_title_scene_settings_backspace_returns_to_main);

// ---------------------------------------------------------------
// Integração: ScriptedInputSource + TitleScene
// ---------------------------------------------------------------
static void test_integration_scripted_title_to_town() {
    mion::DifficultyLevel diff = mion::DifficultyLevel::Normal;
    mion::TitleScene      scene;
    scene.set_difficulty_target(&diff);
    scene.enter();

    mion::ScriptedInputSource src;
    mion::InputState        to_diff;
    to_diff.attack_pressed = true;
    src.push(to_diff);
    scene.fixed_update(0.016f, src.read_state());
    EXPECT_EQ(scene.next_scene(), mion::SceneId::kNone);

    mion::InputState ok;
    ok.confirm_pressed = true;
    src.push(ok);
    scene.fixed_update(0.016f, src.read_state());
    EXPECT_EQ(scene.next_scene(), mion::SceneId::kWorld);
    EXPECT_TRUE(diff == mion::DifficultyLevel::Normal);
}
REGISTER_TEST(test_integration_scripted_title_to_town);
