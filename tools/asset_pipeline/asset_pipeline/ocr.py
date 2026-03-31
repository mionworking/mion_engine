from __future__ import annotations

import subprocess
from dataclasses import dataclass
from pathlib import Path

from .utils import command_exists, normalize_text


@dataclass(slots=True)
class OCRResult:
    backend: str
    text: str
    available: bool


class OCRBackend:
    def extract_text(self, path: Path) -> OCRResult:
        raise NotImplementedError


class FilenameOCRBackend(OCRBackend):
    def extract_text(self, path: Path) -> OCRResult:
        raw = " ".join(path.parts)
        return OCRResult(
            backend="filename-fallback",
            text=normalize_text(raw),
            available=True,
        )


class TesseractOCRBackend(OCRBackend):
    def __init__(self) -> None:
        self._available = command_exists("tesseract")

    @property
    def available(self) -> bool:
        return self._available

    def extract_text(self, path: Path) -> OCRResult:
        if not self._available:
            return OCRResult(backend="tesseract", text="", available=False)

        result = subprocess.run(
            ["tesseract", str(path), "stdout", "--psm", "6"],
            capture_output=True,
            text=True,
            check=False,
        )
        if result.returncode != 0:
            return OCRResult(backend="tesseract", text="", available=False)

        return OCRResult(
            backend="tesseract",
            text=normalize_text(result.stdout),
            available=True,
        )


class AutoOCRBackend(OCRBackend):
    def __init__(self) -> None:
        self._tesseract = TesseractOCRBackend()
        self._fallback = FilenameOCRBackend()

    def extract_text(self, path: Path) -> OCRResult:
        result = self._tesseract.extract_text(path)
        if result.available and result.text:
            return result
        return self._fallback.extract_text(path)
