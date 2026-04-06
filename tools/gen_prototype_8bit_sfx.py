#!/usr/bin/env python3
"""
Gera WAVs mono 16-bit (estilo prototype / chiptune: onda quadrada + env simples).
Uso (na raiz do repo):
  python3 tools/gen_prototype_8bit_sfx.py [--output-dir assets/audio]

Substitui os fisticheiros listados por versões genéricas até haver áudio final.
"""
from __future__ import annotations

import argparse
import math
import struct
import wave
from pathlib import Path

SR = 22050


def _clip_i16(x: float) -> int:
    v = int(x * 28000.0)
    return max(-32768, min(32767, v))


def _square(phase: float) -> float:
    p = phase % 1.0
    return 1.0 if p < 0.5 else -1.0


def _noise(seed: int) -> float:
    # PRNG simples determinístico / amostra
    seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF
    return (seed / 0x7FFFFFFF) * 2.0 - 1.0


def render_square(freq: float, duration_s: float, vol: float = 0.45) -> list[float]:
    n = int(SR * duration_s)
    out: list[float] = []
    ph = 0.0
    inc = freq / SR
    for i in range(n):
        t = i / SR
        env = math.exp(-t * 12.0)  # decay rápido
        out.append(_square(ph) * vol * env)
        ph += inc
    return out


def render_sweep(f0: float, f1: float, duration_s: float, vol: float = 0.4) -> list[float]:
    n = int(SR * duration_s)
    out: list[float] = []
    ph = 0.0
    for i in range(n):
        u = i / max(1, n - 1)
        f = f0 + (f1 - f0) * u
        ph += f / SR
        env = 1.0 - u * 0.85
        out.append(_square(ph) * vol * env)
    return out


def render_arpeggio(freqs: list[float], note_len: float, vol: float = 0.38) -> list[float]:
    out: list[float] = []
    for f in freqs:
        out.extend(render_square(f, note_len, vol * 0.9))
    return out


def render_hit() -> list[float]:
    n = int(SR * 0.08)
    out: list[float] = []
    ph = 0.0
    f = 140.0
    for i in range(n):
        t = i / SR
        ph += f / SR
        nmix = _square(ph) * 0.55 + _noise(i) * 0.35
        out.append(nmix * math.exp(-t * 35.0) * 0.5)
    return out


def render_nova() -> list[float]:
    # acorde curto + ring
    acc = []
    base = 220.0
    for k, mul in enumerate((1.0, 1.25, 1.5)):
        acc.extend(
            render_square(base * mul, 0.045, 0.22 / (k + 1))
        )
    tail = render_square(330.0, 0.12, 0.18)
    return acc + tail


def render_chain() -> list[float]:
    freqs = [520.0, 690.0, 880.0, 1040.0]
    return render_arpeggio(freqs, 0.035, 0.42)


def render_battle_cry() -> list[float]:
    n = int(SR * 0.28)
    out: list[float] = []
    ph = 0.0
    f = 155.0
    for i in range(n):
        t = i / SR
        fmod = f + 8.0 * math.sin(t * 38.0)
        ph += fmod / SR
        env = min(1.0, t * 80.0) * math.exp(-t * 4.5)
        out.append(_square(ph) * 0.48 * env)
    return out


def render_ui_confirm() -> list[float]:
    a = render_square(660.0, 0.05, 0.35)
    b = render_square(880.0, 0.08, 0.38)
    return a + b


def render_ui_delete() -> list[float]:
    a = render_square(880.0, 0.06, 0.35)
    b = render_square(440.0, 0.1, 0.32)
    return a + b


