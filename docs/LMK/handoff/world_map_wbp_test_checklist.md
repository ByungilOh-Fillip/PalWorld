# 월드맵 WBP 테스트 체크리스트

## Purpose

현재 Git에 `WBP_WorldMap` 자산이 잡혀 있지 않아 C++ 변경 이후 실제 UMG 동작은 이 문서 기준으로 에디터에서 확인한다.
이번 변경은 PlayerController BP에 `PW_WorldMapControllerComponent` 하나만 붙여도 월드맵 탐험 기록과 WBP 표시가 동작하게 만드는 것이 목표다.

## Commit Summary Draft

```text
feat(ui): simplify world map exploration setup

월드맵 탐험 기본 경로를 PlayerController 컴포넌트 하나로 단순화했다.

- PW_WorldMapControllerComponent가 PlayerId, 맵 범위, 그리드, 밝힘 반경, 자동 탐험 타이머를 직접 관리하도록 변경
- 현재 Possess 중인 Pawn 위치를 기준으로 MapSubsystem 탐험 셀을 갱신하도록 구성
- 월드맵 표시 시 Controller 컴포넌트가 WBP에 PlayerId, 추적 Pawn, 맵 설정값을 주입하도록 변경
- 기존 WBP 호환을 위해 ConfigureMapWidget()과 BP_OnMapDataRefreshed 이벤트 시그니처, BindWidget 이름은 유지
- PW_MapExplorerComponent는 삭제하지 않고 레거시/선택 컴포넌트로 유지
- WBP가 Git에 없어 실제 UMG 검증은 추후 에디터에서 수행한다
```

## Required BP Setup

PlayerController BP에 `PW_WorldMapControllerComponent`를 추가한다.

- `WorldMapWidgetClass`: `WBP_WorldMap`
- `PlayerId`: 싱글 플레이는 기본값 `LocalPlayer`
- `WorldMin`, `WorldMax`: 월드맵 머티리얼 이미지와 맞는 월드 좌표 범위
- `GridWidth`, `GridHeight`: 탐험 셀 해상도
- `RevealRadius`: 현재 위치 주변 밝힘 반경
- `bAutoReveal`: true
- `RevealUpdateIntervalSeconds`: 기본값 `0.5`

Player/Pawn BP에는 기본 경로 기준으로 `PW_MapExplorerComponent`를 추가하지 않는다.
이미 붙어 있는 경우에는 이번 변경과 충돌하지 않아야 하지만, 최종 정리 전에는 제거 여부를 에디터에서 확인한다.

## WBP Required Surface

`WBP_WorldMap`은 `UPW_WorldMapWidget`을 상속해야 한다.

아래 BindWidget 이름이 있으면 C++ 기본 갱신이 동작한다.

- `MapBackgroundImage`
- `MapViewportRoot`
- `MapZoomRoot`
- `UnvisitedCellCanvas`
- `VisitedDarkOverlayImage`
- `InitialMapCoverImage`
- `PlayerMarkerWidget`
- `CurrentAreaHighlightWidget`

권장 계층 구조는 아래와 같다.

```text
CanvasPanel (Root)
└─ MapViewportRoot (Canvas Panel, Clip to Bounds)
   └─ MapZoomRoot (Canvas Panel)
      ├─ MapBackgroundImage
      ├─ VisitedDarkOverlayImage
      ├─ UnvisitedCellCanvas
      ├─ CurrentAreaHighlightWidget
      ├─ PlayerMarkerWidget
      └─ InitialMapCoverImage
```

`MapViewportRoot`는 화면 중앙에 고정하고, `MapZoomRoot`는 `MapViewportRoot`와 같은 크기의 직접 자식으로 둔다.
액자 테두리나 배경 장식은 `MapZoomRoot` 안에 넣지 않는다. `MapZoomRoot` 아래에 넣은 위젯은 확대/축소 대상이므로, 고정되어야 하는 프레임은 `MapViewportRoot` 바깥 형제 위젯이나 부모 쪽에 둔다.

기존 Blueprint 이벤트 `BP_OnMapDataRefreshed`는 그대로 유지한다.

## Test Steps

