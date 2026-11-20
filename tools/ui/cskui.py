"""찹쌀킹 UI 리스타일 공용 헬퍼 (UE Python) — WBP 템플릿 위젯을 직접 수정하고 컴파일·저장한다."""
import json
import os

import unreal

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
META = json.load(open(os.path.join(ROOT, "ArtSource", "UI", "Generated", "meta.json")))
TEX_DIR = "/Game/UI/Textures"
FONT_PATH = "/Game/UI/Fonts/F_Jua"


# ---------------------------------------------------------------- 색
def srgb(hexstr, a=1.0):
    """#RRGGBB → 선형 LinearColor (Slate 색은 선형 공간)."""
    h = hexstr.lstrip("#")

    def lin(c):
        c = c / 255.0
        return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4
    return unreal.LinearColor(lin(int(h[0:2], 16)), lin(int(h[2:4], 16)), lin(int(h[4:6], 16)), a)


def slate(color):
    return unreal.SlateColor(specified_color=color)


WHITE = unreal.LinearColor(1, 1, 1, 1)
CLEAR = unreal.LinearColor(1, 1, 1, 0)
NO_SHADOW = ((0, 0), CLEAR)

# 메뉴(타이틀·로비) — 찹쌀떡 파스텔
COCOA = srgb("#5B3A4A")
COCOA_SOFT = srgb("#8A6A78")
PINK_SEL = srgb("#FF7FAA")
PINK_RIM = srgb("#C93F72")

# 인게임
NAVY = srgb("#1E1645")
GOLD = srgb("#FFD23F")
PINK_HUD = srgb("#FF3D8B")
GLASS = srgb("#0A0818", 0.42)


# ---------------------------------------------------------------- WBP 접근
class Wbp:
    def __init__(self, path):
        self.path = path
        self.name = path.split("/")[-1]
        self.bp = unreal.load_asset(path)

    def w(self, name):
        obj = unreal.find_object(None, "%s.%s:WidgetTree.%s" % (self.path, self.name, name))
        if obj is None:
            raise RuntimeError("%s: widget '%s' not found" % (self.name, name))
        return obj

    def save(self):
        unreal.BlueprintEditorLibrary.compile_blueprint(self.bp)
        ok = unreal.EditorAssetLibrary.save_loaded_asset(self.bp, False)
        unreal.log("SAVED %s -> %s" % (self.name, ok))


def margin(l, t=None, r=None, b=None):
    if t is None:
        t = r = b = l
    return unreal.Margin(l, t, r, b)


def slot_padding(widget, m):
    widget.get_editor_property("slot").set_editor_property("padding", m)


def vec2(x, y):
    # DeprecateSlateVector2D는 필드가 Python에 노출돼 있지 않아 텍스트로 넣는다
    v = unreal.DeprecateSlateVector2D()
    v.import_text("(X=%f,Y=%f)" % (x, y))
    return v


def no_shear(widget, scale=1.0, angle=0.0):
    widget.set_editor_property("render_transform", unreal.WidgetTransform(
        translation=unreal.Vector2D(0, 0), scale=unreal.Vector2D(scale, scale), shear=unreal.Vector2D(0, 0), angle=angle))


# ---------------------------------------------------------------- 텍스트
def text(tb, size, color, outline=0, outline_color=NAVY, shadow=NO_SHADOW, text_value=None, justify=None,
         outline_shadow=False):
    fi = tb.get_editor_property("font")
    fi.set_editor_property("font_object", unreal.load_asset(FONT_PATH))
    fi.set_editor_property("typeface_font_name", "Regular")
    fi.set_editor_property("size", size)
    os_ = fi.get_editor_property("outline_settings")
    os_.set_editor_property("outline_size", int(outline))
    os_.set_editor_property("outline_color", outline_color)
    os_.set_editor_property("apply_outline_to_drop_shadows", outline_shadow)
    fi.set_editor_property("outline_settings", os_)
    tb.set_editor_property("font", fi)
    tb.set_editor_property("color_and_opacity", slate(color))
    off, col = shadow
    tb.set_editor_property("shadow_offset", unreal.Vector2D(off[0], off[1]))
    tb.set_editor_property("shadow_color_and_opacity", col)
    if text_value is not None:
        tb.set_editor_property("text", text_value)
    if justify is not None:
        tb.set_editor_property("justification", justify)


