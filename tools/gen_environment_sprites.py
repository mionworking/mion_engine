#!/usr/bin/env python3
"""
Gera sprites de ambiente placeholder para town e dungeon.

Assets gerados:
  Town:
    assets/tiles/town_floor.png          — tileset 64x32 (2 tiles 32x32: grama clara/escura)
    assets/props/building_tavern.png     — fachada 400x300
    assets/props/building_forge.png     — fachada 300x300
    assets/props/building_elder.png     — fachada 300x300
    assets/props/fountain.png           — fonte 200x200
    assets/npcs/npc_mira.png            — spritesheet 256x256 (Puny layout, 8x8, amarelo)
    assets/npcs/npc_forge.png           — spritesheet 256x256 (laranja)
    assets/npcs/npc_villager.png        — spritesheet 256x256 (verde)
    assets/npcs/npc_elder.png           — spritesheet 256x256 (verde acinzentado)

  Dungeon:
    assets/tiles/dungeon_tileset.png    — tileset 64x32 (floor col0 / wall col1)
    assets/tiles/corridor_floor.png     — faixa 192×32 (6 tiles) para corredor town→dungeon
    assets/props/door_closed.png        — porta 32x48
    assets/props/door_open.png          — porta aberta 32x48

Uso:
  python3 tools/gen_environment_sprites.py
  python3 tools/gen_environment_sprites.py --overwrite
"""
from __future__ import annotations

import argparse
import math
import struct
import zlib
from pathlib import Path

# ---------------------------------------------------------------------------
# PNG writer (stdlib, sem dependência)
# ---------------------------------------------------------------------------

def _crc32(data: bytes) -> int:
    return zlib.crc32(data) & 0xFFFFFFFF

def _chunk(tag: bytes, payload: bytes) -> bytes:
    return struct.pack(">I", len(payload)) + tag + payload + struct.pack(">I", _crc32(tag + payload))

def write_png_rgba(path: Path, w: int, h: int, rgba: bytes) -> None:
    assert len(rgba) == w * h * 4
    raw = bytearray()
    stride = w * 4
    for y in range(h):
        raw.append(0)
        raw.extend(rgba[y * stride: y * stride + stride])
    compressed = zlib.compress(bytes(raw), 6)
    png = bytearray(b"\x89PNG\r\n\x1a\n")
    png.extend(_chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0)))
    png.extend(_chunk(b"IDAT", compressed))
    png.extend(_chunk(b"IEND", b""))
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(bytes(png))

# ---------------------------------------------------------------------------
# Buffer helpers
# ---------------------------------------------------------------------------

RGBA = tuple[int, int, int, int]

def new_buf(w: int, h: int) -> bytearray:
    return bytearray(w * h * 4)

def spx(buf: bytearray, W: int, H: int, x: int, y: int, c: RGBA) -> None:
    if 0 <= x < W and 0 <= y < H:
        i = (y * W + x) * 4
        buf[i:i+4] = c

def fill(buf: bytearray, W: int, H: int, x0: int, y0: int, x1: int, y1: int, c: RGBA) -> None:
    for y in range(max(0, y0), min(H, y1)):
        for x in range(max(0, x0), min(W, x1)):
            i = (y * W + x) * 4
            buf[i:i+4] = c

def fill_alpha(buf: bytearray, W: int, H: int, x0: int, y0: int, x1: int, y1: int, c: RGBA, alpha: int) -> None:
    c2 = (c[0], c[1], c[2], alpha)
    fill(buf, W, H, x0, y0, x1, y1, c2)

def rect_outline(buf: bytearray, W: int, H: int, x0: int, y0: int, x1: int, y1: int, c: RGBA, t: int = 1) -> None:
    fill(buf, W, H, x0, y0, x1, y0 + t, c)
    fill(buf, W, H, x0, y1 - t, x1, y1, c)
    fill(buf, W, H, x0, y0, x0 + t, y1, c)
    fill(buf, W, H, x1 - t, y0, x1, y1, c)

def line(buf: bytearray, W: int, H: int, x0: int, y0: int, x1: int, y1: int, c: RGBA) -> None:
    dx, dy = abs(x1 - x0), abs(y1 - y0)
    sx = 1 if x0 < x1 else -1
    sy = 1 if y0 < y1 else -1
    err = dx - dy
    while True:
        spx(buf, W, H, x0, y0, c)
        if x0 == x1 and y0 == y1:
            break
        e2 = 2 * err
        if e2 > -dy:
            err -= dy; x0 += sx
        if e2 < dx:
            err += dx; y0 += sy

def circle(buf: bytearray, W: int, H: int, cx: int, cy: int, r: int, c: RGBA) -> None:
    for y in range(cy - r, cy + r + 1):
        for x in range(cx - r, cx + r + 1):
            if (x - cx)**2 + (y - cy)**2 <= r * r:
                spx(buf, W, H, x, y, c)

# ---------------------------------------------------------------------------
# Tileset 32x32 (2 tiles lado a lado = 64x32)
# col 0 = floor, col 1 = wall/alt
# ---------------------------------------------------------------------------

