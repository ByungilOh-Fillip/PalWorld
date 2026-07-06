# 전체 지도 탐험 시스템 구현 기록

## Summary

정적 월드맵 이미지 위에 지도 마커와 탐험 상태를 표시하기 위한 C++ 기반을 추가했다.
플레이어가 방문한 셀은 런타임에 기록되고, 미방문 지역은 UI에서 검정 오버레이로 가릴 수 있도록 방문 셀 인덱스를 위젯 이벤트로 전달한다.
저장 시스템이 아직 확정되지 않았기 때문에 실제 SaveGame 파일 입출력은 만들지 않고, 저장/로드에 사용할 DTO 생성과 적용 함수까지만 제공했다.

## Implemented Files

- 계획 문서
  - `docs/LMK/plan/2026-07-03_world_map_exploration_v1.md`
- 지도 코어
  - `PalWorld/Source/PalWorld/LMK/Public/Map/PW_MapTypes.h`
  - `PalWorld/Source/PalWorld/LMK/Public/Map/PW_MapSubsystem.h`
  - `PalWorld/Source/PalWorld/LMK/Private/Map/PW_MapSubsystem.cpp`
  - `PalWorld/Source/PalWorld/LMK/Public/Map/PW_MapExplorerComponent.h`
  - `PalWorld/Source/PalWorld/LMK/Private/Map/PW_MapExplorerComponent.cpp`
  - `PalWorld/Source/PalWorld/LMK/Public/Map/PW_TeleportPointActor.h`
  - `PalWorld/Source/PalWorld/LMK/Private/Map/PW_TeleportPointActor.cpp`
- 지도 UI 베이스
  - `PalWorld/Source/PalWorld/LMK/Public/UI/PW_WorldMapWidget.h`
  - `PalWorld/Source/PalWorld/LMK/Private/UI/PW_WorldMapWidget.cpp`
  - `PalWorld/Source/PalWorld/LMK/Public/UI/PW_WorldMapControllerComponent.h`
  - `PalWorld/Source/PalWorld/LMK/Private/UI/PW_WorldMapControllerComponent.cpp`

## Responsibility Split

| 위치 | 책임 |
|---|---|
| `UPW_MapSubsystem` | 월드 좌표와 지도 UV 변환, 방문 셀 기록, 지도 마커 수집, 탐험 저장 DTO 제공 |
| `UPW_MapExplorerComponent` | 레거시/선택 컴포넌트. 플레이어 외 Actor가 직접 탐험을 밝히는 경우에만 사용 |
| `APW_TeleportPointActor` | 레벨 배치용 텔레포트 포인트, 발견 여부와 지도 표시 정보 보유 |
| `UPW_WorldMapWidget` | Blueprint 위젯에 지도 데이터 전달 |
| `UPW_WorldMapControllerComponent` | PlayerController에 붙여 탐험 갱신, 월드맵 WBP 생성/제거, 마우스 커서와 InputMode 처리 |
| `WBP_WorldMap` | 배경 이미지, 검정 마스크, 방문 셀, 현재 위치 밝기, 마커 표시 |

## Behavior

- `UPW_MapSubsystem`은 Dedicated Server가 아닌 월드에서 로컬 플레이어 위치를 주기적으로 샘플링한다.
- 플레이어 위치 기준 `RevealRadius` 안에 들어온 셀은 방문 처리된다.
- 기본 경로는 PlayerController BP에 `UPW_WorldMapControllerComponent`만 붙이는 방식이다.
- `UPW_WorldMapControllerComponent`는 현재 Possess 중인 Pawn 위치 기준으로 방문 처리를 수행하고, SaveGame 연동용 데이터를 컴포넌트에서 바로 가져올 수 있다.
- PlayerController BP에 `UPW_WorldMapControllerComponent`를 붙이면 `M` 입력에서 `ToggleWorldMap()`만 호출해 월드맵 표시/미표시를 제어할 수 있다.
- `UPW_WorldMapControllerComponent`는 `PlayerId`, 현재 Possess 중인 Pawn, `WorldMin`, `WorldMax`, `GridWidth`, `GridHeight`, `RevealRadius` 값을 `WBP_WorldMap`에 넘긴다.
- `WBP_WorldMap`은 전달받은 Pawn 위치와 Controller 컴포넌트가 주입한 맵 설정으로 플레이어 마커와 현재 밝힘 위치를 계산한다.
- `GetVisitedCellIndices()`는 방문한 셀 인덱스만 정렬해서 반환한다.
- `GetMapMarkers()`는 발견된 `APW_TeleportPointActor`와 현재 월드의 `APW_BaseCampActor`를 마커로 반환한다.
- `UPW_WorldMapWidget::RefreshMapData()`는 마커, 방문 셀, 플레이어 UV, 밝기 반경 UV를 `BP_OnMapDataRefreshed` 이벤트로 넘긴다.
- `UPW_WorldMapWidget::NativeOnMouseWheel()`은 휠 입력으로 `MapZoomRoot`의 Render Scale을 조정한다.
- `UPW_WorldMapWidget`은 선택 바인딩 위젯이 있으면 미방문 셀, 방문 지역 어둡게 처리, 현재 위치 하이라이트, 플레이어 마커를 기본 갱신한다.
- 기본 미방문/방문 오버레이와 최초 커버는 `MapBackgroundImage`의 Canvas Slot 위치, 정렬, 크기를 기준으로 동기화된다.
- 지도 열림 직후에는 UMG 레이아웃 확정 전 잘못된 위치가 노출되지 않도록 생성 시각 요소를 숨기고, `InitialRefreshDelaySeconds` 뒤 첫 갱신이 끝나면 표시한다.
- `UPW_WorldMapWidget`은 좌클릭 드래그 중 `MapZoomRoot`의 Render Translation을 조정해 확대 상태에서 지도를 이동한다.
- `APW_TeleportPointActor::SetDiscovered()`는 서버 권한에서만 발견 상태를 바꾸고 복제한다.

