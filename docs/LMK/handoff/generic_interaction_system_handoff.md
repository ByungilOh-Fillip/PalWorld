# 범용 상호작용 시스템 인수인계

이 문서는 BaseCamp, StorageBox, NPC 등 여러 대상에서 같은 방식으로 F 키 상호작용을 붙이기 위한 구현 내용을 정리한다.

## 구조

| 구성 | 책임 |
|---|---|
| `IPWInteractable` | 상호작용 대상이 제공해야 하는 최소 계약 |
| `UPWInteractableTargetComponent` | 반경, 프롬프트, 우선순위, 활성 상태 설정 |
| `UPWInteractionScannerComponent` | 플레이어 주변 후보 탐색, 현재 대상 선택, `TryInteract()` 진입점 |
| BaseCamp / StorageBox | 대상별 `CanInteract`, `Interact`, Prompt, Priority 구현 |

플레이어 쪽 C++은 수정하지 않았다.
플레이어 BP에 `UPWInteractionScannerComponent`를 붙이고, Enhanced Input의 F 입력에서 `TryInteract()`를 호출해서 연결한다.

## 대상 판정

`UPWInteractionScannerComponent`는 주기적으로 `IPWInteractable` 인터페이스를 구현한 Actor를 찾는다.
물리 Overlap에 의존하지 않기 때문에 충돌 컴포넌트가 없는 BaseCamp도 감지된다.

후보가 되려면 아래 조건을 모두 만족해야 한다.

- Actor가 `IPWInteractable`을 구현한다.
- Actor에 `UPWInteractableTargetComponent`가 있다.
- 타겟 컴포넌트가 활성 상태다.
- 플레이어와 타겟의 거리가 `InteractionRadius` 안쪽이다.
- 플레이어와 타겟의 거리가 스캐너의 `ScanRadius` 안쪽이다.
- 대상의 `CanInteract(Interactor)`가 `true`를 반환한다.

## 겹침 해결

여러 대상의 상호작용 반경이 겹치면 하나만 선택한다.

1. `Priority`가 높은 대상.
2. 같은 Priority면 카메라 정면에 가까운 대상.
3. 그래도 같으면 더 가까운 대상.
4. 그래도 같으면 기존 선택 대상 유지.

현재 기본값은 StorageBox `100`, BaseCamp `50`이다.
따라서 두 반경이 겹치면 StorageBox가 먼저 선택된다.

## 현재 연동 대상

| Actor | Prompt | Radius | Priority | Interact 동작 |
|---|---:|---:|---:|---|
| `APW_BaseCampActor` | `Open Base Camp` | `350` | `50` | 로그와 좌측 상단 디버그 메시지 출력 |
| `APW_StorageBoxActor` | `Open Storage Box` | `250` | `100` | 로그와 좌측 상단 디버그 메시지 출력 |

현재 `Interact()`는 UI를 열지 않는다.
기능 호출 확인을 위해 `UE_LOG`와 `GEngine->AddOnScreenDebugMessage()`만 수행한다.

## 에디터 연결 절차

1. 플레이어 BP에 `UPWInteractionScannerComponent`를 추가한다.
2. `IA_Interact`를 만들고 `IMC_GamePlay`에 F 키로 매핑한다.
3. 플레이어 BP 또는 PlayerController BP에서 `IA_Interact Started` 이벤트를 받는다.
4. 플레이어의 `UPWInteractionScannerComponent`를 가져와 `TryInteract()`를 호출한다.
5. 프롬프트 UI가 필요하면 `GetCurrentPrompt()`를 읽어서 임시 텍스트로 표시한다.

## 테스트 체크리스트

- 빌드 성공.
- BaseCamp 단독 반경 진입 시 `Open Base Camp`가 선택되는지 확인.
- StorageBox 단독 반경 진입 시 `Open Storage Box`가 선택되는지 확인.
- F 입력 시 좌측 상단에 `[Interaction] BaseCamp: ...` 또는 `[Interaction] StorageBox: ...`가 출력되는지 확인.
- BaseCamp와 StorageBox 반경이 겹칠 때 StorageBox가 선택되는지 확인.
- 반경 밖에서 F 입력 시 아무 동작도 하지 않는지 확인.
- PIE 2인 Listen Server에서 클라이언트 입력도 서버 검증 후 처리되는지 확인.

## 후속 작업

- BaseCamp UI 또는 StorageBox Inventory UI가 정해지면 각 Actor의 `Interact()`에서 해당 진입점을 호출한다.
- NPC는 Actor에 `UPWInteractableTargetComponent`를 붙이고 `IPWInteractable`만 구현하면 같은 스캐너에서 감지된다.
- 월드에 상호작용 대상이 매우 많아지면 `GetAllActorsWithInterface()` 방식 대신 등록형 Subsystem으로 후보 목록을 관리한다.
