"""동화책 인트로의 책 모양 스프라이트 생성 (시스템 Python + Pillow).

사용: python tools/intro/gen_book_sprites.py
결과: ArtSource/Intro/UI/*.png — build_intro_assets.py가 /Game/Art/UI/Intro로 임포트한다.

  T_Intro_SoftShadow  흐린 둥근 사각 그림자. 9-slice(가장자리 SHADOW_SLICE 비율)로 늘려 쓴다
  T_Intro_Gutter      펼침 가운데 접히는 골의 그림자. 세로로 늘려 쓴다
  T_Intro_Vignette    배경 가장자리를 어둡게 누르는 비네트
"""
import os

from PIL import Image, ImageDraw, ImageFilter

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
OUT = os.path.join(ROOT, "ArtSource", "Intro", "UI")

# 그림자: 256 정사각, 가장자리 96px(=0.375)이 흐림과 모서리를 담는다
SHADOW_SIZE = 256
SHADOW_INSET = 64
SHADOW_BLUR = 22


def soft_shadow():
    img = Image.new("L", (SHADOW_SIZE, SHADOW_SIZE), 0)
    ImageDraw.Draw(img).rounded_rectangle(
        (SHADOW_INSET, SHADOW_INSET, SHADOW_SIZE - SHADOW_INSET, SHADOW_SIZE - SHADOW_INSET), radius=18, fill=255)
    alpha = img.filter(ImageFilter.GaussianBlur(SHADOW_BLUR))
    out = Image.new("RGBA", img.size, (20, 12, 18, 0))
    out.putalpha(alpha)
    return out


def gutter():
    """가로 256 × 세로 8. 가운데가 가장 어둡고 양옆으로 빠르게 옅어지며, 골 바로 옆에 얇은 밝은 결."""
    w, h = 256, 8
    out = Image.new("RGBA", (w, h))
    px = out.load()
    for x in range(w):
        t = abs(x - (w - 1) / 2) / ((w - 1) / 2)  # 0 = 가운데, 1 = 끝
        shade = 0.42 * (1 - t) ** 3.2
        light = 0.10 * max(0.0, 1 - abs(t - 0.16) / 0.06)
        if light > shade:
            col, a = (255, 250, 240), light
        else:
            col, a = (52, 32, 24), shade
        for y in range(h):
            px[x, y] = (*col, int(a * 255))
    return out


def vignette():
    w, h = 640, 360
    out = Image.new("RGBA", (w, h))
    px = out.load()
    for y in range(h):
        for x in range(w):
            dx = (x - w / 2) / (w / 2)
            dy = (y - h / 2) / (h / 2)
            r = min(1.0, (dx * dx * 0.8 + dy * dy) ** 0.5)
            px[x, y] = (10, 6, 14, int(255 * 0.6 * r ** 2.2))
    return out


os.makedirs(OUT, exist_ok=True)
for name, img in [("T_Intro_SoftShadow", soft_shadow()), ("T_Intro_Gutter", gutter()), ("T_Intro_Vignette", vignette())]:
    path = os.path.join(OUT, name + ".png")
    img.save(path)
    print("wrote", path, img.size)
