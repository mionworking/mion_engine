#!/usr/bin/env python3
"""
Gera sprite sheets animadas placeholder no layout Puny Characters.

Layout: 768x256 PNG, frame 32x32, 24 colunas × 8 linhas
  Col  0- 5 : Walk   (6 frames, loop)
  Col  6-11 : Attack (6 frames, one-shot)
  Col 12-15 : Idle   (4 frames, leve bob)
  Col 16-18 : Cast   (3 frames, magia)
  Col 19    : Hurt   (1 frame — flash)
  Col 20-23 : Death  (4 frames, queda)

Linhas por direção (Puny Characters):
  Row 0 = Sul (frente)
  Row 1 = Sul-variant
  Row 2 = Norte (costas)
  Row 3 = Leste (Oeste = espelho no renderer)
  Row 4-7 = extras/repetições

Estilos de corpo (body_style):
  "soldier"  — torso largo, capacete
  "orc"      — torso muito largo, ombros salientes, testa baixa
  "skeleton" — corpo fino, ossos visíveis
  "mage"     — capuz, bastão, silhueta mais esbelta
  "player"        — igual soldier mas com escudo no braço esquerdo
  "ghost"         — oval flutuante, sem pernas, semi-transparente
  "boss_grimjaw"  — orc imenso, armadura pesada, glow laranja
  "civil"         — civil simples, sem armadura, cabeca redonda

Uso:
  python3 tools/gen_animated_sprites.py
  python3 tools/gen_animated_sprites.py --overwrite
  python3 tools/gen_animated_sprites.py --only player
"""
from __future__ import annotations

import argparse
import math
import struct
import zlib
from pathlib import Path

# ---------------------------------------------------------------------------
# PNG writer
# ---------------------------------------------------------------------------

def _crc32(data: bytes) -> int:
    return zlib.crc32(data) & 0xFFFFFFFF


def _chunk(tag: bytes, payload: bytes) -> bytes:
    return struct.pack(">I", len(payload)) + tag + payload + struct.pack(">I", _crc32(tag + payload))


def write_png_rgba(path: Path, width: int, height: int, rgba: bytes) -> None:
    assert len(rgba) == width * height * 4
    raw = bytearray()
    stride = width * 4
    for y in range(height):
        raw.append(0)
        raw.extend(rgba[y * stride: y * stride + stride])
    compressed = zlib.compress(bytes(raw), 6)
    png = bytearray(b"\x89PNG\r\n\x1a\n")
    png.extend(_chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)))
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


def px(buf: bytearray, W: int, H: int, x: int, y: int, c: RGBA) -> None:
    if 0 <= x < W and 0 <= y < H:
        i = (y * W + x) * 4
        buf[i:i+4] = c


def fill(buf: bytearray, W: int, H: int, x0: int, y0: int, x1: int, y1: int, c: RGBA) -> None:
    for y in range(max(0, y0), min(H, y1)):
        for x in range(max(0, x0), min(W, x1)):
            i = (y * W + x) * 4
            buf[i:i+4] = c


def circle(buf: bytearray, W: int, H: int, cx: int, cy: int, r: int, c: RGBA) -> None:
    for y in range(cy - r, cy + r + 1):
        for x in range(cx - r, cx + r + 1):
            if (x - cx) ** 2 + (y - cy) ** 2 <= r * r:
                px(buf, W, H, x, y, c)


def ring(buf: bytearray, W: int, H: int, cx: int, cy: int, r: int, c: RGBA) -> None:
    """Círculo oco (borda)."""
    for y in range(cy - r - 1, cy + r + 2):
        for x in range(cx - r - 1, cx + r + 2):
            d2 = (x - cx) ** 2 + (y - cy) ** 2
            if (r - 1) ** 2 <= d2 <= (r + 1) ** 2:
                px(buf, W, H, x, y, c)


def line(buf: bytearray, W: int, H: int, x0: int, y0: int, x1: int, y1: int, c: RGBA) -> None:
    dx, dy = abs(x1 - x0), abs(y1 - y0)
    sx = 1 if x0 < x1 else -1
    sy = 1 if y0 < y1 else -1
    err = dx - dy
    while True:
        px(buf, W, H, x0, y0, c)
        if x0 == x1 and y0 == y1:
            break
        e2 = 2 * err
        if e2 > -dy:
            err -= dy
            x0 += sx
        if e2 < dx:
            err += dx
            y0 += sy


def blend_px(buf: bytearray, W: int, H: int, x: int, y: int, c: RGBA, alpha: int) -> None:
    if 0 <= x < W and 0 <= y < H:
        i = (y * W + x) * 4
        a = alpha / 255.0
        buf[i]   = int(buf[i]   * (1 - a) + c[0] * a)
        buf[i+1] = int(buf[i+1] * (1 - a) + c[1] * a)
        buf[i+2] = int(buf[i+2] * (1 - a) + c[2] * a)
        buf[i+3] = min(255, buf[i+3] + int(c[3] * a))


def _apply_outline(buf: bytearray, W: int, H: int, ox: int, oy: int, fw: int, fh: int, color: RGBA) -> None:
    # dois passes: coleta posições a preencher, depois escreve (evita cascade)
    to_fill: list[int] = []
    for y in range(max(0, oy), min(H, oy + fh)):
        for x in range(max(0, ox), min(W, ox + fw)):
            i = (y * W + x) * 4
            if buf[i + 3] > 10:
                continue
            for ddx, ddy in ((-1, 0), (1, 0), (0, -1), (0, 1)):
                nx, ny = x + ddx, y + ddy
                if 0 <= nx < W and 0 <= ny < H:
                    ni = (ny * W + nx) * 4
                    if buf[ni + 3] > 80:
                        to_fill.append(i)
                        break
    for i in to_fill:
        buf[i:i + 4] = color


def _apply_lighting(buf: bytearray, W: int, H: int, ox: int, oy: int, fw: int, fh: int, intensity: int) -> None:
    if intensity <= 0:
        return
    cx = ox + fw // 2
    cy = oy + fh // 2 - 2  # ligeiramente acima do centro, longe da sombra
    for y in range(max(0, oy), min(H, oy + fh)):
        for x in range(max(0, ox), min(W, ox + fw)):
            i = (y * W + x) * 4
            if buf[i + 3] < 80:
                continue
            dx = (x - cx) / (fw * 0.5)
            dy = (y - cy) / (fh * 0.5)
            # luz de cima-esquerda: dx negativo e dy negativo = mais brilho
            factor = (-dx * 0.6 - dy * 0.8) * 0.5
            delta = int(factor * intensity)
            buf[i]     = max(0, min(255, buf[i]     + delta))
            buf[i + 1] = max(0, min(255, buf[i + 1] + delta))
            buf[i + 2] = max(0, min(255, buf[i + 2] + delta))


def _post_frame(buf: bytearray, W: int, H: int, ox: int, oy: int, fw: int, fh: int, pal: dict) -> None:
    _apply_lighting(buf, W, H, ox, oy, fw, fh, pal.get("light_intensity", 30))
    _apply_outline(buf, W, H, ox, oy, fw, fh, pal["outline"])


# ---------------------------------------------------------------------------
# Palettes
# ---------------------------------------------------------------------------

