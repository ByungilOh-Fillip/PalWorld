# PalWorld Title Menu UI Handover

## 목적

Palworld 스타일을 참고한 타이틀 메뉴 UI 작업을 다른 PC/다른 Codex 세션에서 이어가기 위한 인수인계 문서입니다.

원본 스크린샷의 구성감은 참고했지만, 원본 로고/캐릭터/개인정보/버전 표기는 그대로 복제하지 않습니다.

## 현재 구현 상태

- 메뉴 맵: `PalWorld/Content/_Private/LMK/Menu/MenuMap.umap`
- 메뉴 GameMode BP: `PalWorld/Content/_Private/LMK/Menu/BP_MenuGameMode.uasset`
- 메뉴 PlayerController BP: `PalWorld/Content/_Private/LMK/Menu/BP_MenuPlayerController.uasset`
- 메인 메뉴 WBP: `PalWorld/Content/_Private/LMK/UI/Menu/WBP_MainMenu.uasset`
- 세션 슬롯 WBP: `PalWorld/Content/_Private/LMK/UI/Menu/WBP_SessionSlot.uasset`
- 옵션 WBP: `PalWorld/Content/_Private/LMK/UI/Menu/WBP_Options.uasset`
- 생성 배경 원본 PNG: `PalWorld/Content/_Private/LMK/UI/Menu/T_MainMenu_Background_Source.png`
- 임포트된 배경 텍스처: `/Game/_Private/LMK/UI/Menu/T_MainMenu_Background`

## 중요한 설계 결정

현재 WBP들은 디자이너 Hierarchy에 실제 UI를 많이 담고 있지 않습니다. 핵심 UI는 C++에서 런타임/프리뷰 시점에 생성됩니다.

핵심 파일:

- `PalWorld/Source/PalWorld/LMK/Public/UI/PW_MainMenuWidget.h`
- `PalWorld/Source/PalWorld/LMK/Private/UI/PW_MainMenuWidget.cpp`
- `PalWorld/Source/PalWorld/LMK/Private/Menu/PW_MenuPlayerController.cpp`
- `PalWorld/Source/PalWorld/LMK/Private/UI/PW_SessionSlotWidget.cpp`
- `PalWorld/Source/PalWorld/LMK/Private/UI/PW_OptionsWidget.cpp`

핵심 흐름:

1. `WBP_MainMenu`는 부모 클래스를 `UPW_MainMenuWidget`으로 가진 껍데기 Widget Blueprint입니다.
2. `UPW_MainMenuWidget::RebuildWidget()`에서 `MenuSwitcher == nullptr`이면 `BuildFallbackLayout()`을 호출합니다.
3. `BuildFallbackLayout()`이 `WidgetTree->ConstructWidget<...>()`로 Canvas, 배경 Image, Title, Button, WidgetSwitcher 페이지들을 생성합니다.
4. C++에서 직접 만든 버튼을 `Button_GameStart`, `Button_Options` 같은 기존 멤버에 대입합니다.
5. 기존 `BindButtonEvents()`가 그 멤버에 이벤트를 연결합니다.

따라서 WBP Hierarchy가 비어 보여도 Preview/PIE에서 UI가 보일 수 있습니다. UMG Designer가 부모 C++ 클래스의 `RebuildWidget()` 결과를 미리보기로 렌더링하기 때문입니다.

## 현재 UI 구성

`UPW_MainMenuWidget`의 fallback layout이 구성하는 화면:

- Fullscreen root Canvas
- Fullscreen background image
- 약한 검정 vignette overlay
- 중앙 상단 대형 `PALWORLD` 타이틀
- 중앙 하단 세로 메뉴
- 좌하단 얼리 액세스 안내문
- `WidgetSwitcher` 6페이지
  - MainTitle
  - PlayModeSelect
  - CreateRoom
  - FindRoom
  - DedicatedUnavailable
  - Options

제거된 항목:

- 우측상단 Steam 계정명 표기
- 우측하단 저작권/버전 표기
- 타이틀 아래 장식선

## 게임 맵 전환 관련

메뉴에서 게임 시작 시 GameMode가 메뉴 GameMode로 남아 조작이 안 되는 문제가 있었습니다.

대응:

- `UPW_MainMenuWidget`에 `GameMapTravelOptions` 추가
- 기본값: `game=/Game/_Private/LMK/BluePrints/BP_PWGameMode.BP_PWGameMode_C`
- 싱글 시작은 `UGameplayStatics::OpenLevel(this, GameMapName, true, GameMapTravelOptions)` 사용
- 세션/listen travel도 `/Game/_Private/LMK/Levels/Palworld?game=...` 형태로 이동