1. 에디터에서 PlayerController BP를 열고 `PW_WorldMapControllerComponent`가 추가되어 있는지 확인한다.
2. `WorldMapWidgetClass`에 기존 `WBP_WorldMap`을 지정한다.
3. `WorldMin`, `WorldMax`, `GridWidth`, `GridHeight`, `RevealRadius`가 월드맵 머티리얼 이미지 기준과 맞는지 확인한다.
4. Player/Pawn BP에 `PW_MapExplorerComponent`가 없어도 PIE가 시작되는지 확인한다.
5. PIE 시작 후 움직이지 않아도 시작 위치 주변 방문 셀이 밝혀지는지 확인한다.
6. 플레이어를 이동시키고 방문 셀 수가 증가하는지 확인한다.
7. `M` 키 입력에서 `PW_WorldMapControllerComponent.ToggleWorldMap()`이 호출되는지 확인한다.
8. 월드맵 오픈 시 현재 Pawn 위치에 `PlayerMarkerWidget`이 배치되는지 확인한다.
9. 현재 위치 주변 `CurrentAreaHighlightWidget` 크기가 `RevealRadius`에 맞게 보이는지 확인한다.
10. 미방문 지역은 검정 마스크로 남고 방문 지역은 지도 머티리얼이 보이는지 확인한다.
11. 마우스 휠로 `MapZoomRoot`가 확대/축소되는지 확인한다.
12. `MinMapZoom` 상태에서 좌클릭 드래그해도 지도 프레임과 내용물이 움직이지 않는지 확인한다.
13. 확대 후 좌클릭 드래그하면 고정된 `MapViewportRoot` 안에서 지도 내용물만 이동하는지 확인한다.
14. 마우스 휠 확대/축소 시 `MapViewportRoot`의 화면 위치와 크기는 그대로이고 `MapZoomRoot` 내부 지도만 커지거나 작아지는지 확인한다.
15. 다시 `MinMapZoom`까지 축소하면 지도 내용물이 중앙 위치로 복귀하는지 확인한다.
16. 발견 처리된 `APW_TeleportPointActor`만 텔레포트 마커로 표시되는지 확인한다.
17. 배치된 `APW_BaseCampActor`가 거점 마커로 표시되는지 확인한다.
18. `MakeExplorationSaveData()` 결과를 저장한 뒤 `ApplyExplorationSaveData()`로 방문 셀이 복원되는지 확인한다.

## Expected Result

- 기본 세팅은 PlayerController BP의 `PW_WorldMapControllerComponent` 하나로 끝난다.
- WBP는 Controller 컴포넌트에서 주입된 PlayerId, Pawn, 맵 설정값으로 갱신된다.
- Player/Pawn BP에 `PW_MapExplorerComponent`가 없어도 월드맵 열기와 탐험 셀 갱신이 동작한다.
- 기존 WBP 그래프의 `BP_OnMapDataRefreshed` 노드는 깨지지 않는다.

## If Something Fails

- WBP가 열리지 않으면 `WorldMapWidgetClass` 지정과 `ToggleWorldMap()` 입력 연결을 먼저 확인한다.
- 마커 위치가 틀리면 Controller 컴포넌트의 `WorldMin`, `WorldMax`를 확인한다.
- 밝힘 범위가 너무 작거나 크면 `RevealRadius`와 `GridWidth`, `GridHeight`를 같이 확인한다.
- 마스크가 전체를 덮거나 안 보이면 `UnvisitedCellCanvas`, `VisitedDarkOverlayImage`, `InitialMapCoverImage` BindWidget 이름을 확인한다.
- 플레이어 마커가 안 보이면 `PlayerMarkerWidget` 이름과 부모 좌표계가 `MapBackgroundImage`와 맞는지 확인한다.
- 확대/축소 때 지도 창 전체가 움직이면 `MapViewportRoot`가 고정 부모이고 `MapZoomRoot`가 그 직접 자식인지 확인한다. 고정 액자용 이미지나 Border가 `MapZoomRoot` 아래에 있으면 같이 확대되므로 밖으로 빼야 한다.
- 기존 Player/Pawn BP에 `PW_MapExplorerComponent`가 남아 있다면 중복 탐험 갱신 자체는 허용되지만, 최종 정리 전 제거 여부를 결정한다.
