#!/usr/bin/env python3
"""
Build a contact-sheet preview for placeholder textures listed in
tools/texture_manifest.json.

Output:
  assets/placeholders/preview.png
"""
from __future__ import annotations

import argparse
import json
import math
import struct
import zlib
from pathlib import Path


def _crc32(data: bytes) -> int:
    return zlib.crc32(data) & 0xFFFFFFFF


def _chunk(tag: bytes, payload: bytes) -> bytes:
    return (
        struct.pack(">I", len(payload))
        + tag
        + payload
        + struct.pack(">I", _crc32(tag + payload))
    )


def write_png_rgb(path: Path, width: int, height: int, rgb: bytes) -> None:
    if len(rgb) != width * height * 3:
        raise ValueError("invalid RGB size for dimensions")
    raw = bytearray()
    stride = width * 3
    for y in range(height):
        raw.append(0)
        row = y * stride
        raw.extend(rgb[row : row + stride])
    data = zlib.compress(bytes(raw), 9)
    png = bytearray()
    png.extend(b"\x89PNG\r\n\x1a\n")
    ihdr = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    png.extend(_chunk(b"IHDR", ihdr))
    png.extend(_chunk(b"IDAT", data))
    png.extend(_chunk(b"IEND", b""))
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(bytes(png))


def _set_px(buf: bytearray, w: int, h: int, x: int, y: int, c: tuple[int, int, int]) -> None:
    if x < 0 or y < 0 or x >= w or y >= h:
        return
    i = (y * w + x) * 3
    buf[i] = c[0]
    buf[i + 1] = c[1]
    buf[i + 2] = c[2]


def _fill_rect(
    buf: bytearray, w: int, h: int, x0: int, y0: int, x1: int, y1: int, c: tuple[int, int, int]
) -> None:
    xa = max(0, min(x0, x1))
    xb = min(w, max(x0, x1))
    ya = max(0, min(y0, y1))
    yb = min(h, max(y0, y1))
    for y in range(ya, yb):
        row = (y * w) * 3
        for x in range(xa, xb):
            i = row + x * 3
            buf[i] = c[0]
            buf[i + 1] = c[1]
            buf[i + 2] = c[2]


def draw_box(buf: bytearray, w: int, h: int, x: int, y: int, bw: int, bh: int, ok: bool) -> None:
    c_bg = (30, 34, 42)
    c_ok = (95, 190, 115)
    c_no = (210, 65, 65)
    _fill_rect(buf, w, h, x, y, x + bw, y + bh, c_bg)
    c = c_ok if ok else c_no
    t = 2
    _fill_rect(buf, w, h, x, y, x + bw, y + t, c)
    _fill_rect(buf, w, h, x, y + bh - t, x + bw, y + bh, c)
    _fill_rect(buf, w, h, x, y, x + t, y + bh, c)
    _fill_rect(buf, w, h, x + bw - t, y, x + bw, y + bh, c)


def copy_fit(
    dst: bytearray,
    dw: int,
    dh: int,
    src: bytes,
    sw: int,
    sh: int,
    x: int,
    y: int,
    bw: int,
    bh: int,
) -> None:
    if sw <= 0 or sh <= 0:
        return
    scale = min(bw / sw, bh / sh)
    tw = max(1, int(sw * scale))
    th = max(1, int(sh * scale))
    ox = x + (bw - tw) // 2
    oy = y + (bh - th) // 2

    for yy in range(th):
        sy = min(sh - 1, int(yy / scale))
        for xx in range(tw):
            sx = min(sw - 1, int(xx / scale))
            si = (sy * sw + sx) * 3
            _set_px(dst, dw, dh, ox + xx, oy + yy, (src[si], src[si + 1], src[si + 2]))


