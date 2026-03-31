from __future__ import annotations

import struct
import wave
from pathlib import Path

from .models import AssetRecord
from .ocr import OCRBackend
from .utils import tokenize_path


SAFE_IMAGE_EXTS = {".png"}
CONVERTIBLE_IMAGE_EXTS = {".jpg", ".jpeg", ".bmp", ".tga"}
SAFE_AUDIO_EXTS = {".wav"}
CONVERTIBLE_AUDIO_EXTS = {".mp3", ".ogg", ".flac"}
TEXT_DOC_EXTS = {".txt", ".md", ".csv", ".json"}


def _read_png_size(path: Path) -> tuple[int, int] | None:
    with path.open("rb") as stream:
        header = stream.read(24)

    if len(header) < 24:
        return None
    if header[:8] != b"\x89PNG\r\n\x1a\n":
        return None
    if header[12:16] != b"IHDR":
        return None

    width = struct.unpack(">I", header[16:20])[0]
    height = struct.unpack(">I", header[20:24])[0]
    return width, height


def _read_wave_metadata(path: Path) -> dict[str, float | int]:
    with wave.open(str(path), "rb") as stream:
        frames = stream.getnframes()
        framerate = stream.getframerate()
        duration = (frames / framerate) if framerate else 0.0
        return {
            "channels": stream.getnchannels(),
            "sample_rate": framerate,
            "sample_width_bits": stream.getsampwidth() * 8,
            "frames": frames,
            "duration_seconds": round(duration, 3),
        }


def _classify_extension(ext: str) -> tuple[str, bool, bool]:
    if ext in SAFE_IMAGE_EXTS:
        return "image", True, True
    if ext in CONVERTIBLE_IMAGE_EXTS:
        return "image", False, True
    if ext in SAFE_AUDIO_EXTS:
        return "audio", True, True
    if ext in CONVERTIBLE_AUDIO_EXTS:
        return "audio", False, True
    if ext in TEXT_DOC_EXTS or ext == ".pdf":
        return "document", False, False
    return "unsupported", False, False


def scan_assets(source_root: Path, ocr_backend: OCRBackend) -> list[AssetRecord]:
    assets: list[AssetRecord] = []

    for path in sorted(p for p in source_root.rglob("*") if p.is_file()):
        relative = path.relative_to(source_root)
        ext = path.suffix.casefold()
        media_kind, safe_format, convertible = _classify_extension(ext)
        metadata: dict[str, object] = {}
        notes: list[str] = []

        if media_kind == "image" and ext == ".png":
            size = _read_png_size(path)
            if size is not None:
                metadata["width"], metadata["height"] = size
            else:
                notes.append("png_header_invalid_or_unreadable")
        elif media_kind == "audio" and ext == ".wav":
            try:
                metadata.update(_read_wave_metadata(path))
            except wave.Error:
                notes.append("wav_unreadable")

        text_evidence = ""
        if media_kind == "image":
            text_evidence = ocr_backend.extract_text(path).text
        elif media_kind == "document" and ext in TEXT_DOC_EXTS:
            try:
                text_evidence = path.read_text(encoding="utf-8", errors="ignore")[:2048]
            except OSError:
                notes.append("document_unreadable")

        assets.append(
            AssetRecord(
                source_path=str(path),
                relative_path=str(relative),
                media_kind=media_kind,
                extension=ext,
                safe_format=safe_format,
                convertible=convertible,
                size_bytes=path.stat().st_size,
                metadata=metadata,
                text_evidence=text_evidence,
                tokens=tokenize_path(relative),
                notes=notes,
            )
        )

    return assets


def summarize_inventory(assets: list[AssetRecord]) -> dict[str, object]:
    counts: dict[str, int] = {}
    for asset in assets:
        counts[asset.media_kind] = counts.get(asset.media_kind, 0) + 1

    return {
        "total_files": len(assets),
        "by_media_kind": counts,
        "safe_assets": sum(1 for asset in assets if asset.safe_format),
        "convertible_assets": sum(1 for asset in assets if asset.convertible),
    }
