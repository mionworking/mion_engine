#pragma once

namespace mion {

struct EngineConfig {
    int   target_fps                   = 60;
    float fixed_delta_seconds          = 1.0f / 60.0f;
    float max_frame_time_seconds       = 0.05f;   // 50ms cap prevents the spiral of death
    int   window_width                 = 1280;
    int   window_height                = 720;
    const char* window_title           = "mion_engine";
};

} // namespace mion