def load_png_rgb(path: Path) -> tuple[int, int, bytes] | None:
    # Minimal PNG reader for 8-bit RGB / RGBA / grayscale+alpha via zlib + no filter conversion complexity.
    # For reliability in this preview tool, fallback to None if format differs.
    data = path.read_bytes()
    if not data.startswith(b"\x89PNG\r\n\x1a\n"):
        return None
    p = 8
    width = height = 0
    color_type = -1
    idat = bytearray()
    while p + 12 <= len(data):
        ln = struct.unpack(">I", data[p : p + 4])[0]
        tag = data[p + 4 : p + 8]
        payload = data[p + 8 : p + 8 + ln]
        p += 12 + ln
        if tag == b"IHDR":
            width, height, bit_depth, color_type, *_ = struct.unpack(">IIBBBBB", payload)
            if bit_depth != 8:
                return None
        elif tag == b"IDAT":
            idat.extend(payload)
        elif tag == b"IEND":
            break
    if width <= 0 or height <= 0:
        return None
    raw = zlib.decompress(bytes(idat))
    bpp = {0: 1, 2: 3, 4: 2, 6: 4}.get(color_type)
    if bpp is None:
        return None

    out = bytearray(width * height * 3)
    stride = width * bpp
    rp = 0
    prev = bytearray(stride)
    cur = bytearray(stride)

    def paeth(a: int, b: int, c: int) -> int:
        p = a + b - c
        pa = abs(p - a)
        pb = abs(p - b)
        pc = abs(p - c)
        if pa <= pb and pa <= pc:
            return a
        if pb <= pc:
            return b
        return c

    for y in range(height):
        f = raw[rp]
        rp += 1
        cur[:] = raw[rp : rp + stride]
        rp += stride
        for i in range(stride):
            a = cur[i - bpp] if i >= bpp else 0
            b = prev[i]
            c = prev[i - bpp] if i >= bpp else 0
            if f == 1:
                cur[i] = (cur[i] + a) & 255
            elif f == 2:
                cur[i] = (cur[i] + b) & 255
            elif f == 3:
                cur[i] = (cur[i] + ((a + b) >> 1)) & 255
            elif f == 4:
                cur[i] = (cur[i] + paeth(a, b, c)) & 255
            elif f != 0:
                return None

        for x in range(width):
            si = x * bpp
            di = (y * width + x) * 3
            if color_type == 0:
                g = cur[si]
                out[di : di + 3] = bytes((g, g, g))
            elif color_type == 2:
                out[di : di + 3] = cur[si : si + 3]
            elif color_type == 4:
                g, a = cur[si], cur[si + 1]
                out[di] = (g * a + 18 * (255 - a)) // 255
                out[di + 1] = (g * a + 18 * (255 - a)) // 255
                out[di + 2] = (g * a + 18 * (255 - a)) // 255
            elif color_type == 6:
                r, g, b, a = cur[si : si + 4]
                out[di] = (r * a + 18 * (255 - a)) // 255
                out[di + 1] = (g * a + 18 * (255 - a)) // 255
                out[di + 2] = (b * a + 18 * (255 - a)) // 255
        prev, cur = cur, prev

    return width, height, bytes(out)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--manifest", default="tools/texture_manifest.json")
    ap.add_argument("--output", default="assets/placeholders/preview.png")
    ap.add_argument("--thumb", type=int, default=128)
    ap.add_argument("--cols", type=int, default=4)
    args = ap.parse_args()

    root = Path(__file__).resolve().parents[1]
    manifest = root / args.manifest
    data = json.loads(manifest.read_text(encoding="utf-8"))
    if "textures" in data and isinstance(data["textures"], list):
        textures = [x for x in data["textures"] if isinstance(x, dict) and x.get("path")]
    elif "groups" in data and isinstance(data["groups"], list):
        textures = []
        for g in data["groups"]:
            if not isinstance(g, dict):
                continue
            for item in g.get("items", []):
                if isinstance(item, dict) and item.get("path"):
                    textures.append(item)
    else:
        textures = []

    cols = max(1, args.cols)
    n = len(textures)
    rows = max(1, math.ceil(n / cols))
    pad = 14
    cell_w = args.thumb + 2 * pad
    cell_h = args.thumb + 2 * pad
    W = cols * cell_w + pad
    H = rows * cell_h + pad

    buf = bytearray(W * H * 3)
    _fill_rect(buf, W, H, 0, 0, W, H, (16, 18, 24))

    missing = 0
    for i, t in enumerate(textures):
        r = i // cols
        c = i % cols
        x = pad + c * cell_w
        y = pad + r * cell_h
        target = root / t["path"]
        ok = target.exists()
        draw_box(buf, W, H, x, y, cell_w - pad, cell_h - pad, ok)
        if not ok:
            missing += 1
            continue
        loaded = load_png_rgb(target)
        if loaded is None:
            missing += 1
            continue
        sw, sh, src = loaded
        copy_fit(buf, W, H, src, sw, sh, x + 6, y + 6, args.thumb - 12, args.thumb - 12)

    out = root / args.output
    write_png_rgb(out, W, H, bytes(buf))
    print(f"written {out}")
    print(f"textures={n} missing_or_unreadable={missing}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
