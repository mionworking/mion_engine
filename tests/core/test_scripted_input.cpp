#include "../test_common.hpp"
#include "core/input.hpp"
#include "core/scripted_input.hpp"

static void test_scripted_input_replay_in_order() {
    mion::ScriptedInputSource src;
    mion::InputState a, b, c;
    a.move_x = 1.0f;
    b.move_y = -1.0f;
    c.attack_pressed = true;
    src.push(a); src.push(b); src.push(c);

    EXPECT_NEAR(src.read_state().move_x,  1.0f, 0.001f);
    EXPECT_NEAR(src.read_state().move_y, -1.0f, 0.001f);
    EXPECT_TRUE(src.read_state().attack_pressed);
}
REGISTER_TEST(test_scripted_input_replay_in_order);

static void test_scripted_input_exhausted_repeats_last() {
    mion::ScriptedInputSource src;
    mion::InputState last; last.dash_pressed = true;
    src.push(mion::InputState{}); // frame 1
    src.push(last);               // frame 2 (last)

    src.read_state(); // consume frame 1
    src.read_state(); // consume frame 2
    EXPECT_TRUE(src.done());

    // Frames extras devem retornar o último
    EXPECT_TRUE(src.read_state().dash_pressed);
    EXPECT_TRUE(src.read_state().dash_pressed);
}
REGISTER_TEST(test_scripted_input_exhausted_repeats_last);

static void test_scripted_input_empty_returns_default() {
    mion::ScriptedInputSource src;
    mion::InputState s = src.read_state();
    EXPECT_FALSE(s.attack_pressed);
    EXPECT_NEAR(s.move_x, 0.0f, 0.001f);
}
REGISTER_TEST(test_scripted_input_empty_returns_default);

static void test_scripted_input_reset() {
    mion::ScriptedInputSource src;
    mion::InputState a; a.move_x = 1.0f;
    src.push(a);
    src.read_state(); // consume
    EXPECT_TRUE(src.done());
    src.reset();
    EXPECT_FALSE(src.done());
    EXPECT_NEAR(src.read_state().move_x, 1.0f, 0.001f);
}
REGISTER_TEST(test_scripted_input_reset);
