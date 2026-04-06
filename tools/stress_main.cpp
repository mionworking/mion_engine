#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>

#include "core/input.hpp"
#include "scenes/world_scene.hpp"

namespace {

int parse_int(const char* text, int fallback, int min_value) {
    if (!text || !*text) return fallback;

    char* end = nullptr;
    long parsed = std::strtol(text, &end, 10);
    if (end == text) return fallback;
    if (parsed < min_value) parsed = min_value;
    if (parsed > 100000) parsed = 100000;
    return (int)parsed;
}

int read_arg_or_env(int argc, char* argv[], int arg_index,
                    const char* env_name, int fallback, int min_value)
{
    if (argc > arg_index)
        return parse_int(argv[arg_index], fallback, min_value);

    return parse_int(std::getenv(env_name), fallback, min_value);
}

} // namespace

int main(int argc, char* argv[]) {
    const int enemy_count = read_arg_or_env(argc, argv, 1,
                                            "MION_STRESS_ENEMIES", 180, 3);
    const int frames = read_arg_or_env(argc, argv, 2,
                                       "MION_STRESS_FRAMES", 600, 1);

    mion::WorldScene dungeon;
    dungeon.enable_stress_mode(enemy_count);
    dungeon.enter();

    mion::InputState input;
    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < frames; ++i)
        dungeon.fixed_update(1.0f / 60.0f, input);

    auto end = std::chrono::steady_clock::now();
    dungeon.exit();

    double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
    double avg_ms = elapsed_ms / (double)frames;
    double sim_fps = avg_ms > 0.0 ? 1000.0 / avg_ms : 0.0;

    std::printf("mion_stress enemies=%d living=%d frames=%d total_ms=%.2f avg_ms=%.4f sim_fps=%.2f\n",
                dungeon.enemy_count(),
                dungeon.living_enemy_count(),
                frames,
                elapsed_ms,
                avg_ms,
                sim_fps);

    return 0;
}
