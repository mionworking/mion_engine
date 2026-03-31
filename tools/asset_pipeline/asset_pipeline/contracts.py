from __future__ import annotations

import json
from pathlib import Path

from .models import AudioContract, ContractBundle, ImageContract, SlotContract


def default_contract_path() -> Path:
    return Path(__file__).resolve().parent.parent / "configs" / "engine_asset_contract.json"


def load_contract(path: Path | None = None) -> ContractBundle:
    contract_path = path or default_contract_path()
    raw = json.loads(contract_path.read_text(encoding="utf-8"))

    slots: list[SlotContract] = []
    for slot in raw["slots"]:
        image = None
        audio = None
        if "image" in slot:
            image = ImageContract(**slot["image"])
        if "audio" in slot:
            audio = AudioContract(**slot["audio"])

        slots.append(
            SlotContract(
                slot_id=slot["slot_id"],
                kind=slot["kind"],
                required=slot.get("required", True),
                output_path=slot["output_path"],
                keywords=slot.get("keywords", []),
                min_positive_hits=slot.get("min_positive_hits", 1),
                negative_keywords=slot.get("negative_keywords", []),
                aliases=slot.get("aliases", []),
                notes=slot.get("notes", []),
                image=image,
                audio=audio,
            )
        )

    return ContractBundle(
        version=raw.get("version", "0"),
        notes=raw.get("notes", []),
        slots=slots,
    )
