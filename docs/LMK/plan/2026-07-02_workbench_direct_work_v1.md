# 2026-07-02_workbench_direct_work_v1 구현 기록

## Summary

`APW_WorkBuildingBase`가 기존 F 키 상호작용 시스템을 통해 플레이어 직접 작업과 제작 UI 진입점을 제공하도록 한다.
작업대 Actor는 거점 등록과 상호작용 진입점만 담당하고, 요구 작업 태그와 작업 진행 상태는 `UPW_WorkBuildingComponent`로 분리한다.
멀티플레이 기준으로 서버가 작업 상태의 유일한 소유자가 되도록 `Idle`, `Reserved`, `InProgress`, `Completed` 상태와 활성 작업자 수 복제를 추가했다.

## Implemented Files

- 작업대/거점 연동
  - `PalWorld/Source/PalWorld/LMK/Public/Base/PW_WorkBuildingBase.h`
  - `PalWorld/Source/PalWorld/LMK/Private/Base/PW_WorkBuildingBase.cpp`
  - `PalWorld/Source/PalWorld/LMK/Public/Base/PW_WorkBuildingComponent.h`
  - `PalWorld/Source/PalWorld/LMK/Private/Base/PW_WorkBuildingComponent.cpp`
  - `PalWorld/Source/PalWorld/LMK/Public/Base/PW_BaseCampActor.h`
  - `PalWorld/Source/PalWorld/LMK/Private/Base/PW_BaseCampActor.cpp`
  - `PalWorld/Source/PalWorld/LMK/Public/Base/PW_BaseWorkTargetRegistryComponent.h`
  - `PalWorld/Source/PalWorld/LMK/Private/Base/PW_BaseWorkTargetRegistryComponent.cpp`
  - `PalWorld/Source/PalWorld/LMK/Public/Base/PW_BasePalAssignmentComponent.h`
  - `PalWorld/Source/PalWorld/LMK/Private/Base/PW_BasePalAssignmentComponent.cpp`
- 공용 상호작용 확장
  - `PalWorld/Source/PalWorld/Global/Components/public/PWHoldInteractable.h`
  - `PalWorld/Source/PalWorld/Global/Components/public/PWLocalInteractable.h`
  - `PalWorld/Source/PalWorld/Global/Components/public/PWInteractionScannerComponent.h`
  - `PalWorld/Source/PalWorld/Global/Components/private/PWInteractionScannerComponent.cpp`
- 팰 적성 조회
  - `PalWorld/Source/PalWorld/OBI/Components/public/PWSkillComponent.h`
  - `PalWorld/Source/PalWorld/OBI/Components/private/PWSkillComponent.cpp`
  - `PalWorld/Source/PalWorld/OBI/Pal/public/PWPalBase.h`

## Responsibility Split

| 위치 | 책임 |
|---|---|
| `APW_WorkBuildingBase` | 건물 Actor, 거점 등록, F 키 상호작용 진입점, 로컬 Blueprint UI hook |
| `UPW_WorkBuildingComponent` | 작업 상태, 요구 작업 태그, 서버 작업자 목록, 작업 진행률, 작업 위치 |
| `UPWInteractionScannerComponent` | 현재 상호작용 대상 탐색, hold 상호작용 시작/종료 서버 요청, 로컬 UI fallback |
| `UPW_BaseWorkTargetRegistryComponent` | 작업대 등록, 태그별 후보 목록 제공 |
| `UPW_BasePalAssignmentComponent` | 팰 슬롯과 작업 대상 배정 상태 기록 |
| `UPWSkillComponent` | 팰의 `FWorkAttitude` GameplayTag 조회 |
| Pal/AI Part | 배정된 작업 대상까지 이동하고 도착 후 작업 시작/중단 호출 |

## Behavior

