"""찹쌀킹 UI 스프라이트 생성기 — '찹쌀떡 파스텔' (시스템 Python + Pillow).

사용: python tools/ui/gen_sprites.py [출력 폴더]   (기본: ArtSource/UI/Generated)

모두 2배 해상도로 그리고, 9-slice 여백(margin)·내용 여백(content)·눌림 이동량(press)을
meta.json에 픽셀로 남긴다. UE 쪽 cskui.sprite()/chunky_button()이 이 값을 그대로 쓴다.
"""
import json
import os
import sys
from PIL import Image, ImageDraw, ImageFilter

from shapes import crown_mask, dome_mask

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, "ArtSource", "UI", "Generated")
os.makedirs(OUT, exist_ok=True)
SS = 3
META = {}

INDIGO = (30, 22, 69, 255)


def hexc(h, a=255):
    h = h.lstrip("#")
    return (int(h[0:2], 16), int(h[2:4], 16), int(h[4:6], 16), a)


def lighten(c, k):
    return tuple(min(255, int(v + (255 - v) * k)) for v in c[:3]) + (c[3],)


def vgrad(size, top, bottom, y0, y1):
    w, h = size
    col = Image.new("RGBA", (1, h))
    span = max(1, y1 - y0)
    for y in range(h):
        t = min(1.0, max(0.0, (y - y0) / span))
        col.putpixel((0, y), tuple(int(top[i] + (bottom[i] - top[i]) * t) for i in range(4)))
    return col.resize((w, h))


def rmask(size, box, r):
    m = Image.new("L", size, 0)
    ImageDraw.Draw(m).rounded_rectangle(box, radius=max(0, r), fill=255)
    return m


def put(base, layer, mask):
    base.alpha_composite(Image.composite(layer, Image.new("RGBA", layer.size, (0, 0, 0, 0)), mask))


def soft_shadow(size, box, r, color, blur):
    sh = Image.new("RGBA", size, color[:3] + (0,))
    ImageDraw.Draw(sh).rounded_rectangle(box, radius=r, fill=color)
    return sh.filter(ImageFilter.GaussianBlur(blur))


def save(img, name):
    img.save(os.path.join(OUT, name + ".png"))


# ================================================================ 형태별 그리기
def jelly(name, W, H, r, top, bot, rim, lip, shadow, rim_w=4, lip_h=8, press=0.0, gloss=0.55, m=14):
    """통통한 젤리 버튼. W×H = 두께 포함 전체, r = 모서리, press = 눌림(두께만큼 내려앉음)."""
    S = SS
    TW, TH = W + m * 2, H + m * 2
    img = Image.new("RGBA", (TW * S, TH * S), (0, 0, 0, 0))
    img.alpha_composite(soft_shadow(img.size, (m * S, (m + lip_h + 3) * S, (m + W) * S, (m + H + 3) * S), r * S, shadow, 7 * S))
    d = ImageDraw.Draw(img)
    d.rounded_rectangle((m * S, (m + lip_h) * S, (m + W) * S, (m + H) * S), radius=r * S, fill=lip)
    off = lip_h * 0.8 * press
    x0, y0, x1, y1 = m, m + off, m + W, m + H - lip_h + off
    d.rounded_rectangle((x0 * S, y0 * S, x1 * S, y1 * S), radius=r * S, fill=rim)
    fx0, fy0, fx1, fy1 = x0 + rim_w, y0 + rim_w, x1 - rim_w, y1 - rim_w
    put(img, vgrad(img.size, top, bot, int(fy0 * S), int(fy1 * S)),
        rmask(img.size, (fx0 * S, fy0 * S, fx1 * S, fy1 * S), (r - rim_w) * S))
    if gloss:
        fh = fy1 - fy0
        gx0, gx1 = fx0 + 10, fx1 - 10
        gy0, gy1 = fy0 + fh * 0.10, fy0 + fh * 0.42
        put(img, vgrad(img.size, (255, 255, 255, int(255 * gloss)), (255, 255, 255, int(255 * gloss * 0.25)), int(gy0 * S), int(gy1 * S)),
            rmask(img.size, (gx0 * S, gy0 * S, gx1 * S, gy1 * S), min((gy1 - gy0) / 2, r * 0.6) * S))
    save(img.resize((TW, TH), Image.LANCZOS), name)
    META[name] = {"w": TW, "h": TH, "margin": [m + r, m + r, m + r, m + r],
                  "content": [m + rim_w, m + rim_w, m + rim_w, m + rim_w + lip_h], "press": [0, lip_h * 0.8]}