def make_dungeon_tileset() -> tuple[int, int, bytes]:
    W, H = 64, 32
    buf = new_buf(W, H)
    # Floor (col 0): pedra escura com variacao sutil e juntas
    for y in range(H):
        for x in range(32):
            base = 42 + ((x + y) % 2) * 5
            noise = ((x * 7 + y * 13) % 8) - 4
            v = max(20, min(80, base + noise))
            buf_i = (y * W + x) * 4
            buf[buf_i:buf_i+4] = (v, v - 4, v + 2, 255)
    # Linhas de junta no floor
    for x in range(0, 32, 8):
        for y in range(H):
            spx(buf, W, H, x, y, (22, 18, 26, 255))
    for y in range(0, H, 8):
        for x in range(32):
            spx(buf, W, H, x, y, (22, 18, 26, 255))
    # Rachaduras no floor (linhas diagonais finas)
    for x0, y0, x1, y1 in [(3, 5, 7, 9), (18, 14, 23, 18), (10, 2, 13, 6)]:
        line(buf, W, H, x0, y0, x1, y1, (18, 14, 22, 255))
    # Musgo nas juntas (tint verde sutil em algumas intersecoes)
    for gx, gy in [(0, 8), (16, 0), (24, 16), (8, 24)]:
        for dy in range(3):
            for dx in range(3):
                nx, ny = gx + dx, gy + dy
                if 0 <= nx < 32 and 0 <= ny < H:
                    i = (ny * W + nx) * 4
                    buf[i + 1] = min(255, buf[i + 1] + 14)
                    buf[i + 2] = max(0,   buf[i + 2] -  6)

    # Wall (col 1): pedra mais clara, textura de tijolo
    for y in range(H):
        for x in range(32, 64):
            base = 62 + ((x + y) % 3) * 4
            noise = ((x * 5 + y * 11) % 6) - 3
            v = max(40, min(100, base + noise))
            buf_i = (y * W + x) * 4
            buf[buf_i:buf_i+4] = (v, v - 6, v + 2, 255)
    # Linhas de tijolo na wall
    offset = 0
    for y in range(0, H, 8):
        for x in range(32, 64, 16):
            xo = x + offset
            if xo < 64:
                for yy in range(y, min(H, y + 8)):
                    spx(buf, W, H, xo, yy, (35, 28, 38, 255))
        for x in range(32, 64):
            spx(buf, W, H, x, y, (35, 28, 38, 255))
        offset = 8 - offset
    # Highlight na borda superior de cada tijolo (profundidade)
    for y in range(1, H, 8):
        for x in range(33, 64):
            i = (y * W + x) * 4
            buf[i]     = min(255, buf[i]     + 10)
            buf[i + 1] = min(255, buf[i + 1] +  8)
            buf[i + 2] = min(255, buf[i + 2] + 10)

    return W, H, bytes(buf)


def make_corridor_floor() -> tuple[int, int, bytes]:
    """Faixa horizontal 6×32 px (largura do corredor em tiles) no estilo do chão dungeon."""
    W, H = 192, 32
    buf = new_buf(W, H)
    for y in range(H):
        for x in range(W):
            bx = x % 32
            base = 42 + ((bx + y) % 2) * 5
            noise = ((x * 7 + y * 13) % 8) - 4
            v = max(20, min(80, base + noise))
            buf_i = (y * W + x) * 4
            buf[buf_i:buf_i + 4] = (v, v - 4, v + 2, 255)
    for x in range(0, W, 8):
        for y in range(H):
            spx(buf, W, H, x, y, (22, 18, 26, 255))
    for y in range(0, H, 8):
        for x in range(W):
            spx(buf, W, H, x, y, (22, 18, 26, 255))
    return W, H, bytes(buf)


def make_town_tileset() -> tuple[int, int, bytes]:
    """2 tiles 32x32: grama (col 0) e caminho/terra (col 1)."""
    W, H = 64, 32
    buf = new_buf(W, H)

    # Grama (col 0): variacao de verde mais rica
    for y in range(H):
        for x in range(32):
            base = 55 + ((x * 3 + y * 7) % 10) - 5
            patch = ((x * 5 + y * 11) % 3)  # micro-variacao de patch
            r = max(18, min(58, base - 12 + patch))
            g = max(52, min(115, base + 22 + patch * 2))
            b = max(12, min(38, base - 18))
            buf_i = (y * W + x) * 4
            buf[buf_i:buf_i+4] = (r, g, b, 255)
    # Tufos de grama (mais, variados)
    tuft_positions = [(2, 4), (9, 14), (14, 2), (20, 20), (5, 24), (26, 10),
                      (17, 8), (28, 26), (3, 18), (22, 4), (12, 28), (7, 8)]
    for gx, gy in tuft_positions:
        dark = 24 + (gx * 3) % 12
        spx(buf, W, H, gx,     gy,     (dark, 100 + dark, dark - 4, 255))
        spx(buf, W, H, gx + 1, gy - 1, (dark - 2, 95 + dark, dark - 6, 255))
        spx(buf, W, H, gx - 1, gy - 1, (dark + 2, 92 + dark, dark - 2, 255))
    # Flores pequenas (3 pixels coloridos)
    for fx, fy, fc in [(6, 6, (220, 220, 80, 255)), (22, 18, (220, 100, 100, 255)),
                       (15, 27, (240, 240, 255, 255))]:
        spx(buf, W, H, fx, fy, fc)

    # Caminho/terra (col 1): trilhos de carroça + pedrinhas variadas
    for y in range(H):
        for x in range(32, 64):
            base = 80 + ((x * 3 + y * 5) % 12) - 6
            r = max(62, min(112, base + 10))
            g = max(56, min(102, base + 5))
            b = max(38, min(78, base - 10))
            buf_i = (y * W + x) * 4
            buf[buf_i:buf_i+4] = (r, g, b, 255)
    # Trilhos de carroça (sulcos horizontais sutis)
    for y in [8, 22]:
        for x in range(33, 63):
            i = (y * W + x) * 4
            buf[i]     = max(0,   buf[i]     - 10)
            buf[i + 1] = max(0,   buf[i + 1] - 8)
            buf[i + 2] = max(0,   buf[i + 2] - 6)
    # Pedrinhas variadas
    stone_positions = [(35, 5, 2), (44, 18, 3), (52, 8, 2), (38, 26, 2), (58, 20, 3),
                       (42, 12, 1), (50, 28, 2), (33, 15, 1)]
    for px2, py2, r2 in stone_positions:
        circle(buf, W, H, px2, py2, r2, (100 + r2 * 4, 92 + r2 * 3, 70 + r2 * 2, 255))

    return W, H, bytes(buf)

