#!/usr/bin/env python3
from __future__ import annotations

import json
import re
from pathlib import Path
import argparse


ASSET_RE = re.compile(r'assets/[^\s"\\]+\.(?:png|jpg|jpeg|webp|bmp|gif)', re.IGNORECASE)
SIZE_RE = re.compile(
    r'"(?P<path>assets/[^\s"\\]+\.(?:png|jpg|jpeg|webp|bmp|gif))"\s*,\s*(?P<w>[0-9]+(?:\.[0-9]+)?)\s*,\s*(?P<h>[0-9]+(?:\.[0-9]+)?)',
    re.IGNORECASE,
)


def scan_code_refs(src_dir: Path) -> tuple[set[str], dict[str, tuple[str, str]]]:
    refs: set[str] = set()
    hints: dict[str, tuple[str, str]] = {}
    for p in src_dir.rglob("*"):
        if p.suffix not in {".hpp", ".cpp", ".h", ".c"}:
            continue
        try:
            text = p.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            continue
        for m in ASSET_RE.findall(text):
            refs.add(m)
        for m in SIZE_RE.finditer(text):
            refs.add(m.group("path"))
            hints[m.group("path")] = (m.group("w"), m.group("h"))
    return refs, hints


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    src = root / "src"
    ap = argparse.ArgumentParser()
    ap.add_argument("--manifest", default="tools/texture_manifest.json")
    ap.add_argument("--backlog", default="tools/texture_backlog_contract.json")
    ap.add_argument("--output", default="tools/texture_contract_inventory.md")
    args = ap.parse_args()
    manifest_path = root / args.manifest
    backlog_path = root / args.backlog
    report_path = root / args.output

    refs, size_hints = scan_code_refs(src)
    refs_sorted = sorted(refs)

    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    tex = manifest.get("textures", [])
    in_manifest = {t.get("path", "") for t in tex if t.get("path")}

    missing_in_manifest = sorted([p for p in refs_sorted if p not in in_manifest])
    extra_in_manifest = sorted([p for p in in_manifest if p not in refs])

    backlog_items: list[dict] = []
    if backlog_path.exists():
        backlog = json.loads(backlog_path.read_text(encoding="utf-8"))
        for g in backlog.get("groups", []):
            for item in g.get("items", []):
                if item.get("path"):
                    backlog_items.append(item)
    backlog_paths = {x["path"] for x in backlog_items}
    backlog_already_in_code = sorted([p for p in backlog_paths if p in refs])
    backlog_missing_in_code = sorted([p for p in backlog_paths if p not in refs])

    lines: list[str] = []
    lines.append("# Texture Contract Inventory\n")
    lines.append("Gerado automaticamente por `tools/audit_texture_contract.py`.\n")
    lines.append(f"- Referências no código: **{len(refs_sorted)}**")
    lines.append(f"- Entradas no manifesto: **{len(in_manifest)}**")
    lines.append(f"- Em falta no manifesto: **{len(missing_in_manifest)}**")
    lines.append(f"- Extras no manifesto (não referenciados no código): **{len(extra_in_manifest)}**\n")
    lines.append(f"- Entradas no backlog planejado: **{len(backlog_paths)}**")
    lines.append(f"- Backlog já referenciado no código: **{len(backlog_already_in_code)}**")
    lines.append(f"- Backlog ainda não referenciado: **{len(backlog_missing_in_code)}**\n")

    lines.append("## Referências do código\n")
    for p in refs_sorted:
        hint = ""
        if p in size_hints:
            w, h = size_hints[p]
            hint = f" _(hint: {w} x {h})_"
        lines.append(f"- `{p}`{hint}")

    lines.append("\n## Em falta no manifesto\n")
    if not missing_in_manifest:
        lines.append("- Nenhuma")
    else:
        for p in missing_in_manifest:
            hint = ""
            if p in size_hints:
                w, h = size_hints[p]
                hint = f" _(hint: {w} x {h})_"
            lines.append(f"- `{p}`{hint}")

    lines.append("\n## Extras no manifesto\n")
    if not extra_in_manifest:
        lines.append("- Nenhum")
    else:
        for p in extra_in_manifest:
            lines.append(f"- `{p}`")

    lines.append("\n## Backlog planejado (v2)\n")
    if not backlog_items:
        lines.append("- Nenhum backlog encontrado")
    else:
        by_group: dict[str, list[dict]] = {}
        backlog = json.loads(backlog_path.read_text(encoding="utf-8")) if backlog_path.exists() else {}
        for g in backlog.get("groups", []):
            by_group[g.get("name", "ungrouped")] = g.get("items", [])
        for group_name, items in by_group.items():
            lines.append(f"\n### {group_name}")
            for item in items:
                p = item.get("path", "")
                if not p:
                    continue
                marker = "in-code" if p in refs else "planned"
                wh = ""
                if item.get("width") and item.get("height"):
                    wh = f" ({item['width']}x{item['height']})"
                lines.append(f"- `{p}`{wh} — `{marker}`")

    lines.append(
        "\n## Nota de escopo atual\n\n"
        "No estado atual do código, **chão, portas e magias** não dependem de texturas PNG dedicadas;\n"
        "eles são renderizados via fallback geométrico/retângulos/sistemas de efeito. Se forem adicionados\n"
        "novos caminhos `assets/...png` no código, este relatório passa a listá-los automaticamente.\n"
    )

    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"written {report_path}")
    print(
        f"refs={len(refs_sorted)} manifest={len(in_manifest)} "
        f"missing={len(missing_in_manifest)} extra={len(extra_in_manifest)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
