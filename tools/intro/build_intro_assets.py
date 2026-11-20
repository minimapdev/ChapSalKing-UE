"""동화책 인트로 에셋 일괄 생성·갱신 (UE Python, 커맨드렛). 에디터는 꺼져 있어야 한다.

사용: tools/ui/run.sh ../intro/build_intro_assets.py
여러 번 돌려도 결과가 같다 — 있는 에셋·위젯은 찾아서 값만 다시 넣는다.

원본
  그림   ArtSource/Intro/Storybook/Pages/*.png   (확정본, Docs/ChapSalKing_IntroStorybook.md)
  책 모양 ArtSource/Intro/UI/*.png                (tools/intro/gen_book_sprites.py)
  소리   ArtSource/Intro/Audio/S_Intro_PageTurn.wav (tools/intro/gen_page_turn_sound.py)
  본문   아래 SPREADS — 스펙 문서 2절을 리치 텍스트 태그로 옮긴 것

만드는 것
  /Game/Art/UI/Intro/T_Intro_*            그림·책 모양 텍스처 (UI, 밉 없음)
  /Game/Audio/UI/S_Intro_PageTurn          책장 넘김 소리
  /Game/UI/Materials/M_UI_PageFlip         RetainerBox용 책장 넘김 (파라미터 Progress, Side, ShowLeft, Aspect, Texture)
  /Game/UI/Text/DT_IntroRichText           리치 텍스트 스타일 (Default / sfx / name)
  /Game/Data/DA_IntroBook                  UCSKIntroBookData
  /Game/Blueprints/UI/WBP_CSKIntroBook     UCSKIntroBookWidget 레이아웃
연결
  WBP_CSKMenuRoot.ScreenSwitcher 4번째 슬롯 = IntroScreen (ECSKMenuScreen::Intro), 맨 위 TransitionVeil
  WBP_CSKTitle 오른쪽 위 ReplayIntroButton
  DA_CSKSetting.UISettings.IntroBookData
"""
import json
import os
import sys

import unreal

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
sys.path.append(os.path.join(ROOT, "tools", "ui"))
from cskui import (COCOA, FONT_PATH, WHITE, chunky_button, margin, rounded, srgb, text, vec2)  # noqa: E402

EAL = unreal.EditorAssetLibrary
EUL = unreal.EditorUtilityLibrary
MEL = unreal.MaterialEditingLibrary
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()

PAGES_DIR = os.path.join(ROOT, "ArtSource", "Intro", "Storybook", "Pages")
BOOK_UI_DIR = os.path.join(ROOT, "ArtSource", "Intro", "UI")
SOUND_FILE = os.path.join(ROOT, "ArtSource", "Intro", "Audio", "S_Intro_PageTurn.wav")

TEX_DIR = "/Game/Art/UI/Intro"
AUDIO_DIR = "/Game/Audio/UI"
MAT_DIR = "/Game/UI/Materials"
TEXT_DIR = "/Game/UI/Text"
DA_PATH = "/Game/Data/DA_IntroBook"
WBP_DIR = "/Game/Blueprints/UI"

A = unreal.CSKIntroTextAnchor
# (그림 파일, 본문 자리, 본문) — 본문 줄바꿈은 \n, 의성어 <sfx>, 이름 <name>
# 한 줄 = 한 호흡, 17자 안팎 — 본문 칸(WRAP_WIDTH)에서 다시 접히지 않는 길이
SPREADS = [
    ("P01", A.TOP_LEFT, "1년에 단 한 번, 보름달 뜨는 밤.\n"
                        "달나라 토끼들이 선물을 보내요."),
    ("P02", A.TOP_RIGHT, "세상에 단 하나뿐인 절구공이를\n"
                         "<sfx>휘이익!</> 저 아래 섬으로 던졌어요."),
    ("P03", A.TOP_LEFT, "<sfx>쿵!</>\n"
                        "공이가 섬 한가운데 꽂혔어요."),
    ("P04", A.TOP_LEFT, "반죽들이 번쩍 눈을 떴어요.\n"
                        "공이를 달의 절구로 가져가면 <name>찹쌀킹</>!"),
    ("P05", A.TOP_RIGHT, "재료를 잔뜩 묻히고\n"
                         "<sfx>퐁! 퐁! 철퍽!</> 서로 쏘아 대요."),
    ("P06", A.TOP_LEFT, "<sfx>쿵! 데굴데굴!</>\n"
                        "공이 주인이 쉴 새 없이 바뀌어요."),
    ("P07", A.TOP_LEFT, "공이를 차지한 건 찹쌀반죽!\n"
                        "<sfx>뽀잉, 뽀잉!</> 널뛰기판으로!"),
    ("P08", A.TOP_LEFT, "널뛰기판을 <sfx>쿵!</> 구르고 <sfx>슝!</>\n"
                        "달의 절구 속으로 <sfx>퐁당!</>"),
    ("P09", A.TOP_RIGHT, "짜잔, <name>찹쌀킹</> 탄생!\n"
                         "이번엔 누가 <name>찹쌀킹</>이 될까요?"),
]
COVER_FILE = "P00_cover"
SUBTITLE = "달나라 절구와 여덟 반죽 이야기"
SKIP_HINT = "길게 누르면 건너뛰기"
REPLAY_LABEL = "인트로 다시 보기"

