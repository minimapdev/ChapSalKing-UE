"""생성 스프라이트를 /Game/UI/Textures로 (재)임포트 (UE Python, 커맨드렛).

사용: tools/ui/run.sh import_textures.py
같은 이름이면 덮어써서 WBP·머티리얼 참조가 그대로 유지된다.
"""
import os

import unreal

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
SRC = os.path.join(ROOT, "ArtSource", "UI", "Generated")
DEST = "/Game/UI/Textures"

tasks = []
for f in sorted(os.listdir(SRC)):
    if not f.endswith(".png"):
        continue
    t = unreal.AssetImportTask()
    t.set_editor_property("filename", os.path.join(SRC, f))
    t.set_editor_property("destination_path", DEST)
    t.set_editor_property("automated", True)
    t.set_editor_property("replace_existing", True)
    t.set_editor_property("save", False)
    tasks.append(t)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

for t in tasks:
    name = os.path.splitext(os.path.basename(t.get_editor_property("filename")))[0]
    tex = unreal.load_asset("%s/%s" % (DEST, name))
    if name == "T_UI_Pattern":
        # 배경 머티리얼에서 타일링·패닝 — 밉과 반복 주소가 필요하다
        tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_WORLD)
        tex.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP)
        tex.set_editor_property("address_x", unreal.TextureAddress.TA_WRAP)
        tex.set_editor_property("address_y", unreal.TextureAddress.TA_WRAP)
    else:
        tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
        tex.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
        tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    tex.set_editor_property("srgb", True)
    unreal.EditorAssetLibrary.save_loaded_asset(tex, False)
    unreal.log("TEX %s %dx%d" % (name, tex.blueprint_get_size_x(), tex.blueprint_get_size_y()))
unreal.log("IMPORT_TEXTURES DONE %d" % len(tasks))