추가 확인:

- `/Game/_Private/LMK/Levels/Palworld` World Settings의 GameMode Override가 `BP_PWGameMode`인지 확인
- PIE Play Settings에 메뉴 GameMode override가 잡혀 있지 않은지 확인

## Unreal Python 스크립트

자산 생성/보정용 스크립트:

- `PalWorld/Scripts/CreateTitleMenuAssets.py`
- `PalWorld/Scripts/SetGameMapGameMode.py`
- `PalWorld/Scripts/RunCreateTitleMenuAssets.bat`

에디터 Python 콘솔에서 실행:

```python
exec(open(r"D:\Github\PalWorld\PalWorld\Scripts\CreateTitleMenuAssets.py", encoding="utf-8").read())
```

게임 맵 GameMode override 보정:

```python
exec(open(r"D:\Github\PalWorld\PalWorld\Scripts\SetGameMapGameMode.py", encoding="utf-8").read())
```

성공 로그:

```text
CreateTitleMenuAssets: finished
SetGameMapGameMode: finished
```

주의:

- Unreal Output Log의 `Cmd` 입력창은 OS 명령 프롬프트가 아닙니다.
- 에디터 안에서 실행할 때는 Output Log 입력 타입을 `Python`으로 바꿔서 실행해야 합니다.
- 외부에서 실행하려면 실제 UE 5.8 `UnrealEditor-Cmd.exe` 경로가 필요합니다.

## 빌드/컴파일 주의

프로젝트 지침상 Unreal 관련 컴파일/빌드는 사용자가 직접 수행합니다.

Codex는 C++ 파일을 수정할 수 있지만, Unreal 빌드/컴파일 실행은 사용자가 해야 합니다.

다른 PC에서 이어갈 때:

1. repo pull
2. Unreal 프로젝트 열기
3. 사용자 측에서 C++ 컴파일
4. 필요하면 Python 스크립트 2개 재실행
5. `/Game/_Private/LMK/Menu/MenuMap` 열고 PIE 확인

## 다음 작업 후보

현재 구조는 "동작 우선 C++ fallback UI"입니다. 다음 단계에서 선택할 수 있는 개선 방향:

1. 디자이너 편집 가능한 실제 WBP Hierarchy로 이전
   - 현재 C++ fallback을 참고해서 `WBP_MainMenu`에 실제 Canvas/Overlay/Button/Text를 구성
   - C++ fallback은 backup path로 유지하거나 제거
2. 버튼 스타일 정리
   - Hover/Pressed 색, 폰트 크기, 그림자, 모바일 스케일 보정
3. 배경 교체/추가 생성
   - 원본과 더 유사한 분위기지만 비복제 이미지로 추가 생성 가능
4. 각 페이지 UX 완성
   - CreateRoom/FindRoom/Options 화면을 실제 게임 UX에 맞게 정리
5. PIE/GameMode 검증
   - 싱글 시작, listen session, join session 각각에서 GameMode/PlayerController 전환 확인

## 다른 Codex 세션에 줄 요약

```text
PalWorld 메뉴 UI 이어서 작업.

현재 WBP_MainMenu/WBP_SessionSlot/WBP_Options는 빈 Widget Blueprint에 가깝고, 실제 UI는 UPW_MainMenuWidget::RebuildWidget() -> BuildFallbackLayout()에서 C++ WidgetTree로 생성된다.
APW_MenuPlayerController는 MainMenuWidgetClass가 비어도 UPW_MainMenuWidget::StaticClass()로 메뉴를 띄운다.
CreateTitleMenuAssets.py가 배경 텍스처, WBP, BP_MenuGameMode, BP_MenuPlayerController, MenuMap을 생성한다.
SetGameMapGameMode.py가 게임맵 /Game/_Private/LMK/Levels/Palworld의 GameMode override를 BP_PWGameMode로 지정한다.
게임 시작 시 game=/Game/_Private/LMK/BluePrints/BP_PWGameMode.BP_PWGameMode_C 옵션을 붙이도록 C++ 수정되어 있다.

주의: Unreal 빌드/컴파일은 사용자가 직접 한다.
다음 목표가 디자이너 편집 가능한 WBP라면 실제 UMG hierarchy를 구성하고 C++ fallback을 보조 경로로 낮춰야 한다.
```