# 디자인 기준 크기 — 펼침 1536×1024 (그림 원본 해상도). 닫힌 책의 표지 칸 = 펼침의 오른쪽 절반
SPREAD_SIZE = (1536, 1024)
COVER_SIZE = (1024, 1536)
COVER_HALF = (SPREAD_SIZE[0] // 2, SPREAD_SIZE[1])

# 책 판(그림자·표지 판·책장 두께를 담는 틀). 펼침 칸(PAGES_MARGIN)은 정확히 그림 크기가 되도록 잡았다.
# 판 층은 왼쪽 절반 기준 여백(L, T, R, B) — 오른쪽 절반은 좌우를 뒤집는다. 두 절반은 책등에서 OVERLAP만큼 겹쳐 이음매를 감춘다.
# 그림자 칸 = 판 칸을 64(스프라이트 흐림 폭)만큼 넓히고 아래로 20 내린 것. 책등 쪽 끝은 판 밑에 숨는다
SPREAD_FRAME = (1760, 1250)
PAGES_MARGIN = (112, 100, 112, 126)
SPINE = SPREAD_FRAME[0] // 2
OVERLAP = 8
HALF_LAYERS = {
    "Shadow": (26, 38, SPINE, 12),
    "Board": (90, 82, SPINE - OVERLAP, 96),
    "EdgeBack": (102, 94, SPINE - OVERLAP, 110),
    "EdgeFront": (107, 97, SPINE - OVERLAP, 118),
}
BOARD_HEX, BOARD_RIM_HEX = "#6E4A5A", "#4E3240"
EDGE_BACK_HEX, EDGE_BACK_RIM_HEX = "#E3D6BE", "#C9B99C"
EDGE_FRONT_HEX, EDGE_FRONT_RIM_HEX = "#F1E8D6", "#D3C5AA"
BACKDROP_TINT = ("#1C1828", 0.55)
# 본문 칸 최대 너비 — 펼침 반쪽(768)에서 카드 여백 56·안쪽 여백 30×2를 뺀 만큼. 카드가 책등을 넘지 않는다
WRAP_WIDTH = 640.0

H = unreal.HorizontalAlignment
V = unreal.VerticalAlignment
VIS = unreal.SlateVisibility


def save(asset):
    ok = EAL.save_loaded_asset(asset, False)
    unreal.log("SAVED %s -> %s" % (asset.get_name(), ok))


# ================================================================ 1. 그림 텍스처
def import_file(filename, dest_dir, dest_name):
    t = unreal.AssetImportTask()
    t.set_editor_property("filename", filename)
    t.set_editor_property("destination_path", dest_dir)
    t.set_editor_property("destination_name", dest_name)
    t.set_editor_property("automated", True)
    t.set_editor_property("replace_existing", True)
    t.set_editor_property("save", False)
    TOOLS.import_asset_tasks([t])
    return unreal.load_asset("%s/%s" % (dest_dir, dest_name))


def import_art(file_stem, asset_name):
    tex = import_file(os.path.join(PAGES_DIR, file_stem + ".png"), TEX_DIR, asset_name)
    tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    tex.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_BC7)
    tex.set_editor_property("srgb", True)
    save(tex)
    return tex


