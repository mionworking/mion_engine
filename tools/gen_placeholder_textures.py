#!/usr/bin/env python3
"""
Generate PNG placeholder textures that conform to tools/texture_contract_inventory.md.

All character sprite sheets (Player, NPC, Monster) share a base grid of
GRID_BASE_PX x GRID_BASE_PX pixels per frame so the bounding-box remains
consistent across entity types and visual de-sync in the engine is avoided.

Usage:
  python3 tools/gen_placeholder_textures.py
  python3 tools/gen_placeholder_textures.py --overwrite
  python3 tools/gen_placeholder_textures.py --only assets/props
  python3 tools/gen_placeholder_textures.py --dry-run
  python3 tools/gen_placeholder_textures.py --include-backlog
"""
from __future__ import annotations

import argparse
import json
import os
import struct
import zlib
from pathlib import Path


# All character sprite sheets must use this as their per-frame cell size.
# Width and Height of the sheet must be multiples of this value.
GRID_BASE_PX: int = 32

PALETTES = {
    # === environment ===
    "dungeon_prop":   ((120, 90,  50),  (190, 150, 80),  (45,  30,  18)),
    "dungeon_stone":  ((72,  76,  88),  (122, 132, 156), (22,  24,  30)),
    # === player ===
    "default":        ((96,  96,  96),  (180, 180, 180), (30,  30,  30)),
    # === NPCs (warmer, friendly tones) ===
    "npc_townfolk":   ((60,  80,  110), (130, 170, 210), (25,  35,  55)),
    "npc_merchant":   ((100, 75,  30),  (200, 160, 70),  (40,  28,  10)),
    # === monsters (enemy types) ===
    "enemy_red":      ((110, 34,  34),  (220, 80,  80),  (40,  18,  18)),
    "enemy_orc":      ((56,  92,  44),  (136, 188, 96),  (24,  40,  18)),
    "enemy_cyan":     ((40,  86,  100), (92,  196, 220), (16,  34,  38)),
    "enemy_purple":   ((72,  52,  108), (158, 112, 220), (30,  22,  44)),
    "enemy_blue":     ((44,  62,  120), (92,  132, 228), (18,  26,  50)),
    "enemy_skeleton": ((160, 148, 130), (220, 210, 195), (55,  48,  40)),
    "enemy_boss":     ((80,  20,  20),  (200, 50,  50),  (180, 160, 0)),
    # === FX / UI ===
    "fx_magic":       ((30,  20,  80),  (120, 80,  240), (15,  10,  40)),
    "fx_physical":    ((120, 80,  20),  (240, 200, 60),  (50,  30,  5)),
}


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
        raw.append(0)  # filter type: None
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


def _check_grid_alignment(tex: dict) -> list[str]:
    """Return warning strings if a character sprite sheet violates GRID_BASE_PX."""
    warnings: list[str] = []
    kind = tex.get("kind", "")
    path = tex.get("path", "")
    if kind != "sprite_sheet":
        return warnings
    # Only character sheets (Puny Characters path OR sprites/player) need grid check.
    if "Puny-Characters" not in path and "sprites/" not in path:
        return warnings
    w = int(tex.get("width",  0))
    h = int(tex.get("height", 0))
    fx = int(tex.get("frames_x", 1))
    fy = int(tex.get("frames_y", 1))
    cw = w // fx if fx else 0
    ch = h // fy if fy else 0
    if cw != GRID_BASE_PX:
        warnings.append(
            f"  GRID WARN {path}: frame width {cw}px != GRID_BASE_PX {GRID_BASE_PX}px "
            f"(sheet {w}x{h}, frames {fx}x{fy})"
        )
    if ch != GRID_BASE_PX:
        warnings.append(
            f"  GRID WARN {path}: frame height {ch}px != GRID_BASE_PX {GRID_BASE_PX}px "
            f"(sheet {w}x{h}, frames {fx}x{fy})"
        )
    return warnings


