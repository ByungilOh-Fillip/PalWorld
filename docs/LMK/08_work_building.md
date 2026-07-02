# Work Building 시스템

목표는 작업 건물이 요구 작업 태그를 제공하고, 플레이어/팰 파트가 이를 기준으로 작업을 수행할 수 있게 하는 것이다.

현재 1차 구현은 에디터에 직접 배치한 작업대를 기준으로 한다.
건설 UI와 고스트 프리뷰는 후속 단계다.
팰 자동 작업은 서버 배정 상태와 이동 목표 제공 API까지만 구현되어 있고, 실제 AI 이동/행동트리 연결은 후속 단계다.

## 주요 클래스

- `APW_WorkBuildingBase`
- `UPW_WorkBuildingComponent`
- `UPW_BaseWorkTargetRegistryComponent`
- `UPWInteractionScannerComponent`
- `IPWHoldInteractable`
- `IPWLocalInteractable`

## 책임 분리

| 위치 | 책임 |
|---|---|
| `APW_WorkBuildingBase` | 건물 Actor, Replication, 거점 등록, F 키 상호작용 진입점 |
| `UPW_WorkBuildingComponent` | 작업 상태, 요구 태그, 서버 작업자 목록, 진행률, 작업 위치 |
| `UPW_BaseWorkTargetRegistryComponent` | 거점 내부 작업 대상 등록/후보 목록 검색 |
| `UPW_BasePalAssignmentComponent` | 거점 팰 슬롯, 작업 대상 배정 상태 기록 |
| `UPWInteractionScannerComponent` | 현재 상호작용 대상 탐색, hold 상호작용 시작/종료 서버 요청, 로컬 UI fallback |
| Pal/AI Part | 배정된 작업 대상까지 이동하고 도착 후 작업 시작/중단 호출 |

## 현재 구현 상태

- [x] 작업대 Actor가 기존 `IPWInteractable` 상호작용 대상이 됨
- [x] 작업대 Actor가 hold 상호작용 대상(`IPWHoldInteractable`)이 됨
- [x] 작업대에 `UPWInteractableTargetComponent` 추가
- [x] 작업대에 `UPW_WorkBuildingComponent` 추가
- [x] `UPW_WorkBuildingComponent`가 요구 작업 태그와 작업 진행 상태를 보관
- [x] 작업 상태가 `Idle`, `Reserved`, `InProgress`, `Completed`로 분리됨
- [x] 작업 진행률, 작업 상태, 활성 작업자 수가 복제됨
- [x] 실제 작업자 목록은 서버 내부 상태로 유지됨
- [x] 작업 예약/진행 중이면 F hold로 플레이어 직접 작업 시작
- [x] F release/canceled에서 플레이어 직접 작업 종료
- [x] 작업 예약/진행 중이 아니면 로컬 클라이언트에서 `OnOpenWorkbenchUI` Blueprint hook 호출
- [x] 거점 안 작업대가 base work registry에 등록됨
- [x] registry에서 태그별 작업대 후보 목록을 반환할 수 있음
- [x] 거점 팰 슬롯에 작업 대상 ID와 상태를 서버 기준으로 기록할 수 있음
- [x] 거점 액터가 유휴 팰의 `PWSkillComponent.CanWork()`를 검사해 작업 후보에 배정할 수 있음
- [x] 팰 `PWSkillComponent`에서 `FWorkAttitude`를 GameplayTag로 조회 가능
- [x] 제작 요청은 재료를 소비하고 작업 예약을 생성하며, 결과물은 작업 완료 이벤트에서 지급됨
- [x] 작업대 내부 인벤토리는 제작 결과 fallback 저장소로 유지
- [x] 테스트용 `Start With Reserved Work`로 PIE 시작 시 예약 작업 상태를 만들 수 있음

## BP 입력 연결

플레이어 BP에 `UPWInteractionScannerComponent`를 붙이고 F 입력에서 아래처럼 호출한다.

```text
F Pressed -> TryBeginHoldInteraction()
F Released/Canceled -> EndHoldInteraction()
```

`TryBeginHoldInteraction()`은 hold 대상이면 hold 작업을 시작하고, hold 대상이 아니면 기존 `TryInteract()`로 fallback한다.
작업대 UI는 `IPWLocalInteractable` 경로로 로컬 클라이언트에서만 열린다.
따라서 StorageBox, BaseCamp, NPC 같은 단일 F 키 상호작용도 같은 입력 경로를 사용한다.

작업 예약 상태를 테스트하려면 작업대 BP의 `WorkBuildingComponent`에서 `Start With Reserved Work`를 체크한다.
`Has Reserved Work`는 런타임 복제 상태 확인용이라 에디터에서 직접 수정하지 않는다.

## 구현 항목

- [x] 건물별 요구 작업 태그 설정
- [x] 작업 건물을 거점 작업 대상 Registry에 등록
- [x] 작업 슬롯 수 설정
- [x] 슬롯 점유 상태 관리
- [x] 작업 가능 거리 또는 반경 설정
- [x] 작업 진행률 관리
- [x] 서버 권한으로 작업 슬롯 예약
- [x] 서버 권한으로 작업 시작
- [x] 서버 권한으로 작업 중단
- [x] 서버 권한으로 작업 완료
- [x] 작업 수행자의 `Work.*` 태그 보유 여부 검사 API
- [ ] 작업 수행자의 건강/상태 태그 검사 hook
- [ ] 작업 속도 보정 hook
- [x] 작업 완료 시 제작 결과 지급 hook
- [x] 작업 상태 Replication

## 1차 건물 후보

| 건물 유형 | 요구 태그 | 1차 목표 |
|---|---|---|
| 벌목장 | `Work.Lumbering` | 팰이 작업 슬롯을 예약하고 진행률을 채움 |
| 채석장 | `Work.Mining` | 벌목장과 같은 구조로 다른 태그 검증 |
| 발전기 | `Work.GeneratingElectricity` | 작업 중일 때 건물 활성 상태 유지 |
| 냉장고 | `Work.Cooling` | 작업 중일 때 온도 보정 hook 검증 |

## 작업 흐름

```text
Player BP -> InteractionScanner.TryBeginHoldInteraction
InteractionScanner -> 현재 대상 hold 가능 여부 검사
WorkBuilding -> WorkBuildingComponent.BeginWork
Player BP -> InteractionScanner.EndHoldInteraction
WorkBuilding -> WorkBuildingComponent.EndWork
```

팰 자동 작업은 서버 배정 API까지만 구현되어 있다. 후속 AI 흐름은 아래와 같다.

```text
BaseCamp -> Base WorkTargetRegistry 후보 검색
BaseCamp -> 유휴 Pal 슬롯 검색
BaseCamp -> PWSkillComponent.CanWork 검사
BaseCamp -> PalAssignment에 MovingToWork 상태와 WorkTargetId 기록
Pal AI -> 작업대 정면 위치로 이동
WorkBuilding -> 서버에서 작업 시작/진행/완료 처리
```

## 완료 기준

- [x] 팰 파트가 건물의 요구 작업 태그를 조회할 수 있음
- [x] 작업 건물이 팰 구체 클래스에 직접 의존하지 않음
- [x] 슬롯 예약/해제가 서버 기준으로 일관되게 처리됨
- [x] 작업 진행 상태가 클라이언트에 동기화됨
- [ ] 최소 2개 이상의 서로 다른 `Work.*` 태그 건물이 같은 기반 구조를 사용함