- 작업 예약 또는 진행 중인 작업대에서 F를 누르고 있으면 직접 작업을 시작한다.
- F를 떼면 직접 작업을 중단한다.
- 예약/진행 중인 작업이 없는 제작 작업대는 로컬 클라이언트에서 `OnOpenWorkbenchUI` Blueprint 이벤트를 호출한다.
- 일반 1회 상호작용 대상은 `TryBeginHoldInteraction()` 내부에서 기존 `TryInteract()`로 fallback한다.
- 작업대 정면 작업 위치는 Actor forward 기준으로 계산한다.
- 작업자 목록은 서버에만 있고, 클라이언트에는 `ActiveWorkerCount`만 복제한다.
- 작업대는 거리 이탈, 작업자 파괴, 작업대 종료, 작업 완료 시 서버에서 작업자를 정리한다.
- `TryCraftFromBaseInventory()`는 서버에서 재료를 소비하고 `RecipeId`를 작업 ID로 예약한다.
- 작업 완료 시 예약된 `RecipeId`의 결과물을 거점 저장소에 넣고, 실패하면 작업대 내부 인벤토리에 저장한다.
- `InternalInventoryComponent`는 제작 결과물을 거점 저장소에 넣지 못했을 때의 작업대 내부 fallback 저장소로 유지한다.
- `Has Reserved Work`는 런타임 복제 상태로 읽기 전용이며, 에디터 테스트는 `Start With Reserved Work`를 사용한다.
- `APW_BaseCampActor.TryAssignIdlePalToWorkTargetByTag()`는 서버에서 유휴 팰 적성과 작업대 후보를 검사하고 `MovingToWork` 상태를 기록한다.

## BP Integration

- 플레이어 BP에 `UPWInteractionScannerComponent`를 붙인다.
- F Pressed에서 `TryBeginHoldInteraction()`을 호출한다.
- F Released 또는 Canceled에서 `EndHoldInteraction()`을 호출한다.
- PJH 플레이어 C++ 입력 코드는 수정하지 않는다.

## Test Plan

- 빌드 성공 확인.
- 작업대 BP를 거점 안에 에디터 배치했을 때 base work registry에 등록되는지 확인.
- 플레이어 BP의 scanner가 작업대를 후보로 잡고 prompt를 반환하는지 확인.
- 작업 예약이 있는 작업대에서 F Pressed/Released로 작업 시작/중단이 호출되는지 확인.
- 에디터 테스트 시 작업대 BP의 `WorkBuildingComponent.Start With Reserved Work`를 체크해 예약 상태를 만든다.
- 작업 예약이 없는 제작 작업대에서 `OnOpenWorkbenchUI`가 호출되는지 확인.
- Client에서 작업 예약이 없는 제작 작업대 F 입력 시 UI가 해당 로컬 클라이언트에만 열리는지 확인.
- 제작 예약 요청 시 재료가 소비되고, 완료 전에는 결과물이 지급되지 않는지 확인.
- 작업 완료 시 결과물이 거점 저장소 또는 작업대 내부 fallback에 저장되는지 확인.
- Client가 거리 밖에서 hold RPC를 보내도 서버에서 작업 시작이 거부되는지 확인.
- 작업 중 Worker 파괴, 작업대 Destroy, 거리 이탈, 완료 시 `ActiveWorkerCount`가 0으로 정리되는지 확인.
- 두 플레이어가 동시에 F hold를 시도했을 때 `MaxActiveWorkers` 기준으로 서버 상태가 유지되는지 확인.
- `RequiredWorkTag`와 팰 `FWorkAttitude.WorkTypeTag`가 같은 경우 `CanWork()`가 true를 반환하는지 확인.
- `TryAssignIdlePalToWorkTargetByTag()`가 적성 없는 팰은 배정하지 않고, 적성 있는 유휴 팰만 `MovingToWork`로 기록하는지 확인.
- BaseCamp, StorageBox 같은 기존 1회 F 상호작용이 fallback으로 계속 동작하는지 확인.

## Excluded Scope

- 팰이 실제로 작업대까지 이동하고 도착 후 `BeginWork()`를 호출하는 AI/BT 흐름.
- 건설 UI, 고스트 프리뷰, 실제 설치 확정 흐름.
- 제작 큐 UI, 결과물 수령 UI, 작업 완료 보상 hook.
- 작업자별 속도 보정.