cover_tex = import_art(COVER_FILE, "T_Intro_Cover")
spread_tex = [import_art(stem, "T_Intro_" + stem) for stem, _, _ in SPREADS]


def import_book_sprite(name):
    tex = import_file(os.path.join(BOOK_UI_DIR, name + ".png"), TEX_DIR, name)
    tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    tex.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    tex.set_editor_property("srgb", True)
    save(tex)
    return tex


shadow_tex = import_book_sprite("T_Intro_SoftShadow")
gutter_tex = import_book_sprite("T_Intro_Gutter")
vignette_tex = import_book_sprite("T_Intro_Vignette")

# ================================================================ 2. 책장 넘김 소리
turn_sound = import_file(SOUND_FILE, AUDIO_DIR, "S_Intro_PageTurn")
save(turn_sound)

# ================================================================ 3. 책장 넘김 머티리얼
# RetainerBox가 펼침 하나를 그린 렌더 타깃을 Texture로 넘긴다.
# 오른쪽 장이 종이처럼 원통에 말려 넘어가 왼쪽에 내려앉는다(책등에 붙어 있어 날아가지 않는다).
#   - 말리는 선: 오른쪽 아래 모서리에서 시작해 기울기를 풀며 책등까지 온다. 끝에서 책등과 겹친다
#   - 말림 반지름: 처음·끝은 0(평평), 가운데서 가장 크다
#   - 말린 원통의 아랫면 = 앞면(지금 펼침), 윗면과 넘어간 날개 = 뒷면(다음 펼침의 왼쪽 절반이 놓일 자리)
#   Progress  0 = 오른쪽에 평평, 1 = 왼쪽에 내려앉음
#   Side      0 = 앞면 겹: 왼쪽 절반 그대로(ShowLeft로 숨김), 오른쪽은 평평한 부분 + 원통 아랫면 + 아래 장에 드리운 그림자
#             1 = 뒷면 겹: 원통 윗면과 날개 + 날개가 그 밑 장에 드리운 그림자. 다음 펼침을 거울로 비춰 그 장이 내려앉을 자리의 그림을 그린다
#   Aspect    펼침의 가로/세로 비 — 말림이 찌그러지지 않게 좌표를 맞춘다
FLIP_HLSL = """
float A = Aspect;
float2 q = float2(UV.x * A, UV.y);
float lifted = sin(PI * Progress);
float2 n = normalize(float2(1.0, 0.3 * pow(1.0 - Progress, 1.5)));
float bx = lerp(A + 0.02, 0.5 * A, Progress);
float R = 0.11 * lifted;
float d = dot(q - float2(bx, 1.0), n);
float4 result = float4(0.0, 0.0, 0.0, 0.0);

if (Side < 0.5)
{
    if (q.x < 0.5 * A)
    {
        float4 t = Texture2DSample(Tex, TexSampler, UV);
        result = float4(t.rgb, t.a * ShowLeft);
    }
    else if (d < 0.0)
    {
        float4 t = Texture2DSample(Tex, TexSampler, UV);
        float shade = 1.0 - 0.28 * lifted * saturate(1.0 + d / max(2.0 * R, 0.001));
        result = float4(t.rgb * shade, t.a);
    }
    else if (d < R)
    {
        float a = asin(d / R);
        float2 qo = q + n * (R * a - d);
        if (qo.x >= 0.5 * A && qo.x <= A && qo.y >= 0.0 && qo.y <= 1.0)
        {
            float4 t = Texture2DSample(Tex, TexSampler, float2(qo.x / A, qo.y));
            result = float4(t.rgb * (0.55 + 0.45 * cos(a)), t.a);
        }
    }
    else
    {
        result = float4(0.0, 0.0, 0.0, 0.35 * lifted * exp(-(d - R) / 0.10));
    }
}
else
{
    float2 qo = float2(-1.0, -1.0);
    float shade = 1.0;
    if (d < 0.0)
    {
        qo = q + n * (PI * R - 2.0 * d);
        shade = 0.94;
    }
    else if (d < R)
    {
        float a = asin(d / R);
        qo = q + n * (R * (PI - a) - d);
        shade = 0.78 + 0.22 * cos(a);
    }
    if (qo.x >= 0.5 * A && qo.x <= A && qo.y >= 0.0 && qo.y <= 1.0)
    {
        float4 t = Texture2DSample(Tex, TexSampler, float2(1.0 - qo.x / A, qo.y));
        result = float4(t.rgb * shade, t.a);
    }
    else if (d < 0.0 && qo.x > A && qo.y >= 0.0 && qo.y <= 1.0)
    {
        result = float4(0.0, 0.0, 0.0, 0.28 * lifted * exp(-(qo.x - A) / 0.12));
    }
}
return result;
"""


