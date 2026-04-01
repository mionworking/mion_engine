#include "test_common.hpp"

void run_credits_scene_tests();
void run_scenes_v2_tests();

int main() {
    run_credits_scene_tests();
    run_scenes_v2_tests();

    RunAllRegisteredTests("V2 Scenes Auto-Registered Tests");

    std::printf("\n[v2-scenes] %d passed, %d failed\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}
