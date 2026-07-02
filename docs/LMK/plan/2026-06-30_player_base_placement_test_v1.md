# 2026-06-30_player_base_placement_test_v1 구현 계획

## 구현 목표

플레이어 BP에 `UPW_PlayerBasePlacementComponent`를 붙여 `B` 입력으로 베이스캠프 설치 테스트를 할 수 있게 한다.
설치 모드에서는 디버그 스피어로 `CampRadius` 프리뷰를 표시하고, 서버에서 `APW_BaseCampActor` 상속 BP를 스폰한다.

## 구현 항목

1. `UPW_PlayerBasePlacementComponent`
   - `BlueprintSpawnableComponent`
   - `BaseCampClass`
   - `PlacementTraceDistance`
   - `PreviewCampRadius`
   - `TogglePlacementMode`
   - `ConfirmPlacement`
   - `CancelPlacement`
   - `GetCurrentPlacementTransform`
   - `CanPlaceAtCurrentPreview`
2. 설치 프리뷰
   - 설치 모드에서만 Tick 활성
   - 카메라/컨트롤러 방향 라인트레이스
   - 설치 가능하면 파란 디버그 스피어
   - 설치 불가하면 빨간 디버그 스피어
3. 서버 설치
   - `ServerRequestPlaceBaseCamp`
   - 서버에서 반경 겹침 재검증
   - `BaseCampClass` 스폰
   - Player OwnerId 설정

## 에디터 적용

- `APW_BaseCampActor` 상속 BP를 만든다.
- 임시 StaticMesh를 추가한다.
- 플레이어 BP에 `PW_PlayerBasePlacementComponent`를 추가한다.
- `BaseCampClass`에 테스트 Base BP를 지정한다.
- `IA_BasePlacement`를 만들고 `B` 키를 매핑한다.
- 입력 이벤트에서 `TogglePlacementMode()`를 호출한다.

## 테스트 기준

- `B` 키 첫 입력으로 설치 프리뷰가 표시된다.
- 기존 Base와 겹치면 빨간 스피어가 표시된다.
- 설치 가능 위치면 파란 스피어가 표시된다.
- `B` 키 두 번째 입력으로 서버에 설치 요청한다.
- 성공 시 테스트 Base BP가 스폰되고 BaseCampSubsystem에서 조회된다.

## 제외 범위

- 정식 설치 UI, 설치 비용, 인벤토리 소비, 프리뷰 Mesh, 취소 입력은 후속 작업이다.
