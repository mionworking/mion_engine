from __future__ import annotations

import shutil
import wave
from collections import deque
from pathlib import Path

import numpy as np

from .models import AnalysisReport, AudioContract, ConversionRecord, ImageContract
from .utils import ensure_parent


def _has_pillow() -> bool:
    try:
        import PIL  # noqa: F401
    except ModuleNotFoundError:
        return False
    return True


def _source_grid_boxes(width: int, height: int, cols: int = 6, rows: int = 5) -> list[list[tuple[int, int, int, int]]]:
    boxes: list[list[tuple[int, int, int, int]]] = []
    for row in range(rows):
        row_boxes: list[tuple[int, int, int, int]] = []
        y0 = round(row * height / rows)
        y1 = round((row + 1) * height / rows)
        for col in range(cols):
            x0 = round(col * width / cols)
            x1 = round((col + 1) * width / cols)
            row_boxes.append((x0, y0, x1, y1))
        boxes.append(row_boxes)
    return boxes


def _largest_component(mask: np.ndarray) -> np.ndarray | None:
    height, width = mask.shape
    seen = np.zeros((height, width), dtype=bool)
    best_points: list[tuple[int, int]] = []

    for y in range(height):
        for x in range(width):
            if not mask[y, x] or seen[y, x]:
                continue

            points: list[tuple[int, int]] = []
            queue = deque([(y, x)])
            seen[y, x] = True

            while queue:
                cy, cx = queue.popleft()
                points.append((cy, cx))
                for ny, nx in ((cy - 1, cx), (cy + 1, cx), (cy, cx - 1), (cy, cx + 1)):
                    if 0 <= ny < height and 0 <= nx < width and mask[ny, nx] and not seen[ny, nx]:
                        seen[ny, nx] = True
                        queue.append((ny, nx))

            if len(points) > len(best_points):
                best_points = points

    if not best_points:
        return None

    component = np.zeros((height, width), dtype=bool)
    ys, xs = zip(*best_points)
    component[np.array(ys), np.array(xs)] = True
    return component


def _extract_components(mask: np.ndarray) -> list[dict[str, object]]:
    height, width = mask.shape
    seen = np.zeros((height, width), dtype=bool)
    components: list[dict[str, object]] = []

    for y in range(height):
        for x in range(width):
            if not mask[y, x] or seen[y, x]:
                continue

            points: list[tuple[int, int]] = []
            queue = deque([(y, x)])
            seen[y, x] = True

            while queue:
                cy, cx = queue.popleft()
                points.append((cy, cx))
                for ny, nx in ((cy - 1, cx), (cy + 1, cx), (cy, cx - 1), (cy, cx + 1)):
                    if 0 <= ny < height and 0 <= nx < width and mask[ny, nx] and not seen[ny, nx]:
                        seen[ny, nx] = True
                        queue.append((ny, nx))

            ys = np.array([point[0] for point in points])
            xs = np.array([point[1] for point in points])
            component_mask = np.zeros((height, width), dtype=bool)
            component_mask[ys, xs] = True
            components.append(
                {
                    "area": len(points),
                    "bbox": (int(xs.min()), int(ys.min()), int(xs.max()) + 1, int(ys.max()) + 1),
                    "mask": component_mask,
                }
            )

    return components


