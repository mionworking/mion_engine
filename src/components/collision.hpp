#pragma once

namespace mion {

struct AABB {
    float min_x = 0.0f;
    float max_x = 0.0f;
    float min_y = 0.0f;
    float max_y = 0.0f;

    bool intersects(const AABB& other) const {
        return min_x < other.max_x && max_x > other.min_x
            && min_y < other.max_y && max_y > other.min_y;
    }
};

struct CollisionBox {
    float half_w = 0.5f;
    float half_h = 0.5f;

    AABB bounds_at(float cx, float cy) const {
        return { cx - half_w, cx + half_w, cy - half_h, cy + half_h };
    }
};

struct HurtBox {
    float half_w    = 0.4f;
    float half_h    = 0.4f;
    float offset_x  = 0.0f;
    float offset_y  = 0.0f;
    bool  enabled   = true;

    AABB bounds_at(float cx, float cy) const {
        return {
            cx + offset_x - half_w,
            cx + offset_x + half_w,
            cy + offset_y - half_h,
            cy + offset_y + half_h
        };
    }
};

struct MeleeHitBox {
    float half_w           = 0.5f;
    float half_h           = 0.3f;
    float forward_offset   = 0.6f;  // para frente do facing
    float lateral_offset   = 0.0f;
    bool  enabled          = true;

    AABB bounds_at(float cx, float cy, float facing_x, float facing_y) const {
        float ox = facing_x * forward_offset + facing_y * lateral_offset;
        float oy = facing_y * forward_offset + facing_x * lateral_offset;
        return {
            cx + ox - half_w,
            cx + ox + half_w,
            cy + oy - half_h,
            cy + oy + half_h
        };
    }
};

} // namespace mion