PALETTES: dict[str, dict[str, RGBA]] = {
    "player": {
        "body":    (72,  120, 200, 255),
        "outline": (20,  40,  80,  255),
        "head":    (220, 180, 130, 255),
        "weapon":  (200, 200, 60,  255),
        "accent":  (255, 220, 80,  255),   # detalhes / escudo
        "hurt":    (255, 255, 255, 255),
        "shadow":  (20,  20,  40,  80),
        "style":   "soldier",
    },
    "skeleton": {
        "body":    (210, 210, 190, 255),
        "outline": (80,  70,  50,  255),
        "head":    (230, 230, 210, 255),
        "weapon":  (160, 140, 100, 255),
        "accent":  (180, 160, 100, 255),
        "hurt":    (255, 80,  80,  255),
        "shadow":  (20,  20,  20,  80),
        "style":   "skeleton",
    },
    "orc": {
        "body":    (80,  140, 60,  255),
        "outline": (30,  60,  20,  255),
        "head":    (100, 160, 80,  255),
        "weapon":  (140, 100, 60,  255),
        "accent":  (200, 80,  30,  255),   # detalhes de couro/metal
        "hurt":    (255, 255, 100, 255),
        "shadow":  (20,  40,  10,  80),
        "style":   "orc",
    },
    "mage_cyan": {
        "body":    (50,  160, 180, 255),
        "outline": (20,  60,  80,  255),
        "head":    (200, 220, 230, 255),
        "weapon":  (100, 240, 255, 255),
        "accent":  (0,   200, 255, 200),   # glow da magia
        "hurt":    (255, 255, 255, 255),
        "shadow":  (10,  40,  60,  80),
        "style":   "mage",
    },
    "mage_purple": {
        "body":    (130, 60,  180, 255),
        "outline": (50,  20,  80,  255),
        "head":    (210, 180, 230, 255),
        "weapon":  (220, 120, 255, 255),
        "accent":  (200, 80,  255, 200),   # glow roxo
        "hurt":    (255, 255, 255, 255),
        "shadow":  (40,  10,  60,  80),
        "style":   "mage",
    },
    "soldier_red": {
        "body":    (180, 50,  50,  255),
        "outline": (80,  20,  20,  255),
        "head":    (200, 140, 110, 255),
        "weapon":  (160, 160, 180, 255),
        "accent":  (220, 180, 50,  255),   # detalhes dourados
        "hurt":    (255, 220, 100, 255),
        "shadow":  (60,  10,  10,  80),
        "style":   "soldier",
    },
    "soldier_blue": {
        "body":    (50,  80,  180, 255),
        "outline": (20,  30,  80,  255),
        "head":    (190, 160, 130, 255),
        "weapon":  (180, 180, 200, 255),
        "accent":  (100, 180, 255, 255),
        "hurt":    (200, 220, 255, 255),
        "shadow":  (10,  20,  60,  80),
        "style":   "soldier",
    },
    "elite_skeleton": {
        "body":    (218, 218, 198, 255),
        "outline": (55,  45,  25,  255),
        "head":    (238, 238, 218, 255),
        "weapon":  (185, 30,  30,  255),   # lâmina vermelha / ensanguentada
        "accent":  (200, 170, 30,  255),   # coroa dourada
        "hurt":    (255, 60,  60,  255),
        "shadow":  (30,  10,  10,  80),
        "style":   "elite_skeleton",
        "light_intensity": 28,
    },
    "patrol_guard": {
        "body":    (38,  58,  138, 255),   # armadura azul-escura
        "outline": (14,  24,  68,  255),
        "head":    (178, 152, 118, 255),
        "weapon":  (158, 162, 178, 255),   # lança de metal
        "accent":  (78,  118, 198, 255),   # emblema / detalhes
        "hurt":    (178, 208, 255, 255),
        "shadow":  (10,  14,  50,  80),
        "style":   "patrol_guard",
        "light_intensity": 35,
    },
    "archer": {
        "body":    (68,  88,  48,  255),   # roupa verde/marrom de ranger
        "outline": (28,  38,  18,  255),
        "head":    (192, 158, 118, 255),
        "weapon":  (118, 82,  42,  255),   # arco de madeira
        "accent":  (158, 118, 58,  255),   # aljava / detalhes
        "hurt":    (218, 255, 178, 255),
        "shadow":  (24,  34,  14,  80),
        "style":   "archer",
        "light_intensity": 30,
    },
    "ghost": {
        "body":    (180, 220, 255, 155),  # azul fantasma semi-transparente
        "outline": (100, 160, 220, 200),
        "head":    (220, 245, 255, 200),
        "weapon":  (80,  190, 255, 220),  # pulso ectoplasmatico
        "accent":  (120, 200, 255, 220),  # glow frio
        "hurt":    (255, 255, 255, 240),
        "shadow":  (80,  100, 140,  35),
        "style":   "ghost",
        "light_intensity": 18,
    },
    "boss_grimjaw": {
        "body":    (65,  105,  45, 255),   # verde orc profundo
        "outline": (18,   40,   8, 255),
        "head":    (85,  135,  60, 255),
        "weapon":  (160,  80,  20, 255),   # machado de metal escuro
        "accent":  (255, 140,  30, 255),   # glow laranja boss
        "hurt":    (255, 200,  80, 255),
        "shadow":  ( 20,  35,  10, 100),
        "style":   "boss_grimjaw",
        "light_intensity": 50,
    },
    "npc_civil": {
        "body":    (155, 115,  65, 255),   # tunica marrom
        "outline": ( 75,  50,  25, 255),
        "head":    (210, 170, 130, 255),
        "weapon":  (155, 115,  65, 255),   # sem arma real
        "accent":  (185, 145,  85, 255),   # cinto / barra
        "hurt":    (255, 220, 200, 255),
        "shadow":  ( 40,  28,  12,  80),
        "style":   "civil",
        "light_intensity": 22,
    },
    # NPCs da town em estilo Puny — variacoes de soldier/mage com cores civis.
    "npc_mira": {
        "body":    (190, 150, 80,  255),   # vestido amarelo/dourado
        "outline": (110, 80,  40,  255),
        "head":    (225, 190, 145, 255),
        "weapon":  (150, 140, 160, 255),
        "accent":  (245, 215, 100, 255),
        "hurt":    (255, 255, 255, 255),
        "shadow":  (35,  30,  20,  80),
        "style":   "soldier",
    },
    "npc_forge": {
        "body":    (150, 90,  50,  255),   # avental marrom/laranja
        "outline": (90,  45,  25,  255),
        "head":    (210, 170, 130, 255),
        "weapon":  (160, 150, 130, 255),
        "accent":  (230, 140, 70,  255),
        "hurt":    (255, 240, 200, 255),
        "shadow":  (40,  25,  18,  80),
        "style":   "soldier",
    },
    "npc_villager": {
        "body":    (90,  140, 90,  255),   # tons de verde/terra
        "outline": (50,  80,  45,  255),
        "head":    (215, 180, 135, 255),
        "weapon":  (150, 150, 140, 255),
        "accent":  (130, 190, 120, 255),
        "hurt":    (240, 255, 240, 255),
        "shadow":  (25,  45,  25,  80),
        "style":   "soldier",
    },
    "npc_elder": {
        "body":    (110, 115, 95,  255),   # túnica neutra
        "outline": (60,  65,  45,  255),
        "head":    (220, 215, 200, 255),
        "weapon":  (170, 150, 200, 255),
        "accent":  (170, 180, 140, 255),
        "hurt":    (255, 255, 255, 255),
        "shadow":  (25,  25,  20,  80),
        "style":   "mage",
    },
}

# ---------------------------------------------------------------------------
# Shadow
# ---------------------------------------------------------------------------

def _draw_shadow(buf, W, H, ox, oy, fw, fh, pal):
    cx = ox + fw // 2
    cy = oy + fh - 3
    for dy in range(-2, 3):
        for dx in range(-7, 8):
            dist2 = (dx / 7.0) ** 2 + (dy / 2.0) ** 2
            if dist2 <= 1.0:
                a = int(pal["shadow"][3] * (1.0 - dist2))
                blend_px(buf, W, H, cx + dx, cy + dy, pal["shadow"], a)


# ---------------------------------------------------------------------------
# Body drawing por estilo
# ---------------------------------------------------------------------------

