#include "test_common.hpp"

void run_keybind_config_tests();
void run_gamepad_input_tests();
void run_locale_tests();

int main() {
    run_keybind_config_tests();
    run_gamepad_input_tests();
    run_locale_tests();

    RunAllRegisteredTests("V2 Input Auto-Registered Tests");

    std::printf("\n[v2-input] %d passed, %d failed\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}
