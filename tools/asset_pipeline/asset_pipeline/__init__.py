"""Pipeline Python para triagem e conversao de assets do mion_engine."""

from pathlib import Path
import sys


_VENDOR = Path(__file__).resolve().parent.parent / ".vendor"
if _VENDOR.exists():
    vendor_path = str(_VENDOR)
    if vendor_path not in sys.path:
        sys.path.insert(0, vendor_path)
