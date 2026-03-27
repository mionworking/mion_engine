#pragma once
#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>
#include <climits>
#include "../world/tilemap.hpp"
#include "../world/room.hpp"
#include "../entities/actor.hpp"  // para mion::Path

namespace mion {

// ---------------------------------------------------------------
// NavGrid — mapa de navegabilidade baked uma vez por sala
// ---------------------------------------------------------------
struct NavGrid {
    int cols      = 0;
    int rows      = 0;
    int tile_size = 32;
    std::vector<bool> walkable;  // [row * cols + col]

    // Constrói a partir do tilemap + obstáculos da sala
    void build(const Tilemap& map, const RoomDefinition& room) {
        cols      = map.cols;
        rows      = map.rows;
        tile_size = map.tile_size;
        walkable.assign(cols * rows, false);

        // Tiles de chão são transitáveis
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c)
                walkable[r * cols + c] = (map.get(c, r) == TileType::Floor);

        // Obstáculos bloqueiam seus tiles
        for (const auto& obs : room.obstacles) {
            int c0 = (int)(obs.bounds.min_x / tile_size);
            int r0 = (int)(obs.bounds.min_y / tile_size);
            int c1 = (int)(obs.bounds.max_x / tile_size);
            int r1 = (int)(obs.bounds.max_y / tile_size);
            for (int r = r0; r <= r1; ++r)
                for (int c = c0; c <= c1; ++c)
                    if (c >= 0 && c < cols && r >= 0 && r < rows)
                        walkable[r * cols + c] = false;
        }
    }

    bool is_walkable(int col, int row) const {
        if (col < 0 || col >= cols || row < 0 || row >= rows) return false;
        return walkable[row * cols + col];
    }

    void world_to_tile(float px, float py, int& col, int& row) const {
        col = std::max(0, std::min(cols - 1, (int)(px / tile_size)));
        row = std::max(0, std::min(rows - 1, (int)(py / tile_size)));
    }
};

// ---------------------------------------------------------------
// Pathfinder — A* sobre NavGrid, 4 direções
// Path está definida em actor.hpp (incluído acima)
// ---------------------------------------------------------------
struct Pathfinder {
    NavGrid nav;

    void build_nav(const Tilemap& map, const RoomDefinition& room) {
        nav.build(map, room);
    }

    // Encontra caminho de (sx,sy) até (gx,gy) em coordenadas de mundo.
    // Retorna Path com waypoints em centros de tiles.
    Path find_path(float sx, float sy, float gx, float gy) const {
        Path result;
        if (nav.cols == 0 || nav.rows == 0) return result;

        int sc, sr, gc, gr;
        nav.world_to_tile(sx, sy, sc, sr);
        nav.world_to_tile(gx, gy, gc, gr);

        if (sc == gc && sr == gr) {
            result.valid = true;  // já está no tile do destino
            return result;
        }

        const int N = nav.cols * nav.rows;
        std::vector<int>  g_cost(N, INT_MAX);
        std::vector<int>  parent(N, -1);
        std::vector<bool> closed(N, false);

        // Min-heap: (f, idx)
        using PQ = std::pair<int, int>;
        std::priority_queue<PQ, std::vector<PQ>, std::greater<PQ>> open;

        int start_idx = sr * nav.cols + sc;
        int goal_idx  = gr * nav.cols + gc;

        g_cost[start_idx] = 0;
        open.push({ std::abs(sc - gc) + std::abs(sr - gr), start_idx });

        const int dx[] = { 0,  0, 1, -1 };
        const int dy[] = { 1, -1, 0,  0 };

        while (!open.empty()) {
            auto [f, cur_idx] = open.top(); open.pop();
            (void)f;

            if (closed[cur_idx]) continue;
            closed[cur_idx] = true;

            if (cur_idx == goal_idx) {
                // Reconstrói o caminho do objetivo até o início
                std::vector<int> tiles;
                int idx = goal_idx;
                while (idx != start_idx) {
                    tiles.push_back(idx);
                    idx = parent[idx];
                    if (idx == -1) { return result; }  // falha (não deveria acontecer)
                }
                std::reverse(tiles.begin(), tiles.end());

                result.valid = true;
                result.waypoints.reserve(tiles.size());
                for (int ti : tiles) {
                    int col = ti % nav.cols;
                    int row = ti / nav.cols;
                    result.waypoints.push_back({
                        (col + 0.5f) * (float)nav.tile_size,
                        (row + 0.5f) * (float)nav.tile_size
                    });
                }
                return result;
            }

            int cur_col = cur_idx % nav.cols;
            int cur_row = cur_idx / nav.cols;
            int cur_g   = g_cost[cur_idx];

            for (int d = 0; d < 4; ++d) {
                int nc = cur_col + dx[d];
                int nr = cur_row + dy[d];
                if (!nav.is_walkable(nc, nr)) continue;
                int nidx = nr * nav.cols + nc;
                if (closed[nidx]) continue;
                int ng = cur_g + 1;
                if (ng < g_cost[nidx]) {
                    g_cost[nidx] = ng;
                    parent[nidx] = cur_idx;
                    int nh = std::abs(nc - gc) + std::abs(nr - gr);
                    open.push({ ng + nh, nidx });
                }
            }
        }

        return result;  // sem caminho
    }
};

} // namespace mion
