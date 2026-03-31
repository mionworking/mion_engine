from __future__ import annotations

from dataclasses import asdict, dataclass, field
from typing import Any


@dataclass(slots=True)
class ImageContract:
    safe_format: str
    frame_width: int
    frame_height: int
    sheet_width: int
    sheet_height: int
    rows: list[str]
    frames_per_row: dict[str, int]
    background: str
    facing: str


@dataclass(slots=True)
class AudioContract:
    safe_format: str
    sample_rate: int
    channels: int
    sample_width_bits: int
    loop: bool
    suggested_max_duration_seconds: float | None = None


@dataclass(slots=True)
class SlotContract:
    slot_id: str
    kind: str
    required: bool
    output_path: str
    keywords: list[str]
    min_positive_hits: int = 1
    negative_keywords: list[str] = field(default_factory=list)
    aliases: list[str] = field(default_factory=list)
    notes: list[str] = field(default_factory=list)
    image: ImageContract | None = None
    audio: AudioContract | None = None


@dataclass(slots=True)
class ContractBundle:
    version: str
    notes: list[str]
    slots: list[SlotContract]

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)


@dataclass(slots=True)
class AssetRecord:
    source_path: str
    relative_path: str
    media_kind: str
    extension: str
    safe_format: bool
    convertible: bool
    size_bytes: int
    metadata: dict[str, Any] = field(default_factory=dict)
    text_evidence: str = ""
    tokens: list[str] = field(default_factory=list)
    notes: list[str] = field(default_factory=list)

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)


@dataclass(slots=True)
class CandidateMatch:
    slot_id: str
    source_path: str
    relative_path: str
    score: int
    confidence: str
    action: str
    reasons: list[str]
    metadata: dict[str, Any]

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)


@dataclass(slots=True)
class SlotSelection:
    slot: SlotContract
    status: str
    summary: str
    selected: CandidateMatch | None = None
    alternatives: list[CandidateMatch] = field(default_factory=list)

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)


@dataclass(slots=True)
class AnalysisReport:
    source_root: str
    generated_at: str
    contract: ContractBundle
    inventory_summary: dict[str, Any]
    selections: list[SlotSelection]
    assets: list[AssetRecord]

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)


@dataclass(slots=True)
class ConversionRecord:
    slot_id: str
    output_path: str
    status: str
    source_path: str | None
    message: str

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)
