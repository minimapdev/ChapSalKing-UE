"""책장 넘김 효과음 생성 (시스템 Python + numpy).

사용: python tools/intro/gen_page_turn_sound.py
결과: ArtSource/Intro/Audio/S_Intro_PageTurn.wav (44.1kHz 16bit 모노, 약 1초)

종이가 쓸리는 소리(대역 필터 노이즈, 중심 주파수가 내려감) + 내려앉는 짧은 소리.
같은 시드라 몇 번을 돌려도 같은 파일이 나온다.
"""
import os
import wave

import numpy as np

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
OUT = os.path.join(ROOT, "ArtSource", "Intro", "Audio", "S_Intro_PageTurn.wav")
SR = 44100


def bandpassed_noise(rng, seconds, f_start, f_end, q):
    """중심 주파수가 f_start → f_end로 미끄러지는 2차 대역 필터 노이즈."""
    n = int(SR * seconds)
    x = rng.uniform(-1.0, 1.0, n)
    f = np.geomspace(f_start, f_end, n)
    y = np.zeros(n)
    x1 = x2 = y1 = y2 = 0.0
    for i in range(n):
        w0 = 2 * np.pi * f[i] / SR
        alpha = np.sin(w0) / (2 * q)
        b0, b2 = alpha, -alpha
        a0, a1, a2 = 1 + alpha, -2 * np.cos(w0), 1 - alpha
        yi = (b0 * x[i] + b2 * x2 - a1 * y1 - a2 * y2) / a0
        x2, x1 = x1, x[i]
        y2, y1 = y1, yi
        y[i] = yi
    return y


def envelope(n, attack, peak_at):
    t = np.linspace(0.0, 1.0, n)
    env = np.where(t < attack, t / attack, np.exp(-(t - attack) / max(1e-4, 1 - peak_at) * 3.0))
    return env


rng = np.random.default_rng(7)
swish = bandpassed_noise(rng, 0.62, 2600, 900, 0.8) * envelope(int(SR * 0.62), 0.22, 0.3) * 0.9
land = bandpassed_noise(rng, 0.18, 700, 260, 1.2) * envelope(int(SR * 0.18), 0.08, 0.1) * 1.1

total = np.zeros(int(SR * 1.0))
total[: len(swish)] += swish
start = int(SR * 0.6)
total[start:start + len(land)] += land
total /= np.max(np.abs(total)) / 0.7
pcm = (total * 32767).astype(np.int16)

os.makedirs(os.path.dirname(OUT), exist_ok=True)
with wave.open(OUT, "wb") as w:
    w.setnchannels(1)
    w.setsampwidth(2)
    w.setframerate(SR)
    w.writeframes(pcm.tobytes())
print("wrote", OUT)