# ---------------------------------------------------------------------------
# Portas
# ---------------------------------------------------------------------------

def make_door_closed() -> tuple[int, int, bytes]:
    W, H = 32, 48
    buf = new_buf(W, H)
    # Frame de pedra
    fill(buf, W, H, 0, 0, W, H, (55, 48, 62, 255))
    # Porta de madeira
    fill(buf, W, H, 3, 4, W - 3, H - 2, (80, 55, 30, 255))
    # Tábuas horizontais
    for y in range(4, H - 2, 7):
        fill(buf, W, H, 3, y, W - 3, y + 1, (60, 40, 20, 255))
    # Maçaneta
    circle(buf, W, H, W - 7, H // 2, 3, (180, 150, 50, 255))
    # Frame outline
    rect_outline(buf, W, H, 3, 4, W - 3, H - 2, (40, 28, 14, 255), 2)
    # Sombra no frame
    rect_outline(buf, W, H, 0, 0, W, H, (30, 26, 36, 255), 2)
    return W, H, bytes(buf)


def make_door_open() -> tuple[int, int, bytes]:
    W, H = 32, 48
    buf = new_buf(W, H)
    # Frame de pedra
    fill(buf, W, H, 0, 0, W, H, (55, 48, 62, 255))
    # Abertura escura (passagem)
    fill(buf, W, H, 4, 4, W - 4, H - 1, (10, 8, 14, 255))
    # Porta entreaberta na lateral esquerda
    fill(buf, W, H, 4, 4, 11, H - 1, (80, 55, 30, 255))
    for y in range(4, H - 1, 7):
        fill(buf, W, H, 4, y, 11, y + 1, (60, 40, 20, 255))
    # Glow leve no interior (luz vinda de dentro)
    for y in range(4, H - 1):
        for x in range(11, W - 4):
            alpha = max(0, 30 - abs(x - (W // 2)) * 2)
            if alpha > 0:
                i = (y * W + x) * 4
                buf[i] = min(255, buf[i] + alpha // 2)
                buf[i+1] = min(255, buf[i+1] + alpha // 3)
    # Frame outline
    rect_outline(buf, W, H, 0, 0, W, H, (30, 26, 36, 255), 2)
    return W, H, bytes(buf)

# ---------------------------------------------------------------------------
# Buildings da town
# ---------------------------------------------------------------------------

def make_building_tavern() -> tuple[int, int, bytes]:
    """Taverna: telhado triangular com ponta no TOPO (y=0), parede + porta na base."""
    W, H = 400, 300
    buf = new_buf(W, H)

    ROOF_H  = 70   # altura do telhado
    WALL_C  = (115, 88, 52, 255)   # parede bege-madeira
    ROOF_C  = (70, 40, 22, 255)    # telhado escuro
    ROOF_B  = (88, 52, 28, 255)    # beirada do telhado
    CHIMNEY = (72, 62, 58, 255)

    # --- Parede ---
    fill(buf, W, H, 0, ROOF_H, W, H, WALL_C)

    # Textura de tábuas verticais na parede
    for x in range(0, W, 20):
        fill(buf, W, H, x, ROOF_H, x + 2, H, (90, 68, 38, 255))

    # Sombra embaixo do beiral
    fill(buf, W, H, 0, ROOF_H, W, ROOF_H + 8, (70, 52, 30, 255))

    # --- Telhado triangular: ponta no centro topo, alarga até a base ---
    mid = W // 2
    for y in range(ROOF_H):
        t = y / float(ROOF_H - 1)       # 0.0 no topo, 1.0 na base
        half = int(t * (mid + 30))       # 0 no topo → mid+30 na base
        fill(buf, W, H, mid - half, y, mid + half, y + 1, ROOF_C)
        # Linha de destaque nas bordas do telhado
        if half > 2:
            spx(buf, W, H, mid - half,     y, ROOF_B)
            spx(buf, W, H, mid - half + 1, y, ROOF_B)
            spx(buf, W, H, mid + half - 1, y, ROOF_B)
            spx(buf, W, H, mid + half,     y, ROOF_B)

    # Beiral horizontal na junção telhado-parede
    fill(buf, W, H, 0, ROOF_H - 4, W, ROOF_H + 4, (58, 36, 18, 255))

    # --- Chaminé (sai do telhado) ---
    cx_ch = mid + 70
    fill(buf, W, H, cx_ch - 12, 0, cx_ch + 12, ROOF_H + 10, CHIMNEY)
    fill(buf, W, H, cx_ch - 15, 0, cx_ch + 15, 12, (55, 50, 48, 255))
    # Fumaça (círculos)
    for i, (ox2, oy2, r2, a) in enumerate([(-4, -8, 6, 160), (2, -16, 8, 120), (-2, -26, 10, 80)]):
        for dy in range(-r2, r2 + 1):
            for dx in range(-r2, r2 + 1):
                if dx*dx + dy*dy <= r2*r2:
                    spx(buf, W, H, cx_ch + ox2 + dx, oy2 + dy + ROOF_H // 2,
                        (180, 180, 185, a))

    # --- Janelas ---
    for wx in [55, 190, 325]:
        wy = ROOF_H + 28
        # Vidro com reflexo
        fill(buf, W, H, wx, wy, wx + 55, wy + 60, (140, 160, 120, 200))
        fill(buf, W, H, wx + 4, wy + 4, wx + 20, wy + 26, (180, 200, 160, 180))
        # Cruz de madeira
        fill(buf, W, H, wx + 26, wy, wx + 30, wy + 60, (65, 42, 22, 255))
        fill(buf, W, H, wx, wy + 28, wx + 55, wy + 32, (65, 42, 22, 255))
        rect_outline(buf, W, H, wx, wy, wx + 55, wy + 60, (55, 34, 16, 255), 3)
        # Peitoril
        fill(buf, W, H, wx - 4, wy + 60, wx + 59, wy + 66, (80, 52, 28, 255))

    # --- Porta dupla ---
    px0, px1 = 155, 245
    fill(buf, W, H, px0, H - 110, px1, H, (75, 50, 28, 255))
    fill(buf, W, H, px0 + 5, H - 105, px0 + (px1 - px0)//2 - 3, H, (82, 56, 32, 255))
    fill(buf, W, H, px0 + (px1 - px0)//2 + 3, H - 105, px1 - 5, H, (82, 56, 32, 255))
    # Divisor central
    fill(buf, W, H, (px0+px1)//2 - 2, H - 110, (px0+px1)//2 + 2, H, (50, 32, 16, 255))
    # Tábuas horizontais
    for y in range(H - 110, H, 16):
        fill(buf, W, H, px0, y, px1, y + 2, (55, 36, 18, 255))
    # Maçanetas
    circle(buf, W, H, px0 + (px1-px0)//2 - 10, H - 55, 4, (160, 130, 40, 255))
    circle(buf, W, H, px0 + (px1-px0)//2 + 10, H - 55, 4, (160, 130, 40, 255))
    rect_outline(buf, W, H, px0, H - 110, px1, H, (45, 28, 12, 255), 3)

    # Placa acima da porta
    fill(buf, W, H, 130, H - 125, 270, H - 112, (140, 100, 38, 255))
    rect_outline(buf, W, H, 130, H - 125, 270, H - 112, (80, 55, 18, 255), 2)

    # Outline geral da parede
    rect_outline(buf, W, H, 0, ROOF_H, W, H, (72, 50, 26, 255), 3)
    return W, H, bytes(buf)


def make_building_forge() -> tuple[int, int, bytes]:
    """Ferraria: telhado de duas águas (shed roof), chaminé larga, braseiro."""
    W, H = 300, 300
    buf = new_buf(W, H)

    ROOF_H  = 65
    WALL_C  = (82, 78, 88, 255)
    ROOF_C  = (50, 46, 56, 255)
    RIDGE_C = (35, 32, 42, 255)

    # --- Parede ---
    fill(buf, W, H, 0, ROOF_H, W, H, WALL_C)
    # Pedras na parede
    for row in range(ROOF_H, H, 24):
        offset = 12 if ((row - ROOF_H) // 24) % 2 == 0 else 0
        for col in range(-12, W, 24):
            x0, y0 = col + offset, row
            fill(buf, W, H, x0 + 1, y0 + 1, x0 + 22, y0 + 22,
                 (88, 84, 95, 255))
            rect_outline(buf, W, H, x0 + 1, y0 + 1, x0 + 22, y0 + 22,
                         (62, 58, 70, 255), 1)

    # --- Telhado duas aguas: solido com ridge central e shading direcional ---
    mid = W // 2
    fill(buf, W, H, 0, 0, W, ROOF_H, ROOF_C)
    # Face esquerda ligeiramente mais clara (sol da esquerda)
    for y in range(ROOF_H):
        for x in range(0, mid):
            i = (y * W + x) * 4
            buf[i]     = min(255, buf[i]     + 14)
            buf[i + 1] = min(255, buf[i + 1] + 12)
            buf[i + 2] = min(255, buf[i + 2] + 14)
    # Cume central
    fill(buf, W, H, mid - 4, 0, mid + 4, ROOF_H, RIDGE_C)
    # Beiral
    fill(buf, W, H, 0, ROOF_H - 5, W, ROOF_H + 5, (38, 34, 44, 255))

    # --- Chaminé larga (sai do cume) ---
    fill(buf, W, H, mid - 22, 0, mid + 22, ROOF_H + 18, (58, 54, 62, 255))
    fill(buf, W, H, mid - 26, 0, mid + 26, 10, (48, 44, 52, 255))
    # Fumaça
    for i, (ox2, oy2, r2, a) in enumerate([(0, -6, 7, 150), (-5, -15, 9, 110), (4, -25, 11, 70)]):
        for dy in range(-r2, r2 + 1):
            for dx in range(-r2, r2 + 1):
                if dx*dx + dy*dy <= r2*r2:
                    spx(buf, W, H, mid + ox2 + dx, oy2 + dy,
                        (160, 155, 165, a))

    # --- Janela com braseiro ---
    fill(buf, W, H, 25, ROOF_H + 30, 125, ROOF_H + 100, (18, 12, 14, 255))
    # Chamas
    for flame_y in range(ROOF_H + 60, ROOF_H + 100):
        t2 = (flame_y - (ROOF_H + 60)) / 40.0
        flame_w = int(28 * (1.0 - t2 * 0.7))
        r2 = int(220 * (1.0 - t2 * 0.5))
        g2 = int(80 + 60 * (1.0 - t2))
        fill(buf, W, H, 75 - flame_w, flame_y, 75 + flame_w, flame_y + 1,
             (r2, g2, 20, 220))
    rect_outline(buf, W, H, 25, ROOF_H + 30, 125, ROOF_H + 100, (40, 36, 44, 255), 3)

    # --- Janela direita ---
    fill(buf, W, H, 175, ROOF_H + 30, 265, ROOF_H + 90, (190, 170, 120, 150))
    fill(buf, W, H, 185, ROOF_H + 38, 205, ROOF_H + 62, (210, 195, 145, 170))
    fill(buf, W, H, 218, ROOF_H + 30, 222, ROOF_H + 90, (58, 52, 62, 255))
    fill(buf, W, H, 175, ROOF_H + 58, 265, ROOF_H + 62, (58, 52, 62, 255))
    rect_outline(buf, W, H, 175, ROOF_H + 30, 265, ROOF_H + 90, (42, 38, 48, 255), 3)

    # --- Porta ---
    fill(buf, W, H, 105, H - 110, 195, H, (62, 46, 32, 255))
    for y in range(H - 110, H, 14):
        fill(buf, W, H, 105, y, 195, y + 2, (48, 34, 22, 255))
    circle(buf, W, H, 182, H - 58, 4, (150, 120, 38, 255))
    rect_outline(buf, W, H, 105, H - 110, 195, H, (42, 30, 18, 255), 3)

    rect_outline(buf, W, H, 0, ROOF_H, W, H, (55, 50, 62, 255), 3)
    return W, H, bytes(buf)


def make_building_elder() -> tuple[int, int, bytes]:
    """Casa do Elder: telhado pontiagudo estilo gótico, ornamentos mágicos."""
    W, H = 300, 300
    buf = new_buf(W, H)

    ROOF_H  = 90
    WALL_C  = (62, 46, 80, 255)
    ROOF_C  = (42, 28, 62, 255)
    TRIM_C  = (88, 60, 120, 255)

    # --- Parede ---
    fill(buf, W, H, 0, ROOF_H, W, H, WALL_C)
    # Pedras aparelhadas
    for row in range(ROOF_H, H, 20):
        for col in range(0, W, 30):
            fill(buf, W, H, col + 1, row + 1, col + 28, row + 18,
                 (68, 50, 88, 255))
            rect_outline(buf, W, H, col + 1, row + 1, col + 28, row + 18,
                         (48, 34, 66, 255), 1)

    # --- Telhado pontiagudo: triângulo estreito, ponta afiada no topo ---
    mid = W // 2
    for y in range(ROOF_H):
        t = y / float(ROOF_H - 1)
        # Cresce de forma não-linear (mais abrupto no topo)
        half = int((mid + 10) * (t ** 0.7))
        fill(buf, W, H, mid - half, y, mid + half, y + 1, ROOF_C)
        # Bordas destacadas (trim mágico)
        if half > 1:
            spx(buf, W, H, mid - half,     y, TRIM_C)
            spx(buf, W, H, mid - half + 1, y, TRIM_C)
            spx(buf, W, H, mid + half - 1, y, TRIM_C)
            spx(buf, W, H, mid + half,     y, TRIM_C)

    # Beiral
    fill(buf, W, H, 0, ROOF_H - 5, W, ROOF_H + 6, (36, 24, 54, 255))

    # Ornamento no topo (pináculo)
    circle(buf, W, H, mid, 4, 5, (160, 120, 220, 255))
    circle(buf, W, H, mid, 4, 3, (200, 160, 255, 255))
    fill(buf, W, H, mid - 1, 4, mid + 2, 18, (130, 90, 180, 255))

    # --- Runas brilhantes nas paredes ---
    for rx, ry in [(50, ROOF_H+35), (W//2-8, ROOF_H+35), (W-62, ROOF_H+35),
                   (50, ROOF_H+100), (W-62, ROOF_H+100)]:
        circle(buf, W, H, rx, ry, 8, (100, 60, 150, 140))
        circle(buf, W, H, rx, ry, 5, (160, 110, 220, 200))
        circle(buf, W, H, rx, ry, 2, (220, 180, 255, 255))

    # --- Janela circular (rosácea) ---
    cx2, cy2 = W // 2, ROOF_H + 55
    circle(buf, W, H, cx2, cy2, 32, (28, 18, 42, 255))
    circle(buf, W, H, cx2, cy2, 26, (140, 100, 200, 180))
    # Divisões da rosácea
    for angle_deg in range(0, 360, 45):
        a = math.radians(angle_deg)
        for r2 in range(0, 27):
            x2 = int(cx2 + r2 * math.cos(a))
            y2 = int(cy2 + r2 * math.sin(a))
            spx(buf, W, H, x2, y2, (28, 18, 42, 255))
    circle(buf, W, H, cx2, cy2, 10, (80, 48, 120, 255))
    circle(buf, W, H, cx2, cy2, 5,  (200, 155, 255, 255))

    # --- Porta arqueada ---
    arch_cx = W // 2
    arch_top = H - 115
    arch_r   = 32
    # Retângulo inferior
    fill(buf, W, H, arch_cx - arch_r, arch_top + arch_r, arch_cx + arch_r, H, (20, 12, 32, 255))
    # Arco superior
    for y in range(arch_top, arch_top + arch_r):
        hw = int(math.sqrt(max(0, arch_r**2 - (y - arch_top - arch_r)**2)))
        fill(buf, W, H, arch_cx - hw, y, arch_cx + hw, y + 1, (20, 12, 32, 255))
    # Trim da porta
    for y in range(arch_top, H):
        hw = arch_r if y > arch_top + arch_r else int(math.sqrt(max(0, arch_r**2 - (y - arch_top - arch_r)**2)))
        spx(buf, W, H, arch_cx - hw,     y, TRIM_C)
        spx(buf, W, H, arch_cx - hw + 1, y, TRIM_C)
        spx(buf, W, H, arch_cx + hw - 1, y, TRIM_C)
        spx(buf, W, H, arch_cx + hw,     y, TRIM_C)
    # Keystone
    circle(buf, W, H, arch_cx, arch_top, 6, (160, 110, 220, 255))

    rect_outline(buf, W, H, 0, ROOF_H, W, H, (44, 28, 64, 255), 3)
    return W, H, bytes(buf)


def make_fountain() -> tuple[int, int, bytes]:
    """Fonte circular vista ligeiramente de cima — bacia de pedra com agua e jatos."""
    W, H = 200, 200
    buf = new_buf(W, H)
    cx, cy = W // 2, H // 2

    STONE_RIM  = (82,  88, 108, 255)   # borda externa da bacia
    STONE_WALL = (62,  68,  88, 255)   # parede interna
    WATER_D    = (38,  88, 155, 220)   # agua funda
    PILLAR_C   = (100, 105, 122, 255)

    # --- Bacia: anel de pedra visto de cima ---
    for dy in range(-82, 83):
        for dx in range(-88, 89):
            if (dx / 88.0)**2 + (dy / 82.0)**2 <= 1.0:
                spx(buf, W, H, cx + dx, cy + dy, STONE_RIM)

    for dy in range(-72, 73):
        for dx in range(-78, 79):
            if (dx / 78.0)**2 + (dy / 72.0)**2 <= 1.0:
                spx(buf, W, H, cx + dx, cy + dy, STONE_WALL)

    # --- Agua dentro da bacia ---
    for dy in range(-62, 63):
        for dx in range(-68, 69):
            if (dx / 68.0)**2 + (dy / 62.0)**2 <= 1.0:
                spx(buf, W, H, cx + dx, cy + dy, WATER_D)

    # Reflexo claro (canto superior esquerdo da agua)
    for dy in range(-35, 5):
        for dx in range(-40, 5):
            dist2 = (dx / 40.0)**2 + (dy / 35.0)**2
            if dist2 <= 1.0:
                alpha = int(70 * (1.0 - dist2))
                i = ((cy + dy) * W + (cx + dx)) * 4
                if 0 <= cx + dx < W and 0 <= cy + dy < H and buf[i + 3] > 100:
                    buf[i]     = min(255, buf[i]     + alpha)
                    buf[i + 1] = min(255, buf[i + 1] + alpha // 2)
                    buf[i + 2] = min(255, buf[i + 2] + alpha // 3)

    # Ripples (aneis de onda na agua)
    for r_rip in [22, 38, 52]:
        for angle_deg in range(0, 360, 3):
            a2 = math.radians(angle_deg)
            rx2 = int(cx + r_rip * math.cos(a2))
            ry2 = int(cy + r_rip * 0.88 * math.sin(a2))
            i = (ry2 * W + rx2) * 4
            if 0 <= rx2 < W and 0 <= ry2 < H and buf[i + 3] > 100:
                buf[i]     = min(255, buf[i]     + 18)
                buf[i + 1] = min(255, buf[i + 1] + 22)
                buf[i + 2] = min(255, buf[i + 2] + 35)

    # --- Pilar central ---
    circle(buf, W, H, cx, cy, 18, (68, 65, 82, 255))
    circle(buf, W, H, cx, cy, 13, PILLAR_C)
    circle(buf, W, H, cx, cy,  6, (120, 118, 135, 255))

    # --- Jatos d'agua em 4 direcoes (arcos parabolicos) ---
    for angle_deg in [45, 135, 225, 315]:
        a = math.radians(angle_deg)
        for r in range(10, 42):
            arc = math.sin(r / 41.0 * math.pi)  # elevacao parabolica
            wx = int(cx + r * math.cos(a))
            wy = int(cy + r * math.sin(a) - arc * 20)
            alpha = int(210 * arc)
            spx(buf, W, H, wx, wy, (165, 215, 255, alpha))
            spx(buf, W, H, wx + 1, wy, (145, 195, 245, alpha // 2))

    return W, H, bytes(buf)

# ---------------------------------------------------------------------------
# Dungeon props
# ---------------------------------------------------------------------------

def make_barrel() -> tuple[int, int, bytes]:
    """Barril de madeira com arcos de ferro — vista frontal ligeiramente elevada."""
    W, H = 64, 64
    buf = new_buf(W, H)
    cx = W // 2

    WOOD_L = (148, 102, 54, 255)
    WOOD_D = (102, 66, 28, 255)
    HOOP   = ( 52,  38, 16, 255)
    SHADE  = ( 68,  42, 16, 255)

    # Sombra eliptica embaixo
    for dy in range(-3, 4):
        for dx in range(-18, 19):
            if (dx / 18.0)**2 + (dy / 3.0)**2 <= 1.0:
                alpha = int(100 * (1.0 - (dx / 18.0)**2 - (dy / 3.0)**2))
                spx(buf, W, H, cx + dx + 2, 56 + dy, (10, 6, 2, alpha))

    # Tampa oval no topo
    for dy in range(-6, 7):
        for dx in range(-18, 19):
            if (dx / 18.0)**2 + (dy / 6.0)**2 <= 1.0:
                c = WOOD_L if dy < 0 else SHADE
                spx(buf, W, H, cx + dx, 13 + dy, c)

    # Corpo: lados levemente curvos (mais largo no meio)
    for y in range(14, 52):
        t = abs(2.0 * (y - 33) / 38.0)
        w_body = int(18 + 2 * max(0.0, 1.0 - t * t))
        for x in range(cx - w_body, cx + w_body + 1):
            spx(buf, W, H, x, y, WOOD_D)

    # Estaves verticais (linhas de madeira)
    for sx in [-12, -6, 0, 6, 12]:
        for y in range(14, 52):
            if abs(sx) <= 18:
                spx(buf, W, H, cx + sx, y, SHADE)

    # Arcos horizontais
    for hy in [16, 32, 50]:
        t = abs(2.0 * (hy - 33) / 38.0)
        w_h = int(18 + 2 * max(0.0, 1.0 - t * t)) + 1
        for x in range(cx - w_h, cx + w_h + 1):
            spx(buf, W, H, x, hy,     HOOP)
            spx(buf, W, H, x, hy + 1, HOOP)

    # Bung (rolha) no lado direito
    circle(buf, W, H, cx + 13, 33, 3, (66, 44, 18, 255))
    circle(buf, W, H, cx + 13, 33, 2, (44, 28, 10, 255))

    # Highlight na tampa (reflexo de luz)
    for dx in range(-8, 5):
        spx(buf, W, H, cx + dx - 3, 10, (185, 140, 80, 180))

    return W, H, bytes(buf)


def make_altar() -> tuple[int, int, bytes]:
    """Altar de pedra com velas acesas nos cantos e runa central."""
    W, H = 80, 80
    buf = new_buf(W, H)

    STONE_D = ( 60,  55,  72, 255)
    STONE_L = ( 88,  82, 102, 255)
    STONE_T = (100,  94, 116, 255)  # superficie do topo
    TRIM    = ( 44,  38,  58, 255)
    CANDLE  = (210, 195, 140, 255)
    FLAME_Y = (255, 200,  40, 240)
    FLAME_O = (255, 120,  20, 160)

    # Base/plinto (bloco mais largo e baixo)
    fill(buf, W, H, 6, 55, 74, 76, STONE_D)
    fill(buf, W, H, 8, 53, 72, 57, STONE_L)  # face frontal do plinto
    rect_outline(buf, W, H, 6, 53, 74, 76, TRIM, 2)

    # Mesa/altar (bloco central)
    fill(buf, W, H, 10, 30, 70, 55, STONE_D)
    fill(buf, W, H, 12, 28, 68, 32, STONE_T)  # superficie do topo
    rect_outline(buf, W, H, 10, 28, 70, 55, TRIM, 2)

    # Runa no topo — circulo + cruz
    rx, ry = W // 2, 36
    circle(buf, W, H, rx, ry, 8, (80, 50, 120, 160))
    circle(buf, W, H, rx, ry, 5, (140, 90, 200, 200))
    line(buf, W, H, rx, ry - 7, rx, ry + 7, (180, 120, 255, 220))
    line(buf, W, H, rx - 7, ry, rx + 7, ry, (180, 120, 255, 220))
    circle(buf, W, H, rx, ry, 2, (220, 180, 255, 255))

    # Velas nos 4 cantos do topo
    for cx2, cy2 in [(14, 18), (66, 18), (14, 52), (66, 52)]:
        # Corpo da vela
        fill(buf, W, H, cx2 - 2, cy2 - 10, cx2 + 3, cy2, CANDLE)
        # Pavio
        spx(buf, W, H, cx2, cy2 - 11, (40, 30, 20, 255))
        # Chama
        circle(buf, W, H, cx2, cy2 - 13, 3, FLAME_O)
        circle(buf, W, H, cx2, cy2 - 14, 2, FLAME_Y)
        spx(buf, W, H, cx2, cy2 - 16, (255, 240, 160, 200))
        # Glow laranja ao redor da chama
        for gdy in range(-5, 4):
            for gdx in range(-4, 5):
                dist2 = gdx * gdx + gdy * gdy
                if 4 < dist2 <= 20:
                    alpha = int(60 * (1.0 - dist2 / 20.0))
                    nx2, ny2 = cx2 + gdx, cy2 - 13 + gdy
                    if 0 <= nx2 < W and 0 <= ny2 < H:
                        i = (ny2 * W + nx2) * 4
                        buf[i]     = min(255, buf[i]     + alpha)
                        buf[i + 1] = min(255, buf[i + 1] + alpha // 3)

    return W, H, bytes(buf)


def make_pillar() -> tuple[int, int, bytes]:
    """Pilar de pedra de dungeon com capital, fuste e base."""
    W, H = 96, 128
    buf = new_buf(W, H)
    cx = W // 2

    STONE_D = ( 58,  54,  68, 255)
    STONE_M = ( 80,  76,  94, 255)
    STONE_L = (104, 100, 120, 255)
    TRIM    = ( 38,  34,  50, 255)
    CRACK   = ( 32,  28,  42, 255)

    def _stone_fill(x0: int, y0: int, x1: int, y1: int, base: RGBA) -> None:
        for y in range(max(0, y0), min(H, y1)):
            for x in range(max(0, x0), min(W, x1)):
                noise = ((x * 7 + y * 13) % 8) - 4
                v = noise * 2
                i = (y * W + x) * 4
                buf[i]     = max(0, min(255, base[0] + v))
                buf[i + 1] = max(0, min(255, base[1] + v))
                buf[i + 2] = max(0, min(255, base[2] + v))
                buf[i + 3] = 255

    # Base (mais larga)
    _stone_fill(cx - 22, 108, cx + 22, 126, STONE_M)
    fill(buf, W, H, cx - 22, 106, cx + 22, 110, STONE_L)  # face superior da base
    rect_outline(buf, W, H, cx - 22, 106, cx + 22, 126, TRIM, 2)

    # Fuste (coluna principal)
    _stone_fill(cx - 14, 24, cx + 14, 108, STONE_D)
    # Highlight vertical (luz da esquerda)
    for y in range(24, 108):
        spx(buf, W, H, cx - 12, y, STONE_M)
        spx(buf, W, H, cx - 11, y, STONE_L)
    rect_outline(buf, W, H, cx - 14, 24, cx + 14, 108, TRIM, 1)

    # Capital (mais largo no topo)
    _stone_fill(cx - 20, 10, cx + 20, 28, STONE_M)
    fill(buf, W, H, cx - 20, 8, cx + 20, 12, STONE_L)  # face superior do capital
    rect_outline(buf, W, H, cx - 20, 8, cx + 20, 28, TRIM, 2)
    # Detalhe esculpido no capital
    fill(buf, W, H, cx - 18, 16, cx + 18, 18, TRIM)

    # Rachadura diagonal no fuste
    line(buf, W, H, cx + 4, 40, cx + 8, 60, CRACK)
    line(buf, W, H, cx + 8, 60, cx + 5, 80, CRACK)

    # Sombra ao pe do pilar
    for dy in range(4):
        for dx in range(-20, 21):
            alpha = int(80 * (1.0 - dy / 4.0) * (1.0 - abs(dx) / 20.0))
            if 0 <= cx + dx < W and 126 + dy < H:
                i = ((126 + dy) * W + cx + dx) * 4
                buf[i + 3] = max(buf[i + 3], alpha)

    return W, H, bytes(buf)


# ---------------------------------------------------------------------------
# Specs
# ---------------------------------------------------------------------------

ASSETS: list[dict] = [
    # Tilesets
    {"path": "assets/tiles/dungeon_tileset.png", "fn": make_dungeon_tileset},
    {"path": "assets/tiles/corridor_floor.png",  "fn": make_corridor_floor},
    {"path": "assets/tiles/town_tileset.png",    "fn": make_town_tileset},
    # Portas
    {"path": "assets/props/door_closed.png", "fn": make_door_closed},
    {"path": "assets/props/door_open.png",   "fn": make_door_open},
    # Buildings
    {"path": "assets/props/building_tavern.png", "fn": make_building_tavern},
    {"path": "assets/props/building_forge.png",  "fn": make_building_forge},
    {"path": "assets/props/building_elder.png",  "fn": make_building_elder},
    {"path": "assets/props/fountain.png",        "fn": make_fountain},
    # Props de dungeon
    {"path": "assets/props/barrel.png",          "fn": make_barrel},
    {"path": "assets/props/altar.png",           "fn": make_altar},
    {"path": "assets/props/dungeon_pillar.png",  "fn": make_pillar},
]

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--overwrite", action="store_true")
    args = ap.parse_args()

    root = Path(__file__).resolve().parents[1]
    created = skipped = 0

    for spec in ASSETS:
        out = root / spec["path"]
        if out.exists() and not args.overwrite:
            print(f"skip  {spec['path']} (existe, use --overwrite)")
            skipped += 1
            continue
        w, h, rgba = spec["fn"]()
        write_png_rgba(out, w, h, rgba)
        print(f"write {spec['path']}  ({w}x{h})")
        created += 1

    print(f"\nsummary: created={created} skipped={skipped}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
