from __future__ import annotations

import argparse
from pathlib import Path

from .classifier import select_assets
from .contracts import default_contract_path, load_contract
from .conversion import convert_report
from .ocr import AutoOCRBackend
from .scanner import scan_assets, summarize_inventory
from .serialize import write_conversion_summary, write_report_bundle
from .utils import write_json


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Pipeline de triagem de assets do mion_engine")
    parser.add_argument("--contract", type=Path, default=default_contract_path())

    subparsers = parser.add_subparsers(dest="command", required=True)

    scan_parser = subparsers.add_parser("scan", help="gera inventário bruto")
    scan_parser.add_argument("--source", type=Path, required=True)
    scan_parser.add_argument("--output", type=Path, required=True)

    plan_parser = subparsers.add_parser("plan", help="gera plano de seleção")
    plan_parser.add_argument("--source", type=Path, required=True)
    plan_parser.add_argument("--report-dir", type=Path, required=True)
    plan_parser.add_argument("--stem", default="asset_plan")

    convert_parser = subparsers.add_parser("convert", help="converte os selecionados")
    convert_parser.add_argument("--source", type=Path, required=True)
    convert_parser.add_argument("--report-dir", type=Path, required=True)
    convert_parser.add_argument("--dest-root", type=Path, required=True)
    convert_parser.add_argument("--stem", default="asset_plan")
    convert_parser.add_argument("--allow-resize-images", action="store_true")

    return parser


def _scan_assets(source: Path):
    return scan_assets(source, AutoOCRBackend())


def main(argv: list[str] | None = None) -> int:
    parser = _build_parser()
    args = parser.parse_args(argv)

    contract = load_contract(args.contract)

    if args.command == "scan":
        assets = _scan_assets(args.source)
        write_json(
            args.output,
            {
                "source_root": str(args.source),
                "inventory_summary": summarize_inventory(assets),
                "assets": [asset.to_dict() for asset in assets],
            },
        )
        print(f"inventory salvo em {args.output}")
        return 0

    if args.command == "plan":
        assets = _scan_assets(args.source)
        report = select_assets(contract, assets, args.source)
        write_report_bundle(report, args.report_dir, args.stem)
        print(f"plano salvo em {args.report_dir}")
        return 0

    if args.command == "convert":
        assets = _scan_assets(args.source)
        report = select_assets(contract, assets, args.source)
        write_report_bundle(report, args.report_dir, args.stem)
        records = convert_report(
            report,
            args.dest_root,
            allow_resize_images=args.allow_resize_images,
        )
        write_conversion_summary(records, args.report_dir / f"{args.stem}_conversion.json")
        print(f"conversão registrada em {args.report_dir}")
        return 0

    parser.error("comando inválido")
    return 2