def render_ambient_loop() -> list[float]:
    """~2.5 s loop suave (drone + LFO) para música de protótipo."""
    duration = 2.5
    n = int(SR * duration)
    out: list[float] = []
    for i in range(n):
        t = i / SR
        f1, f2 = 55.0, 82.5
        ph1 = t * f1
        ph2 = t * f2
        lfo = 0.08 * math.sin(t * math.pi * 2 * 0.35)
        s = (
            _square(ph1 % 1.0) * 0.12
            + _square(ph2 % 1.0) * 0.08
            + _noise(i // 4) * 0.04
        ) * (0.65 + lfo)
        out.append(s * 0.35)
    return out


def render_music_town() -> list[float]:
    dur, n = 8.0, int(SR * 8.0)
    out: list[float] = []
    for i in range(n):
        t = i / SR
        step = int(t * 2.0) % 4
        mel = (196.0, 220.0, 246.94, 174.61)[step]
        ph = t * mel + 0.25 * math.sin(t * 3.14159)
        bas = t * 98.0
        s = _square(ph % 1.0) * 0.11 + _square(bas % 1.0) * 0.07
        out.append(s * 0.32)
    return out


def render_music_dungeon_calm() -> list[float]:
    n = int(SR * 9.0)
    out: list[float] = []
    for i in range(n):
        t = i / SR
        mel = 146.0 + 36.0 * math.sin(t * 0.9)
        ph = t * mel
        dr = t * 55.0
        s = _square(ph % 1.0) * 0.1 + _square(dr % 1.0) * 0.09 + _noise(i // 6) * 0.025
        out.append(s * 0.3)
    return out


def render_music_dungeon_combat() -> list[float]:
    n = int(SR * 8.0)
    out: list[float] = []
    for i in range(n):
        t = i / SR
        step = int(t * 5.5) % 3
        freqs = (185.0, 207.65, 246.94)
        f = freqs[step]
        ph = t * f * 2.0
        b = t * 82.0
        s = _square(ph % 1.0) * 0.13 + _square(b % 1.0) * 0.11
        out.append(s * 0.36)
    return out


def render_music_boss() -> list[float]:
    n = int(SR * 10.0)
    out: list[float] = []
    for i in range(n):
        t = i / SR
        f = 110.0 + 15.0 * math.sin(t * 2.8)
        ph = t * f
        ph2 = t * f * 1.5
        s = _square(ph % 1.0) * 0.15 + _square(ph2 % 1.0) * 0.1
        out.append(s * 0.4)
    return out


def render_music_victory() -> list[float]:
    notes = (523.0, 587.0, 659.0, 784.0)
    acc: list[float] = []
    for _ in range(2):
        for nf in notes:
            acc.extend(render_square(nf, 0.12, 0.22))
    acc.extend(render_square(880.0, 0.25, 0.2))
    return acc


def render_ambient_drips() -> list[float]:
    dur, n = 3.2, int(SR * 3.2)
    out = [0.0] * n
    for k in range(8):
        start = int((k / 8.0) * n + SR * 0.15)
        ln = int(SR * 0.04)
        for j in range(ln):
            if start + j >= n:
                break
            t = j / SR
            s = math.sin(t * 1800.0 * math.pi * 2) * math.exp(-t * 45.0) * 0.35
            out[start + j] += s
    m = max(abs(x) for x in out) or 1.0
    return [x / m * 0.45 for x in out]


def render_ambient_torch() -> list[float]:
    n = int(SR * 2.8)
    out: list[float] = []
    for i in range(n):
        t = i / SR
        out.append((_noise(i) * 0.25 + _square(t * 12.0 % 1.0) * 0.05) * 0.4)
    return out


def render_ambient_wind() -> list[float]:
    n = int(SR * 4.5)
    out: list[float] = []
    for i in range(n):
        t = i / SR
        w = 0.35 * math.sin(t * math.pi * 2 * 0.15 + _noise(i) * 0.1)
        b = _square(t * 45.0 % 1.0) * 0.06
        out.append((w * 0.12 + b + _noise(i // 2) * 0.05) * 0.35)
    return out


def render_ambient_transition_wind() -> list[float]:
    """Corredor town→dungeon: mais grave, ruído e eco leve (loop)."""
    n = int(SR * 5.0)
    delay = int(SR * 0.11)
    out: list[float] = []
    for i in range(n):
        t = i / SR
        w = 0.48 * math.sin(t * math.pi * 2 * 0.07 + _noise(i) * 0.18)
        rumble = _square(t * 18.0 % 1.0) * 0.055
        out.append((w * 0.11 + rumble + _noise(i // 3) * 0.065) * 0.38)
    for i in range(delay, n):
        out[i] += out[i - delay] * 0.2
    return out


def render_ambient_birds() -> list[float]:
    n = int(SR * 5.0)
    out = [0.0] * n
    pos = int(SR * 0.2)
    while pos < n - 80:
        f = 1900.0 + (pos % 420)
        ln = int(SR * 0.045)
        for j in range(ln):
            if pos + j >= n:
                break
            u = j / SR
            ph = u * f
            out[pos + j] += _square(ph % 1.0) * math.exp(-u * 75.0) * 0.16
        pos += int(SR * (0.38 + (pos % 400) / 900.0))
    return out


def render_footstep(soft: bool) -> list[float]:
    f = 180.0 if soft else 95.0
    n = int(SR * (0.035 if soft else 0.05))
    out: list[float] = []
    ph = 0.0
    for i in range(n):
        t = i / SR
        ph += f / SR
        v = 0.35 if soft else 0.5
        nmix = _square(ph % 1.0) * 0.5 + _noise(i) * 0.4
        out.append(nmix * v * math.exp(-t * 90.0))
    return out


def render_spell_frost() -> list[float]:
    return render_sweep(1400.0, 400.0, 0.14, 0.38)


def render_skill_cleave() -> list[float]:
    return render_sweep(320.0, 90.0, 0.16, 0.42)


def render_skill_multishot() -> list[float]:
    b = render_square(690.0, 0.025, 0.32)
    mid = render_square(820.0, 0.025, 0.34)
    h = render_square(760.0, 0.03, 0.3)
    return b + mid + h


def render_skill_poison() -> list[float]:
    n = int(SR * 0.18)
    out: list[float] = []
    ph = 0.0
    for i in range(n):
        t = i / SR
        f = 420.0 + 60.0 * math.sin(t * 120.0)
        ph += f / SR
        out.append(_square(ph % 1.0) * 0.4 * math.exp(-t * 18.0))
    return out


def to_bytes(samples: list[float]) -> bytes:
    return b"".join(struct.pack("<h", _clip_i16(s)) for s in samples)


def write_wav(path: Path, samples: list[float]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(path), "w") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(SR)
        w.writeframes(to_bytes(samples))


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "--output-dir",
        type=Path,
        default=Path("assets/audio"),
        help="Pasta de saída (relativa à raiz do repo)",
    )
    args = ap.parse_args()
    d = args.output_dir

    specs: dict[str, list[float]] = {
        "hit.wav": render_hit(),
        "player_attack.wav": render_sweep(420.0, 180.0, 0.09, 0.42),
        "enemy_death.wav": render_arpeggio([360.0, 270.0, 180.0, 120.0], 0.07, 0.4),
        "player_death.wav": render_sweep(300.0, 80.0, 0.45, 0.36),
        "dash.wav": render_sweep(200.0, 900.0, 0.14, 0.38),
        "ranged_attack.wav": render_square(780.0, 0.06, 0.4),
        "spell_bolt.wav": render_sweep(700.0, 1200.0, 0.11, 0.39),
        "spell_nova.wav": render_nova(),
        "sfx_spell_frost.wav": render_spell_frost(),
        "sfx_spell_chain.wav": render_chain(),
        "sfx_skill_cleave.wav": render_skill_cleave(),
        "sfx_skill_multishot.wav": render_skill_multishot(),
        "sfx_skill_poison_arrow.wav": render_skill_poison(),
        "sfx_skill_battle_cry.wav": render_battle_cry(),
        "parry.wav": render_square(1240.0, 0.05, 0.45) + render_square(1860.0, 0.03, 0.28),
        "ui_confirm.wav": render_ui_confirm(),
        "ui_delete.wav": render_ui_delete(),
        "sfx_footstep_player.wav": render_footstep(True),
        "sfx_footstep_enemy.wav": render_footstep(False),
        "dungeon_ambient.wav": render_ambient_loop(),
        "music_town.wav": render_music_town(),
        "music_dungeon_calm.wav": render_music_dungeon_calm(),
        "music_dungeon_combat.wav": render_music_dungeon_combat(),
        "music_boss.wav": render_music_boss(),
        "music_victory.wav": render_music_victory(),
        "ambient_dungeon_drips.wav": render_ambient_drips(),
        "ambient_dungeon_torch.wav": render_ambient_torch(),
        "ambient_town_wind.wav": render_ambient_wind(),
        "ambient_town_birds.wav": render_ambient_birds(),
        "ambient_transition_wind.wav": render_ambient_transition_wind(),
    }

    for name, samples in specs.items():
        out = d / name
        write_wav(out, samples)
        print("written", out, f"({len(samples) / SR:.2f}s)")


if __name__ == "__main__":
    main()