def expr(m, cls, x, y, **props):
    e = MEL.create_material_expression(m, cls, x, y)
    for k, v in props.items():
        e.set_editor_property(k, v)
    return e


flip_path = "%s/M_UI_PageFlip" % MAT_DIR
if EAL.does_asset_exist(flip_path):
    flip = unreal.load_asset(flip_path)
    MEL.delete_all_material_expressions(flip)
else:
    flip = TOOLS.create_asset("M_UI_PageFlip", MAT_DIR, unreal.Material, unreal.MaterialFactoryNew())
flip.set_editor_property("material_domain", unreal.MaterialDomain.MD_UI)
flip.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)

uv = expr(flip, unreal.MaterialExpressionTextureCoordinate, -900, -260)
progress = expr(flip, unreal.MaterialExpressionScalarParameter, -900, -120, parameter_name="Progress", default_value=0.0)
side = expr(flip, unreal.MaterialExpressionScalarParameter, -900, 0, parameter_name="Side", default_value=0.0)
show_left = expr(flip, unreal.MaterialExpressionScalarParameter, -900, 120, parameter_name="ShowLeft", default_value=1.0)
aspect = expr(flip, unreal.MaterialExpressionScalarParameter, -900, 200, parameter_name="Aspect", default_value=1.5)
tex_obj = expr(flip, unreal.MaterialExpressionTextureObjectParameter, -900, 300, parameter_name="Texture",
               texture=unreal.load_asset("/Engine/EngineResources/DefaultTexture"))
cust = expr(flip, unreal.MaterialExpressionCustom, -500, 0, output_type=unreal.CustomMaterialOutputType.CMOT_FLOAT4)
inputs = []
for n in ["UV", "Progress", "Side", "ShowLeft", "Aspect", "Tex"]:
    ci = unreal.CustomInput()
    ci.set_editor_property("input_name", n)
    inputs.append(ci)
cust.set_editor_property("inputs", inputs)
cust.set_editor_property("code", FLIP_HLSL)
for src, pin in [(uv, "UV"), (progress, "Progress"), (side, "Side"), (show_left, "ShowLeft"), (aspect, "Aspect"), (tex_obj, "Tex")]:
    MEL.connect_material_expressions(src, "", cust, pin)