def _body_south(buf, W, H, ox, oy, fw, fh, pal, bob, leg_phase, arm_swing):
    style = pal.get("style", "soldier")
    cx = ox + fw // 2
    bot = oy + fh - 4

    if style == "orc":
        # Orc: torso muito largo, ombros grandes
        w = 7
        lleg_y = bob + int(3 * math.sin(leg_phase))
        rleg_y = bob + int(3 * math.sin(leg_phase + math.pi))
        fill(buf, W, H, cx - w, bot - 10 + lleg_y, cx - 1, bot + lleg_y, pal["body"])
        fill(buf, W, H, cx + 1, bot - 10 + rleg_y, cx + w, bot + rleg_y, pal["body"])
        fill(buf, W, H, cx - 8, oy + 9 + bob, cx + 8, oy + 19 + bob, pal["body"])
        # Ombros salientes
        fill(buf, W, H, cx - 11, oy + 9 + bob, cx - 8, oy + 14 + bob, pal["outline"])
        fill(buf, W, H, cx + 8,  oy + 9 + bob, cx + 11, oy + 14 + bob, pal["outline"])
        la = int(6 * arm_swing)
        fill(buf, W, H, cx - 11, oy + 9 + bob + la, cx - 8, oy + 17 + bob + la, pal["body"])
        fill(buf, W, H, cx + 8,  oy + 9 + bob - la, cx + 11, oy + 17 + bob - la, pal["body"])
        # Cabeça baixa e larga (testa de orc)
        circle(buf, W, H, cx, oy + 5 + bob, 6, pal["head"])
        # Presas
        px(buf, W, H, cx - 2, oy + 10 + bob, (255, 255, 240, 255))
        px(buf, W, H, cx + 2, oy + 10 + bob, (255, 255, 240, 255))

    elif style in ("skeleton", "elite_skeleton"):
        # Skeleton / Elite-Skeleton: fino, costelas visíveis
        w = 3
        lleg_y = bob + int(3 * math.sin(leg_phase))
        rleg_y = bob + int(3 * math.sin(leg_phase + math.pi))
        fill(buf, W, H, cx - 3, bot - 9 + lleg_y, cx - 1, bot + lleg_y, pal["body"])
        fill(buf, W, H, cx + 1, bot - 9 + rleg_y, cx + 3, bot + rleg_y, pal["body"])
        # Torso estreito + costelas
        fill(buf, W, H, cx - 3, oy + 10 + bob, cx + 3, oy + 19 + bob, pal["body"])
        for rib_y in [oy+11+bob, oy+13+bob, oy+15+bob]:
            line(buf, W, H, cx - 4, rib_y, cx + 4, rib_y, pal["outline"])
        la = int(4 * arm_swing)
        fill(buf, W, H, cx - 6, oy + 11 + bob + la, cx - 3, oy + 18 + bob + la, pal["outline"])
        fill(buf, W, H, cx + 3, oy + 11 + bob - la, cx + 6, oy + 18 + bob - la, pal["outline"])
        # Crânio
        circle(buf, W, H, cx, oy + 5 + bob, 5, pal["head"])
        if style == "elite_skeleton":
            # Glow vermelho nas órbitas
            circle(buf, W, H, cx - 2, oy + 5 + bob, 2, (200, 20, 20, 220))
            circle(buf, W, H, cx + 2, oy + 5 + bob, 2, (200, 20, 20, 220))
            # Coroa dourada
            crown_y = oy + 1 + bob
            fill(buf, W, H, cx - 4, crown_y + 2, cx + 5, crown_y + 4, pal["accent"])
            for cx_off, h in ((-3, 4), (0, 5), (3, 4)):
                for dh in range(h):
                    px(buf, W, H, cx + cx_off, crown_y + 1 - dh, pal["accent"])
        else:
            px(buf, W, H, cx - 2, oy + 5 + bob, (20, 20, 20, 255))
            px(buf, W, H, cx + 2, oy + 5 + bob, (20, 20, 20, 255))

    elif style == "archer":
        # Archer: capuz, corpo de ranger, pernas como soldier
        lleg_y = bob + int(3 * math.sin(leg_phase))
        rleg_y = bob + int(3 * math.sin(leg_phase + math.pi))
        fill(buf, W, H, cx - 4, bot - 8 + lleg_y, cx - 1, bot + lleg_y, pal["body"])
        fill(buf, W, H, cx + 1, bot - 8 + rleg_y, cx + 4, bot + rleg_y, pal["body"])
        fill(buf, W, H, cx - 4, oy + 10 + bob, cx + 4, oy + 18 + bob, pal["body"])
        la = int(5 * arm_swing)
        fill(buf, W, H, cx - 7, oy + 11 + bob + la, cx - 4, oy + 17 + bob + la, pal["body"])
        fill(buf, W, H, cx + 4, oy + 11 + bob - la, cx + 7, oy + 17 + bob - la, pal["body"])
        # Capuz (menor que mage, mais justo)
        circle(buf, W, H, cx, oy + 5 + bob, 5, pal["outline"])
        circle(buf, W, H, cx, oy + 6 + bob, 4, pal["body"])
        circle(buf, W, H, cx, oy + 7 + bob, 3, pal["head"])

    elif style == "mage":
        # Mage: capuz largo, manto
        w = 4
        lleg_y = bob + int(2 * math.sin(leg_phase))
        rleg_y = bob + int(2 * math.sin(leg_phase + math.pi))
        fill(buf, W, H, cx - 3, bot - 8 + lleg_y, cx - 1, bot + lleg_y, pal["outline"])
        fill(buf, W, H, cx + 1, bot - 8 + rleg_y, cx + 3, bot + rleg_y, pal["outline"])
        # Manto: trapézio mais largo em baixo
        fill(buf, W, H, cx - 5, oy + 10 + bob, cx + 5, oy + 19 + bob, pal["body"])
        fill(buf, W, H, cx - 6, oy + 14 + bob, cx + 6, oy + 20 + bob, pal["body"])
        la = int(4 * arm_swing)
        fill(buf, W, H, cx - 8, oy + 12 + bob + la, cx - 5, oy + 18 + bob + la, pal["body"])
        fill(buf, W, H, cx + 5, oy + 12 + bob - la, cx + 8, oy + 18 + bob - la, pal["body"])
        # Capuz
        circle(buf, W, H, cx, oy + 5 + bob, 6, pal["outline"])
        circle(buf, W, H, cx, oy + 6 + bob, 5, pal["body"])
        # Rosto no capuz (pequeno)
        circle(buf, W, H, cx, oy + 7 + bob, 3, pal["head"])

    elif style == "ghost":
        # Corpo oval flutuante, sem pernas, cauda wispy semi-transparente
        tail_base = bot
        for dt in range(6):
            ty = tail_base - dt + bob
            spread = dt + 1
            alpha = 30 + dt * 22
            c_tail = (pal["body"][0], pal["body"][1], pal["body"][2], alpha)
            for tx in range(cx - spread, cx + spread + 1):
                px(buf, W, H, tx, ty, c_tail)
        for dy in range(13):
            w_g = max(2, 5 - dy // 3)
            fill(buf, W, H, cx - w_g, oy + 10 + bob + dy, cx + w_g + 1, oy + 11 + bob + dy, pal["body"])
        la = int(4 * arm_swing)
        fill(buf, W, H, cx - 10, oy + 12 + bob + la, cx - 5, oy + 17 + bob + la, pal["body"])
        fill(buf, W, H, cx + 5,  oy + 12 + bob - la, cx + 10, oy + 17 + bob - la, pal["body"])
        circle(buf, W, H, cx, oy + 5 + bob, 5, pal["head"])
        fill(buf, W, H, cx - 3, oy + 3 + bob, cx - 1, oy + 6 + bob, (10, 10, 40, 230))
        fill(buf, W, H, cx + 1, oy + 3 + bob, cx + 3, oy + 6 + bob, (10, 10, 40, 230))

    elif style == "boss_grimjaw":
        # Orc boss: proporcoes enormes, armadura pesada, glow laranja
        lleg_y = bob + int(4 * math.sin(leg_phase))
        rleg_y = bob + int(4 * math.sin(leg_phase + math.pi))
        fill(buf, W, H, cx - 9,  bot - 12 + lleg_y, cx - 1, bot + lleg_y, pal["body"])
        fill(buf, W, H, cx + 1,  bot - 12 + rleg_y, cx + 9, bot + rleg_y, pal["body"])
        fill(buf, W, H, cx - 10, oy + 8  + bob, cx + 10, oy + 20 + bob, pal["body"])
        fill(buf, W, H, cx - 9,  oy + 9  + bob, cx + 9,  oy + 12 + bob, pal["outline"])
        fill(buf, W, H, cx - 9,  oy + 15 + bob, cx + 9,  oy + 18 + bob, pal["outline"])
        fill(buf, W, H, cx - 13, oy + 8  + bob, cx - 9,  oy + 15 + bob, pal["outline"])
        fill(buf, W, H, cx + 9,  oy + 8  + bob, cx + 13, oy + 15 + bob, pal["outline"])
        px(buf, W, H, cx - 12, oy + 11 + bob, pal["accent"])
        px(buf, W, H, cx + 12, oy + 11 + bob, pal["accent"])
        la = int(7 * arm_swing)
        fill(buf, W, H, cx - 13, oy + 9 + bob + la, cx - 9,  oy + 18 + bob + la, pal["body"])
        fill(buf, W, H, cx + 9,  oy + 9 + bob - la, cx + 13, oy + 18 + bob - la, pal["body"])
        circle(buf, W, H, cx, oy + 4 + bob, 7, pal["head"])
        fill(buf, W, H, cx - 3, oy + 10 + bob, cx - 1, oy + 13 + bob, (255, 252, 220, 255))
        fill(buf, W, H, cx + 1, oy + 10 + bob, cx + 3, oy + 13 + bob, (255, 252, 220, 255))
        fill(buf, W, H, cx - 4, oy + 3  + bob, cx - 1, oy + 5  + bob, pal["accent"])
        fill(buf, W, H, cx + 1, oy + 3  + bob, cx + 4, oy + 5  + bob, pal["accent"])

    elif style == "civil":
        # Tunica simples, sem armadura, cabeca redonda
        lleg_y = bob + int(3 * math.sin(leg_phase))
        rleg_y = bob + int(3 * math.sin(leg_phase + math.pi))
        fill(buf, W, H, cx - 3, bot - 8 + lleg_y, cx - 1, bot + lleg_y, pal["body"])
        fill(buf, W, H, cx + 1, bot - 8 + rleg_y, cx + 3, bot + rleg_y, pal["body"])
        fill(buf, W, H, cx - 4, oy + 10 + bob, cx + 4, oy + 18 + bob, pal["body"])
        fill(buf, W, H, cx - 5, oy + 13 + bob, cx + 5, oy + 18 + bob, pal["body"])
        line(buf, W, H, cx - 4, oy + 16 + bob, cx + 4, oy + 16 + bob, pal["accent"])
        la = int(4 * arm_swing)
        fill(buf, W, H, cx - 7, oy + 11 + bob + la, cx - 4, oy + 17 + bob + la, pal["body"])
        fill(buf, W, H, cx + 4, oy + 11 + bob - la, cx + 7, oy + 17 + bob - la, pal["body"])
        circle(buf, W, H, cx, oy + 5 + bob, 5, pal["head"])

    else:
        # Soldier / Player: torso médio
        w = 5
        lleg_y = bob + int(3 * math.sin(leg_phase))
        rleg_y = bob + int(3 * math.sin(leg_phase + math.pi))
        fill(buf, W, H, cx - w, bot - 8 + lleg_y, cx - 1, bot + lleg_y, pal["body"])
        fill(buf, W, H, cx + 1, bot - 8 + rleg_y, cx + w, bot + rleg_y, pal["body"])
        fill(buf, W, H, cx - w, oy + 10 + bob, cx + w, oy + 18 + bob, pal["body"])
        line(buf, W, H, cx - w, oy + 10 + bob, cx + w - 1, oy + 10 + bob, pal["outline"])
        line(buf, W, H, cx - w, oy + 17 + bob, cx + w - 1, oy + 17 + bob, pal["outline"])
        la = int(5 * arm_swing)
        fill(buf, W, H, cx - 8, oy + 11 + bob + la, cx - 5, oy + 17 + bob + la, pal["body"])
        fill(buf, W, H, cx + 5, oy + 11 + bob - la, cx + 8, oy + 17 + bob - la, pal["body"])
        # Escudo no braço esquerdo para player
        if style == "player":
            fill(buf, W, H, cx - 10, oy + 11 + bob + la, cx - 8, oy + 17 + bob + la, pal["accent"])
        # Capacete
        circle(buf, W, H, cx, oy + 6 + bob, 5, pal["head"])
        fill(buf, W, H, cx - 5, oy + 3 + bob, cx + 5, oy + 7 + bob, pal["body"])  # viseira
        circle(buf, W, H, cx, oy + 5 + bob, 5, pal["outline"])
        # Visor fechado (faixa horizontal) para patrol_guard
        if style == "patrol_guard":
            fill(buf, W, H, cx - 4, oy + 6 + bob, cx + 4, oy + 8 + bob, pal["outline"])


def _body_north(buf, W, H, ox, oy, fw, fh, pal, bob, leg_phase, arm_swing):
    style = pal.get("style", "soldier")
    cx = ox + fw // 2
    bot = oy + fh - 4

    if style == "orc":
        lleg_y = bob + int(3 * math.sin(leg_phase))
        rleg_y = bob + int(3 * math.sin(leg_phase + math.pi))
        fill(buf, W, H, cx - 7, bot - 10 + lleg_y, cx - 1, bot + lleg_y, pal["outline"])
        fill(buf, W, H, cx + 1, bot - 10 + rleg_y, cx + 7, bot + rleg_y, pal["outline"])
        fill(buf, W, H, cx - 8, oy + 9 + bob, cx + 8, oy + 19 + bob, pal["outline"])
        fill(buf, W, H, cx - 7, oy + 10 + bob, cx + 7, oy + 18 + bob, pal["body"])
        # Costas: marcação vertical
        line(buf, W, H, cx, oy + 10 + bob, cx, oy + 18 + bob, pal["outline"])
        la = int(6 * arm_swing)
        fill(buf, W, H, cx - 11, oy + 9 + bob - la, cx - 8, oy + 17 + bob - la, pal["body"])
        fill(buf, W, H, cx + 8,  oy + 9 + bob + la, cx + 11, oy + 17 + bob + la, pal["body"])
        # Cabeça de costas
        circle(buf, W, H, cx, oy + 5 + bob, 6, pal["head"])

    elif style in ("skeleton", "elite_skeleton"):
        lleg_y = bob + int(3 * math.sin(leg_phase))
        rleg_y = bob + int(3 * math.sin(leg_phase + math.pi))
        fill(buf, W, H, cx - 3, bot - 9 + lleg_y, cx - 1, bot + lleg_y, pal["outline"])
        fill(buf, W, H, cx + 1, bot - 9 + rleg_y, cx + 3, bot + rleg_y, pal["outline"])
        fill(buf, W, H, cx - 3, oy + 10 + bob, cx + 3, oy + 19 + bob, pal["body"])
        for rib_y in [oy+11+bob, oy+13+bob, oy+15+bob]:
            line(buf, W, H, cx - 4, rib_y, cx + 4, rib_y, pal["outline"])
        la = int(4 * arm_swing)
        fill(buf, W, H, cx - 6, oy + 11 + bob - la, cx - 3, oy + 18 + bob - la, pal["outline"])
        fill(buf, W, H, cx + 3, oy + 11 + bob + la, cx + 6, oy + 18 + bob + la, pal["outline"])
        # Crânio de costas
        circle(buf, W, H, cx, oy + 5 + bob, 5, pal["head"])
        # Coluna
        line(buf, W, H, cx, oy + 10 + bob, cx, oy + 19 + bob, pal["outline"])
        # Coroa visível de costas (elite)
        if style == "elite_skeleton":
            crown_y = oy + 1 + bob
            fill(buf, W, H, cx - 4, crown_y + 2, cx + 5, crown_y + 4, pal["accent"])
            for cx_off, h in ((-3, 4), (0, 5), (3, 4)):
                for dh in range(h):
                    px(buf, W, H, cx + cx_off, crown_y + 1 - dh, pal["accent"])

    elif style == "archer":
        lleg_y = bob + int(3 * math.sin(leg_phase))
        rleg_y = bob + int(3 * math.sin(leg_phase + math.pi))
        fill(buf, W, H, cx - 4, bot - 8 + lleg_y, cx - 1, bot + lleg_y, pal["body"])
        fill(buf, W, H, cx + 1, bot - 8 + rleg_y, cx + 4, bot + rleg_y, pal["body"])
        fill(buf, W, H, cx - 4, oy + 10 + bob, cx + 4, oy + 18 + bob, pal["body"])
        la = int(5 * arm_swing)
        fill(buf, W, H, cx - 7, oy + 11 + bob - la, cx - 4, oy + 17 + bob - la, pal["body"])
        fill(buf, W, H, cx + 4, oy + 11 + bob + la, cx + 7, oy + 17 + bob + la, pal["body"])
        # Aljava nas costas
        fill(buf, W, H, cx + 2, oy + 9 + bob, cx + 5, oy + 18 + bob, pal["accent"])
        # Capuz de costas
        circle(buf, W, H, cx, oy + 4 + bob, 5, pal["outline"])
        circle(buf, W, H, cx, oy + 5 + bob, 4, pal["body"])

    elif style == "mage":
        lleg_y = bob + int(2 * math.sin(leg_phase))
        rleg_y = bob + int(2 * math.sin(leg_phase + math.pi))
        fill(buf, W, H, cx - 3, bot - 8 + lleg_y, cx - 1, bot + lleg_y, pal["outline"])
        fill(buf, W, H, cx + 1, bot - 8 + rleg_y, cx + 3, bot + rleg_y, pal["outline"])
        fill(buf, W, H, cx - 5, oy + 10 + bob, cx + 5, oy + 19 + bob, pal["body"])
        fill(buf, W, H, cx - 6, oy + 14 + bob, cx + 6, oy + 20 + bob, pal["body"])
        la = int(4 * arm_swing)
        fill(buf, W, H, cx - 8, oy + 12 + bob - la, cx - 5, oy + 18 + bob - la, pal["body"])
        fill(buf, W, H, cx + 5, oy + 12 + bob + la, cx + 8, oy + 18 + bob + la, pal["body"])
        # Capuz de costas: ponta do capuz visível
        circle(buf, W, H, cx, oy + 4 + bob, 6, pal["outline"])
        circle(buf, W, H, cx, oy + 5 + bob, 5, pal["body"])
        # Ponta cônica
        for dy in range(4):
            w2 = max(1, 5 - dy)
            fill(buf, W, H, cx - w2, oy + bob - dy, cx + w2, oy + bob - dy + 1, pal["body"])

    elif style == "ghost":
        tail_base = bot
        for dt in range(6):
            ty = tail_base - dt + bob
            spread = dt + 1
            alpha = 30 + dt * 22
            c_tail = (pal["body"][0], pal["body"][1], pal["body"][2], alpha)
            for tx in range(cx - spread, cx + spread + 1):
                px(buf, W, H, tx, ty, c_tail)
        for dy in range(13):
            w_g = max(2, 5 - dy // 3)
            fill(buf, W, H, cx - w_g, oy + 10 + bob + dy, cx + w_g + 1, oy + 11 + bob + dy, pal["body"])
        la = int(4 * arm_swing)
        fill(buf, W, H, cx - 10, oy + 12 + bob - la, cx - 5, oy + 17 + bob - la, pal["body"])
        fill(buf, W, H, cx + 5,  oy + 12 + bob + la, cx + 10, oy + 17 + bob + la, pal["body"])
        circle(buf, W, H, cx, oy + 5 + bob, 5, pal["head"])

    elif style == "boss_grimjaw":
        lleg_y = bob + int(4 * math.sin(leg_phase))
        rleg_y = bob + int(4 * math.sin(leg_phase + math.pi))
        fill(buf, W, H, cx - 9,  bot - 12 + lleg_y, cx - 1, bot + lleg_y, pal["outline"])
        fill(buf, W, H, cx + 1,  bot - 12 + rleg_y, cx + 9, bot + rleg_y, pal["outline"])
        fill(buf, W, H, cx - 10, oy + 8  + bob, cx + 10, oy + 20 + bob, pal["outline"])
        fill(buf, W, H, cx - 9,  oy + 9  + bob, cx + 9,  oy + 19 + bob, pal["body"])
        line(buf, W, H, cx, oy + 9 + bob, cx, oy + 19 + bob, pal["outline"])
        fill(buf, W, H, cx - 8,  oy + 10 + bob, cx + 8,  oy + 12 + bob, pal["outline"])
        fill(buf, W, H, cx - 13, oy + 8  + bob, cx - 9,  oy + 15 + bob, pal["outline"])
        fill(buf, W, H, cx + 9,  oy + 8  + bob, cx + 13, oy + 15 + bob, pal["outline"])
        la = int(7 * arm_swing)
        fill(buf, W, H, cx - 13, oy + 9 + bob - la, cx - 9,  oy + 18 + bob - la, pal["body"])
        fill(buf, W, H, cx + 9,  oy + 9 + bob + la, cx + 13, oy + 18 + bob + la, pal["body"])
        circle(buf, W, H, cx, oy + 4 + bob, 7, pal["head"])

    elif style == "civil":
        lleg_y = bob + int(3 * math.sin(leg_phase))
        rleg_y = bob + int(3 * math.sin(leg_phase + math.pi))
        fill(buf, W, H, cx - 3, bot - 8 + lleg_y, cx - 1, bot + lleg_y, pal["body"])
        fill(buf, W, H, cx + 1, bot - 8 + rleg_y, cx + 3, bot + rleg_y, pal["body"])
        fill(buf, W, H, cx - 4, oy + 10 + bob, cx + 4, oy + 18 + bob, pal["body"])
        fill(buf, W, H, cx - 5, oy + 13 + bob, cx + 5, oy + 18 + bob, pal["body"])
        line(buf, W, H, cx - 4, oy + 16 + bob, cx + 4, oy + 16 + bob, pal["accent"])
        la = int(4 * arm_swing)
        fill(buf, W, H, cx - 7, oy + 11 + bob - la, cx - 4, oy + 17 + bob - la, pal["body"])
        fill(buf, W, H, cx + 4, oy + 11 + bob + la, cx + 7, oy + 17 + bob + la, pal["body"])
        circle(buf, W, H, cx, oy + 5 + bob, 5, pal["head"])

    else:
        # Soldier / Player / Patrol-Guard costas
        lleg_y = bob + int(3 * math.sin(leg_phase))
        rleg_y = bob + int(3 * math.sin(leg_phase + math.pi))
        fill(buf, W, H, cx - 5, bot - 8 + lleg_y, cx - 1, bot + lleg_y, pal["outline"])
        fill(buf, W, H, cx + 1, bot - 8 + rleg_y, cx + 5, bot + rleg_y, pal["outline"])
        fill(buf, W, H, cx - 5, oy + 10 + bob, cx + 5, oy + 18 + bob, pal["outline"])
        fill(buf, W, H, cx - 4, oy + 11 + bob, cx + 4, oy + 17 + bob, pal["body"])
        la = int(5 * arm_swing)
        fill(buf, W, H, cx - 8, oy + 11 + bob - la, cx - 5, oy + 17 + bob - la, pal["body"])
        fill(buf, W, H, cx + 5, oy + 11 + bob + la, cx + 8, oy + 17 + bob + la, pal["body"])
        # Costas do capacete / elmo
        circle(buf, W, H, cx, oy + 6 + bob, 5, pal["body"])
        fill(buf, W, H, cx - 5, oy + 3 + bob, cx + 5, oy + 7 + bob, pal["body"])
        circle(buf, W, H, cx, oy + 5 + bob, 5, pal["outline"])
        # Capa no player
        if style == "player":
            fill(buf, W, H, cx - 4, oy + 12 + bob, cx + 4, oy + 22 + bob, pal["accent"])


def _body_east(buf, W, H, ox, oy, fw, fh, pal, bob, leg_phase, arm_swing):
    style = pal.get("style", "soldier")
    cx = ox + fw // 2
    bot = oy + fh - 4

    if style == "orc":
        leg_f = int(5 * math.sin(leg_phase))
        # Pernas mais grossas
        fill(buf, W, H, cx - 4, bot - 11, cx + 1 + leg_f, bot, pal["body"])
        fill(buf, W, H, cx - 2, bot - 11, cx + 3 - leg_f, bot, pal["outline"])
        fill(buf, W, H, cx - 5, oy + 9 + bob, cx + 6, oy + 19 + bob, pal["body"])
        line(buf, W, H, cx - 5, oy + 9 + bob, cx + 5, oy + 9 + bob, pal["outline"])
        # Ombro grande
        fill(buf, W, H, cx + 4, oy + 8 + bob, cx + 9, oy + 14 + bob, pal["outline"])
        arm_y = int(5 * arm_swing)
        fill(buf, W, H, cx + 5, oy + 11 + bob + arm_y, cx + 9, oy + 18 + bob + arm_y, pal["body"])
        # Cabeça larga de perfil
        circle(buf, W, H, cx + 1, oy + 5 + bob, 6, pal["head"])
        # Maxilar saliente
        fill(buf, W, H, cx + 3, oy + 9 + bob, cx + 7, oy + 12 + bob, pal["head"])
        px(buf, W, H, cx + 7, oy + 10 + bob, (255, 255, 220, 255))  # presa lateral

    elif style in ("skeleton", "elite_skeleton"):
        leg_f = int(4 * math.sin(leg_phase))
        fill(buf, W, H, cx - 2, bot - 9, cx + 1 + leg_f, bot, pal["body"])
        fill(buf, W, H, cx - 1, bot - 9, cx + 3 - leg_f, bot, pal["outline"])
        fill(buf, W, H, cx - 3, oy + 10 + bob, cx + 3, oy + 19 + bob, pal["body"])
        for rib_y in [oy+11+bob, oy+13+bob, oy+15+bob]:
            line(buf, W, H, cx - 4, rib_y, cx + 4, rib_y, pal["outline"])
        arm_y = int(4 * arm_swing)
        fill(buf, W, H, cx + 3, oy + 11 + bob + arm_y, cx + 5, oy + 18 + bob + arm_y, pal["outline"])
        # Crânio de perfil
        circle(buf, W, H, cx + 1, oy + 5 + bob, 5, pal["head"])
        if style == "elite_skeleton":
            circle(buf, W, H, cx + 5, oy + 6 + bob, 2, (200, 20, 20, 220))
            # Coroa de perfil (1 ponta visível)
            fill(buf, W, H, cx - 2, oy + 1 + bob, cx + 4, oy + 3 + bob, pal["accent"])
            px(buf, W, H, cx + 1, oy + bob, pal["accent"])
        else:
            px(buf, W, H, cx + 5, oy + 6 + bob, (20, 20, 20, 255))  # olho oco
        # Mandíbula
        line(buf, W, H, cx - 1, oy + 9 + bob, cx + 5, oy + 9 + bob, pal["outline"])

    elif style == "archer":
        leg_f = int(4 * math.sin(leg_phase))
        fill(buf, W, H, cx - 3, bot - 8, cx + 1 + leg_f, bot, pal["body"])
        fill(buf, W, H, cx - 1, bot - 8, cx + 3 - leg_f, bot, pal["outline"])
        fill(buf, W, H, cx - 4, oy + 10 + bob, cx + 4, oy + 18 + bob, pal["body"])
        arm_y = int(4 * arm_swing)
        fill(buf, W, H, cx + 3, oy + 11 + bob + arm_y, cx + 6, oy + 17 + bob + arm_y, pal["body"])
        # Aljava visível no perfil
        fill(buf, W, H, cx - 6, oy + 10 + bob, cx - 3, oy + 18 + bob, pal["accent"])
        # Capuz de perfil
        circle(buf, W, H, cx + 1, oy + 6 + bob, 4, pal["outline"])
        circle(buf, W, H, cx + 1, oy + 7 + bob, 3, pal["body"])
        circle(buf, W, H, cx + 2, oy + 8 + bob, 2, pal["head"])

    elif style == "mage":
        leg_f = int(3 * math.sin(leg_phase))
        fill(buf, W, H, cx - 2, bot - 8, cx + 1 + leg_f, bot, pal["outline"])
        fill(buf, W, H, cx - 1, bot - 8, cx + 3 - leg_f, bot, pal["outline"])
        # Manto lateral
        fill(buf, W, H, cx - 4, oy + 10 + bob, cx + 4, oy + 20 + bob, pal["body"])
        arm_y = int(4 * arm_swing)
        fill(buf, W, H, cx + 3, oy + 11 + bob + arm_y, cx + 7, oy + 18 + bob + arm_y, pal["body"])
        # Capuz de perfil cônico
        circle(buf, W, H, cx + 1, oy + 6 + bob, 5, pal["outline"])
        circle(buf, W, H, cx + 1, oy + 7 + bob, 4, pal["body"])
        # Ponta do capuz para cima/trás
        line(buf, W, H, cx - 2, oy + 3 + bob, cx + 3, oy + 7 + bob, pal["outline"])
        # Rosto no capuz
        circle(buf, W, H, cx + 2, oy + 8 + bob, 3, pal["head"])

    elif style == "ghost":
        tail_base = bot
        for dt in range(5):
            ty = tail_base - dt + bob
            alpha = 40 + dt * 22
            c_tail = (pal["body"][0], pal["body"][1], pal["body"][2], alpha)
            for tx in range(cx - 1, cx + 2):
                px(buf, W, H, tx, ty, c_tail)
        fill(buf, W, H, cx - 4, oy + 10 + bob, cx + 5, oy + 21 + bob, pal["body"])
        arm_y = int(4 * arm_swing)
        fill(buf, W, H, cx + 4, oy + 12 + bob + arm_y, cx + 9, oy + 17 + bob + arm_y, pal["body"])
        circle(buf, W, H, cx + 1, oy + 5 + bob, 5, pal["head"])
        fill(buf, W, H, cx + 3, oy + 3 + bob, cx + 5, oy + 6 + bob, (10, 10, 40, 230))

    elif style == "boss_grimjaw":
        leg_f = int(6 * math.sin(leg_phase))
        fill(buf, W, H, cx - 5, bot - 13, cx + 2 + leg_f, bot, pal["body"])
        fill(buf, W, H, cx - 3, bot - 13, cx + 4 - leg_f, bot, pal["outline"])
        fill(buf, W, H, cx - 6, oy + 8  + bob, cx + 7,  oy + 21 + bob, pal["body"])
        fill(buf, W, H, cx - 5, oy + 9  + bob, cx + 6,  oy + 12 + bob, pal["outline"])
        fill(buf, W, H, cx + 5, oy + 7  + bob, cx + 10, oy + 15 + bob, pal["outline"])
        arm_y = int(6 * arm_swing)
        fill(buf, W, H, cx + 6, oy + 11 + bob + arm_y, cx + 10, oy + 19 + bob + arm_y, pal["body"])
        circle(buf, W, H, cx + 2, oy + 4 + bob, 7, pal["head"])
        fill(buf, W, H, cx + 5, oy + 9 + bob, cx + 9,  oy + 13 + bob, pal["head"])
        fill(buf, W, H, cx + 9, oy + 9 + bob, cx + 11, oy + 13 + bob, (255, 252, 220, 255))
        fill(buf, W, H, cx + 5, oy + 3 + bob, cx + 8,  oy + 5  + bob, pal["accent"])

    elif style == "civil":
        leg_f = int(4 * math.sin(leg_phase))
        fill(buf, W, H, cx - 2, bot - 8, cx + 2 + leg_f, bot, pal["body"])
        fill(buf, W, H, cx - 1, bot - 8, cx + 3 - leg_f, bot, pal["outline"])
        fill(buf, W, H, cx - 4, oy + 10 + bob, cx + 4, oy + 18 + bob, pal["body"])
        fill(buf, W, H, cx - 5, oy + 13 + bob, cx + 5, oy + 18 + bob, pal["body"])
        line(buf, W, H, cx - 5, oy + 16 + bob, cx + 4, oy + 16 + bob, pal["accent"])
        arm_y = int(4 * arm_swing)
        fill(buf, W, H, cx + 3, oy + 11 + bob + arm_y, cx + 6, oy + 17 + bob + arm_y, pal["body"])
        circle(buf, W, H, cx + 1, oy + 6 + bob, 4, pal["head"])

    else:
        # Soldier / Player / Patrol-Guard perfil
        leg_f = int(4 * math.sin(leg_phase))
        fill(buf, W, H, cx - 3, bot - 8, cx + 1 + leg_f, bot, pal["body"])
        fill(buf, W, H, cx - 1, bot - 8, cx + 3 - leg_f, bot, pal["outline"])
        fill(buf, W, H, cx - 4, oy + 10 + bob, cx + 4, oy + 18 + bob, pal["body"])
        line(buf, W, H, cx - 4, oy + 10 + bob, cx + 3, oy + 10 + bob, pal["outline"])
        line(buf, W, H, cx - 4, oy + 17 + bob, cx + 3, oy + 17 + bob, pal["outline"])
        arm_y = int(4 * arm_swing)
        fill(buf, W, H, cx + 3, oy + 11 + bob + arm_y, cx + 6, oy + 17 + bob + arm_y, pal["body"])
        # Escudo traseiro no player
        if style == "player":
            fill(buf, W, H, cx - 7, oy + 11 + bob, cx - 4, oy + 17 + bob, pal["accent"])
        # Cabeça / elmo perfil
        circle(buf, W, H, cx + 1, oy + 6 + bob, 4, pal["head"])
        fill(buf, W, H, cx - 2, oy + 3 + bob, cx + 4, oy + 7 + bob, pal["body"])  # elmo
        line(buf, W, H, cx - 2, oy + 3 + bob, cx + 4, oy + 7 + bob, pal["outline"])
        # Visor fechado no patrol_guard
        if style == "patrol_guard":
            fill(buf, W, H, cx, oy + 5 + bob, cx + 4, oy + 7 + bob, pal["outline"])


# ---------------------------------------------------------------------------
# Weapons / effects
# ---------------------------------------------------------------------------

def _weapon_south(buf, W, H, ox, oy, fw, fh, pal, bob, attack_t):
    cx = ox + fw // 2
    swing = int(10 * math.sin(attack_t * math.pi))
    wx = cx + 8 + swing
    wy = oy + 8 + bob - swing // 2
    line(buf, W, H, cx + 6, oy + 14 + bob, wx, wy, pal["weapon"])
    fill(buf, W, H, wx - 1, wy - 3, wx + 2, wy + 1, pal["weapon"])


def _weapon_east(buf, W, H, ox, oy, fw, fh, pal, bob, attack_t):
    cx = ox + fw // 2
    thrust = int(8 * attack_t)
    wx0, wy0 = cx + 5, oy + 13 + bob
    wx1, wy1 = cx + 12 + thrust, oy + 8 + bob - thrust // 2
    line(buf, W, H, wx0, wy0, wx1, wy1, pal["weapon"])
    fill(buf, W, H, wx1 - 1, wy1 - 1, wx1 + 2, wy1 + 2, pal["weapon"])


def _staff_south(buf, W, H, ox, oy, fw, fh, pal, bob, attack_t):
    cx = ox + fw // 2
    glow_r = int(3 + 2 * math.sin(attack_t * math.pi * 2))
    sx = cx + 9
    sy = oy + 6 + bob
    line(buf, W, H, cx + 6, oy + 16 + bob, sx, sy + 4, pal["weapon"])
    circle(buf, W, H, sx, sy, glow_r, (pal["weapon"][0], pal["weapon"][1], pal["weapon"][2], 200))
    ring(buf, W, H, sx, sy, glow_r + 1, pal["weapon"])


def _weapon_north(buf, W, H, ox, oy, fw, fh, pal, bob, attack_t):
    cx = ox + fw // 2
    swing = int(8 * math.sin(attack_t * math.pi))
    wx = cx + 4 + swing // 2
    wy = oy + 3 + bob - swing // 2
    line(buf, W, H, cx + 3, oy + 12 + bob, wx, wy, pal["weapon"])
    fill(buf, W, H, wx - 1, wy - 3, wx + 2, wy + 1, pal["weapon"])


def _staff_north(buf, W, H, ox, oy, fw, fh, pal, bob, attack_t):
    cx = ox + fw // 2
    glow_r = int(3 + 2 * math.sin(attack_t * math.pi * 2))
    sx = cx + 6
    sy = oy + 1 + bob - int(3 * attack_t)
    line(buf, W, H, cx + 4, oy + 14 + bob, sx, sy + 4, pal["weapon"])
    circle(buf, W, H, sx, sy, glow_r, (pal["weapon"][0], pal["weapon"][1], pal["weapon"][2], 200))
    ring(buf, W, H, sx, sy, glow_r + 1, pal["weapon"])


def _staff_east(buf, W, H, ox, oy, fw, fh, pal, bob, attack_t):
    cx = ox + fw // 2
    thrust = int(6 * attack_t)
    sx0, sy0 = cx + 4, oy + 16 + bob
    sx1, sy1 = cx + 10 + thrust, oy + 5 + bob - thrust // 3
    line(buf, W, H, sx0, sy0, sx1, sy1, pal["weapon"])
    glow_r = int(2 + 2 * attack_t)
    circle(buf, W, H, sx1, sy1, glow_r, (pal["weapon"][0], pal["weapon"][1], pal["weapon"][2], 200))


def _lance_south(buf, W, H, ox, oy, fw, fh, pal, bob, attack_t):
    cx = ox + fw // 2
    thrust = int(6 * math.sin(attack_t * math.pi))
    lx = cx + 9
    line(buf, W, H, lx, oy + fh - 8 + bob, lx, oy + 2 + bob - thrust, pal["weapon"])
    fill(buf, W, H, lx - 1, oy + 1 + bob - thrust, lx + 2, oy + 4 + bob - thrust, pal["weapon"])
    px(buf, W, H, lx, oy + bob - thrust, pal["accent"])


def _lance_north(buf, W, H, ox, oy, fw, fh, pal, bob, attack_t):
    cx = ox + fw // 2
    thrust = int(6 * math.sin(attack_t * math.pi))
    lx = cx + 7
    line(buf, W, H, lx, oy + fh - 8 + bob, lx, oy + 1 + bob - thrust, pal["weapon"])
    fill(buf, W, H, lx - 1, oy + bob - thrust, lx + 2, oy + 3 + bob - thrust, pal["weapon"])


def _lance_east(buf, W, H, ox, oy, fw, fh, pal, bob, attack_t):
    cx = ox + fw // 2
    thrust = int(8 * attack_t)
    ly = oy + 11 + bob
    line(buf, W, H, cx - 4, ly, cx + 14 + thrust, ly, pal["weapon"])
    fill(buf, W, H, cx + 13 + thrust, ly - 1, cx + 16 + thrust, ly + 2, pal["accent"])


def _bow_south(buf, W, H, ox, oy, fw, fh, pal, bob, attack_t):
    cx = ox + fw // 2
    bx = cx + 9
    by_top, by_bot = oy + 4 + bob, oy + 18 + bob
    for y in range(by_top, by_bot + 1):
        t = (y - by_top) / float(by_bot - by_top)
        bend = int(3 * math.sin(t * math.pi))
        px(buf, W, H, bx + bend, y, pal["weapon"])
    tension = int(4 * math.sin(attack_t * math.pi))
    mid_y = (by_top + by_bot) // 2
    line(buf, W, H, bx + 3, by_top, bx + 3 - tension, mid_y, pal["accent"])
    line(buf, W, H, bx + 3 - tension, mid_y, bx + 3, by_bot, pal["accent"])
    if attack_t < 0.8:
        line(buf, W, H, cx + 5, mid_y, bx + 3 - tension, mid_y, pal["accent"])


def _bow_north(buf, W, H, ox, oy, fw, fh, pal, bob, attack_t):
    cx = ox + fw // 2
    bx = cx + 7
    by_top, by_bot = oy + 3 + bob, oy + 16 + bob
    for y in range(by_top, by_bot + 1):
        t = (y - by_top) / float(by_bot - by_top)
        bend = int(3 * math.sin(t * math.pi))
        px(buf, W, H, bx + bend, y, pal["weapon"])
    tension = int(4 * math.sin(attack_t * math.pi))
    mid_y = (by_top + by_bot) // 2
    line(buf, W, H, bx + 3, by_top, bx + 3 - tension, mid_y, pal["accent"])
    line(buf, W, H, bx + 3 - tension, mid_y, bx + 3, by_bot, pal["accent"])


def _bow_east(buf, W, H, ox, oy, fw, fh, pal, bob, attack_t):
    cx = ox + fw // 2
    bx = cx + 9
    by_top, by_bot = oy + 5 + bob, oy + 17 + bob
    for y in range(by_top, by_bot + 1):
        t = (y - by_top) / float(by_bot - by_top)
        bend = int(2 * math.sin(t * math.pi))
        px(buf, W, H, bx + bend, y, pal["weapon"])
    tension = int(5 * math.sin(attack_t * math.pi))
    mid_y = (by_top + by_bot) // 2
    line(buf, W, H, bx, by_top, bx - tension, mid_y, pal["accent"])
    line(buf, W, H, bx - tension, mid_y, bx, by_bot, pal["accent"])
    if attack_t < 0.7:
        line(buf, W, H, bx - tension, mid_y, cx + 15, mid_y, pal["accent"])


def _no_weapon_south(buf, W, H, ox, oy, fw, fh, pal, bob, attack_t):
    # Gesto de susto/afastar: bracos levantados, sem arma
    cx = ox + fw // 2
    raise_a = int(6 * math.sin(attack_t * math.pi))
    fill(buf, W, H, cx - 9, oy + 9 + bob - raise_a, cx - 4, oy + 15 + bob - raise_a, pal["body"])
    fill(buf, W, H, cx + 4, oy + 9 + bob - raise_a, cx + 9, oy + 15 + bob - raise_a, pal["body"])


def _no_weapon_north(buf, W, H, ox, oy, fw, fh, pal, bob, attack_t):
    cx = ox + fw // 2
    raise_a = int(6 * math.sin(attack_t * math.pi))
    fill(buf, W, H, cx - 9, oy + 9 + bob - raise_a, cx - 4, oy + 15 + bob - raise_a, pal["body"])
    fill(buf, W, H, cx + 4, oy + 9 + bob - raise_a, cx + 9, oy + 15 + bob - raise_a, pal["body"])


def _no_weapon_east(buf, W, H, ox, oy, fw, fh, pal, bob, attack_t):
    cx = ox + fw // 2
    raise_a = int(6 * attack_t)
    fill(buf, W, H, cx + 3, oy + 9 + bob - raise_a, cx + 8, oy + 15 + bob - raise_a, pal["body"])


def _draw_cast_south(buf, W, H, ox, oy, fw, fh, pal, bob, cast_t):
    """Caster com braços levantados e orb de magia crescendo."""
    cx = ox + fw // 2
    # Braços levantados
    raise_l = int(8 * cast_t)
    raise_r = int(10 * cast_t)
    fill(buf, W, H, cx - 9, oy + 10 + bob - raise_l, cx - 5, oy + 15 + bob - raise_l, pal["body"])
    fill(buf, W, H, cx + 5, oy + 10 + bob - raise_r, cx + 9, oy + 15 + bob - raise_r, pal["body"])
    # Orb entre as mãos
    orb_r = int(3 + 4 * cast_t)
    orb_x = cx
    orb_y = oy + 4 + bob - int(4 * cast_t)
    alpha_orb = int(120 + 120 * cast_t)
    c = pal["accent"]
    circle(buf, W, H, orb_x, orb_y, orb_r, (c[0], c[1], c[2], alpha_orb))
    ring(buf, W, H, orb_x, orb_y, orb_r + 1, pal["weapon"])
    # Particulas — anel externo girando + contra-anel interno
    for i in range(4):
        angle = cast_t * math.pi * 2 + i * (math.pi / 2)
        px_x = orb_x + int((orb_r + 3) * math.cos(angle))
        px_y = orb_y + int((orb_r + 3) * math.sin(angle))
        px(buf, W, H, px_x, px_y, pal["weapon"])
    for i in range(4):
        angle2 = -cast_t * math.pi * 3 + i * (math.pi / 2) + math.pi / 4
        px_x2 = orb_x + int((orb_r + 1) * math.cos(angle2))
        px_y2 = orb_y + int((orb_r + 1) * math.sin(angle2))
        px(buf, W, H, px_x2, px_y2, pal["accent"])


def _draw_cast_north(buf, W, H, ox, oy, fw, fh, pal, bob, cast_t):
    cx = ox + fw // 2
    raise_l = int(8 * cast_t)
    raise_r = int(10 * cast_t)
    fill(buf, W, H, cx - 9, oy + 10 + bob - raise_l, cx - 5, oy + 15 + bob - raise_l, pal["body"])
    fill(buf, W, H, cx + 5, oy + 10 + bob - raise_r, cx + 9, oy + 15 + bob - raise_r, pal["body"])
    # Orb acima (visto de costas)
    orb_r = int(3 + 4 * cast_t)
    orb_x = cx
    orb_y = oy + 2 + bob - int(5 * cast_t)
    c = pal["accent"]
    circle(buf, W, H, orb_x, orb_y, orb_r, (c[0], c[1], c[2], int(120 + 120 * cast_t)))
    ring(buf, W, H, orb_x, orb_y, orb_r + 1, pal["weapon"])


def _draw_cast_east(buf, W, H, ox, oy, fw, fh, pal, bob, cast_t):
    cx = ox + fw // 2
    raise_a = int(8 * cast_t)
    fill(buf, W, H, cx + 5, oy + 9 + bob - raise_a, cx + 9, oy + 14 + bob - raise_a, pal["body"])
    # Orb à frente
    orb_r = int(2 + 4 * cast_t)
    orb_x = cx + 10 + int(2 * cast_t)
    orb_y = oy + 6 + bob - int(4 * cast_t)
    c = pal["accent"]
    circle(buf, W, H, orb_x, orb_y, orb_r, (c[0], c[1], c[2], int(120 + 120 * cast_t)))
    ring(buf, W, H, orb_x, orb_y, orb_r + 1, pal["weapon"])
    # Raio/linha de energia
    line(buf, W, H, cx + 9, oy + 12 + bob - raise_a, orb_x, orb_y + orb_r, pal["weapon"])
    # Contra-anel de particulas
    for i in range(4):
        angle2 = -cast_t * math.pi * 3 + i * (math.pi / 2) + math.pi / 4
        px(buf, W, H, orb_x + int((orb_r + 1) * math.cos(angle2)),
           orb_y + int((orb_r + 1) * math.sin(angle2)), pal["accent"])


def _draw_hurt_flash(buf, W, H, ox, oy, fw, fh, pal):
    for y in range(oy, min(H, oy + fh)):
        for x in range(ox, min(W, ox + fw)):
            i = (y * W + x) * 4
            if buf[i + 3] > 80:
                buf[i:i+4] = pal["hurt"]


def _draw_death(buf, W, H, ox, oy, fw, fh, pal, t):
    cx = ox + fw // 2
    cy = oy + fh - 6
    angle = t * math.pi / 2
    hw, hh = 5, 10
    cos_a, sin_a = math.cos(angle), math.sin(angle)
    for dy in range(-hh, hh + 1):
        for dx in range(-hw, hw + 1):
            rx = int(cx + dx * cos_a - dy * sin_a)
            ry = int(cy + dx * sin_a + dy * cos_a)
            # Bordas desvanecem mais rapido que o centro
            edge_dist = max(abs(dx) / float(hw), abs(dy) / float(hh))
            alpha = max(0, int(255 * (1.0 - t * 0.6 - edge_dist * 0.35 * t)))
            c: RGBA = (pal["body"][0], pal["body"][1], pal["body"][2], alpha)
            px(buf, W, H, rx, ry, c)
    head_x = int(cx + hh * cos_a + 4 * t)
    head_y = int(cy + hh * sin_a)
    head_a = max(0, int(255 * (1.0 - t * 0.7)))
    circle(buf, W, H, head_x, head_y, 4,
           (pal["head"][0], pal["head"][1], pal["head"][2], head_a))
    # Particulas de poeira espalhando ao cair
    if t > 0.25:
        spread = int(3 + t * 8)
        for i in range(4):
            angle_p = t * math.pi * 4 + i * (math.pi / 2)
            px_x = cx + int(spread * math.cos(angle_p))
            px_y = cy + int(spread * 0.5 * math.sin(angle_p))  # achatado no chao
            alpha_p = max(0, int(160 * (1.0 - t)))
            px(buf, W, H, px_x, px_y, (pal["body"][0], pal["body"][1], pal["body"][2], alpha_p))


# ---------------------------------------------------------------------------
# Sheet builder — 768×256 (COLS=24, SW=768)
# ---------------------------------------------------------------------------

FW, FH = 32, 32
COLS, ROWS = 24, 8
SW, SH = FW * COLS, FH * ROWS   # 768 × 256

DIR_ROWS = {"s": 0, "n": 2, "e": 3}
EXTRA_ROWS = {"s": [1], "n": [4, 5], "e": [6, 7]}


def render_sheet(pal: dict, weapon_fn_s, weapon_fn_n, weapon_fn_e) -> bytes:
    buf = new_buf(SW, SH)

    for dir_name, base_row in DIR_ROWS.items():
        for row in [base_row] + EXTRA_ROWS[dir_name]:
            oy = row * FH

            # --- Walk: cols 0-5 ---
            for f in range(6):
                ox = f * FW
                t = f / 6.0
                bob = int(1.5 * math.sin(t * math.pi * 2))
                leg_phase = t * math.pi * 2
                arm_swing = math.sin(t * math.pi * 2) * 0.5
                _draw_shadow(buf, SW, SH, ox, oy, FW, FH, pal)
                if dir_name == "s":
                    _body_south(buf, SW, SH, ox, oy, FW, FH, pal, bob, leg_phase, arm_swing)
                elif dir_name == "n":
                    _body_north(buf, SW, SH, ox, oy, FW, FH, pal, bob, leg_phase, arm_swing)
                else:
                    _body_east(buf, SW, SH, ox, oy, FW, FH, pal, bob, leg_phase, arm_swing)
                _post_frame(buf, SW, SH, ox, oy, FW, FH, pal)

            # --- Attack: cols 6-11 ---
            for f in range(6):
                ox = (f + 6) * FW
                t = f / 5.0
                # Antecipacao: corpo recua (bob negativo) antes do golpe
                attack_bob = int(-2 * math.sin(t * math.pi * 0.7))
                arm_strike = 0.5 * math.sin(t * math.pi)
                _draw_shadow(buf, SW, SH, ox, oy, FW, FH, pal)
                if dir_name == "s":
                    _body_south(buf, SW, SH, ox, oy, FW, FH, pal, attack_bob, 0.0, arm_strike)
                    weapon_fn_s(buf, SW, SH, ox, oy, FW, FH, pal, attack_bob, t)
                elif dir_name == "n":
                    _body_north(buf, SW, SH, ox, oy, FW, FH, pal, attack_bob, 0.0, arm_strike)
                    weapon_fn_n(buf, SW, SH, ox, oy, FW, FH, pal, attack_bob, t)
                else:
                    _body_east(buf, SW, SH, ox, oy, FW, FH, pal, attack_bob, 0.0, arm_strike)
                    weapon_fn_e(buf, SW, SH, ox, oy, FW, FH, pal, attack_bob, t)
                _post_frame(buf, SW, SH, ox, oy, FW, FH, pal)

            # --- Idle: cols 12-15 ---
            for f in range(4):
                ox = (f + 12) * FW
                breath = math.sin(f * math.pi / 2)
                bob = int(1 * breath)
                # Micro balanco de respiracao — bracos expandem no inale
                breath_arm = 0.07 * breath
                _draw_shadow(buf, SW, SH, ox, oy, FW, FH, pal)
                if dir_name == "s":
                    _body_south(buf, SW, SH, ox, oy, FW, FH, pal, bob, 0.0, breath_arm)
                elif dir_name == "n":
                    _body_north(buf, SW, SH, ox, oy, FW, FH, pal, bob, 0.0, breath_arm)
                else:
                    _body_east(buf, SW, SH, ox, oy, FW, FH, pal, bob, 0.0, breath_arm)
                _post_frame(buf, SW, SH, ox, oy, FW, FH, pal)

            # --- Cast: cols 16-18 ---
            for f in range(3):
                ox = (f + 16) * FW
                cast_t = f / 2.0  # 0, 0.5, 1.0
                bob = 0
                _draw_shadow(buf, SW, SH, ox, oy, FW, FH, pal)
                if dir_name == "s":
                    _body_south(buf, SW, SH, ox, oy, FW, FH, pal, bob, 0.0, 0.0)
                    _draw_cast_south(buf, SW, SH, ox, oy, FW, FH, pal, bob, cast_t)
                elif dir_name == "n":
                    _body_north(buf, SW, SH, ox, oy, FW, FH, pal, bob, 0.0, 0.0)
                    _draw_cast_north(buf, SW, SH, ox, oy, FW, FH, pal, bob, cast_t)
                else:
                    _body_east(buf, SW, SH, ox, oy, FW, FH, pal, bob, 0.0, 0.0)
                    _draw_cast_east(buf, SW, SH, ox, oy, FW, FH, pal, bob, cast_t)
                _post_frame(buf, SW, SH, ox, oy, FW, FH, pal)

            # --- Hurt: col 19 ---
            ox_hurt = 19 * FW
            _draw_shadow(buf, SW, SH, ox_hurt, oy, FW, FH, pal)
            if dir_name == "s":
                _body_south(buf, SW, SH, ox_hurt, oy, FW, FH, pal, 2, 0.0, 0.0)
            elif dir_name == "n":
                _body_north(buf, SW, SH, ox_hurt, oy, FW, FH, pal, 2, 0.0, 0.0)
            else:
                _body_east(buf, SW, SH, ox_hurt, oy, FW, FH, pal, 2, 0.0, 0.0)
            _post_frame(buf, SW, SH, ox_hurt, oy, FW, FH, pal)
            _draw_hurt_flash(buf, SW, SH, ox_hurt, oy, FW, FH, pal)

            # --- Death: cols 20-23 ---
            for f in range(4):
                ox = (f + 20) * FW
                t = f / 3.0
                _draw_shadow(buf, SW, SH, ox, oy, FW, FH, pal)
                _draw_death(buf, SW, SH, ox, oy, FW, FH, pal, t)
                _post_frame(buf, SW, SH, ox, oy, FW, FH, pal)

    return bytes(buf)


# ---------------------------------------------------------------------------
# Specs
# ---------------------------------------------------------------------------

SPRITES: list[dict] = [
    {
        "path":    "assets/Puny-Characters/Puny-Characters/Soldier-Red.png",
        "palette": "soldier_red",
        "weapon":  "melee",
    },
    {
        "path":    "assets/Puny-Characters/Puny-Characters/Orc-Grunt.png",
        "palette": "orc",
        "weapon":  "melee",
    },
    {
        "path":    "assets/Puny-Characters/Puny-Characters/Mage-Cyan.png",
        "palette": "mage_cyan",
        "weapon":  "staff",
    },
    {
        "path":    "assets/Puny-Characters/Puny-Characters/Mage-Purple.png",
        "palette": "mage_purple",
        "weapon":  "staff",
    },
    {
        "path":    "assets/Puny-Characters/Puny-Characters/Soldier-Blue.png",
        "palette": "soldier_blue",
        "weapon":  "melee",
    },
    {
        "path":    "assets/sprites/player.png",
        "palette": "player",
        "weapon":  "melee",
    },
    {
        "path":    "assets/Puny-Characters/Puny-Characters/Elite-Skeleton.png",
        "palette": "elite_skeleton",
        "weapon":  "melee",
    },
    {
        "path":    "assets/Puny-Characters/Puny-Characters/Patrol-Guard.png",
        "palette": "patrol_guard",
        "weapon":  "lance",
    },
    {
        "path":    "assets/Puny-Characters/Puny-Characters/Archer.png",
        "palette": "archer",
        "weapon":  "bow",
    },
    {
        "path":    "assets/Puny-Characters/Puny-Characters/Ghost.png",
        "palette": "ghost",
        "weapon":  "none",
    },
    {
        "path":    "assets/Puny-Characters/Puny-Characters/Boss-Grimjaw.png",
        "palette": "boss_grimjaw",
        "weapon":  "melee",
    },
    {
        "path":    "assets/npcs/npc_civil.png",
        "palette": "npc_civil",
        "weapon":  "none",
    },
    # NPCs da town em estilo Puny (usarao o mesmo rig de soldier/mage)
    {
        "path":    "assets/npcs/npc_mira.png",
        "palette": "npc_mira",
        "weapon":  "melee",
    },
    {
        "path":    "assets/npcs/npc_forge.png",
        "palette": "npc_forge",
        "weapon":  "melee",
    },
    {
        "path":    "assets/npcs/npc_villager.png",
        "palette": "npc_villager",
        "weapon":  "melee",
    },
    {
        "path":    "assets/npcs/npc_elder.png",
        "palette": "npc_elder",
        "weapon":  "staff",
    },
]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--overwrite", action="store_true")
    ap.add_argument("--only", default="", help="Gerar apenas paths que contenham este prefixo")
    args = ap.parse_args()

    root = Path(__file__).resolve().parents[1]

    created = skipped = 0
    for spec in SPRITES:
        rel = spec["path"]
        if args.only and args.only not in rel:
            continue
        out = root / rel
        if out.exists() and not args.overwrite:
            print(f"skip  {rel} (existe, use --overwrite)")
            skipped += 1
            continue

        pal = PALETTES[spec["palette"]]
        _wmap = {
            "staff": (_staff_south,      _staff_north,      _staff_east),
            "lance": (_lance_south,      _lance_north,      _lance_east),
            "bow":   (_bow_south,        _bow_north,        _bow_east),
            "melee": (_weapon_south,     _weapon_north,     _weapon_east),
            "none":  (_no_weapon_south,  _no_weapon_north,  _no_weapon_east),
        }
        fn_s, fn_n, fn_e = _wmap.get(spec["weapon"], _wmap["melee"])
        rgba = render_sheet(pal, weapon_fn_s=fn_s, weapon_fn_n=fn_n, weapon_fn_e=fn_e)
        write_png_rgba(out, SW, SH, rgba)
        print(f"write {rel}  ({SW}x{SH})")
        created += 1

    print(f"\nsummary: created={created} skipped={skipped}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
