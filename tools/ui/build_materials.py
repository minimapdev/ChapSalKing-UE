"""로비 3D 머티리얼 재구성 (UE Python, 커맨드렛) — 그래프를 지우고 다시 짠다.

사용: tools/ui/run.sh build_materials.py

- M_LobbyBackdrop_Pattern: 하늘색 그라데이션 + 퍼지는 링 + 흐르는 왕관·반죽 패턴 + 비네트 (Custom HLSL 한 노드)
- M_LobbyCard_Flat: 카드 텍스처 RGB × 밝기(톤매핑 보정), 불투명도 = 알파
두 머티리얼을 DA_CSKSetting.LobbySettings에 연결한다.
"""
import unreal

MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary
MAT_DIR = "/Game/UI/Materials"


def material(name, blend):
    path = "%s/%s" % (MAT_DIR, name)
    if EAL.does_asset_exist(path):
        m = unreal.load_asset(path)
        MEL.delete_all_material_expressions(m)
    else:
        m = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, MAT_DIR, unreal.Material, unreal.MaterialFactoryNew())
    m.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    m.set_editor_property("blend_mode", blend)
    m.set_editor_property("two_sided", True)
    return m


def expr(m, cls, x, y, **props):
    e = MEL.create_material_expression(m, cls, x, y)
    for k, v in props.items():
        e.set_editor_property(k, v)
    return e


# ---------------------------------------------------------------- 배경
BACKDROP_HLSL = """
float3 top = float3(0.16, 0.64, 0.94);
float3 bot = float3(0.03, 0.34, 0.80);
float3 col = lerp(top, bot, saturate(UV.y));
float2 c = (UV - float2(0.33, 0.50)) * float2(1.7778, 1.0);
float band = step(0.5, frac(length(c) * 5.0 - T * 0.06));
col = lerp(col, lerp(col, float3(1.0, 1.0, 1.0), 0.10), band);
col = lerp(col, float3(1.0, 1.0, 1.0), P * 0.38);
float2 v = (UV - 0.5) * float2(1.1, 1.3);
col *= 1.0 - 0.18 * dot(v, v);
return col * 1.15;
"""
bd = material("M_LobbyBackdrop_Pattern", unreal.BlendMode.BLEND_OPAQUE)
uv0 = expr(bd, unreal.MaterialExpressionTextureCoordinate, -1200, -200)
tm = expr(bd, unreal.MaterialExpressionTime, -1200, 0)
uvt = expr(bd, unreal.MaterialExpressionTextureCoordinate, -1400, 250, u_tiling=4.4, v_tiling=2.5)
pan = expr(bd, unreal.MaterialExpressionPanner, -1200, 250, speed_x=0.02, speed_y=-0.012)
MEL.connect_material_expressions(uvt, "", pan, "Coordinate")
ptex = expr(bd, unreal.MaterialExpressionTextureSample, -1000, 250, texture=unreal.load_asset("/Game/UI/Textures/T_UI_Pattern"))
MEL.connect_material_expressions(pan, "", ptex, "UVs")
cust = expr(bd, unreal.MaterialExpressionCustom, -600, 0, output_type=unreal.CustomMaterialOutputType.CMOT_FLOAT3)
inputs = []
for n in ["UV", "T", "P"]:
    ci = unreal.CustomInput()
    ci.set_editor_property("input_name", n)
    inputs.append(ci)
cust.set_editor_property("inputs", inputs)
cust.set_editor_property("code", BACKDROP_HLSL)
MEL.connect_material_expressions(uv0, "", cust, "UV")
MEL.connect_material_expressions(tm, "", cust, "T")
MEL.connect_material_expressions(ptex, "A", cust, "P")
MEL.connect_material_property(cust, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
MEL.recompile_material(bd)
EAL.save_loaded_asset(bd, False)

# ---------------------------------------------------------------- 카드
card = material("M_LobbyCard_Flat", unreal.BlendMode.BLEND_TRANSLUCENT)
ctex = expr(card, unreal.MaterialExpressionTextureSample, -600, 0, texture=unreal.load_asset("/Game/UI/Textures/T_UI_LobbyCard"))
cbr = expr(card, unreal.MaterialExpressionScalarParameter, -600, 250, parameter_name="Brightness", default_value=1.85)
cmul = expr(card, unreal.MaterialExpressionMultiply, -300, 0)
MEL.connect_material_expressions(ctex, "RGB", cmul, "A")
MEL.connect_material_expressions(cbr, "", cmul, "B")
MEL.connect_material_property(cmul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
MEL.connect_material_property(ctex, "A", unreal.MaterialProperty.MP_OPACITY)
MEL.recompile_material(card)
EAL.save_loaded_asset(card, False)

# ---------------------------------------------------------------- 설정 연결
da = unreal.load_asset("/Game/Data/DA_CSKSetting")
ls = da.get_editor_property("lobby_settings")
ls.set_editor_property("slot_card_material", card)
ls.set_editor_property("backdrop_material", bd)
da.set_editor_property("lobby_settings", ls)
EAL.save_loaded_asset(da, False)
unreal.log("BUILD_MATERIALS DONE")
