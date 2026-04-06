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
    # Floor (col 0): pedra escura com variação sutil e juntas
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

    # Grama (col 0)
    for y in range(H):
        for x in range(32):
            base = 55 + ((x * 3 + y * 7) % 10) - 5
            r = max(20, min(60, base - 10))
            g = max(50, min(110, base + 20))
            b = max(15, min(40, base - 15))
            buf_i = (y * W + x) * 4
            buf[buf_i:buf_i+4] = (r, g, b, 255)
    # Tufos de grama
    for i in range(6):
        gx = (i * 5 + 2) % 28
        gy = (i * 7 + 3) % 28
        spx(buf, W, H, gx, gy,     (30, 100, 28, 255))
        spx(buf, W, H, gx+1, gy-1, (28, 95,  25, 255))

    # Caminho/terra (col 1)
    for y in range(H):
        for x in range(32, 64):
            base = 80 + ((x * 3 + y * 5) % 12) - 6
            r = max(60, min(110, base + 10))
            g = max(55, min(100, base + 5))
            b = max(40, min(80, base - 10))
            buf_i = (y * W + x) * 4
            buf[buf_i:buf_i+4] = (r, g, b, 255)
    # Pedrinhas no caminho
    for i in range(5):
        px2 = 32 + (i * 6 + 3) % 26
        py2 = (i * 7 + 4) % 28
        circle(buf, W, H, px2, py2, 2, (100, 92, 70, 255))

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

    # --- Telhado duas águas: duas metades, cada uma inclinada para o centro ---
    mid = W // 2
    # Metade esquerda: y=0 no lado esquerdo (beirada), sobe até o centro
    for y in range(ROOF_H):
        t = y / float(ROOF_H - 1)
        left_start = 0
        left_end   = int(mid * t)
        fill(buf, W, H, left_start, y, left_end, y + 1, ROOF_C)
    # Metade direita: espelho
    for y in range(ROOF_H):
        t = y / float(ROOF_H - 1)
        right_start = W - int(mid * t)
        right_end   = W
        fill(buf, W, H, right_start, y, right_end, y + 1, ROOF_C)
    # Cume central (ridge)
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
    W, H = 200, 200
    buf = new_buf(W, H)
    cx, cy = W // 2, H // 2
    # Bacia externa
    circle(buf, W, H, cx, cy, 80, (70, 80, 100, 255))
    circle(buf, W, H, cx, cy, 74, (40, 60, 90, 255))
    # Água
    circle(buf, W, H, cx, cy, 70, (50, 100, 160, 200))
    # Reflexo na água
    for dy in range(-20, 20):
        for dx in range(-20, 20):
            if dx*dx + dy*dy < 300:
                ax = cx + dx + 15
                ay = cy + dy - 10
                if 0 <= ax < W and 0 <= ay < H:
                    i = (ay * W + ax) * 4
                    if buf[i+3] > 100:
                        buf[i] = min(255, buf[i] + 30)
                        buf[i+1] = min(255, buf[i+1] + 40)
                        buf[i+2] = min(255, buf[i+2] + 50)
    # Pilar central
    circle(buf, W, H, cx, cy, 14, (90, 85, 100, 255))
    circle(buf, W, H, cx, cy, 10, (110, 105, 120, 255))
    # Jatos d'água
    for angle_deg in [0, 90, 180, 270]:
        a = math.radians(angle_deg)
        for r in range(5, 28):
            wx = int(cx + r * math.cos(a))
            wy = int(cy + r * math.sin(a) - r * 0.4)
            alpha = max(80, 220 - r * 5)
            spx(buf, W, H, wx, wy, (160, 200, 255, alpha))
            spx(buf, W, H, wx+1, wy, (140, 180, 240, alpha // 2))
    # Base
    circle(buf, W, H, cx, cy + 30, 90, (65, 70, 85, 255))
    circle(buf, W, H, cx, cy + 30, 84, (55, 60, 75, 255))
    rect_outline(buf, W, H, cx - 85, cy - 55, cx + 85, cy + 85, (50, 55, 70, 255), 2)
    return W, H, bytes(buf)

# ---------------------------------------------------------------------------
# NPC sprites (Puny Characters layout 256x256, frame 32x32)
# Só Idle (cols 12-15) e Walk (cols 0-5) — NPCs não atacam
# ---------------------------------------------------------------------------

FW, FH = 32, 32
# Layout Puny Characters: 24 colunas × 8 linhas = 768×256
NCOLS, NROWS = 24, 8
NSW, NSH = FW * NCOLS, FH * NROWS  # 768 × 256

NPC_PALETTES: dict[str, dict[str, tuple[int,int,int,int]]] = {
    "mira": {
        "dress":   (180, 140, 60,  255),
        "skin":    (220, 180, 130, 255),
        "hair":    (80,  50,  20,  255),
        "outline": (100, 70,  20,  255),
        "shadow":  (40,  30,  10,  80),
        "accent":  (240, 200, 80,  255),
    },
    "forge_npc": {
        "dress":   (160, 80,  40,  255),
        "skin":    (200, 150, 110, 255),
        "hair":    (60,  40,  30,  255),
        "outline": (80,  40,  20,  255),
        "shadow":  (50,  20,  10,  80),
        "accent":  (220, 140, 60,  255),
    },
    "villager": {
        "dress":   (80,  130, 70,  255),
        "skin":    (210, 175, 130, 255),
        "hair":    (140, 110, 70,  255),
        "outline": (40,  70,  30,  255),
        "shadow":  (20,  40,  10,  80),
        "accent":  (120, 180, 100, 255),
    },
    "elder": {
        "dress":   (100, 110, 80,  255),
        "skin":    (200, 175, 140, 255),
        "hair":    (200, 200, 200, 255),
        "outline": (50,  55,  35,  255),
        "shadow":  (20,  25,  10,  80),
        "accent":  (160, 170, 130, 255),
    },
}

def _npc_draw_shadow(buf: bytearray, ox: int, oy: int, pal: dict) -> None:
    cx, cy = ox + FW // 2, oy + FH - 3
    for dy in range(-1, 2):
        for dx in range(-5, 6):
            if (dx/5)**2 + (dy/1.5)**2 <= 1.0:
                i = ((cy+dy)*NSW + (cx+dx)) * 4
                if 0 <= cx+dx < NSW and 0 <= cy+dy < NSH:
                    buf[i+3] = min(255, buf[i+3] + pal["shadow"][3])

def _npc_draw_frame(buf: bytearray, ox: int, oy: int, pal: dict,
                    bob: int, leg_phase: float, dir_name: str) -> None:
    cx = ox + FW // 2
    bot = oy + FH - 4

    lleg_y = bob + int(2 * math.sin(leg_phase))
    # Vestido/roupa
    for dy in range(8):
        width = 6 + dy // 2
        y = bot - dy + lleg_y
        fill(buf, NSW, NSH, cx - width, y, cx + width, y + 1, pal["dress"])

    # Torso
    fill(buf, NSW, NSH, cx - 4, oy + 10 + bob, cx + 4, oy + 18 + bob, pal["dress"])
    # Braços
    arm_swing = math.sin(leg_phase) * 0.3
    la = int(3 * arm_swing)
    fill(buf, NSW, NSH, cx - 7, oy + 11 + bob + la, cx - 4, oy + 17 + bob + la, pal["skin"])
    fill(buf, NSW, NSH, cx + 4, oy + 11 + bob - la, cx + 7, oy + 17 + bob - la, pal["skin"])
    # Cabeça
    for dy in range(-4, 5):
        for dx in range(-4, 5):
            if dx*dx + dy*dy <= 16:
                spx(buf, NSW, NSH, cx+dx, oy+6+bob+dy, pal["skin"])
    # Cabelo
    for dy in range(-4, 0):
        for dx in range(-4, 5):
            if dx*dx + dy*dy <= 16:
                spx(buf, NSW, NSH, cx+dx, oy+6+bob+dy, pal["hair"])
    # Detalhe accent
    fill(buf, NSW, NSH, cx - 2, oy + 11 + bob, cx + 2, oy + 14 + bob, pal["accent"])

def render_npc_sheet(pal: dict) -> bytes:
    buf = new_buf(NSW, NSH)
    DIR_ROWS = {"s": 0, "n": 2, "e": 3}
    EXTRA = {"s": [1], "n": [5], "e": [4, 6, 7]}

    for dir_name, base_row in DIR_ROWS.items():
        for row in [base_row] + EXTRA[dir_name]:
            oy = row * FH

            # Walk cols 0-5
            for f in range(6):
                ox = f * FW
                t = f / 6.0
                bob = int(1.5 * math.sin(t * math.pi * 2))
                leg_phase = t * math.pi * 2
                _npc_draw_shadow(buf, ox, oy, pal)
                _npc_draw_frame(buf, ox, oy, pal, bob, leg_phase, dir_name)

            # Idle cols 12-15
            for f in range(4):
                ox = (f + 12) * FW
                bob = int(1 * math.sin(f * math.pi / 2))
                _npc_draw_shadow(buf, ox, oy, pal)
                _npc_draw_frame(buf, ox, oy, pal, bob, 0.0, dir_name)

            # Attack cols 6-11 = igual idle (NPCs não atacam, mas layout precisa existir)
            for f in range(6):
                ox = (f + 6) * FW
                _npc_draw_shadow(buf, ox, oy, pal)
                _npc_draw_frame(buf, ox, oy, pal, 0, 0.0, dir_name)

            # Hurt col 19 = flash branco
            ox_h = 19 * FW
            _npc_draw_shadow(buf, ox_h, oy, pal)
            _npc_draw_frame(buf, ox_h, oy, pal, 0, 0.0, dir_name)
            for py in range(oy, oy + FH):
                for px in range(ox_h, ox_h + FW):
                    i = (py * NSW + px) * 4
                    if buf[i+3] > 80:
                        buf[i:i+4] = (255, 255, 255, 255)

            # Death cols 20-23
            for f in range(4):
                ox = (f + 20) * FW
                t = f / 3.0
                _npc_draw_shadow(buf, ox, oy, pal)
                alpha = max(0, int(255 * (1.0 - t * 0.7)))
                cx2 = ox + FW // 2
                cy2 = oy + FH - 8
                for dy in range(-8, 9):
                    for dx in range(-4, 5):
                        angle = t * math.pi / 2
                        rx = int(cx2 + dx * math.cos(angle) - dy * math.sin(angle))
                        ry = int(cy2 + dx * math.sin(angle) + dy * math.cos(angle))
                        c = (pal["dress"][0], pal["dress"][1], pal["dress"][2], alpha)
                        spx(buf, NSW, NSH, rx, ry, c)

    return bytes(buf)

# ---------------------------------------------------------------------------
# Specs
# ---------------------------------------------------------------------------

ASSETS: list[dict] = [
    # Tilesets
    {"path": "assets/tiles/dungeon_tileset.png", "fn": make_dungeon_tileset},
    {"path": "assets/tiles/corridor_floor.png", "fn": make_corridor_floor},
    {"path": "assets/tiles/town_tileset.png",    "fn": make_town_tileset},
    # Portas
    {"path": "assets/props/door_closed.png", "fn": make_door_closed},
    {"path": "assets/props/door_open.png",   "fn": make_door_open},
    # Buildings
    {"path": "assets/props/building_tavern.png", "fn": make_building_tavern},
    {"path": "assets/props/building_forge.png",  "fn": make_building_forge},
    {"path": "assets/props/building_elder.png",  "fn": make_building_elder},
    {"path": "assets/props/fountain.png",         "fn": make_fountain},
    # NPC sprites
    {"path": "assets/npcs/npc_mira.png",     "fn": lambda: (NSW, NSH, render_npc_sheet(NPC_PALETTES["mira"]))},
    {"path": "assets/npcs/npc_forge.png",    "fn": lambda: (NSW, NSH, render_npc_sheet(NPC_PALETTES["forge_npc"]))},
    {"path": "assets/npcs/npc_villager.png", "fn": lambda: (NSW, NSH, render_npc_sheet(NPC_PALETTES["villager"]))},
    {"path": "assets/npcs/npc_elder.png",    "fn": lambda: (NSW, NSH, render_npc_sheet(NPC_PALETTES["elder"]))},
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
