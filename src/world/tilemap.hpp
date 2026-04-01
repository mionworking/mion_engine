#pragma once
#include <vector>
#include <cstdint>

namespace mion {

enum class TileType : uint8_t {
    Void  = 0,  // outside the map — rendered black
    Floor = 1,  // walkable floor
    Wall  = 2,  // solid wall (blocks movement)
};

struct Tilemap {
    int   cols      = 0;
    int   rows      = 0;
    int   tile_size = 32;  // pixels per tile

    std::vector<TileType> tiles;  // [row * cols + col]

    void init(int c, int r, int ts = 32) {
        cols      = c;
        rows      = r;
        tile_size = ts;
        tiles.assign(c * r, TileType::Void);
    }

    TileType get(int col, int row) const {
        if (col < 0 || col >= cols || row < 0 || row >= rows)
            return TileType::Void;
        return tiles[row * cols + col];
    }

    void set(int col, int row, TileType t) {
        if (col < 0 || col >= cols || row < 0 || row >= rows) return;
        tiles[row * cols + col] = t;
    }

    // Fill a rectangle of tiles with the given type.
    void fill(int c0, int r0, int c1, int r1, TileType t) {
        for (int r = r0; r <= r1; ++r)
            for (int c = c0; c <= c1; ++c)
                set(c, r, t);
    }

    // Map dimensions in pixels.
    float world_width()  const { return (float)(cols * tile_size); }
    float world_height() const { return (float)(rows * tile_size); }

    // Returns the tile column and row that contain world position (px, py).
    void world_to_tile(float px, float py, int& col, int& row) const {
        col = (int)(px / tile_size);
        row = (int)(py / tile_size);
    }
};

} // namespace mion
