# 2026-07-03_world_map_exploration_v1 구현 계획

## Summary

정적 월드맵 Texture2D 위에 플레이어, 거점, 텔레포트 포인트 마커를 표시하는 전체 지도 v1을 만든다.
플레이어가 방문하지 않은 지역은 검정색으로 가리고, 한 번 방문한 지역은 색이 보이도록 탐험 셀을 기록한다.
플레이어 현재 위치 주변은 별도 하이라이트로 더 밝게 표현할 수 있도록 UI 이벤트에 데이터를 넘긴다.

## Key Changes

- `PW_MapTypes`
  - 지도 마커 타입, 탐험 셀 상태, 지도 마커 데이터, 탐험 저장 데이터 구조를 정의한다.
- `UPW_MapSubsystem`
  - 월드 좌표와 지도 UV 변환을 담당한다.
  - 로컬 플레이어 위치를 주기적으로 샘플링해 방문 셀을 갱신한다.
  - 텔레포트 포인트와 베이스캠프 Actor를 지도 마커로 수집한다.
  - `MakeExplorationSaveData()` / `ApplyExplorationSaveData()`로 SaveGame 연동용 DTO를 제공한다.
- `UPW_MapExplorerComponent`
  - 플레이어 Actor에 붙여 Owner 위치 기준 방문 처리를 수행한다.
  - 플레이어별 `PlayerId`, 밝힘 반경, 그리드 설정을 Blueprint에서 관리한다.
  - SaveGame 연동 시 컴포넌트에서 탐험 저장 DTO를 바로 만들고 적용할 수 있게 한다.
- `APW_TeleportPointActor`
  - 레벨 배치용 텔레포트 지점 Actor를 추가한다.
  - v1에서는 지도 표시와 발견/텔레포트 가능 플래그만 가진다.
- `UPW_WorldMapWidget`
  - Blueprint 위젯 베이스를 추가한다.
  - 마커, 방문 셀, 플레이어 UV, 현재 위치 밝기 반경을 Blueprint 이벤트로 넘긴다.
- `UPW_WorldMapControllerComponent`
  - PlayerController에 붙여 월드맵 WBP 생성/제거와 토글을 담당한다.
  - 월드맵 표시 중 마우스 커서와 InputMode를 처리한다.
  - 현재 Pawn의 `UPW_MapExplorerComponent` 설정을 읽어 WBP에 같은 PlayerId와 맵 설정을 주입한다.

## Rule Decisions

- C++ 구현은 `PalWorld/Source/PalWorld/LMK/Public`, `PalWorld/Source/PalWorld/LMK/Private` 아래에만 둔다.
- PJH PlayerController/HUD C++는 수정하지 않는다.
- `M` 키 입력은 Blueprint에서 `UPW_WorldMapControllerComponent::ToggleWorldMap()`에 연결한다.
- 월드맵 WBP 생성/제거는 `UPW_WorldMapControllerComponent`가 담당한다.
- 실제 SaveGame 파일 입출력은 저장 시스템 확정 후 연결하고, v1은 저장 가능한 데이터 구조와 적용 함수까지만 제공한다.
- 실제 텔레포트 이동, 미니맵, 상단 나침반, 지도 캡처 자동화는 제외한다.

## Test Plan

- 사용자가 언리얼 컴파일을 수행한다.
- `WBP_WorldMap`이 `UPW_WorldMapWidget`을 상속하고, `RefreshMapData()` 호출 시 이벤트 데이터가 들어오는지 확인한다.
- 플레이어 이동 후 `VisitedCellIndices`가 증가하는지 확인한다.
- 미방문 지역은 검정, 방문 지역은 월드맵 색, 현재 플레이어 주변은 밝게 표시되는지 확인한다.
- `APW_TeleportPointActor` 배치 시 텔레포트 마커가 표시되는지 확인한다.
- 설치된 `APW_BaseCampActor`가 베이스 마커로 표시되는지 확인한다.
- 저장 시 `MakeExplorationSaveData()` 결과를 SaveGame에 넣고, 로드 후 `ApplyExplorationSaveData()`로 방문 셀이 복원되는지 확인한다.
