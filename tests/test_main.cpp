#include "test_common.hpp"

void run_legacy_tests();
void run_extended_plan_tests();
void run_keybind_config_tests();
void run_gamepad_input_tests();
void run_locale_tests();
void run_credits_scene_tests();

int main() {
    run_legacy_tests();
    run_extended_plan_tests();
    run_keybind_config_tests();
    run_gamepad_input_tests();
    run_locale_tests();
    run_credits_scene_tests();
    std::printf("\n%d passed, %d failed\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}
