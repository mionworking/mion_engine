#pragma once

namespace mion {

struct Transform2D {
    float x = 0.0f;
    float y = 0.0f;

    void translate(float dx, float dy) { x += dx; y += dy; }
    void set_position(float nx, float ny) { x = nx; y = ny; }
};

} // namespace mion
