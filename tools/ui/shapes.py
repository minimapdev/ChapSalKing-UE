"""찹쌀킹 배경 아이콘 실루엣 — 왕관·반죽 돔. gen_sprites.py가 쓴다."""
import math
from PIL import Image, ImageDraw, ImageFilter

S = 4


def _render(w, h, draw, soften):
    """여백 있는 캔버스에 그려 둥글린 뒤 잘라낸다 — 가장자리에 닿은 모서리가 잘리지 않게."""
    W, H = w * S, h * S
    pad = int(W * soften * 4) + 2
    m = Image.new("L", (W + pad * 2, H + pad * 2), 0)
    draw(ImageDraw.Draw(m), pad, W, H)
    m = m.filter(ImageFilter.GaussianBlur(W * soften)).point(lambda v: 255 if v >= 128 else 0)
    return m.crop((pad, pad, pad + W, pad + H)).resize((w, h), Image.LANCZOS)


def crown_mask(w, h):
    def draw(d, p, W, H):
        pts = [(0.05, 0.90), (0.0, 0.14), (0.285, 0.42), (0.5, 0.0), (0.715, 0.42), (1.0, 0.14), (0.95, 0.90)]
        for i in range(1, 12):                  # 바닥은 아주 살짝 웃는 곡선
            t = i / 12
            pts.append((0.95 - 0.90 * t, 0.90 + 0.05 * 4 * t * (1 - t)))
        d.polygon([(p + x * (W - 1), p + y * (H - 1)) for x, y in pts], fill=255)
    return _render(w, h, draw, 0.016)


def dome_mask(w, h):
    def draw(d, p, W, H):
        # 윗면: 살짝 넓은 타원 / 아랫면: 모서리가 둥근 납작한 바닥
        cx, cy, a, b, n = W / 2, H * 0.64, W / 2, H * 0.64, 2.15
        top = []
        for i in range(0, 65):
            t = math.pi + math.pi * i / 64
            c, s = math.cos(t), math.sin(t)
            top.append((p + cx + a * math.copysign(abs(c) ** (2 / n), c), p + cy + b * math.copysign(abs(s) ** (2 / n), s)))
        d.polygon(top, fill=255)
        d.rounded_rectangle((p, p + H * 0.40, p + W - 1, p + H - 1), radius=H * 0.32, fill=255)
    return _render(w, h, draw, 0.022)
