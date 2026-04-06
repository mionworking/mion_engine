#pragma once
#include <SDL3/SDL.h>
#include "../world/tilemap.hpp"
#include "../core/camera.hpp"
#include "../core/sprite.hpp"

namespace mion {

// Cores fallback por tile (usadas quando não há tileset PNG)
struct TileColors {
    Uint8 r, g, b;
};

static constexpr TileColors TILE_COLOR[] = {
    {  0,   0,   0},  // Void
    { 38,  32,  44},  // Floor — pedra escura (atmosfera Diablo)
    { 55,  48,  62},  // Wall  — pedra mais clara
};

// Offset sutil de cor para tiles alternados (quebra a monotonia do chão)
static constexpr int CHECKER_OFFSET = 4;

struct TilemapRenderer {
    // Textura do tileset (opcional).
    // Se nullptr, usa retângulos coloridos como fallback.
    // Suporta atlas 2D: configure floor/wall_tile_{col,row} para apontar
    // para o tile correto dentro da grade do tileset.
    SDL_Texture* tileset       = nullptr;
    int floor_tile_col = 0,  floor_tile_row = 0;  // padrão: col 0, linha 0
    int wall_tile_col  = 1,  wall_tile_row  = 0;  // padrão: col 1, linha 0

    // Chão alternativo (ex.: corredor town→dungeon). Se não-null, só o chão usa esta textura;
    // paredes continuam em `tileset`.
    SDL_Texture* floor_tileset = nullptr;
    // Faixa horizontal de N tiles de `tile_size` px: coluna fonte = (col % floor_strip_cols).
    int floor_strip_cols = 1;

    void render(SDL_Renderer* r,
                const Camera2D& cam,
                const Tilemap&  map) const
    {
        const float ts  = (float)map.tile_size;
        const int   scr_w = (int)cam.viewport_w;
        const int   scr_h = (int)cam.viewport_h;

        // Culling: calcula apenas os tiles visíveis pela câmera
        int col_start = (int)(cam.x / ts) - 1;
        int row_start = (int)(cam.y / ts) - 1;
        int col_end   = (int)((cam.x + scr_w) / ts) + 1;
        int row_end   = (int)((cam.y + scr_h) / ts) + 1;

        col_start = col_start < 0 ? 0 : col_start;
        row_start = row_start < 0 ? 0 : row_start;
        col_end   = col_end   > map.cols ? map.cols : col_end;
        row_end   = row_end   > map.rows ? map.rows : row_end;

        for (int row = row_start; row < row_end; ++row) {
            for (int col = col_start; col < col_end; ++col) {
                TileType type = map.get(col, row);
                if (type == TileType::Void) continue;

                float sx = cam.world_to_screen_x(col * ts);
                float sy = cam.world_to_screen_y(row * ts);
                SDL_FRect dst = { sx, sy, ts, ts };

                if (type == TileType::Floor && floor_tileset) {
                    int fc = floor_tile_col, fr = floor_tile_row;
                    if (floor_strip_cols > 1) {
                        fc = col % floor_strip_cols;
                        fr = 0;
                    }
                    SDL_FRect src = { (float)(fc * map.tile_size), (float)(fr * map.tile_size),
                                      (float)map.tile_size, (float)map.tile_size };
                    SDL_RenderTexture(r, floor_tileset, &src, &dst);
                } else if (tileset) {
                    // Tileset PNG: suporta atlas 2D via floor/wall_tile_{col,row}
                    int tc = (type == TileType::Floor) ? floor_tile_col : wall_tile_col;
                    int tr = (type == TileType::Floor) ? floor_tile_row : wall_tile_row;
                    SDL_FRect src = { (float)(tc * map.tile_size), (float)(tr * map.tile_size),
                                      (float)map.tile_size, (float)map.tile_size };
                    SDL_RenderTexture(r, tileset, &src, &dst);
                } else {
                    // Fallback: retângulo colorido
                    int idx = (int)type;
                    TileColors c = TILE_COLOR[idx];

                    // Checker sutil no chão para quebrar monotonia
                    if (type == TileType::Floor && (col + row) % 2 == 0) {
                        c.r += CHECKER_OFFSET;
                        c.g += CHECKER_OFFSET;
                        c.b += CHECKER_OFFSET;
                    }

                    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, 255);
                    SDL_RenderFillRect(r, &dst);
                }
            }
        }
    }
};

} // namespace mion
