from __future__ import annotations

from pathlib import Path

from .models import AnalysisReport, AssetRecord, CandidateMatch, ContractBundle, SlotContract, SlotSelection
from .utils import normalize_text, tokenize


MIN_CANDIDATE_SCORE = 18
MIN_SELECTED_SCORE = 32


def _score_keywords(slot: SlotContract, asset: AssetRecord, reasons: list[str]) -> tuple[int, int]:
    text_blob = normalize_text(" ".join(asset.tokens) + " " + asset.text_evidence)
    token_set = set(tokenize(text_blob))
    score = 0
    positive_hits = 0

    slot_stem = Path(slot.output_path).stem.casefold()
    asset_stem = Path(asset.relative_path).stem.casefold()
    if asset_stem == slot_stem:
        score += 24
        positive_hits += 1
        reasons.append(f"basename exato '{asset_stem}' apareceu")

    if slot_stem in token_set:
        score += 20
        positive_hits += 1
        reasons.append(f"nome alvo '{slot_stem}' apareceu")

    for alias in slot.aliases:
        alias_tokens = tokenize(alias)
        if alias_tokens and all(token in token_set for token in alias_tokens):
            score += 10
            positive_hits += 1
            reasons.append(f"alias '{alias}' apareceu")

    for keyword in slot.keywords:
        keyword_tokens = tokenize(keyword)
        if keyword_tokens and all(token in token_set for token in keyword_tokens):
            score += 8
            positive_hits += 1
            reasons.append(f"keyword '{keyword}' apareceu")

    for keyword in slot.negative_keywords:
        negative_tokens = tokenize(keyword)
        if negative_tokens and all(token in token_set for token in negative_tokens):
            score -= 10
            reasons.append(f"keyword negativa '{keyword}' apareceu")

    return score, positive_hits


def _image_action(slot: SlotContract, asset: AssetRecord, reasons: list[str]) -> tuple[int, str]:
    score = 0
    if asset.extension == ".png":
        score += 8
        reasons.append("png já é formato seguro")
    elif asset.convertible:
        score += 2
        reasons.append("imagem convertível")
    else:
        return -100, "unsupported_image_format"

    if slot.image and "width" in asset.metadata and "height" in asset.metadata:
        width = int(asset.metadata["width"])
        height = int(asset.metadata["height"])
        if width == slot.image.sheet_width and height == slot.image.sheet_height:
            score += 25
            reasons.append("dimensões da sheet batem exatamente")
            return score, "copy_safe"
        if width >= slot.image.sheet_width and height >= slot.image.sheet_height:
            score += 5
            reasons.append("sheet maior que o alvo; precisa recorte/resize")
            return score, "manual_crop_or_resize"
        score -= 8
        reasons.append("sheet menor que o esperado")
        return score, "manual_review"

    reasons.append("dimensões desconhecidas")
    return score, "manual_review"


def _audio_action(slot: SlotContract, asset: AssetRecord, reasons: list[str]) -> tuple[int, str]:
    score = 0
    if asset.extension == ".wav":
        score += 8
        reasons.append("wav já é formato aceito")
    elif asset.convertible:
        score += 1
        reasons.append("áudio convertível com backend externo")
        return score, "needs_external_audio_backend"
    else:
        return -100, "unsupported_audio_format"

    if slot.audio:
        sample_rate = int(asset.metadata.get("sample_rate", 0))
        channels = int(asset.metadata.get("channels", 0))
        width_bits = int(asset.metadata.get("sample_width_bits", 0))
        duration = float(asset.metadata.get("duration_seconds", 0.0))

        if sample_rate == slot.audio.sample_rate:
            score += 5
            reasons.append("sample rate já bate")
        else:
            reasons.append("sample rate precisa normalização")

        if channels == slot.audio.channels:
            score += 4
            reasons.append("número de canais já bate")
        else:
            reasons.append("número de canais precisa ajuste")

        if width_bits == slot.audio.sample_width_bits:
            score += 4
            reasons.append("bit depth já bate")
        else:
            reasons.append("bit depth precisa ajuste")

        if slot.audio.suggested_max_duration_seconds is not None:
            if duration <= slot.audio.suggested_max_duration_seconds:
                score += 3
                reasons.append("duração está no intervalo esperado")
            else:
                score -= 4
                reasons.append("duração acima do sugerido")

    return score, "convert_wav_safe"


def score_candidate(slot: SlotContract, asset: AssetRecord) -> CandidateMatch | None:
    if asset.media_kind != slot.kind:
        return None

    reasons = [f"tipo compatível: {slot.kind}"]
    keyword_score, positive_hits = _score_keywords(slot, asset, reasons)
    score = 12 + keyword_score

    if positive_hits < slot.min_positive_hits:
        reasons.append(
            f"positivos insuficientes: {positive_hits}/{slot.min_positive_hits}"
        )
        return None

    if slot.kind == "image":
        delta, action = _image_action(slot, asset, reasons)
        score += delta
    else:
        delta, action = _audio_action(slot, asset, reasons)
        score += delta

    if score < MIN_CANDIDATE_SCORE:
        return None

    if score >= 60:
        confidence = "high"
    elif score >= 40:
        confidence = "medium"
    else:
        confidence = "low"

    return CandidateMatch(
        slot_id=slot.slot_id,
        source_path=asset.source_path,
        relative_path=asset.relative_path,
        score=score,
        confidence=confidence,
        action=action,
        reasons=reasons,
        metadata=dict(asset.metadata),
    )


def select_assets(contract: ContractBundle, assets: list[AssetRecord], source_root: Path) -> AnalysisReport:
    used_paths: set[str] = set()
    selections: list[SlotSelection] = []

    for slot in contract.slots:
        candidates = [
            candidate
            for asset in assets
            if (candidate := score_candidate(slot, asset)) is not None
        ]
        candidates.sort(key=lambda item: item.score, reverse=True)

        chosen = None
        for candidate in candidates:
            if candidate.source_path not in used_paths and candidate.score >= MIN_SELECTED_SCORE:
                chosen = candidate
                used_paths.add(candidate.source_path)
                break

        if chosen is None:
            if candidates:
                summary = "há candidatos, mas exigem revisão manual"
                status = "review"
            else:
                summary = "nenhum candidato forte encontrado"
                status = "missing"
        else:
            summary = "candidato selecionado"
            status = "selected"

        selections.append(
            SlotSelection(
                slot=slot,
                status=status,
                summary=summary,
                selected=chosen,
                alternatives=candidates[:5],
            )
        )

    from .scanner import summarize_inventory
    from datetime import datetime, UTC

    return AnalysisReport(
        source_root=str(source_root),
        generated_at=datetime.now(UTC).isoformat(),
        contract=contract,
        inventory_summary=summarize_inventory(assets),
        selections=selections,
        assets=assets,
    )
