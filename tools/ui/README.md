# tools/ui — UI 리스타일 파이프라인

에디터를 열지 않고 UI 아트·위젯 스타일을 만들고 적용한다. **실행 중엔 에디터를 꺼 둘 것**(에셋 저장 충돌).

| 단계 | 명령 | 하는 일 |
| --- | --- | --- |
| 1. 스프라이트 생성 | `python tools/ui/gen_sprites.py` | `ArtSource/UI/Generated/*.png` + `meta.json`(9-slice 여백). 시스템 Python + Pillow |
| 2. 텍스처 임포트 | `tools/ui/run.sh import_textures.py` | `/Game/UI/Textures`에 덮어쓰기(참조 유지) |
| 3. 로비 머티리얼 | `tools/ui/run.sh build_materials.py` | 로비 배경·카드 머티리얼 재구성 + `DA_CSKSetting` 연결 |
| 4. 위젯 스타일 | `tools/ui/run.sh apply_style.py` | 타이틀·로비·HUD·결과 WBP의 색·폰트·여백·브러시 |
| (최초 1회) 폰트 | `tools/ui/run.sh import_font.py --editor` | 주아체(OFL, `ArtSource/Fonts/Jua`) → `/Game/UI/Fonts/F_Jua` |
| 확인 | `.\tools\ui\capture.ps1 -OutFile shot.png [-Map ...] [-Clicks ...]` | 게임을 띄워 스크린샷(PowerShell) |

- 색·크기만 바꿀 땐 `apply_style.py` 값만 고쳐 4단계만 다시 돌리면 된다. 여러 번 돌려도 결과가 같다.
- 스프라이트 모양을 바꾸면 1→2→4 순서로.
- `run.sh`는 Git Bash 기준. 엔진 경로가 다르면 `UE_ENGINE` 환경변수로 지정.
- 위젯을 새로 추가하는 작업은 이 도구 범위 밖이다(`apply_style.py`는 기존 위젯 값만 바꾼다). 새 위젯을 Python으로 넣으면 컴파일 때 GUID ensure가 한 번 찍히는데, 엔진이 GUID를 채우고 넘어가므로 결과물은 정상이다.
- 디자인 기준: 찹쌀떡 파스텔(크림·분홍·코코아), 같은 계열의 얇은 톤 외곽선, 과하지 않은 둥글기. 인게임 HUD는 작고 납작한 반투명 판.