def _extract_sprite_from_cell(cell_image, trim_left_label: bool = False):
    from PIL import Image

    rgba = cell_image.convert("RGBA")
    rgb = np.array(rgba, dtype=np.uint8)[:, :, :3].astype(np.int16)
    height, width, _ = rgb.shape

    patch_size = min(18, width // 6, height // 6)
    patches = [
        rgb[:patch_size, :patch_size].reshape(-1, 3),
        rgb[:patch_size, -patch_size:].reshape(-1, 3),
        rgb[-patch_size:, :patch_size].reshape(-1, 3),
        rgb[-patch_size:, -patch_size:].reshape(-1, 3),
    ]
    corner_colors = np.array([patch.mean(axis=0) for patch in patches], dtype=np.float32)
    pixels = rgb.astype(np.float32)
    dist = np.sqrt(((pixels[:, :, None, :] - corner_colors[None, None, :, :]) ** 2).sum(axis=3))
    background_like = dist.min(axis=2) < 38.0

    background = np.zeros((height, width), dtype=bool)
    queue = deque()
    for sy, sx in ((0, 0), (0, width - 1), (height - 1, 0), (height - 1, width - 1)):
        if background_like[sy, sx]:
            background[sy, sx] = True
            queue.append((sy, sx))

    while queue:
        y, x = queue.popleft()
        for ny, nx in ((y - 1, x), (y + 1, x), (y, x - 1), (y, x + 1)):
            if 0 <= ny < height and 0 <= nx < width and not background[ny, nx] and background_like[ny, nx]:
                background[ny, nx] = True
                queue.append((ny, nx))

    foreground = ~background
    if trim_left_label:
        trim = max(1, int(width * 0.22))
        foreground[:, :trim] = False

    components = _extract_components(foreground)
    if not components:
        return None

    max_area = max(int(component["area"]) for component in components)
    min_keep_area = max(600, int(max_area * 0.15))
    kept = [component for component in components if int(component["area"]) >= min_keep_area]
    if not kept:
        return None

    union = np.zeros_like(foreground)
    for component in kept:
        union |= component["mask"]  # type: ignore[operator]

    if int(union.sum()) < 1500:
        return None

    ys, xs = np.where(union)
    x0 = max(0, int(xs.min()) - 4)
    y0 = max(0, int(ys.min()) - 4)
    x1 = min(width, int(xs.max()) + 5)
    y1 = min(height, int(ys.max()) + 5)

    cropped = rgba.crop((x0, y0, x1, y1))
    alpha = Image.fromarray((union[y0:y1, x0:x1] * 255).astype(np.uint8), mode="L")
    cropped.putalpha(alpha)
    return cropped, int(union.sum())


def _resample_frames(frames: list, target_count: int) -> list:
    if not frames:
        return []
    if len(frames) == target_count:
        return list(frames)
    if len(frames) > target_count:
        indices = np.linspace(0, len(frames) - 1, num=target_count)
        return [frames[int(round(index))] for index in indices]
    return list(frames) + [frames[-1]] * (target_count - len(frames))


def _fit_sprite_to_cell(sprite, frame_width: int, frame_height: int):
    from PIL import Image

    bbox = sprite.getbbox()
    if bbox is None:
        return Image.new("RGBA", (frame_width, frame_height), (0, 0, 0, 0))

    sprite = sprite.crop(bbox)
    src_width, src_height = sprite.size
    if src_width == 0 or src_height == 0:
        return Image.new("RGBA", (frame_width, frame_height), (0, 0, 0, 0))

    scale = min((frame_width - 2) / src_width, (frame_height - 2) / src_height)
    scale = max(scale, 0.01)
    dst_width = max(1, int(round(src_width * scale)))
    dst_height = max(1, int(round(src_height * scale)))
    sprite = sprite.resize((dst_width, dst_height), Image.Resampling.LANCZOS)

    cell = Image.new("RGBA", (frame_width, frame_height), (0, 0, 0, 0))
    x = (frame_width - dst_width) // 2
    y = frame_height - dst_height
    cell.alpha_composite(sprite, (x, y))
    return cell


def _convert_ai_sheet(source_path: Path, output_path: Path, contract: ImageContract) -> ConversionRecord:
    from PIL import Image

    image = Image.open(source_path).convert("RGBA")
    grid = _source_grid_boxes(*image.size)
    output = Image.new("RGBA", (contract.sheet_width, contract.sheet_height), (0, 0, 0, 0))
    frame_width = contract.frame_width
    frame_height = contract.frame_height

    for row_index, row_name in enumerate(contract.rows):
        row_boxes = grid[row_index]
        extracted = []
        for col_index, box in enumerate(row_boxes):
            result = _extract_sprite_from_cell(image.crop(box), trim_left_label=(col_index == 0))
            if result is not None:
                extracted.append(result)

        target_count = contract.frames_per_row[row_name]
        ordered_sprites = [sprite for sprite, _ in extracted][:target_count]
        selected = _resample_frames(ordered_sprites, target_count)
        if not selected:
            return ConversionRecord(
                slot_id="",
                output_path=str(output_path),
                status="failed",
                source_path=str(source_path),
                message=f"nenhum frame detectado para a linha '{row_name}'",
            )

        for frame_index, sprite in enumerate(selected):
            fitted = _fit_sprite_to_cell(sprite, frame_width, frame_height)
            output.alpha_composite(fitted, (frame_index * frame_width, row_index * frame_height))

    ensure_parent(output_path)
    output.save(output_path, format="PNG")
    return ConversionRecord(
        slot_id="",
        output_path=str(output_path),
        status="converted",
        source_path=str(source_path),
        message="sheet AI recortada, limpa e normalizada para o contrato do engine",
    )


def _read_wave_as_float(path: Path) -> tuple[np.ndarray, int]:
    with wave.open(str(path), "rb") as stream:
        channels = stream.getnchannels()
        sample_rate = stream.getframerate()
        sample_width = stream.getsampwidth()
        raw = stream.readframes(stream.getnframes())

    if sample_width == 1:
        pcm = np.frombuffer(raw, dtype=np.uint8).astype(np.float32)
        pcm = (pcm - 128.0) / 128.0
    elif sample_width == 2:
        pcm = np.frombuffer(raw, dtype="<i2").astype(np.float32) / 32768.0
    elif sample_width == 4:
        pcm = np.frombuffer(raw, dtype="<i4").astype(np.float32) / 2147483648.0
    else:
        raise ValueError(f"sample width não suportada: {sample_width}")

    if pcm.size == 0:
        return np.zeros((0, channels), dtype=np.float32), sample_rate

    return pcm.reshape(-1, channels), sample_rate


def _resample_linear(samples: np.ndarray, src_rate: int, dst_rate: int) -> np.ndarray:
    if src_rate == dst_rate or len(samples) == 0:
        return samples

    src_len = len(samples)
    dst_len = max(1, int(round(src_len * dst_rate / src_rate)))
    x_old = np.arange(src_len, dtype=np.float32)
    x_new = np.linspace(0.0, float(src_len - 1), dst_len, dtype=np.float32)

    channels = samples.shape[1]
    resampled = np.empty((dst_len, channels), dtype=np.float32)
    for channel in range(channels):
        resampled[:, channel] = np.interp(x_new, x_old, samples[:, channel])
    return resampled


def _remap_channels(samples: np.ndarray, dst_channels: int) -> np.ndarray:
    src_channels = samples.shape[1] if samples.ndim == 2 else 1
    if src_channels == dst_channels:
        return samples
    if src_channels == 1 and dst_channels == 2:
        return np.repeat(samples, 2, axis=1)
    if src_channels == 2 and dst_channels == 1:
        return samples.mean(axis=1, keepdims=True)
    raise ValueError(f"remapeamento de canais não suportado: {src_channels} -> {dst_channels}")


def _write_wave(path: Path, samples: np.ndarray, contract: AudioContract) -> None:
    ensure_parent(path)
    clipped = np.clip(samples, -1.0, 1.0)
    pcm = (clipped * 32767.0).astype("<i2")

    with wave.open(str(path), "wb") as stream:
        stream.setnchannels(contract.channels)
        stream.setsampwidth(contract.sample_width_bits // 8)
        stream.setframerate(contract.sample_rate)
        stream.writeframes(pcm.tobytes())


def _convert_audio(source_path: Path, output_path: Path, contract: AudioContract) -> ConversionRecord:
    try:
        samples, src_rate = _read_wave_as_float(source_path)
        samples = _remap_channels(samples, contract.channels)
        samples = _resample_linear(samples, src_rate, contract.sample_rate)
        _write_wave(output_path, samples, contract)
        return ConversionRecord(
            slot_id="",
            output_path=str(output_path),
            status="converted",
            source_path=str(source_path),
            message="wav convertido para formato seguro",
        )
    except Exception as exc:  # pragma: no cover - erro operacional
        return ConversionRecord(
            slot_id="",
            output_path=str(output_path),
            status="failed",
            source_path=str(source_path),
            message=f"falha na conversão de áudio: {exc}",
        )


def _convert_image(
    source_path: Path,
    output_path: Path,
    contract: ImageContract,
    allow_resize_images: bool,
) -> ConversionRecord:
    width = None
    height = None
    try:
        from .scanner import _read_png_size  # import local para reaproveitar parser simples

        size = _read_png_size(source_path)
        if size is not None:
            width, height = size
    except Exception:
        pass

    if source_path.suffix.casefold() == ".png" and width == contract.sheet_width and height == contract.sheet_height:
        if source_path.resolve() == output_path.resolve():
            return ConversionRecord(
                slot_id="",
                output_path=str(output_path),
                status="copied",
                source_path=str(source_path),
                message="png já estava no formato e caminho corretos",
            )
        ensure_parent(output_path)
        shutil.copy2(source_path, output_path)
        return ConversionRecord(
            slot_id="",
            output_path=str(output_path),
            status="copied",
            source_path=str(source_path),
            message="png já estava no formato e tamanho corretos",
        )

    if _has_pillow():
        ai_sheet = _convert_ai_sheet(source_path, output_path, contract)
        if ai_sheet.status == "converted":
            return ai_sheet

    if not _has_pillow():
        return ConversionRecord(
            slot_id="",
            output_path=str(output_path),
            status="manual_review",
            source_path=str(source_path),
            message="Pillow ausente; resize/crop de imagem desabilitado",
        )

    if not allow_resize_images:
        return ConversionRecord(
            slot_id="",
            output_path=str(output_path),
            status="manual_review",
            source_path=str(source_path),
            message="resize automático de imagem desabilitado por segurança",
        )

    from PIL import Image

    ensure_parent(output_path)
    image = Image.open(source_path).convert("RGBA")
    image = image.resize((contract.sheet_width, contract.sheet_height), Image.Resampling.NEAREST)
    image.save(output_path, format="PNG")
    return ConversionRecord(
        slot_id="",
        output_path=str(output_path),
        status="converted",
        source_path=str(source_path),
        message="imagem convertida com resize nearest-neighbor",
    )


def convert_report(
    report: AnalysisReport,
    dest_root: Path,
    allow_resize_images: bool = False,
) -> list[ConversionRecord]:
    records: list[ConversionRecord] = []

    for selection in report.selections:
        output_path = dest_root / selection.slot.output_path

        if selection.selected is None:
            records.append(
                ConversionRecord(
                    slot_id=selection.slot.slot_id,
                    output_path=str(output_path),
                    status="skipped",
                    source_path=None,
                    message=f"slot sem seleção: {selection.status}",
                )
            )
            continue

        source_path = Path(selection.selected.source_path)
        if selection.slot.kind == "image" and selection.slot.image is not None:
            result = _convert_image(source_path, output_path, selection.slot.image, allow_resize_images)
        elif selection.slot.kind == "audio" and selection.slot.audio is not None:
            result = _convert_audio(source_path, output_path, selection.slot.audio)
        else:
            result = ConversionRecord(
                slot_id=selection.slot.slot_id,
                output_path=str(output_path),
                status="skipped",
                source_path=str(source_path),
                message="slot sem contrato de conversão",
            )

        result.slot_id = selection.slot.slot_id
        records.append(result)

    return records
