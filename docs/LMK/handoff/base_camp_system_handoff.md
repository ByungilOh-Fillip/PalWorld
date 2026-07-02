# 베이스캠프 시스템 구현 인수인계서

이 문서는 공식 LMK 시스템 목차에 포함되는 설계 문서가 아니라, 베이스캠프/플레이어/팰/건축물 연동 담당자가 현재 구현 내용을 이어받기 위한 인수인계 문서다.
목적은 베이스캠프 구현의 책임 경계, 에디터 설정 방법, 테스트 절차, 후속 작업 지점을 명확히 전달하는 것이다.

## 문서 성격

| 항목 | 내용 |
|---|---|
| 문서 유형 | 인수인계 |
| 대상 | 베이스캠프, 플레이어 설치, 팰 AI, 건축물 담당자 |
| 범위 | 베이스캠프 Actor 중심 구조, 공용 보관함, 작업 대상 등록, 배치 팰 슬롯, NavMesh Invoker, 플레이어 설치 테스트 컴포넌트 |
| 비범위 | 정식 설치 UI, 설치 비용, 길드 권한 테이블, 최종 아이템 DataAsset, 팰 행동 트리, 정식 작업 애니메이션 |
| 참고 계획 | `docs/LMK/plan/2026-06-30_base_camp_actor_centric_v1.md` |

## 현재 구현 상태 요약

베이스캠프는 `APW_BaseCampActor`를 Aggregate Root로 둔다.
`UPW_BaseCampSubsystem`은 베이스 등록, 위치 조회, 소유자 조회, 설치 가능 여부 검증만 담당한다.
인벤토리, 팰 배치, 작업 대상, 작업 시뮬레이션, NavMesh 상태는 베이스 Actor에 붙은 컴포넌트가 각각 처리한다.

| 책임 | 구현 |
|---|---|
| 베이스 루트 Actor | `APW_BaseCampActor` |
| 베이스 Registry/설치 검증 | `UPW_BaseCampSubsystem` |
| 소유권 검사 | `UPW_BaseOwnershipComponent` |
| 캠프 공용 보관함 집계 | `UPW_BaseInventoryAggregatorComponent` |
| 배치 팰 슬롯/표현 Actor 스폰 | `UPW_BasePalAssignmentComponent` |
| 작업 대상 등록/검색 | `UPW_BaseWorkTargetRegistryComponent` |
| 오프라인 작업 정산 | `UPW_BaseWorkSimulationComponent` |
| 베이스 NavMesh 활성 관리 | `UPW_BaseNavigationComponent` |
| 플레이어 주변 NavMesh Invoker 설정 | `UPW_PlayerNavigationInvokerComponent` |
| 플레이어 베이스 설치 테스트 | `UPW_PlayerBasePlacementComponent` |
| 임시 보관함 Actor | `APW_StorageBoxActor` |
| 임시 작업대 Actor | `APW_WorkBuildingBase` |

## 핵심 코드 위치

```text
PalWorld/Source/PalWorld/LMK/Public/Base/PW_BaseTypes.h
PalWorld/Source/PalWorld/LMK/Public/Base/PW_BaseCampActor.h
PalWorld/Source/PalWorld/LMK/Public/Base/PW_BaseCampSubsystem.h
PalWorld/Source/PalWorld/LMK/Public/Base/PW_PlayerBasePlacementComponent.h
PalWorld/Source/PalWorld/LMK/Public/Navigation/PW_PlayerNavigationInvokerComponent.h

PalWorld/Source/PalWorld/LMK/Private/Base/PW_BaseCampActor.cpp
PalWorld/Source/PalWorld/LMK/Private/Base/PW_BaseCampSubsystem.cpp
PalWorld/Source/PalWorld/LMK/Private/Base/PW_PlayerBasePlacementComponent.cpp
PalWorld/Source/PalWorld/LMK/Private/Navigation/PW_PlayerNavigationInvokerComponent.cpp
```

## 베이스캠프 Actor 구조

`APW_BaseCampActor`는 서버 권한으로 베이스 상태를 가진다.
아래 값은 Replication 대상이다.

| 값 | 용도 |
|---|---|
| `BaseCampId` | 베이스 고유 ID |
| `OwnerId` | 현재 Player 소유, 추후 Guild 확장 가능 |
| `CampRadius` | 베이스 범위 |
| `CampLevel` | 추후 확장용 레벨 |

`bDrawDebugRadius`가 켜져 있으면 에디터/PIE에서 `CampRadius`가 파란색 디버그 스피어로 표시된다.
Dedicated Server에서는 시각 디버그를 그리지 않는다.

