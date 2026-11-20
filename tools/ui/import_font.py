"""주아체(Jua, OFL)를 /Game/UI/Fonts에 FontFace + Font로 임포트 (UE Python, 에디터 모드).

사용: tools/ui/run.sh import_font.py --editor
FontFace 임포트가 Slate를 요구해서 커맨드렛으로는 크래시가 난다 — 에디터를 띄워 실행하고 스스로 닫는다.
Font의 CompositeFont 내부 필드는 Python에 노출돼 있지 않아 import_text로 채운다.
"""
import os

import unreal

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
TTF = os.path.join(ROOT, "ArtSource", "Fonts", "Jua", "Jua-Regular.ttf")
DEST = "/Game/UI/Fonts"
FACE = "FF_Jua"
FONT = "F_Jua"

task = unreal.AssetImportTask()
task.set_editor_property("filename", TTF)
task.set_editor_property("destination_path", DEST)
task.set_editor_property("destination_name", FACE)
task.set_editor_property("automated", True)
task.set_editor_property("replace_existing", True)
task.set_editor_property("save", True)
tools = unreal.AssetToolsHelpers.get_asset_tools()
tools.import_asset_tasks([task])

font_path = "%s/%s" % (DEST, FONT)
font = unreal.load_asset(font_path) if unreal.EditorAssetLibrary.does_asset_exist(font_path) else \
    tools.create_asset(FONT, DEST, unreal.Font, unreal.FontFactory())
font.set_editor_property("font_cache_type", unreal.FontCacheType.RUNTIME)
cf = font.get_editor_property("composite_font")
cf.import_text('(DefaultTypeface=(Fonts=((Name="Regular",Font=(FontFaceAsset="%s/%s.%s")))))' % (DEST, FACE, FACE))
font.set_editor_property("composite_font", cf)
unreal.EditorAssetLibrary.save_loaded_asset(font, False)
unreal.log("IMPORT_FONT DONE -> %s" % font.get_editor_property("composite_font").export_text())
unreal.SystemLibrary.quit_editor()
