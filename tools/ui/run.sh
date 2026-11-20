#!/usr/bin/env bash
# UE Python 스크립트를 에디터 창 없이 실행한다. 에디터는 꺼져 있어야 한다(에셋 저장 충돌).
#
#   tools/ui/run.sh apply_style.py            # 커맨드렛(빠름, 대부분의 스크립트)
#   tools/ui/run.sh import_font.py --editor   # Slate가 필요한 작업 — 에디터를 띄웠다가 스크립트가 닫는다
#
# 엔진 경로가 다르면 UE_ENGINE 환경변수로 덮어쓴다.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
ENGINE="${UE_ENGINE:-/c/Program Files/Epic Games/UE_5.6/UE_5.6}"
UPROJECT="$(cygpath -m "$ROOT/ChapSalKing/ChapSalKing.uproject")"
SCRIPT="$(cygpath -m "$HERE/$1")"
LOG="$ROOT/ChapSalKing/Saved/Logs/ui_$(basename "${1%.py}").log"
mkdir -p "$(dirname "$LOG")"

if [ "${2:-}" = "--editor" ]; then
  MODE=(-ExecutePythonScript="$SCRIPT")
else
  MODE=(-run=pythonscript -script="$SCRIPT" -nullrhi)
fi
"$ENGINE/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$UPROJECT" "${MODE[@]}" \
  -unattended -nosplash -stdout -FullStdOutLogOutput > "$LOG" 2>&1
echo "exit=$?  log=$LOG"
# 새 위젯을 추가했을 때의 GUID ensure는 엔진이 자동으로 GUID를 채우고 넘어가므로 무시해도 된다
grep -E 'LogPython: |Traceback|Error: (Assertion|appError)' "$LOG" \
  | grep -v -E 'registering <class|init_unreal|pip-enabled|Python enabled|Using Python|Running start-up|ControlRig' \
  | sed -E 's/^\[[^]]*\]\[[^]]*\]//'