## 베이스 설치 테스트 흐름

플레이어 C++ 클래스는 직접 수정하지 않았다.
테스트는 플레이어 BP에 `PW_PlayerBasePlacementComponent`를 붙여서 진행한다.

### 에디터 준비

1. `APW_BaseCampActor`를 상속한 `BP_BaseCamp_Test`를 만든다.
2. `BP_BaseCamp_Test`에 임시 `StaticMesh`를 추가한다.
3. `BP_BaseCamp_Test`의 `CampRadius`, `bDrawDebugRadius`를 테스트 값으로 설정한다.
4. 플레이어 BP에 `PW_PlayerBasePlacementComponent`를 추가한다.
5. `PW_PlayerBasePlacementComponent.BaseCampClass`에 `BP_BaseCamp_Test`를 지정한다.

### 입력 연결

Enhanced Input을 쓰는 경우:

1. `IA_BasePlacement`를 만든다.
2. `IMC_GamePlay` 같은 Mapping Context에 `B` 키를 매핑한다.
3. 플레이어 BP 또는 PlayerController BP Event Graph에서 `IA_BasePlacement`의 `Started` 이벤트를 받는다.
4. 플레이어의 `PW_PlayerBasePlacementComponent` 참조를 가져온다.
5. `TogglePlacementMode()`를 호출한다.

`B` 키 동작은 현재 테스트용으로 단순화되어 있다.

| 상태 | `B` 입력 결과 |
|---|---|
| 설치 모드 꺼짐 | 설치 모드 켜짐 |
| 설치 모드 켜짐 | 현재 프리뷰 위치에 서버 설치 요청 |

프리뷰 중에는 라인트레이스 위치에 디버그 스피어가 표시된다.
설치 가능하면 파란색, 기존 베이스와 겹치면 빨간색이다.

## NavMesh 설정

프로젝트 설정은 제한형 Dynamic NavMesh 방향으로 잡았다.

```ini
[/Script/NavigationSystem.NavigationSystemV1]
bGenerateNavigationOnlyAroundNavigationInvokers=True

[/Script/NavigationSystem.RecastNavMesh]
RuntimeGeneration=Dynamic
bForceRebuildOnLoad=True
```

레벨에는 여전히 큰 `NavMeshBoundsVolume`이 필요하다.
Invoker가 있어도 Bounds 밖에는 NavMesh가 생성되지 않는다.

## 플레이어 Nav Invoker

야생 팰 스포너가 `GetRandomReachablePointInRadius()`를 사용할 때 실패를 줄이기 위해 `UPW_PlayerNavigationInvokerComponent`를 추가했다.
플레이어 C++ 클래스는 수정하지 않았고, 플레이어 BP에 아래 두 컴포넌트를 수동으로 붙이는 방식이다.

| 컴포넌트 | 역할 |
|---|---|
| `NavigationInvokerComponent` | 엔진 NavMesh Invoker |
| `PW_PlayerNavigationInvokerComponent` | 스포너 기본값에 맞게 Invoker 반경 설정 |

기본값은 현재 야생 팰 스포너 기본값에 맞췄다.

| 값 | 기본값 |
|---|---|
| `GenerationRadius` | `11000` |
| `RemovalRadius` | `13000` |
| `MinGenerationRadius` | `8000` |
| `MaxGenerationRadius` | `15000` |

## 베이스 Nav Invoker

베이스는 항상 NavMesh를 켜두지 않고, 방문자가 있을 때만 Base Invoker를 활성화하는 방향이다.
오랫동안 아무도 방문하지 않은 베이스는 Base Invoker를 비활성화하고, 작업은 `UPW_BaseWorkSimulationComponent`의 시간 차이 기반 Offline Simulation으로 정산한다.

`UPW_BaseNavigationComponent`는 같은 Actor에 붙은 `UNavigationInvokerComponent`를 설정한다.
NavMesh가 비활성 또는 준비되지 않은 경우 팰 Actor 스폰/실제 이동은 보류될 수 있지만, 배치 슬롯과 작업 시뮬레이션 데이터는 유지된다.

## 공용 보관함과 작업대

`APW_StorageBoxActor`는 자기 위치 기준으로 `UPW_BaseCampSubsystem::FindBaseCampAtLocation()`을 호출해 소속 베이스를 찾고, 해당 베이스의 `UPW_BaseInventoryAggregatorComponent`에 등록된다.