## Save Integration

저장 대상은 `FPW_MapExplorationSaveData`다.

| 값 | 용도 |
|---|---|
| `PlayerId` | 플레이어별 방문 기록 분리 |
| `MapId` | 월드/지도 구분 |
| `GridWidth`, `GridHeight` | 저장된 셀 인덱스 호환성 확인 |
| `VisitedCellIndices` | 방문한 셀만 저장 |

저장 시:

```text
UPW_MapSubsystem::MakeExplorationSaveData(PlayerId)
→ SaveGame에 FPW_MapExplorationSaveData 저장
```

로드 시:

```text
SaveGame에서 FPW_MapExplorationSaveData 읽기
→ UPW_MapSubsystem::ApplyExplorationSaveData(SaveData)
```

`MapId`, `GridWidth`, `GridHeight`가 현재 설정과 다르면 `ApplyExplorationSaveData()`는 false를 반환하고 적용하지 않는다.

## BP Setup

1. 에디터에서 `Palworld.umap`을 탑다운으로 캡처해 Texture2D로 임포트하고, 이 Texture를 사용하는 UI 머티리얼을 만든다.
2. PlayerController BP에 `PW_WorldMapControllerComponent`를 추가한다.
   - `WorldMapWidgetClass`: `WBP_WorldMap`을 지정한다.
   - `WorldMapZOrder`: 기본값 `100`을 사용한다.
   - `bApplyGameAndUIInputMode`: 월드맵 열 때 마우스 입력을 같이 받으려면 켠다.
   - `bRestoreGameOnlyInputModeOnHide`: 월드맵 닫을 때 게임 입력으로 복구하려면 켠다.
   - `PlayerId`: 저장 데이터를 구분할 플레이어 ID. 싱글 플레이는 기본 `LocalPlayer`로 충분하다.
   - `WorldMin`, `WorldMax`, `GridWidth`, `GridHeight`, `RevealRadius`: 지도 좌표 변환과 탐험 밝힘 기준이다.
   - `bAutoReveal`: 켜두면 현재 Pawn 위치를 주기적으로 방문 처리한다.
   - `RevealUpdateIntervalSeconds`: 방문 처리 주기. 기본값은 `0.5`다.
3. PlayerController BP에서 `M` 키 입력에 `PW_WorldMapControllerComponent.ToggleWorldMap()`을 연결한다.
4. `UPW_WorldMapWidget`을 상속한 `WBP_WorldMap`을 만든다.
5. `WBP_WorldMap`에 고정 뷰포트 `MapViewportRoot`와 확대/이동 컨테이너 `MapZoomRoot`를 만든다.
   - `MapViewportRoot`: 화면 중앙에 고정되는 Canvas Panel. `Clipping`을 `Clip to Bounds`로 설정한다.
   - `MapZoomRoot`: `MapViewportRoot`의 직접 자식 Canvas Panel. 확대/축소와 드래그 이동은 이 위젯에만 적용된다.
   - 액자 테두리나 고정 배경 장식은 `MapZoomRoot` 안에 넣지 않고 `MapViewportRoot` 바깥 형제 위젯이나 부모 쪽에 둔다.
