from __future__ import annotations

import json
import re
import shutil
from pathlib import Path


TOKEN_RE = re.compile(r"[a-z0-9]+")


def normalize_text(text: str) -> str:
    lowered = text.casefold()
    return " ".join(TOKEN_RE.findall(lowered))


def tokenize(text: str) -> list[str]:
    return normalize_text(text).split()


def tokenize_path(path: Path) -> list[str]:
    return tokenize(str(path.as_posix()))


def command_exists(name: str) -> bool:
    return shutil.which(name) is not None


def ensure_parent(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)


def write_json(path: Path, payload: object) -> None:
    ensure_parent(path)
    path.write_text(json.dumps(payload, indent=2, ensure_ascii=False), encoding="utf-8")