rgb = expr(flip, unreal.MaterialExpressionComponentMask, -200, -60, r=True, g=True, b=True, a=False)
alpha = expr(flip, unreal.MaterialExpressionComponentMask, -200, 80, r=False, g=False, b=False, a=True)
MEL.connect_material_expressions(cust, "", rgb, "")
MEL.connect_material_expressions(cust, "", alpha, "")
MEL.connect_material_property(rgb, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
MEL.connect_material_property(alpha, "", unreal.MaterialProperty.MP_OPACITY)
MEL.recompile_material(flip)
save(flip)

# ================================================================ 4. 리치 텍스트 스타일
COCOA_HEX = "#5B3A4A"
SFX_HEX = "#E0763F"
NAME_HEX = "#8A5FC0"


def style_row(name, size, hex_color):
    c = srgb(hex_color)
    return {
        "Name": name,
        "TextStyle": {
            "Font": {"FontObject": FONT_PATH + "." + FONT_PATH.split("/")[-1], "TypefaceFontName": "Regular", "Size": size},
            "ColorAndOpacity": {"SpecifiedColor": {"R": c.r, "G": c.g, "B": c.b, "A": 1.0},
                                "ColorUseRule": "UseColor_Specified"},
        },
    }


dt_path = "%s/DT_IntroRichText" % TEXT_DIR
if EAL.does_asset_exist(dt_path):
    rich_styles = unreal.load_asset(dt_path)
else:
    dtf = unreal.DataTableFactory()
    dtf.set_editor_property("struct", unreal.RichTextStyleRow.static_struct())
    rich_styles = TOOLS.create_asset("DT_IntroRichText", TEXT_DIR, unreal.DataTable, dtf)
unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(rich_styles, json.dumps([
    style_row("Default", 34, COCOA_HEX),
    style_row("sfx", 42, SFX_HEX),
    style_row("name", 38, NAME_HEX),
]))
unreal.log("RICH ROWS %s" % [str(n) for n in unreal.DataTableFunctionLibrary.get_data_table_row_names(rich_styles)])
save(rich_styles)

# ================================================================ 5. 책 데이터
if EAL.does_asset_exist(DA_PATH):
    book = unreal.load_asset(DA_PATH)
else:
    daf = unreal.DataAssetFactory()
    daf.set_editor_property("data_asset_class", unreal.CSKIntroBookData)
    book = TOOLS.create_asset("DA_IntroBook", "/Game/Data", unreal.CSKIntroBookData, daf)
spreads = []
for (stem, anchor, body), tex in zip(SPREADS, spread_tex):
    s = unreal.CSKIntroSpread()
    s.set_editor_property("art", tex)
    s.set_editor_property("body", body)
    s.set_editor_property("text_anchor", anchor)
    spreads.append(s)
book.set_editor_property("cover_art", cover_tex)
book.set_editor_property("spreads", spreads)
book.set_editor_property("turn_sound", turn_sound)
save(book)

# ================================================================ 6. WBP_CSKIntroBook
wbp_path = "%s/WBP_CSKIntroBook" % WBP_DIR
if EAL.does_asset_exist(wbp_path):
    intro_wbp = unreal.load_asset(wbp_path)
else:
    wf = unreal.WidgetBlueprintFactory()
    wf.set_editor_property("parent_class", unreal.CSKIntroBookWidget)
    intro_wbp = TOOLS.create_asset("WBP_CSKIntroBook", WBP_DIR, unreal.WidgetBlueprint, wf)


def ensure(wbp, cls, name, parent):
    w = EUL.find_source_widget_by_name(wbp, name)
    return w if w is not None else EUL.add_source_widget(wbp, cls, name, parent)


def fill_slot(w, padding=None):
    s = w.get_editor_property("slot")
    s.set_editor_property("horizontal_alignment", H.H_ALIGN_FILL)
    s.set_editor_property("vertical_alignment", V.V_ALIGN_FILL)
    s.set_editor_property("padding", padding or margin(0))


def place(w, h, v, padding):
    s = w.get_editor_property("slot")
    s.set_editor_property("horizontal_alignment", h)
    s.set_editor_property("vertical_alignment", v)
    s.set_editor_property("padding", padding)


def texture_brush(tex, size):
    b = unreal.SlateBrush()
    b.set_editor_property("resource_object", tex)
    b.set_editor_property("image_size", vec2(size[0], size[1]))
    return b


def scaled_frame(prefix, size, parent):
    """화면에 맞춰 비율을 지키며 늘었다 줄었다 하는 size 크기 판. 안쪽 SizeBox를 돌려준다."""
    sb = ensure(intro_wbp, unreal.ScaleBox, prefix + "Scale", parent)
    fill_slot(sb, margin(24))
    sb.set_editor_property("stretch", unreal.Stretch.SCALE_TO_FIT)
    sz = ensure(intro_wbp, unreal.SizeBox, prefix + "Size", sb.get_name())
    sz.set_width_override(size[0])
    sz.set_height_override(size[1])
    return sz


def soft_shadow_brush():
    b = texture_brush(shadow_tex, (256, 256))
    b.set_editor_property("draw_as", unreal.SlateBrushDrawType.BOX)
    b.set_editor_property("margin", unreal.Margin(0.375, 0.375, 0.375, 0.375))
    b.set_editor_property("tint_color", unreal.SlateColor(specified_color=srgb("#FFFFFF", 0.85)))
    return b


def book_half(side):
    """책의 한쪽 절반 — 그림자 → 표지 판 → 책장 두께 두 겹(아래층부터). 왼쪽(BookLeft)은 닫힌 책일 때 C++이 투명하게 한다."""
    group = ensure(intro_wbp, unreal.Overlay, "Book" + side, "SpreadBook")
    fill_slot(group)
    group.set_editor_property("visibility", VIS.HIT_TEST_INVISIBLE)
    brushes = {
        "Shadow": soft_shadow_brush(),
        "Board": rounded(srgb(BOARD_HEX), srgb(BOARD_RIM_HEX), 1.5, radius=16),
        "EdgeBack": rounded(srgb(EDGE_BACK_HEX), srgb(EDGE_BACK_RIM_HEX), 1.0, radius=10),
        "EdgeFront": rounded(srgb(EDGE_FRONT_HEX), srgb(EDGE_FRONT_RIM_HEX), 1.0, radius=8),
    }
    for n in ["Shadow", "Board", "EdgeBack", "EdgeFront"]:
        l, t, r, b = HALF_LAYERS[n]
        w = ensure(intro_wbp, unreal.Image, n + side, group.get_name())
        fill_slot(w, margin(l, t, r, b) if side == "Left" else margin(r, t, l, b))
        w.set_editor_property("brush", brushes[n])
        w.set_editor_property("visibility", VIS.HIT_TEST_INVISIBLE)


def flip_layer(name):
    """펼침 하나를 그려 M_UI_PageFlip으로 잘라 보여 주는 겹. Side·Progress는 C++이 정한다."""
    rb = ensure(intro_wbp, unreal.RetainerBox, name, "SpreadStack")
    fill_slot(rb)
    rb.set_effect_material(flip)
    rb.set_texture_parameter("Texture")
    rb.set_editor_property("visibility", VIS.HIT_TEST_INVISIBLE)
    return rb


def page(prefix, parent):
    """그림 한 장 + 가운데 접힘 그림자 + 본문 카드. 카드의 정렬(본문 자리)은 C++이 펼침마다 바꾼다.
    접힘 그림자를 장마다 넣어 두어 책장이 말려 넘어갈 때 그림과 함께 넘어간다."""
    art = ensure(intro_wbp, unreal.Image, prefix + "Art", parent)
    fill_slot(art)
    art.set_editor_property("brush", texture_brush(spread_tex[0], SPREAD_SIZE))
    card = ensure(intro_wbp, unreal.Border, prefix + "TextCard", parent)
    place(card, H.H_ALIGN_LEFT, V.V_ALIGN_TOP, margin(56))
    card.set_editor_property("background", rounded(srgb("#FFF8EC", 0.90), srgb("#8C6E50", 0.35), 1.5, radius=14))
    card.set_editor_property("padding", margin(30, 22, 30, 22))
    body = ensure(intro_wbp, unreal.RichTextBlock, prefix + "Body", card.get_name())
    body.set_editor_property("text_style_set", rich_styles)
    body.set_editor_property("wrap_text_at", WRAP_WIDTH)
    body.set_editor_property("line_height_percentage", 1.15)
    body.set_editor_property("text", SPREADS[0][2])
    gutter = ensure(intro_wbp, unreal.Image, prefix + "Gutter", parent)
    place(gutter, H.H_ALIGN_CENTER, V.V_ALIGN_FILL, margin(0))
    gutter.set_editor_property("brush", texture_brush(gutter_tex, (220, 8)))
    for w in (art, card, body, gutter):
        w.set_editor_property("visibility", VIS.HIT_TEST_INVISIBLE)


root = ensure(intro_wbp, unreal.Overlay, "Root", "")

# 배경: 흐린 타이틀 키아트 + 어둡게 누르는 막 + 비네트.
# 입력을 받는 면은 Backdrop 하나(Visible). 나머지는 전부 클릭을 통과시켜 위젯 자신이 받는다
backdrop_art = ensure(intro_wbp, unreal.Image, "BackdropArt", "Root")
fill_slot(backdrop_art)
backdrop_art.set_editor_property("brush", texture_brush(unreal.load_asset("/Game/Art/UI/ChapssalKing_Title_Background_4K"), (1920, 1080)))
backdrop_blur = ensure(intro_wbp, unreal.BackgroundBlur, "BackdropBlur", "Root")
fill_slot(backdrop_blur)
backdrop_blur.set_editor_property("blur_strength", 24.0)
backdrop = ensure(intro_wbp, unreal.Image, "Backdrop", "Root")
fill_slot(backdrop)
backdrop.set_editor_property("brush", rounded(srgb(BACKDROP_TINT[0], BACKDROP_TINT[1]), radius=0))
backdrop.set_editor_property("visibility", VIS.VISIBLE)
vignette = ensure(intro_wbp, unreal.Image, "BackdropVignette", "Root")
fill_slot(vignette)
vignette.set_editor_property("brush", texture_brush(vignette_tex, (1920, 1080)))
for w in (backdrop_art, backdrop_blur, vignette):
    w.set_editor_property("visibility", VIS.HIT_TEST_INVISIBLE)

# 책: 왼쪽·오른쪽 절반의 판 위에 펼침 세 겹(Under → Front → Back)
spread_size = scaled_frame("Spread", SPREAD_FRAME, "Root")
spread_book = ensure(intro_wbp, unreal.Overlay, "SpreadBook", spread_size.get_name())
book_half("Left")
book_half("Right")
spread_stack = ensure(intro_wbp, unreal.Overlay, "SpreadStack", "SpreadBook")
fill_slot(spread_stack, margin(*PAGES_MARGIN))
for w in (spread_book, spread_stack):
    w.set_editor_property("visibility", VIS.HIT_TEST_INVISIBLE)

flip_layer("UnderRetainer")
under = ensure(intro_wbp, unreal.Overlay, "UnderPage", "UnderRetainer")
page("Under", "UnderPage")

flip_layer("PageRetainer")
front = ensure(intro_wbp, unreal.Overlay, "FrontContent", "PageRetainer")
top = ensure(intro_wbp, unreal.Overlay, "TopPage", "FrontContent")
fill_slot(top)
page("Top", "TopPage")

# 닫힌 책의 표지 — Front의 오른쪽 절반. 세로 2:3 표지 그림을 3:4 칸에 꽉 채워(위아래 조금 잘림) 놓는다
cover_half = ensure(intro_wbp, unreal.SizeBox, "CoverHalf", "FrontContent")
place(cover_half, H.H_ALIGN_RIGHT, V.V_ALIGN_FILL, margin(0))
cover_half.set_width_override(COVER_HALF[0])
cover_half.set_editor_property("clipping", unreal.WidgetClipping.CLIP_TO_BOUNDS)
cover_page = ensure(intro_wbp, unreal.Overlay, "CoverPage", "CoverHalf")
cover_fill = ensure(intro_wbp, unreal.ScaleBox, "CoverArtFill", "CoverPage")
fill_slot(cover_fill)
cover_fill.set_editor_property("stretch", unreal.Stretch.SCALE_TO_FILL)
cover_art = ensure(intro_wbp, unreal.Image, "CoverArt", "CoverArtFill")
cover_art.set_editor_property("brush", texture_brush(cover_tex, COVER_SIZE))
logo_tex = unreal.load_asset("/Game/Art/UI/Logo")
logo = ensure(intro_wbp, unreal.Image, "CoverLogo", "CoverPage")
logo_w = 600
logo_h = logo_w * logo_tex.blueprint_get_size_y() / logo_tex.blueprint_get_size_x()
place(logo, H.H_ALIGN_CENTER, V.V_ALIGN_TOP, margin(0, 56, 0, 0))
logo.set_editor_property("brush", texture_brush(logo_tex, (logo_w, logo_h)))
subtitle = ensure(intro_wbp, unreal.TextBlock, "CoverSubtitle", "CoverPage")
place(subtitle, H.H_ALIGN_CENTER, V.V_ALIGN_TOP, margin(0, 56 + logo_h + 12, 0, 0))
text(subtitle, 34, WHITE, outline=3, outline_color=COCOA, shadow=((0, 3), srgb("#5B3A4A", 0.35)),
     text_value=SUBTITLE, outline_shadow=True)
for w in (front, cover_half, cover_page, cover_fill, cover_art, logo, subtitle):
    w.set_editor_property("visibility", VIS.HIT_TEST_INVISIBLE)

flip_layer("BackRetainer")
back = ensure(intro_wbp, unreal.Overlay, "BackPage", "BackRetainer")
page("Back", "BackPage")
for w in (under, back):
    w.set_editor_property("visibility", VIS.HIT_TEST_INVISIBLE)

# 건너뛰기 안내 — 누르고 있는 동안만 게이지가 찬다(C++)
hint_row = ensure(intro_wbp, unreal.HorizontalBox, "SkipHint", "Root")
place(hint_row, H.H_ALIGN_RIGHT, V.V_ALIGN_BOTTOM, margin(0, 0, 40, 28))
hint_row.set_editor_property("visibility", VIS.HIT_TEST_INVISIBLE)
gauge_box = ensure(intro_wbp, unreal.SizeBox, "SkipGaugeSize", "SkipHint")
gauge_box.set_width_override(140)
gauge_box.set_height_override(10)
gauge_box.get_editor_property("slot").set_editor_property("vertical_alignment", V.V_ALIGN_CENTER)
gauge_box.get_editor_property("slot").set_editor_property("padding", margin(0, 0, 14, 0))
gauge = ensure(intro_wbp, unreal.ProgressBar, "SkipGauge", "SkipGaugeSize")
st = gauge.get_editor_property("widget_style")
st.set_editor_property("background_image", rounded(srgb("#FFF8EC", 0.25), radius=5, size=(32, 10)))
st.set_editor_property("fill_image", rounded(WHITE, radius=5, size=(32, 10)))
gauge.set_editor_property("widget_style", st)
gauge.set_editor_property("fill_color_and_opacity", srgb("#FF7FAA"))
gauge.set_editor_property("visibility", VIS.HIDDEN)
hint = ensure(intro_wbp, unreal.TextBlock, "SkipHintText", "SkipHint")
text(hint, 22, srgb("#FFF8EC", 0.75), text_value=SKIP_HINT)

unreal.BlueprintEditorLibrary.compile_blueprint(intro_wbp)
save(intro_wbp)

# ================================================================ 7. 연결
# 루트 화면 전환기 — 슬롯 순서 = ECSKMenuScreen (Title, RoomList, Lobby, Intro)
menu_root = unreal.load_asset("%s/WBP_CSKMenuRoot" % WBP_DIR)
switcher = EUL.find_source_widget_by_name(menu_root, "ScreenSwitcher")
if EUL.find_source_widget_by_name(menu_root, "IntroScreen") is None:
    EUL.add_source_widget(menu_root, intro_wbp.generated_class(), "IntroScreen", "ScreenSwitcher")
names = [c.get_name() for c in switcher.get_all_children()]
assert names.index("IntroScreen") == int(unreal.CSKMenuScreen.INTRO.value), names
# 전환 막 — 루트 캔버스 전체를 덮고 모든 화면보다 위. 평소엔 접혀 있고 C++이 전환 때만 켠다
veil = ensure(menu_root, unreal.Image, "TransitionVeil", switcher.get_parent().get_name())
veil_slot = veil.get_editor_property("slot")
veil_slot.set_anchors(unreal.Anchors(minimum=unreal.Vector2D(0, 0), maximum=unreal.Vector2D(1, 1)))
veil_slot.set_offsets(unreal.Margin(0, 0, 0, 0))
veil_slot.set_z_order(100)
veil.set_editor_property("brush", rounded(srgb("#1C1828"), radius=0))
veil.set_editor_property("visibility", VIS.COLLAPSED)
unreal.BlueprintEditorLibrary.compile_blueprint(menu_root)
save(menu_root)

# 타이틀 오른쪽 위 "인트로 다시 보기"
title = unreal.load_asset("%s/WBP_CSKTitle" % WBP_DIR)
title_root = EUL.find_source_widget_by_name(title, "Border_0").get_parent()
replay = ensure(title, unreal.Button, "ReplayIntroButton", title_root.get_name())
place(replay, H.H_ALIGN_RIGHT, V.V_ALIGN_TOP, margin(0, 36, 44, 0))
chunky_button(replay, "T_UI_BtnSecondary")
replay_text = ensure(title, unreal.TextBlock, "ReplayIntroText", "ReplayIntroButton")
text(replay_text, 24, COCOA, text_value=REPLAY_LABEL)
replay_text.get_editor_property("slot").set_editor_property("padding", margin(22, 8, 22, 6))
unreal.BlueprintEditorLibrary.compile_blueprint(title)
save(title)

# 프로젝트 설정
settings = unreal.load_asset("/Game/Data/DA_CSKSetting")
ui = settings.get_editor_property("ui_settings")
ui.set_editor_property("intro_book_data", book)
settings.set_editor_property("ui_settings", ui)
save(settings)

unreal.log("BUILD_INTRO DONE")
