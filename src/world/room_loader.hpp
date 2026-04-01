#pragma once
#include <algorithm>
#include <string>
#include <utility>

#include "room.hpp"
#include "../core/ini_loader.hpp"

namespace mion {

// If `[template_key].ini_obstacles=1` and sections `template_key.obstacle.N` exist, populates
// `room.obstacles` with world-space AABBs. Positions are normalized `nx1..ny2` in [0,1]
// relative to `bounds` (min_x/min_y → max_x/max_y). Optional art keys: `sprite`, `sw`, `sh`,
// `anchor_x`, `anchor_y`.
//
// Returns true if at least one valid obstacle was loaded. If false, the caller should
// fall back to the hardcoded layout (`_layout_*` functions).
inline bool load_room_obstacles_from_ini(RoomDefinition&     room,
                                         const IniData&      data,
                                         const WorldBounds&  bounds,
                                         const std::string&  template_key) {
    if (data.get_int(template_key, "ini_obstacles", 0) == 0)
        return false;

    const std::string        obst_prefix = template_key + ".obstacle";
    std::vector<std::string> sec_names     = data.sections_with_prefix(obst_prefix);
    if (sec_names.empty())
        return false;

    room.obstacles.clear();

    const float bw = bounds.max_x - bounds.min_x;
    const float bh = bounds.max_y - bounds.min_y;
    if (bw <= 0.0f || bh <= 0.0f)
        return false;

    for (const std::string& sec : sec_names) {
        constexpr float kMissing = -1.0f;
        const float     nx1      = data.get_float(sec, "nx1", kMissing);
        const float     ny1      = data.get_float(sec, "ny1", kMissing);
        const float     nx2      = data.get_float(sec, "nx2", kMissing);
        const float     ny2      = data.get_float(sec, "ny2", kMissing);
        if (nx1 < 0.0f || ny1 < 0.0f || nx2 < 0.0f || ny2 < 0.0f)
            continue;

        std::string name = data.get_string(sec, "name", "");
        if (name.empty())
            name = sec;

        float x1 = bounds.min_x + nx1 * bw;
        float y1 = bounds.min_y + ny1 * bh;
        float x2 = bounds.min_x + nx2 * bw;
        float y2 = bounds.min_y + ny2 * bh;
        if (x1 > x2) std::swap(x1, x2);
        if (y1 > y2) std::swap(y1, y2);

        std::string sprite = data.get_string(sec, "sprite", "");
        const float sw     = data.get_float(sec, "sw", 0.0f);
        const float sh     = data.get_float(sec, "sh", 0.0f);
        const float ax     = data.get_float(sec, "anchor_x", 0.5f);
        const float ay     = data.get_float(sec, "anchor_y", 1.0f);

        room.add_obstacle(std::move(name), x1, y1, x2, y2, std::move(sprite), sw, sh, ax, ay);
    }

    return !room.obstacles.empty();
}

} // namespace mion
