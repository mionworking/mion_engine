from __future__ import annotations

import csv
from pathlib import Path

from .models import AnalysisReport, ConversionRecord
from .utils import ensure_parent, write_json


def _selection_row(selection) -> dict[str, object]:
    selected = selection.selected
    return {
        "slot_id": selection.slot.slot_id,
        "kind": selection.slot.kind,
        "required": selection.slot.required,
        "status": selection.status,
        "summary": selection.summary,
        "output_path": selection.slot.output_path,
        "selected_source": selected.relative_path if selected else "",
        "score": selected.score if selected else "",
        "confidence": selected.confidence if selected else "",
        "action": selected.action if selected else "",
        "reasons": " | ".join(selected.reasons[:4]) if selected else "",
    }


def write_report_bundle(report: AnalysisReport, report_dir: Path, stem: str = "asset_plan") -> None:
    report_dir.mkdir(parents=True, exist_ok=True)

    json_path = report_dir / f"{stem}.json"
    csv_path = report_dir / f"{stem}.csv"
    md_path = report_dir / f"{stem}.md"

    write_json(json_path, report.to_dict())

    rows = [_selection_row(selection) for selection in report.selections]
    ensure_parent(csv_path)
    with csv_path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0].keys()))
        writer.writeheader()
        writer.writerows(rows)

    markdown_lines = [
        "# Asset Plan",
        "",
        f"- source_root: `{report.source_root}`",
        f"- generated_at: `{report.generated_at}`",
        f"- total_files: `{report.inventory_summary['total_files']}`",
        f"- safe_assets: `{report.inventory_summary['safe_assets']}`",
        f"- convertible_assets: `{report.inventory_summary['convertible_assets']}`",
        "",
        "## Slots",
        "",
        "| slot | status | output | source | confidence | action |",
        "| --- | --- | --- | --- | --- | --- |",
    ]

    for row in rows:
        markdown_lines.append(
            "| {slot_id} | {status} | `{output_path}` | `{selected_source}` | {confidence} | {action} |".format(**row)
        )

    markdown_lines.extend(["", "## Inventory", ""])
    for key, value in sorted(report.inventory_summary["by_media_kind"].items()):
        markdown_lines.append(f"- {key}: `{value}`")

    ensure_parent(md_path)
    md_path.write_text("\n".join(markdown_lines), encoding="utf-8")


def write_conversion_summary(records: list[ConversionRecord], output_path: Path) -> None:
    write_json(output_path, [record.to_dict() for record in records])