`APW_WorkBuildingBase`도 자기 위치 기준으로 베이스를 찾고, `UPW_BaseWorkTargetRegistryComponent`에 작업 대상으로 등록된다.
작업대는 `TryCraftFromBaseInventory()`에서 캠프 공용 보관함 재료를 소비하고, 결과물을 캠프 보관함 또는 내부 인벤토리에 넣는다.

## 멀티플레이어 책임 경계

서버가 권한을 가진다.

| 기능 | 권한 |
|---|---|
| 베이스 설치 | 서버 검증 후 Spawn |
| 베이스 Registry | 서버/클라이언트 월드별 등록 |
| 소유권 판단 | 서버 기준 |
| 보관함 등록/소비 | 서버 기준 |
| 작업 시뮬레이션 | 서버 기준 |
| 팰 실제 이동 | 서버 AI 기준 |
| 클라이언트 화면 | Replication 결과 표시 |

Listen Server로 먼저 동작시키되, Dedicated Server 확장을 막지 않도록 시각 디버그와 서버 로직을 분리했다.

## 테스트 체크리스트

### 컴파일

- C++ 빌드 성공 여부를 먼저 확인한다.
- 현재 인수인계 시점에서 작성자는 최종 빌드를 실행하지 않았다.

### 베이스 설치

- 플레이어 BP에 `PW_PlayerBasePlacementComponent`가 붙어 있는지 확인한다.
- `BaseCampClass`가 `APW_BaseCampActor` 상속 BP인지 확인한다.
- `B` 키를 누르면 프리뷰 디버그 스피어가 보이는지 확인한다.
- 기존 베이스와 겹치면 빨간색으로 보이는지 확인한다.
- 다시 `B` 키를 누르면 서버에서 Base BP가 Spawn되는지 확인한다.
- Spawn된 베이스의 파란 `CampRadius` 디버그가 보이는지 확인한다.

### 베이스 Registry

- `UPW_BaseCampSubsystem::FindBaseCampAtLocation()`으로 설치된 베이스가 조회되는지 확인한다.
- 겹치는 반경에서 `CanPlaceBaseCampAtLocation()`이 실패하는지 확인한다.

### 보관함/작업대

- 캠프 안에 `APW_StorageBoxActor`를 배치하면 Aggregator에 등록되는지 확인한다.
- 같은 캠프 안 여러 보관함의 아이템 수량이 합산되는지 확인한다.
- `APW_WorkBuildingBase::TryCraftFromBaseInventory()`가 캠프 공용 재료를 소비하는지 확인한다.

### NavMesh

- 레벨에 큰 `NavMeshBoundsVolume`이 있는지 확인한다.
- `P` 키 NavMesh debug로 플레이어 주변 NavMesh가 생성되는지 확인한다.
- 플레이어가 이동하면 NavMesh 생성 영역이 따라오는지 확인한다.
- 베이스 방문 시 Base Invoker가 활성화되고, 오래 방문하지 않으면 비활성화되는지 확인한다.

## 현재 주의할 점

1. 플레이어 BP 입력 연결은 C++에서 자동으로 하지 않는다.
2. Base BP는 반드시 `APW_BaseCampActor`를 상속해야 한다.
3. `NavigationInvokerComponent`는 `PW_PlayerNavigationInvokerComponent`가 자동 생성하지 않는다. 플레이어 BP에 둘 다 붙여야 한다.
4. Invoker 방식이어도 레벨 Bounds 밖에는 NavMesh가 생성되지 않는다.
5. 첫 진입 직후에는 NavMesh 타일 생성 지연으로 스폰/경로 탐색이 한두 번 실패할 수 있다.
6. 작업 시뮬레이션은 데이터 정산까지이며, 실제 팰 AI 행동 트리와 작업 애니메이션은 후속 작업이다.
7. 길드 소유권은 타입 구조만 열어두었고, 실제 길드 권한 테이블은 아직 없다.

## 후속 작업 제안

1. 빌드 에러가 있으면 UE 5.8 API 차이를 먼저 정리한다.
2. 설치 프리뷰 Mesh와 설치 UI를 별도 작업으로 만든다.
3. 정식 아이템 DataAsset과 `FPW_ItemStack` 연동 방식을 확정한다.
4. 팰 AI가 `UPW_BaseWorkTargetRegistryComponent`에서 작업 대상을 받아 이동하는 흐름을 만든다.
5. 보관함/작업대 건축물 타입별 NavMesh 영향 정책을 세분화한다.
6. 길드 시스템이 생기면 `FPW_BaseOwnerId`의 `Guild` 타입에 권한 테이블을 연결한다.
