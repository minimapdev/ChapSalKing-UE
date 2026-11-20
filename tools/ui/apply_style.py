"""전 화면 UI 스타일 적용 (UE Python, 커맨드렛) — 값만 설정하므로 몇 번을 돌려도 결과가 같다.

사용: tools/ui/run.sh apply_style.py
전제: 위젯 트리 구조(TimerPill·BoardPanel·ResultCard·PanelMark·SwatchGloss 등)는 이미 WBP에 있다.
      색·크기·여백을 바꾸려면 여기 값을 고치고 다시 돌리면 된다.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import unreal  # noqa: E402
from cskui import *  # noqa: E402,F401,F403

V2 = unreal.Vector2D
H = unreal.HorizontalAlignment
VA = unreal.VerticalAlignment
LEFT = unreal.TextJustify.LEFT


def pink_label(tb, size, text_value=None):
    """분홍 젤리 위 흰 글자 — 같은 톤의 얇은 외곽선."""
    text(tb, size, WHITE, outline=2, outline_color=PINK_RIM, shadow=((0, 2), srgb("#B03566", 0.45)),
         text_value=text_value, outline_shadow=True)


def soft_heading(tb, size, text_value=None):
    """밝은 배경 위 제목 — 흰 글자 + 코코아 외곽선 + 옅은 그림자."""
    text(tb, size, WHITE, outline=3, outline_color=COCOA, shadow=((0, 3), srgb("#5B3A4A", 0.35)),
         text_value=text_value, outline_shadow=True)


def hud_outlined(tb, size, color=WHITE):
    text(tb, size, color, outline=3, outline_color=NAVY, shadow=((0, 3), srgb("#1E1645", 0.55)), outline_shadow=True)


def hud_heading(tb, size, color):
    text(tb, size, color, outline=5, outline_color=NAVY, shadow=((0, 6), srgb("#1E1645", 0.9)), outline_shadow=True)


def align(widget, h=None, v=None, padding=None):
    s = widget.get_editor_property("slot")
    if h is not None:
        s.set_editor_property("horizontal_alignment", h)
    if v is not None:
        s.set_editor_property("vertical_alignment", v)
    if padding is not None:
        s.set_editor_property("padding", padding)


def canvas(widget, anchors, alignment, pos, size=None, auto=True):
    s = widget.get_editor_property("slot")
    s.set_anchors(unreal.Anchors(minimum=V2(anchors[0], anchors[1]), maximum=V2(anchors[2], anchors[3])))
    s.set_alignment(V2(alignment[0], alignment[1]))
    s.set_position(V2(pos[0], pos[1]))
    if size is not None:
        s.set_size(V2(size[0], size[1]))
    s.set_auto_size(auto)


TOP_C = (0.5, 0, 0.5, 0)
SOFT = ((0, 1), srgb("#000000", 0.55))

# ================================================================ 타이틀
t = Wbp("/Game/Blueprints/UI/WBP_CSKTitle")
border = t.w("Border_0")
border.set_editor_property("background", none_brush())
border.set_editor_property("padding", margin(0))
slot_padding(border, margin(0, 0, 0, 56))
slot_padding(t.w("HorizontalBox_90"), margin(0))
chunky_button(t.w("SingleButton"), "T_UI_BtnPrimary")
pink_label(t.w("TextBlock_101"), 44)
slot_padding(t.w("TextBlock_101"), margin(56, 8, 56, 6))
for btn_name, txt_name in [("MultiButton", "TextBlock"), ("SettingButton", "TextBlock_2"), ("QuitButton", "TextBlock_1")]:
    chunky_button(t.w(btn_name), "T_UI_BtnSecondary")
    text(t.w(txt_name), 32, COCOA)
    slot_padding(t.w(txt_name), margin(32, 11, 32, 9))
for btn_name in ["SingleButton", "MultiButton", "SettingButton", "QuitButton"]:
    no_shear(t.w(btn_name))
    align(t.w(btn_name), v=VA.V_ALIGN_CENTER, padding=margin(8, 0, 8, 0))
t.save()

# ================================================================ 로비
l = Wbp("/Game/Blueprints/UI/WBP_CSKLobby")
title = l.w("TitleText")
soft_heading(title, 62, "반죽 선택")
no_shear(title)
align(title, H.H_ALIGN_LEFT, VA.V_ALIGN_TOP, margin(76, 30, 0, 0))
slot_padding(l.w("RootRow"), margin(70, 120, 70, 56))

l.w("PanelBgImage").set_editor_property("brush", sprite("T_UI_PanelCream"))
slot_padding(l.w("PanelFrame"), margin(0))
align(l.w("PanelColumn"), H.H_ALIGN_FILL, VA.V_ALIGN_FILL, content_margin("T_UI_PanelCream", 0.5, (36, 34, 36, 30)))
mark = l.w("PanelMark")
mark.set_editor_property("brush", image_brush("T_UI_MochiMark", (230, 189), srgb("#FF8FB1", 0.13)))
mark.set_editor_property("visibility", unreal.SlateVisibility.HIT_TEST_INVISIBLE)
align(mark, H.H_ALIGN_RIGHT, VA.V_ALIGN_BOTTOM, margin(0, 0, 34, 140))

l.w("NameFrameImage").set_editor_property("visibility", unreal.SlateVisibility.COLLAPSED)
slot_padding(l.w("NameFrame"), margin(0))
name_tb = l.w("SelectedNameText")
text(name_tb, 52, COCOA, justify=LEFT)
align(name_tb, h=H.H_ALIGN_LEFT)
desc = l.w("SelectedDescText")
text(desc, 22, COCOA_SOFT, justify=LEFT)
desc.set_editor_property("auto_wrap_text", True)
align(desc, h=H.H_ALIGN_FILL, padding=margin(0, 6, 0, 0))
div = l.w("DividerImage")
div.set_editor_property("brush", rounded(srgb("#5B3A4A", 0.10), radius=2, size=(32, 3)))
slot_padding(div, margin(0, 24, 0, 22))
dt = l.w("DoughTypeTitleText")
text(dt, 22, COCOA_SOFT)
align(dt, h=H.H_ALIGN_LEFT, padding=margin(0, 0, 0, 12))
align(l.w("PaletteSizeBox"), h=H.H_ALIGN_LEFT)

start = l.w("StartButton")
chunky_button(start, "T_UI_BtnPrimary")
no_shear(start)
slot_padding(start, margin(0))
pink_label(l.w("TextBlock_338"), 46)
slot_padding(l.w("TextBlock_338"), margin(0, 8, 0, 6))

back = l.w("BackButton")
button(back, rounded(CLEAR, radius=20), rounded(srgb("#5B3A4A", 0.06), radius=20), rounded(srgb("#5B3A4A", 0.12), radius=20))
slot_padding(back, margin(0, 8, 0, 0))
text(l.w("TextBlock_2"), 24, COCOA_SOFT)
slot_padding(l.w("TextBlock_2"), margin(0, 8, 0, 6))

for btn_name, txt_name in [("BotCountDecreaseButton", "TextBlock"), ("BotCountIncreaseButton", "TextBlock_1")]:
    chunky_button(l.w(btn_name), "T_UI_BtnRound")
    text(l.w(txt_name), 32, COCOA)
    slot_padding(l.w(txt_name), margin(12, 0, 12, 0))
soft_heading(l.w("TextBlock_62"), 32, "봇 인원")
slot_padding(l.w("TextBlock_62"), margin(0, 0, 18, 0))
soft_heading(l.w("BotCountText"), 46)
slot_padding(l.w("BotCountText"), margin(20, 0, 20, 0))
slot_padding(l.w("BotRow"), margin(0, 18, 0, 0))
l.save()

# ---------------------------------------------------------------- 로비 반죽 카드 (3D 카드 위에 얹히는 UI)
s = Wbp("/Game/Blueprints/UI/WBP_CSKLobbySlot")
cm = META["T_UI_LobbyCard"]
bx0, by0, bx1, by1 = cm["body"]
fi = s.w("FrameImage")
fi.set_editor_property("brush", rounded(CLEAR, PINK_SEL, 16.0, radius=cm["radius"], outline_uses_alpha=False))
slot_padding(fi, margin(bx0, by0, cm["w"] - bx1, cm["h"] - by1))
s.w("NameBgImage").set_editor_property("brush", sprite("T_UI_PillCocoa", 1.0))
text(s.w("SlotNameText"), 56, WHITE)
slot_padding(s.w("SlotNameText"), margin(28, 14, 28, 16))
crown = s.w("CrownImage")
align(crown, h=H.H_ALIGN_RIGHT, padding=margin(0, -110, -60, 0))
no_shear(crown, scale=0.8, angle=18.0)
s.save()

# ---------------------------------------------------------------- 색 견본 — 통통한 공 + 광택, 선택은 바깥 분홍 링
p = Wbp("/Game/Blueprints/UI/WBP_CSKDoughProfile")
slot_padding(p.w("Overlay_0"), margin(0))
p.w("SwatchImage").set_editor_property("brush", image_brush("T_UI_SwatchBase", (72, 72)))
slot_padding(p.w("SwatchImage"), margin(7))
gloss = p.w("SwatchGloss")
gloss.set_editor_property("brush", image_brush("T_UI_SwatchGloss", (72, 72)))
gloss.set_editor_property("visibility", unreal.SlateVisibility.HIT_TEST_INVISIBLE)
slot_padding(gloss, margin(7))
p.w("Image_275").set_editor_property("brush", rounded(CLEAR, PINK_SEL, 4.5, outline_uses_alpha=False))
slot_padding(p.w("Image_275"), margin(0))
p.save()

# ================================================================ 인게임 HUD — 작고 납작하게
h = Wbp("/Game/Blueprints/UI/WBP_RootHud")
board = h.w("WBP_ScoreBoard")
board.set_editor_property("max_rows", 5)
canvas(board, (0, 0, 0, 0), (0, 0), (24, 24))
pill = h.w("TimerPill")
pill.set_editor_property("background", rounded(GLASS))
pill.set_editor_property("padding", margin(18, 4, 26, 6))
pill.set_editor_property("visibility", unreal.SlateVisibility.HIT_TEST_INVISIBLE)
canvas(pill, TOP_C, (0.5, 0), (0, 20))
clock = h.w("TimerClock")
clock.set_editor_property("brush", image_brush("T_UI_Clock", (44, 44)))
align(clock, v=VA.V_ALIGN_CENTER, padding=margin(0, 0, 12, 0))
align(h.w("WBP_CSKMatchTimer"), v=VA.V_ALIGN_CENTER)
canvas(h.w("WBP_KingStatus"), TOP_C, (0.5, 0), (0, 126), size=(560, 84), auto=False)
canvas(h.w("WBP_CSKEventFeed"), TOP_C, (0.5, 0), (0, 214))
canvas(h.w("WBP_CSKStruggle"), (0.5, 1, 0.5, 1), (0.5, 1), (0, -130), size=(460, 92), auto=False)
h.save()

tm = Wbp("/Game/Blueprints/UI/WBP_CSKMatchTimer")
text(tm.w("TimeText"), 42, WHITE, shadow=((0, 2), srgb("#000000", 0.45)))
tm.save()

sb = Wbp("/Game/Blueprints/UI/WBP_ScoreBoard")
bp = sb.w("BoardPanel")
bp.set_editor_property("background", rounded(GLASS, radius=16))
bp.set_editor_property("padding", margin(16, 10, 16, 10))
text(sb.w("BoardTitle"), 18, srgb("#FFFFFF", 0.7), shadow=SOFT, text_value="순위")
slot_padding(sb.w("BoardTitle"), margin(2, 0, 0, 4))
sb.w("BoardWidth").set_width_override(272)
sb.save()

r = Wbp("/Game/Blueprints/UI/WBP_ScoreRow")
text(r.w("RankText"), 20, GOLD, shadow=SOFT)
text(r.w("NameText"), 20, WHITE, shadow=SOFT)
text(r.w("ScoreText"), 20, srgb("#FFFFFF", 0.85), shadow=SOFT, justify=unreal.TextJustify.RIGHT)
for n in ["RankText", "NameText", "ScoreText"]:
    slot_padding(r.w(n), margin(0, 2, 0, 2))
r.save()

k = Wbp("/Game/Blueprints/UI/WBP_KingStatus")
hud_heading(k.w("KingNameText"), 34, GOLD)
bar(k.w("SessionBar"), GOLD, srgb("#221A4E", 0.9), 20, NAVY, 4.0)
slot_padding(k.w("SessionBar"), margin(0, 8, 0, 0))
k.save()

st = Wbp("/Game/Blueprints/UI/WBP_CSKStruggle")
bar(st.w("StruggleBar"), PINK_HUD, srgb("#221A4E", 0.9), 26, NAVY, 4.0)
hud_outlined(st.w("TextBlock_63"), 28)
slot_padding(st.w("TextBlock_63"), margin(0, 10, 0, 0))
st.save()

f = Wbp("/Game/Blueprints/UI/WBP_CSKEventFeedRow")
hud_outlined(f.w("EventText"), 28)
f.save()

# ---------------------------------------------------------------- 결과
res = Wbp("/Game/Blueprints/UI/WBP_CSKResult")
dim = unreal.SlateBrush()
dim.set_editor_property("tint_color", slate(srgb("#120E26", 0.7)))
res.w("Image_0").set_editor_property("brush", dim)
card = res.w("ResultCard")
card.set_editor_property("background", sprite("T_UI_PanelDark"))
card.set_editor_property("padding", content_margin("T_UI_PanelDark", 0.5, (52, 40, 52, 44)))
canvas(card, (0.5, 0.5, 0.5, 0.5), (0.5, 0.5), (0, 0))
hud_heading(res.w("ResultTitle"), 62, GOLD)
align(res.w("ResultTitle"), h=H.H_ALIGN_CENTER, padding=margin(0, 0, 0, 22))
res.w("RowsSizeBox").set_width_override(520)
align(res.w("ButtonRow"), h=H.H_ALIGN_CENTER, padding=margin(0, 32, 0, 0))
chunky_button(res.w("RestartButton"), "T_UI_BtnPrimary")
chunky_button(res.w("QuitButton"), "T_UI_BtnSecondary")
pink_label(res.w("RestartButtonText"), 32, "다시 하기")
text(res.w("QuitButtonText"), 30, COCOA, text_value="게임 종료")
for n in ["RestartButton", "QuitButton"]:
    no_shear(res.w(n))
    slot_padding(res.w(n), margin(10, 0, 10, 0))
    slot_padding(res.w(n + "Text"), margin(36, 8, 36, 6))
res.save()

unreal.log("APPLY_STYLE DONE")
