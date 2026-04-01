#include "test_common.hpp"

void run_legacy_tests();
// void run_extended_plan_tests(); migrado
void run_dungeon_controller_tests();

void run_player_action_v2_tests();
void run_gaps_v2_tests();

int main() {
    run_legacy_tests();
    // run_extended_plan_tests(); migrado
    run_dungeon_controller_tests();
    run_player_action_v2_tests();
    // run_gaps_v2_tests(); foi migrado!

    RunAllRegisteredTests("V2 Core Auto-Registered Tests");

    std::printf("\n[v2-core] %d passed, %d failed\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}