def draw_placeholder(tex: dict) -> bytes:
    w = int(tex.get("width", 64))
    h = int(tex.get("height", 64))
    palette = PALETTES.get(tex.get("palette", "default"), PALETTES["default"])
    c0, c1, c2 = palette
    kind = tex.get("kind", "prop")

    buf = bytearray(w * h * 3)

    # background checker (tile size based on grid cell, not raw dimensions)
    tile = max(4, min(w, h) // 8)
    for y in range(h):
        for x in range(w):
            checker = ((x // tile) + (y // tile)) % 2
            c = c0 if checker == 0 else c1
            i = (y * w + x) * 3
            buf[i] = c[0]
            buf[i + 1] = c[1]
            buf[i + 2] = c[2]

    # outer border
    b = max(2, min(w, h) // 16)
    _fill_rect(buf, w, h, 0, 0, w, b, c2)
    _fill_rect(buf, w, h, 0, h - b, w, h, c2)
    _fill_rect(buf, w, h, 0, 0, b, h, c2)
    _fill_rect(buf, w, h, w - b, 0, w, h, c2)

    if kind == "sprite_sheet":
        fx = max(1, int(tex.get("frames_x", 8)))
        fy = max(1, int(tex.get("frames_y", 8)))
        cell_w = w // fx
        cell_h = h // fy

        # grid lines between cells
        for i in range(1, fx):
            gx = int(i * w / fx)
            for y in range(h):
                _set_px(buf, w, h, gx, y, c2)
        for j in range(1, fy):
            gy = int(j * h / fy)
            for x in range(w):
                _set_px(buf, w, h, x, gy, c2)

        # per-frame bounding box silhouette (shows where character fits)
        # Uses an 80% inner margin so the silhouette is clearly visible
        margin_pct = 0.10
        for fi in range(fx):
            for fj in range(fy):
                ox = fi * cell_w
                oy = fj * cell_h
                bx0 = ox + max(1, int(cell_w * margin_pct))
                by0 = oy + max(1, int(cell_h * margin_pct))
                bx1 = ox + cell_w - max(1, int(cell_w * margin_pct))
                by1 = oy + cell_h - max(1, int(cell_h * margin_pct))
                # draw only the rectangle outline (1px thick)
                for x in range(bx0, bx1):
                    _set_px(buf, w, h, x, by0, c2)
                    _set_px(buf, w, h, x, by1 - 1, c2)
                for y in range(by0, by1):
                    _set_px(buf, w, h, bx0, y, c2)
                    _set_px(buf, w, h, bx1 - 1, y, c2)

    elif kind == "tile":
        # diagonal cross stroke
        for i in range(min(w, h)):
            _set_px(buf, w, h, i, i, c2)
            _set_px(buf, w, h, w - 1 - i, i, c2)
    else:
        # prop: filled ellipse silhouette
        cx, cy = w // 2, h // 2
        rx, ry = max(6, w // 4), max(6, h // 4)
        for y in range(h):
            for x in range(w):
                dx = (x - cx) / float(rx)
                dy = (y - cy) / float(ry)
                if dx * dx + dy * dy <= 1.0:
                    _set_px(buf, w, h, x, y, c2)

    return bytes(buf)


def should_include(path: str, prefix: str | None) -> bool:
    if not prefix:
        return True
    p = path.replace("\\", "/")
    q = prefix.replace("\\", "/")
    return p.startswith(q)


def _extract_items_from_data(data: dict) -> list[dict]:
    """Extract texture item dicts from either flat {textures:[]} or grouped {groups:[]} JSON."""
    if "textures" in data and isinstance(data["textures"], list):
        return [x for x in data["textures"] if isinstance(x, dict) and x.get("path")]
    if "groups" in data and isinstance(data["groups"], list):
        out: list[dict] = []
        for g in data["groups"]:
            if not isinstance(g, dict):
                continue
            for item in g.get("items", []):
                if isinstance(item, dict) and item.get("path"):
                    out.append(item)
        return out
    return []


def load_manifest_textures(
    manifest_path: Path,
    backlog_path: Path | None = None,
) -> list[dict]:
    """Load textures from the manifest and optionally merge the backlog contract."""
    seen: dict[str, dict] = {}

    data = json.loads(manifest_path.read_text(encoding="utf-8"))
    for item in _extract_items_from_data(data):
        seen[item["path"]] = item

    if backlog_path and backlog_path.exists():
        backlog = json.loads(backlog_path.read_text(encoding="utf-8"))
        for item in _extract_items_from_data(backlog):
            p = item.get("path", "")
            if p and p not in seen:
                # backlog items may lack palette; assign a reasonable default
                if "palette" not in item:
                    item = dict(item)  # don't mutate original
                    item["palette"] = "default"
                seen[p] = item

    return list(seen.values())


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Generate PNG placeholders conforming to texture_contract_inventory.md"
    )
    ap.add_argument("--manifest", default="tools/texture_manifest.json")
    ap.add_argument("--backlog",  default="tools/texture_backlog_contract.json")
    ap.add_argument("--overwrite", action="store_true")
    ap.add_argument("--dry-run",   action="store_true")
    ap.add_argument("--only", default="", help="Generate only paths with this prefix")
    ap.add_argument(
        "--include-backlog", action="store_true",
        help="Also generate placeholders for planned textures in texture_backlog_contract.json"
    )
    args = ap.parse_args()

    manifest_path = Path(args.manifest)
    if not manifest_path.exists():
        print(f"manifest not found: {manifest_path}")
        return 1

    backlog_path: Path | None = None
    if args.include_backlog:
        backlog_path = Path(args.backlog)
        if not backlog_path.exists():
            print(f"  WARN: --include-backlog set but backlog not found: {backlog_path}")
            backlog_path = None

    root = Path(__file__).resolve().parents[1]
    textures = load_manifest_textures(manifest_path, backlog_path)

    # Grid alignment check for all character sprite sheets
    grid_warnings: list[str] = []
    for tex in textures:
        grid_warnings.extend(_check_grid_alignment(tex))
    if grid_warnings:
        print("\n=== Grid alignment warnings (GRID_BASE_PX={}) ===".format(GRID_BASE_PX))
        for w in grid_warnings:
            print(w)
        print()

    created = 0
    skipped = 0
    listed  = 0
    for tex in textures:
        rel = tex.get("path", "")
        if not rel:
            continue
        if not should_include(rel, args.only or None):
            continue
        listed += 1
        out = root / rel
        if out.exists() and not args.overwrite:
            print(f"skip  {rel} (exists)")
            skipped += 1
            continue

        rgb = draw_placeholder(tex)
        if args.dry_run:
            w_val = tex.get("width", "?")
            h_val = tex.get("height", "?")
            print(f"plan  {rel}  ({w_val}x{h_val})")
            continue

        write_png_rgb(out, int(tex["width"]), int(tex["height"]), rgb)
        print(f"write {rel}")
        created += 1

    print(f"\nsummary: listed={listed} created={created} skipped={skipped}")
    if args.dry_run:
        print("dry-run mode: no files written")
    if grid_warnings:
        print(f"{len(grid_warnings)} grid alignment warning(s) — fix manifest dims to use multiples of {GRID_BASE_PX}px per frame.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
