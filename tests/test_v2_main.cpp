#include "test_common.hpp"

void run_legacy_tests();
void run_extended_plan_tests();
void run_keybind_config_tests();
void run_gamepad_input_tests();
void run_locale_tests();
void run_credits_scene_tests();

void run_player_action_v2_tests();
void run_scenes_v2_tests();
void run_gaps_v2_tests();

int main() {
    run_legacy_tests();
    run_extended_plan_tests();
    run_keybind_config_tests();
    run_gamepad_input_tests();
    run_locale_tests();
    run_credits_scene_tests();

    run_player_action_v2_tests();
    run_scenes_v2_tests();
    run_gaps_v2_tests();
    std::printf("\n[v2] %d passed, %d failed\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}