6. `MapZoomRoot` 아래에 지도 구성 위젯을 둔다.
   - 아래 위젯들을 `MapZoomRoot` 아래에 넣으면 휠 확대/축소와 좌클릭 드래그 이동이 같이 적용된다.
   - `MapZoomRoot`는 Canvas Panel로 두고, 아래 위젯들은 같은 부모의 직접 자식으로 둔다.
   - `MapBackgroundImage`: 지도 배경 Image.
   - `InitialMapCoverImage`: 지도 열림 직후 첫 실제 마스크 갱신 전까지 덮는 검정 Image.
   - `VisitedDarkOverlayImage`: 지도 전체를 덮는 반투명 검정 Image. 방문 지역을 어둡게 보이게 한다.
   - `UnvisitedCellCanvas`: 미방문 셀을 검정 사각형으로 생성할 Canvas Panel.
   - `CurrentAreaHighlightWidget`: 현재 위치 주변 밝기 표시용 위젯.
   - `PlayerMarkerWidget`: 플레이어 현재 위치 아이콘 위젯.
   - `PlayerMarkerWidget`과 `CurrentAreaHighlightWidget`은 같은 `PlayerMapUV`로 배치되므로 반드시 같은 부모 좌표계 아래에 둔다.
7. `WBP_WorldMap`에서 `WorldMapMaterial`을 설정한다.
   - `PlayerId`, 추적 Pawn, `WorldMin`, `WorldMax`, `GridWidth`, `GridHeight`, `RevealRadius`는 PlayerController 컴포넌트가 주입한다.
   - 월드맵 배경은 Texture Brush를 직접 쓰지 않고 UI 머티리얼만 사용한다.
   - `WorldMapMaterial`이 비어 있으면 C++이 `MapBackgroundImage` Brush를 비우므로 지도 배경은 표시되지 않는다.
   - 방문 지역이 너무 밝거나 어두우면 `VisitedDarkOverlayColor`의 Alpha를 조정한다. 기본값은 `0.7`이다.
   - 지도 오픈 직후 위치가 한 프레임 틀어지면 `InitialRefreshDelaySeconds`를 `0.05~0.1` 사이에서 조정한다.
   - `MinMapZoom` 상태에서는 드래그 이동이 비활성화되고, 확대 후에만 `MapViewportRoot` 내부에서 지도 내용물이 이동한다.
8. `BP_OnMapDataRefreshed`에서:
   - `VisitedCellIndices`에 없는 셀은 검정 오버레이로 표시한다.
   - 방문 셀은 오버레이를 숨기거나 낮은 투명도로 둔다.
   - `PlayerMapUV` 주변 `RevealRadiusUV` 영역은 밝게 표시한다.
   - `Markers` 배열을 순회해 거점/텔레포트 아이콘을 배치한다.
9. 지도 열림 상태의 마우스 커서와 `GameAndUI`, 닫을 때 `GameOnly` 복구는 `PW_WorldMapControllerComponent`가 처리한다.

## Test Plan

- 사용자가 언리얼 컴파일을 수행한다.
- PlayerController BP에 `PW_WorldMapControllerComponent`를 붙이고 PIE 시작 시 방문 셀이 증가하는지 확인한다.
- `M` 입력으로 `ToggleWorldMap()`이 호출되는지 확인한다.
- PIE에서 `WBP_WorldMap` 생성 시 `BP_OnMapDataRefreshed`가 호출되는지 확인한다.
- 마우스 휠 입력 시 `MapZoomRoot`가 `MinMapZoom`과 `MaxMapZoom` 사이에서 확대/축소되는지 확인한다.
- `MinMapZoom` 상태에서 좌클릭 드래그해도 지도 프레임과 내용물이 움직이지 않는지 확인한다.
- 마우스 휠 확대/축소와 좌클릭 드래그 시 고정된 `MapViewportRoot` 안에서 `MapZoomRoot` 내용물만 이동하는지 확인한다.
- 플레이어 이동 후 방문 셀 수가 증가하는지 확인한다.
- 미방문 지역이 검정색, 방문 지역이 월드맵 색으로 보이는지 확인한다.
- 현재 플레이어 주변이 방문 지역보다 밝게 보이는지 확인한다.
- 발견 처리된 `APW_TeleportPointActor`만 텔레포트 마커로 표시되는지 확인한다.
- 설치된 `APW_BaseCampActor`가 거점 마커로 표시되는지 확인한다.
- 저장 데이터 생성 후 새 세션에서 적용했을 때 방문 셀이 복원되는지 확인한다.

## Excluded Scope

- 실제 텔레포트 이동.
- 미니맵과 상단 나침반.
- 지도 캡처 자동화.
- SaveGame 파일 입출력 구현.
- PJH PlayerController/HUD C++ 자동 연동.