# ---------------------------------------------------------------- 브러시
def rounded(fill, outline_color=None, outline_width=0.0, radius=None, size=None, outline_uses_alpha=True):
    """radius=None 이면 반높이(알약/원). 채움은 투명·테두리만 보이게 하려면 outline_uses_alpha=False."""
    b = unreal.SlateBrush()
    b.set_editor_property("draw_as", unreal.SlateBrushDrawType.ROUNDED_BOX)
    b.set_editor_property("tint_color", slate(fill))
    o = unreal.SlateBrushOutlineSettings()
    if radius is None:
        o.set_editor_property("rounding_type", unreal.SlateBrushRoundingType.HALF_HEIGHT_RADIUS)
    else:
        o.set_editor_property("rounding_type", unreal.SlateBrushRoundingType.FIXED_RADIUS)
        o.set_editor_property("corner_radii", unreal.Vector4(radius, radius, radius, radius))
    o.set_editor_property("color", slate(outline_color if outline_color is not None else CLEAR))
    o.set_editor_property("width", float(outline_width))
    o.set_editor_property("use_brush_transparency", outline_uses_alpha)
    b.set_editor_property("outline_settings", o)
    if size is not None:
        b.set_editor_property("image_size", vec2(size[0], size[1]))
    return b


def none_brush():
    b = unreal.SlateBrush()
    b.set_editor_property("draw_as", unreal.SlateBrushDrawType.NO_DRAW_TYPE)
    return b


def image_brush(texture_name, size, tint=WHITE):
    b = unreal.SlateBrush()
    b.set_editor_property("resource_object", unreal.load_asset("%s/%s" % (TEX_DIR, texture_name)))
    b.set_editor_property("image_size", vec2(size[0], size[1]))
    b.set_editor_property("tint_color", slate(tint))
    return b


def sprite(name, scale=0.5, tint=WHITE):
    """2배로 그린 스프라이트를 scale 배율로 9-slice. 여백은 gen_sprites.py가 META에 남긴 픽셀 값."""
    m = META[name]
    b = unreal.SlateBrush()
    b.set_editor_property("draw_as", unreal.SlateBrushDrawType.BOX)
    b.set_editor_property("resource_object", unreal.load_asset("%s/%s" % (TEX_DIR, name)))
    b.set_editor_property("tint_color", slate(tint))
    b.set_editor_property("image_size", vec2(m["w"] * scale, m["h"] * scale))
    l, t, r, btm = m["margin"]
    b.set_editor_property("margin", unreal.Margin(float(l) / m["w"], float(t) / m["h"], float(r) / m["w"], float(btm) / m["h"]))
    return b


def content_margin(name, scale=0.5, extra=(0, 0, 0, 0)):
    """스프라이트 앞면 안쪽 영역(외곽선·그림자·두께 제외)에 extra를 더한 여백."""
    l, t, r, b = META[name]["content"]
    return unreal.Margin(l * scale + extra[0], t * scale + extra[1], r * scale + extra[2], b * scale + extra[3])


# ---------------------------------------------------------------- 버튼·게이지
def button(btn, normal, hovered, pressed, normal_padding=None, pressed_padding=None):
    st = btn.get_editor_property("widget_style")
    st.set_editor_property("normal", normal)
    st.set_editor_property("hovered", hovered)
    st.set_editor_property("pressed", pressed)
    st.set_editor_property("disabled", normal)
    st.set_editor_property("normal_padding", normal_padding or margin(0))
    st.set_editor_property("pressed_padding", pressed_padding or margin(0))
    btn.set_editor_property("widget_style", st)
    btn.set_editor_property("background_color", WHITE)
    btn.set_editor_property("color_and_opacity", WHITE)


def chunky_button(btn, prefix, scale=0.5):
    """젤리 스프라이트 버튼(_N/_H/_P). 내용은 앞면 가운데, 누르면 두께만큼 내려앉는다."""
    name = prefix + "_N"
    px, py = [v * scale for v in META[name]["press"]]
    normal = content_margin(name, scale)
    button(btn, sprite(prefix + "_N", scale), sprite(prefix + "_H", scale), sprite(prefix + "_P", scale),
           normal, unreal.Margin(normal.left + px, normal.top + py, normal.right - px, normal.bottom - py))


def bar(pb, fill_color, bg, height, outline=None, outline_width=0.0):
    st = pb.get_editor_property("widget_style")
    st.set_editor_property("background_image", rounded(bg, outline, outline_width, size=(32, height)))
    st.set_editor_property("fill_image", rounded(WHITE, None, 0.0, size=(32, height)))
    st.set_editor_property("marquee_image", rounded(WHITE, None, 0.0, size=(32, height)))
    pb.set_editor_property("widget_style", st)
    pb.set_editor_property("fill_color_and_opacity", fill_color)
