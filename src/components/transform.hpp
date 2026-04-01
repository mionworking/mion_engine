#pragma once

namespace mion {

// 2D world position for any entity; used by movement and rendering systems.
struct Transform2D {
    float x = 0.0f;
    float y = 0.0f;

    void translate(float dx, float dy) { x += dx; y += dy; }
    void set_position(float nx, float ny) { x = nx; y = ny; }
};

} // namespace mion