def panel(name, W, H, r, top, bot, rim, rim_w, shadow, m=24):
    """크림 패널 — 얇은 톤 외곽선 + 부드러운 그림자."""
    S = SS
    TW, TH = W + m * 2, H + m * 2
    img = Image.new("RGBA", (TW * S, TH * S), (0, 0, 0, 0))
    img.alpha_composite(soft_shadow(img.size, (m * S, (m + 10) * S, (m + W) * S, (m + H + 6) * S), r * S, shadow, 12 * S))
    ImageDraw.Draw(img).rounded_rectangle((m * S, m * S, (m + W) * S, (m + H) * S), radius=r * S, fill=rim)
    put(img, vgrad(img.size, top, bot, (m + rim_w) * S, (m + H - rim_w) * S),
        rmask(img.size, ((m + rim_w) * S, (m + rim_w) * S, (m + W - rim_w) * S, (m + H - rim_w) * S), (r - rim_w) * S))
    save(img.resize((TW, TH), Image.LANCZOS), name)
    META[name] = {"w": TW, "h": TH, "margin": [m + r, m + r, m + r, m + r],
                  "content": [m + rim_w, m + rim_w, m + rim_w, m + rim_w]}


def sticker(name, W, H, r, top, bot, ow, inner, iw, shadow, hl):
    """인게임 결과 카드용 어두운 판 — 외곽선 + 안쪽 테두리 + 오프셋 그림자."""
    m = 2
    sx, sy = shadow
    TW, TH = W + sx + m * 2, H + sy + m * 2
    S = SS
    img = Image.new("RGBA", (TW * S, TH * S), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rounded_rectangle(((m + sx) * S, (m + sy) * S, (m + sx + W) * S, (m + sy + H) * S), radius=r * S, fill=INDIGO)
    x0, y0, x1, y1 = m, m, m + W, m + H
    d.rounded_rectangle((x0 * S, y0 * S, x1 * S, y1 * S), radius=r * S, fill=INDIGO)
    fx0, fy0, fx1, fy1 = x0 + ow, y0 + ow, x1 - ow, y1 - ow
    put(img, vgrad(img.size, top, bot, int(fy0 * S), int(fy1 * S)), rmask(img.size, (fx0 * S, fy0 * S, fx1 * S, fy1 * S), (r - ow) * S))
    hy1 = fy0 + (fy1 - fy0) * 0.46
    put(img, vgrad(img.size, (255, 255, 255, int(255 * hl)), (255, 255, 255, int(255 * hl * 0.15)), int(fy0 * S), int(hy1 * S)),
        rmask(img.size, ((fx0 + iw) * S, (fy0 + iw) * S, (fx1 - iw) * S, hy1 * S), (r - ow - iw) * S))
    # 반투명 선은 별도 레이어에 그려 합성 — 직접 그리면 픽셀을 덮어써 구멍이 난다
    ln = Image.new("RGBA", img.size, inner[:3] + (0,))
    ImageDraw.Draw(ln).rounded_rectangle(((fx0 + 1) * S, (fy0 + 1) * S, (fx1 - 1) * S, (fy1 - 1) * S),
                                         radius=(r - ow - 1) * S, outline=inner, width=int(iw * S))
    img.alpha_composite(ln)
    save(img.resize((TW, TH), Image.LANCZOS), name)
    edge = m + r
    META[name] = {"w": TW, "h": TH, "margin": [edge, edge, edge + sx, edge + sy],
                  "content": [m + ow + iw, m + ow + iw, m + ow + iw + sx, m + ow + iw + sy], "press": [sx * 0.8, sy * 0.8]}


# ================================================================ 버튼
PINK = dict(top=hexc("#FFA9C6"), bot=hexc("#FF7AA6"), rim=hexc("#E35C8D"), lip=hexc("#CF4579"), shadow=(214, 70, 120, 80))
CREAM = dict(top=hexc("#FFFFFF"), bot=hexc("#FFF0E2"), rim=hexc("#E8CDBA"), lip=hexc("#DDB9A3"), shadow=(150, 100, 80, 60))
for suf, k, pr in [("N", 0.0, 0.0), ("H", 0.14, 0.0), ("P", 0.0, 1.0)]:
    for prefix, c in [("T_UI_BtnPrimary_", PINK), ("T_UI_BtnSecondary_", CREAM)]:
        jelly(prefix + suf, 360, 128, 24, lighten(c["top"], k), lighten(c["bot"], k), c["rim"], c["lip"], c["shadow"], press=pr)
    jelly("T_UI_BtnRound_" + suf, 104, 104, 26, lighten(CREAM["top"], k), lighten(CREAM["bot"], k), CREAM["rim"],
          CREAM["lip"], CREAM["shadow"], press=pr, lip_h=7, m=10)

# ================================================================ 패널·이름표
panel("T_UI_PanelCream", 460, 460, 40, hexc("#FFFCF7"), hexc("#FFF1E6"), hexc("#EFD6C6"), 4, (140, 90, 80, 70))
jelly("T_UI_PillCocoa", 256, 100, 46, hexc("#7A5266"), hexc("#5E3D4E"), hexc("#5E3D4E"), hexc("#4A2E3D"),
      (60, 30, 45, 0), rim_w=0, lip_h=0, gloss=0.18, m=4)
sticker("T_UI_PanelDark", 440, 440, 30, hexc("#3A2E7A", 238), hexc("#221A4E", 242), ow=6, inner=(255, 255, 255, 46),
        iw=3, shadow=(8, 10), hl=0.06)

# ================================================================ 색 견본 — 통통한 공(코드가 색을 곱한다) + 광택
C, S2 = 128, 4
base = Image.new("RGBA", (C * S2, C * S2), (255, 255, 255, 0))
ImageDraw.Draw(base).ellipse((4 * S2, 4 * S2, 124 * S2, 124 * S2), fill=(165, 165, 165, 255))   # 톤 외곽선(색을 곱하면 진한 톤)
put(base, vgrad(base.size, (255, 255, 255, 255), (222, 222, 222, 255), 10 * S2, 118 * S2),
    rmask(base.size, (10 * S2, 10 * S2, 118 * S2, 118 * S2), 54 * S2))
save(base.resize((C, C), Image.LANCZOS), "T_UI_SwatchBase")
gl = Image.new("RGBA", (C * S2, C * S2), (255, 255, 255, 0))
ImageDraw.Draw(gl).ellipse((28 * S2, 20 * S2, 74 * S2, 50 * S2), fill=(255, 255, 255, 170))
gl = gl.filter(ImageFilter.GaussianBlur(3 * S2))
ImageDraw.Draw(gl).ellipse((34 * S2, 26 * S2, 46 * S2, 36 * S2), fill=(255, 255, 255, 240))
save(gl.resize((C, C), Image.LANCZOS), "T_UI_SwatchGloss")

# ================================================================ 3D 로비 카드 — 기존 슬롯 텍스처와 같은 866×1140 (배치 계산이 그대로 맞는다)
CW, CH = 866, 1140
BX0, BY0, BX1, BY1 = 10, 6, CW - 10, CH - 24        # 카드 몸통(그림자 여백 제외)
cr, rim, S2 = 72, 10, 2
card = Image.new("RGBA", (CW * S2, CH * S2), (0, 0, 0, 0))
card.alpha_composite(soft_shadow(card.size, (BX0 * S2, (BY0 + 14) * S2, BX1 * S2, (BY1 + 10) * S2), cr * S2, (150, 100, 90, 90), 10 * S2))
ImageDraw.Draw(card).rounded_rectangle((BX0 * S2, BY0 * S2, BX1 * S2, BY1 * S2), radius=cr * S2, fill=hexc("#EFD3C3"))
fb = ((BX0 + rim) * S2, (BY0 + rim) * S2, (BX1 - rim) * S2, (BY1 - rim) * S2)
fm = rmask(card.size, fb, (cr - rim) * S2)
put(card, vgrad(card.size, hexc("#FFFDF9"), hexc("#FFEEE2"), fb[1], fb[3]), fm)
glow = Image.new("RGBA", card.size, (255, 214, 228, 0))
ImageDraw.Draw(glow).ellipse((CW * 0.12 * S2, CH * 0.10 * S2, CW * 0.88 * S2, CH * 0.62 * S2), fill=(255, 214, 228, 210))
put(card, glow.filter(ImageFilter.GaussianBlur(60 * S2)), fm)
save(card.resize((CW, CH), Image.LANCZOS), "T_UI_LobbyCard")
META["T_UI_LobbyCard"] = {"w": CW, "h": CH, "body": [BX0, BY0, BX1, BY1], "radius": cr}


# ================================================================ 정보 패널 워터마크 — 얼굴 있는 반죽
def mochi_stamp(w, h, color=(255, 255, 255, 255)):
    S4 = 4
    im = Image.new("RGBA", (w * S4, h * S4), color[:3] + (0,))
    d = ImageDraw.Draw(im)
    d.ellipse((0, 0, w * S4, h * 1.55 * S4), fill=color)
    d.rounded_rectangle((0, h * 0.42 * S4, w * S4, h * S4), radius=h * 0.36 * S4, fill=color)
    hole = color[:3] + (0,)
    ex, ey, er = w * 0.27, h * 0.52, w * 0.065
    for cx in (ex, w - ex):
        d.ellipse(((cx - er) * S4, (ey - er * 1.2) * S4, (cx + er) * S4, (ey + er * 1.2) * S4), fill=hole)
    mw = w * 0.13
    d.arc(((w / 2 - mw) * S4, (h * 0.56) * S4, (w / 2) * S4, (h * 0.70) * S4), 0, 180, fill=hole, width=int(w * 0.035 * S4))
    d.arc(((w / 2) * S4, (h * 0.56) * S4, (w / 2 + mw) * S4, (h * 0.70) * S4), 0, 180, fill=hole, width=int(w * 0.035 * S4))
    return im.resize((w, h), Image.LANCZOS)


save(mochi_stamp(512, 420), "T_UI_MochiMark")

# ================================================================ 타이머 시계 아이콘
C, S2 = 96, 4
ic = Image.new("RGBA", (C * S2, C * S2), (255, 255, 255, 0))
d = ImageDraw.Draw(ic)
d.ellipse((8 * S2, 12 * S2, 88 * S2, 92 * S2), fill=INDIGO)                       # 그림자
d.ellipse((4 * S2, 4 * S2, 84 * S2, 84 * S2), fill=INDIGO)
d.ellipse((11 * S2, 11 * S2, 77 * S2, 77 * S2), fill=(255, 255, 255, 255))
d.line((44 * S2, 44 * S2, 44 * S2, 22 * S2), fill=INDIGO, width=7 * S2)
d.line((44 * S2, 44 * S2, 60 * S2, 52 * S2), fill=INDIGO, width=7 * S2)
d.ellipse((38 * S2, 38 * S2, 50 * S2, 50 * S2), fill=hexc("#F23A86"))
save(ic.resize((C, C), Image.LANCZOS), "T_UI_Clock")

# ================================================================ 로비 배경 패턴 1024 타일 — 왕관 + 반죽 돔 + 점
P = 1024
big = Image.new("RGBA", (P * 3, P * 3), (255, 255, 255, 0))   # 3×3로 찍고 가운데만 잘라 이음매를 없앤다
spots = [(120, 140, 118, -12, "crown"), (440, 100, 96, 10, "dome"), (770, 170, 128, 8, "crown"),
         (270, 430, 104, 14, "dome"), (610, 470, 136, -8, "crown"), (930, 520, 92, -15, "dome"),
         (110, 730, 128, 6, "dome"), (470, 810, 104, -10, "crown"), (800, 860, 120, 12, "dome"),
         (990, 960, 78, 0, "crown")]
dots = [(300, 260), (640, 300), (890, 330), (390, 610), (170, 560), (720, 680), (300, 960), (620, 960), (940, 720)]
stamps = []
for cx, cy, w, ang, kind in spots:
    mask = crown_mask(w, int(w * 0.68)) if kind == "crown" else dome_mask(w, int(w * 0.72))
    st = Image.new("RGBA", mask.size, (255, 255, 255, 0))
    st.putalpha(mask)
    stamps.append((cx, cy, st.rotate(ang, resample=Image.BICUBIC, expand=True)))
bd = ImageDraw.Draw(big)
for ox in (0, P, 2 * P):
    for oy in (0, P, 2 * P):
        for cx, cy, st in stamps:
            big.alpha_composite(st, (cx - st.width // 2 + ox, cy - st.height // 2 + oy))
        for cx, cy in dots:
            bd.ellipse((cx - 9 + ox, cy - 9 + oy, cx + 9 + ox, cy + 9 + oy), fill=(255, 255, 255, 255))
save(big.crop((P, P, 2 * P, 2 * P)), "T_UI_Pattern")

with open(os.path.join(OUT, "meta.json"), "w") as fh:
    json.dump(META, fh, indent=1)
print("generated -> %s" % OUT)
