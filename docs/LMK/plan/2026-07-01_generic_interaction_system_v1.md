# 범용 F 키 상호작용 시스템 구현 기록

## Summary

BaseCamp, StorageBox, NPC 같은 대상이 공통 방식으로 F 키 상호작용을 제공하도록 한다.
구현은 SOLID 원칙에 맞춰 대상 계약, 대상 설정, 플레이어 감지/선택, 실제 대상별 동작을 분리한다.
다른 사람 폴더 수정을 줄이기 위해 신규 공용 코드는 `Global` 중심, BaseCamp 연동은 `LMK` 중심으로 둔다.

## Implemented Files

- 공용 신규 코드
  - `PalWorld/Source/PalWorld/Global/Components/public/PWInteractable.h`
  - `PalWorld/Source/PalWorld/Global/Components/public/PWInteractableTargetComponent.h`
  - `PalWorld/Source/PalWorld/Global/Components/private/PWInteractableTargetComponent.cpp`
  - `PalWorld/Source/PalWorld/Global/Components/public/PWInteractionScannerComponent.h`
  - `PalWorld/Source/PalWorld/Global/Components/private/PWInteractionScannerComponent.cpp`
- LMK/Base 연동
  - `APW_BaseCampActor`, `APW_StorageBoxActor`에 인터페이스 구현과 `UPWInteractableTargetComponent` 추가.
- PJH 플레이어 C++
  - C++ 코드는 건드리지 않았다.
  - F 입력은 플레이어 BP에 `UPWInteractionScannerComponent`를 붙이고 Enhanced Input에서 `TryInteract()` 호출하는 방식으로 연결한다.

## Implementation

- `IPWInteractable`
  - `CanInteract(AActor* Interactor) const`
  - `Interact(AActor* Interactor)`
  - `GetInteractionPrompt() const`
  - `GetInteractionPriority() const`
- `UPWInteractableTargetComponent`
  - 상호작용 반경, 프롬프트, 우선순위, 활성 여부만 보관한다.
  - 실제 실행 로직은 Actor의 `IPWInteractable::Interact()`가 담당한다.
- `UPWInteractionScannerComponent`
  - 플레이어 Actor에 붙는 범용 감지 컴포넌트.
  - 주기적으로 `UGameplayStatics::GetAllActorsWithInterface()`로 `IPWInteractable` 대상 후보를 찾는다.
  - 충돌 컴포넌트가 없는 BaseCamp도 감지되도록 물리 Overlap에 의존하지 않는다.
  - `IPWInteractable` 구현 여부와 `CanInteract()`로 필터링한다.
  - 현재 선택 대상과 프롬프트를 Blueprint에서 읽을 수 있게 노출한다.
  - `TryInteract()` 호출 시 서버 권한 경로에서 거리와 가능 여부를 다시 검증한다.
- BaseCamp / StorageBox
  - BaseCamp의 `CampRadius`는 영향권으로 유지한다.
  - F 키 상호작용 반경은 `UPWInteractableTargetComponent.InteractionRadius`로 별도 관리한다.
  - StorageBox 우선순위를 BaseCamp보다 높게 둔다.
  - 현재 `Interact()`는 실제 UI 열기 대신 `UE_LOG`와 좌측 상단 `AddOnScreenDebugMessage()`를 출력한다.

## Defaults

| 대상 | Prompt | Radius | Priority |
|---|---:|---:|---:|
| BaseCamp | `Open Base Camp` | `350` | `50` |
| StorageBox | `Open Storage Box` | `250` | `100` |

## Overlap Resolution

여러 상호작용 반경이 겹치면 하나의 현재 대상만 선택한다.

1. `Priority` 높은 대상 우선.
2. 같은 Priority면 카메라 정면에 가까운 대상 우선.
3. 그래도 같으면 더 가까운 대상 우선.
4. 그래도 같으면 기존 선택 유지.

기본 Priority:

- NPC: `200`
- StorageBox: `100`
- BaseCamp: `50`
- 기타 오브젝트: `10`

## Test Plan

- 빌드 성공 확인.
- 플레이어 BP에 `UPWInteractionScannerComponent` 추가 후 F 입력에서 `TryInteract()` 호출.
- StorageBox 단독 반경 진입 시 선택/상호작용 확인.
- BaseCamp 단독 반경 진입 시 선택/상호작용 확인.
- StorageBox와 BaseCamp 반경이 겹칠 때 StorageBox가 우선 선택되는지 확인.
- 반경 밖에서 F 입력 시 실행되지 않는지 확인.
- 서버 검증으로 클라이언트가 잘못된 거리에서 요청해도 거부되는지 확인.

## Assumptions

- 현재 작업자는 LMK/BaseCamp 쪽 작업자로 보고, PJH 폴더 수정은 피한다.
- 공용 기능은 `Global/Components`에 둔다.
- UI 프롬프트 표시는 1차 구현에서 Blueprint가 `GetCurrentPrompt()`를 읽어 처리한다.
- C++ HUD/PJH 위젯 수정은 이번 범위에서 제외한다.
- 자세한 에디터 연결과 확인 절차는 `docs/LMK/handoff/generic_interaction_system_handoff.md`를 참고한다.
