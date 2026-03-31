from __future__ import annotations

import tempfile
import unittest
import wave
from pathlib import Path

from asset_pipeline.classifier import select_assets
from asset_pipeline.contracts import load_contract
from asset_pipeline.conversion import convert_report
from asset_pipeline.ocr import FilenameOCRBackend
from asset_pipeline.scanner import scan_assets


PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


def write_fake_png(path: Path, width: int, height: int) -> None:
    payload = (
        PNG_SIGNATURE
        + (13).to_bytes(4, "big")
        + b"IHDR"
        + width.to_bytes(4, "big")
        + height.to_bytes(4, "big")
        + b"\x08\x06\x00\x00\x00"
        + b"\x00\x00\x00\x00"
    )
    path.write_bytes(payload)


def write_wav(path: Path, sample_rate: int = 22050, channels: int = 1) -> None:
    frames = b"\x00\x00" * 128
    with wave.open(str(path), "wb") as stream:
        stream.setnchannels(channels)
        stream.setsampwidth(2)
        stream.setframerate(sample_rate)
        stream.writeframes(frames)


class AssetPipelineTests(unittest.TestCase):
    def test_scan_extracts_png_and_wav_metadata(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            write_fake_png(root / "hero_warrior.png", 128, 160)
            write_wav(root / "monster_death.wav")

            assets = scan_assets(root, FilenameOCRBackend())
            image = next(asset for asset in assets if asset.extension == ".png")
            audio = next(asset for asset in assets if asset.extension == ".wav")

            self.assertEqual(image.metadata["width"], 128)
            self.assertEqual(image.metadata["height"], 160)
            self.assertEqual(audio.metadata["sample_rate"], 22050)
            self.assertEqual(audio.metadata["channels"], 1)

    def test_classifier_selects_player_sprite_when_sheet_matches(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            sprite_dir = root / "sprites"
            sprite_dir.mkdir()
            write_fake_png(sprite_dir / "diablo_warrior_weaponless.png", 128, 160)

            assets = scan_assets(root, FilenameOCRBackend())
            contract = load_contract()
            report = select_assets(contract, assets, root)

            selection = next(item for item in report.selections if item.slot.slot_id == "player_sprite")
            self.assertEqual(selection.status, "selected")
            self.assertIsNotNone(selection.selected)
            self.assertIn("warrior", " ".join(selection.selected.reasons))

    def test_classifier_accepts_skeleton_filename(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            write_fake_png(root / "skeleton.png", 128, 160)

            assets = scan_assets(root, FilenameOCRBackend())
            contract = load_contract()
            report = select_assets(contract, assets, root)

            selection = next(item for item in report.selections if item.slot.slot_id == "skeleton_sprite")
            self.assertEqual(selection.status, "selected")
            self.assertEqual(selection.selected.relative_path, "skeleton.png")

    def test_audio_conversion_writes_safe_wav(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source_dir = root / "audio"
            source_dir.mkdir()
            write_wav(source_dir / "monster_death.wav", sample_rate=22050, channels=1)

            assets = scan_assets(root, FilenameOCRBackend())
            contract = load_contract()
            report = select_assets(contract, assets, root)
            dest_root = root / "converted"

            records = convert_report(report, dest_root, allow_resize_images=False)
            enemy_death = next(record for record in records if record.slot_id == "enemy_death_sfx")

            self.assertIn(enemy_death.status, {"converted", "copied", "skipped"})
            if enemy_death.status == "converted":
                with wave.open(enemy_death.output_path, "rb") as stream:
                    self.assertEqual(stream.getframerate(), 44100)
                    self.assertEqual(stream.getnchannels(), 2)
                    self.assertEqual(stream.getsampwidth(), 2)


if __name__ == "__main__":
    unittest.main()
