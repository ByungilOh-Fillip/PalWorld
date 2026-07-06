# 2026-07-04_world_map_controller_single_component_v1 계획

## Summary

월드맵 탐험 시스템의 기본 세팅을 PlayerController 컴포넌트 하나로 단순화한다.
현재 Git에 WBP 자산이 잡혀 있지 않아 위젯 Blueprint를 즉시 검증할 수 없으므로, 기존 WBP가 의존할 가능성이 있는 C++ 변수, BindWidget 이름, Blueprint 이벤트 시그니처는 유지한다.

이번 변경의 목표는 `PW_MapExplorerComponent`를 즉시 삭제하는 것이 아니라, `PW_WorldMapControllerComponent`만 붙여도 월드맵 표시와 탐험 기록이 동작하게 만드는 것이다.

## Current Problem

- 현재 세팅은 플레이어/Pawn BP에 `PW_MapExplorerComponent`, PlayerController BP에 `PW_WorldMapControllerComponent`를 각각 추가해야 한다.
- 탐험 설정(`WorldMin`, `WorldMax`, `GridWidth`, `GridHeight`, `RevealRadius`)이 Player/Pawn 컴포넌트와 WBP 쪽에 중복된다.
- `PW_WorldMapControllerComponent`가 월드맵을 열 때 Pawn 또는 PlayerController에서 `PW_MapExplorerComponent`를 찾아야 하므로, 컴포넌트 하나가 빠지면 월드맵 설정이 불완전해진다.
- WBP 자산이 Git에 없기 때문에 C++ API를 크게 바꾸면 나중에 WBP를 열었을 때 깨진 노드나 변수 참조가 생길 수 있다.

## Decision

기본 사용 경로를 `PW_WorldMapControllerComponent` 하나로 통합한다.

- PlayerController BP에 `PW_WorldMapControllerComponent`만 추가한다.
- Controller 컴포넌트가 PlayerId, 맵 범위, 탐험 그리드, 밝힘 반경, 자동 탐험 타이머를 관리한다.
- Controller 컴포넌트가 현재 Possess 중인 Pawn 위치를 기준으로 `UPW_MapSubsystem::RevealAroundLocation()`을 호출한다.
- 월드맵을 열 때 Controller 컴포넌트가 WBP에 PlayerId, 추적 Pawn, 맵 설정값을 주입한다.
- `PW_MapExplorerComponent`는 당장 삭제하지 않고 레거시/선택 경로로 남긴다.

## Compatibility Rules

WBP 자산이 없는 상태에서 안전하게 수정하기 위해 아래 표면은 유지한다.

- `UPW_WorldMapWidget::BP_OnMapDataRefreshed(...)` 이벤트 시그니처 유지
- `MapBackgroundImage`, `MapZoomRoot`, `UnvisitedCellCanvas`, `VisitedDarkOverlayImage`, `InitialMapCoverImage`, `PlayerMarkerWidget`, `CurrentAreaHighlightWidget` BindWidget 이름 유지
- `WorldMapMaterial`, `WorldMin`, `WorldMax`, `GridWidth`, `GridHeight`, `RevealRadius`, `PlayerId` 등 기존 UPROPERTY 유지
- 월드맵 배경은 Texture Brush를 직접 사용하지 않고 UI 머티리얼만 사용
- `UPW_WorldMapWidget::ConfigureMapWidget(const FString&, AActor*)` 기존 함수는 유지

필요하면 새 함수만 추가한다.

```text
ConfigureMapWidgetWithSettings(PlayerId, TrackedActor, WorldMin, WorldMax, GridWidth, GridHeight, RevealRadius)
```

기존 WBP/Blueprint 노드가 깨지지 않도록 기존 함수 이름과 파라미터를 제거하거나 변경하지 않는다.

## Implementation Plan

1. `PW_WorldMapControllerComponent`에 탐험 설정과 PlayerId를 추가한다.
   - `PlayerId`
   - `WorldMin`, `WorldMax`
   - `GridWidth`, `GridHeight`
   - `RevealRadius`
   - `bAutoReveal`
   - `RevealUpdateIntervalSeconds`

2. `PW_WorldMapControllerComponent`가 `UPW_MapSubsystem`을 직접 설정하도록 한다.
   - `BeginPlay()`에서 맵 설정 적용
   - 자동 탐험 타이머 시작
   - `EndPlay()`에서 타이머 정리

3. 현재 Pawn 위치를 기준으로 탐험 셀을 밝히는 함수를 추가한다.
   - `RevealControlledPawnLocation()`
   - `GetResolvedPlayerId()`
   - `GetControlledPawn()`

4. SaveGame 연동용 Blueprint 함수를 Controller 컴포넌트에 추가한다.
   - `MakeExplorationSaveData()`
   - `ApplyExplorationSaveData()`

5. `ConfigureWorldMapWidget()`에서 `PW_MapExplorerComponent` 의존성을 제거한다.
   - Pawn에서 Explorer 컴포넌트를 찾지 않는다.
   - Controller 컴포넌트의 PlayerId와 맵 설정을 WBP에 주입한다.
   - 월드맵 표시 직전에 현재 Pawn 위치를 한 번 밝힌다.

6. `UPW_WorldMapWidget`에는 호환성 유지 방식으로 설정 주입 함수를 추가한다.
   - 기존 `ConfigureMapWidget()`은 유지한다.
   - 새 함수가 내부 UPROPERTY 값을 덮어쓴 뒤 `ApplyMapSettingsToSubsystem()`을 호출한다.

7. 문서를 갱신한다.
   - 기본 BP 세팅: PlayerController BP에 `PW_WorldMapControllerComponent`만 추가
   - `PW_MapExplorerComponent`: 레거시/선택 컴포넌트로 설명
   - WBP 설정값 중 Controller에서 주입되는 값과 WBP 표시 전용 값을 구분

## Non-Goals

- `PW_MapExplorerComponent` 즉시 삭제
- 기존 WBP 변수/BindWidget/이벤트 제거
- 실제 미니맵 구현
- 상단 나침반 구현
- 실제 텔레포트 이동 구현
- SaveGame 파일 입출력 구현
- Git에 없는 WBP 자산 수정

## Success Criteria

- PlayerController BP에 `PW_WorldMapControllerComponent`만 붙여도 탐험 셀이 갱신된다.
- `M` 키에서 `ToggleWorldMap()` 호출 시 월드맵 WBP가 열리고 현재 Pawn 위치가 전달된다.
- Controller 컴포넌트가 `PW_MapExplorerComponent`를 찾지 않아도 동작한다.
- 기존 `UPW_WorldMapWidget` 이벤트와 BindWidget 표면은 유지된다.
- `PW_MapExplorerComponent`가 붙어 있는 기존 BP가 있어도 C++ 컴파일 단계에서 깨지지 않는다.

## Later Cleanup

WBP와 Player BP 자산이 있는 환경에서 에디터 컴파일/저장 테스트를 완료한 뒤 아래 정리를 검토한다.

- Player/Pawn BP에서 `PW_MapExplorerComponent` 제거
- `PW_MapExplorerComponent` 소스 삭제
- 문서에서 레거시 경로 삭제
- Controller 컴포넌트의 설정이 프로젝트 공통값이면 별도 데이터 에셋 또는 설정 객체로 분리
