#include "../test_common.hpp"
#include "core/dialogue.hpp"
#include "core/input.hpp"
#include "core/scripted_input.hpp"
#include "systems/dialogue_system.hpp"

static mion::DialogueSequence make_seq(const char* id, int num_lines) {
    mion::DialogueSequence seq;
    seq.id = id;
    for (int i = 0; i < num_lines; ++i)
        seq.lines.push_back({"Speaker", "Line text", {255, 255, 255, 255}});
    return seq;
}

static void test_dialogue_starts_active() {
    mion::DialogueSystem sys;
    sys.register_sequence(make_seq("t", 2));
    EXPECT_FALSE(sys.is_active());
    sys.start("t");
    EXPECT_TRUE(sys.is_active());
}
REGISTER_TEST(test_dialogue_starts_active);

static void test_dialogue_unknown_id_stays_inactive() {
    mion::DialogueSystem sys;
    sys.start("does_not_exist");
    EXPECT_FALSE(sys.is_active());
}
REGISTER_TEST(test_dialogue_unknown_id_stays_inactive);

static void test_dialogue_advance_clears_on_last_line() {
    mion::DialogueSystem sys;
    sys.register_sequence(make_seq("s", 3));
    sys.start("s");

    sys.advance();
    EXPECT_TRUE(sys.is_active());  // linha 2
    sys.advance();
    EXPECT_TRUE(sys.is_active());  // linha 3
    sys.advance();
    EXPECT_FALSE(sys.is_active()); // fim
}
REGISTER_TEST(test_dialogue_advance_clears_on_last_line);

static void test_dialogue_on_finish_called_once() {
    mion::DialogueSystem sys;
    sys.register_sequence(make_seq("cb", 1));
    int calls = 0;
    sys.start("cb", [&]() { ++calls; });
    EXPECT_EQ(calls, 0);
    sys.advance();
    EXPECT_EQ(calls, 1);
    EXPECT_FALSE(sys.is_active());
    // segunda chamada não deve re-disparar
    sys.advance();
    EXPECT_EQ(calls, 1);
}
REGISTER_TEST(test_dialogue_on_finish_called_once);

static void test_dialogue_confirm_edge_detection() {
    mion::DialogueSystem sys;
    sys.register_sequence(make_seq("ed", 2));
    sys.start("ed"); // _prev_confirm = true internamente

    mion::InputState held;
    held.confirm_pressed = true;

    // Enter segurado logo após start → não avança (sem borda)
    sys.fixed_update(held);
    EXPECT_TRUE(sys.is_active());

    // Solta
    mion::InputState released;
    released.confirm_pressed = false;
    sys.fixed_update(released);
    EXPECT_TRUE(sys.is_active());

    // Aperta de novo → borda → avança para linha 2
    sys.fixed_update(held);
    EXPECT_TRUE(sys.is_active()); // ainda tem a segunda linha

    // Solta + aperta → avança para fim
    sys.fixed_update(released);
    sys.fixed_update(held);
    EXPECT_FALSE(sys.is_active());
}
REGISTER_TEST(test_dialogue_confirm_edge_detection);

static void test_dialogue_advance_safe_when_inactive() {
    mion::DialogueSystem sys;
    sys.advance(); // não deve crashar
    EXPECT_FALSE(sys.is_active());
}
REGISTER_TEST(test_dialogue_advance_safe_when_inactive);

// ---------------------------------------------------------------
// Integração: ScriptedInputSource + DialogueSystem
// ---------------------------------------------------------------
static void test_integration_scripted_dialogue_flow() {
    mion::DialogueSystem sys;
    mion::DialogueSequence seq;
    seq.id = "scripted_test";
    seq.lines = {
        {"A", "line 1", {255,255,255,255}},
        {"A", "line 2", {255,255,255,255}},
        {"A", "line 3", {255,255,255,255}},
    };
    sys.register_sequence(seq);

    int done = 0;
    sys.start("scripted_test", [&]() { ++done; });
    // start() seta _prev_confirm=true, então precisa release→press para cada avanço

    mion::ScriptedInputSource src;
    mion::InputState on, off;
    on.confirm_pressed  = true;
    off.confirm_pressed = false;
    // 3 pares off/on para avançar as 3 linhas
    src.push(off); src.push(on);
    src.push(off); src.push(on);
    src.push(off); src.push(on);

    for (int i = 0; i < 6; ++i)
        sys.fixed_update(src.read_state());

    EXPECT_FALSE(sys.is_active());
    EXPECT_EQ(done, 1);
}
REGISTER_TEST(test_integration_scripted_dialogue_flow);
