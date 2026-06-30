# 2026-06-30_base_camp_actor_centric_v1 구현 계획

## 구현 목표

`APW_BaseCampActor`를 베이스캠프의 Aggregate Root로 두고, `UPW_BaseCampSubsystem`은 월드 Registry와 설치 검증만 담당한다.
베이스 내부 상태는 Actor Component로 분리하고, `CampRadius`는 개발 중 확인할 수 있도록 파란색 디버그 범위로 표시한다.

## 책임 분리

| 위치 | 책임 |
|---|---|
| `UPW_BaseCampSubsystem` | 베이스 등록/해제, 위치/소유자 조회, 반경 겹침 설치 검증 |
| `APW_BaseCampActor` | 베이스 루트 Actor, 복제 상태, 컴포넌트 소유 |
| `UPW_BaseOwnershipComponent` | Player/Guild 확장 가능한 권한 검사 |
| `UPW_BaseInventoryAggregatorComponent` | 캠프 내 보관함 합산 조회, 서버 권한 아이템 소비/추가 |
| `UPW_BasePalAssignmentComponent` | 배치 팰 슬롯과 표현용 팰 Actor 관리 |
| `UPW_BaseWorkTargetRegistryComponent` | 캠프 내부 작업 대상 등록/검색 |
| `UPW_BaseWorkSimulationComponent` | `LastSimulatedTime` 기반 작업 진행 정산 |
| `UPW_InventoryComponent` | Actor 단위 아이템 스택 보관 |
| `APW_StorageBoxActor` | 캠프 공용 인벤토리에 참여하는 보관함 |
| `APW_WorkBuildingBase` | 캠프 작업 대상 및 제작 재료 소비 진입점 |

## 멀티플레이어 기준

- 클라이언트는 설치, 배치, 아이템 이동, 제작 요청만 보낸다.
- 서버가 상태 변경을 확정하고 Replication 또는 Client RPC로 결과를 전달한다.
- Subsystem과 서버 컴포넌트는 UI, HUD, Viewport, LocalPlayer에 의존하지 않는다.
- Listen Server 우선 검증이지만 Dedicated Server에서도 서버 로직이 동작할 수 있게 시각 요소와 권한 로직을 분리한다.

## 오픈월드 최적화 기준

- 베이스 내부 상태는 `APW_BaseCampActor`와 컴포넌트에 응집한다.
- 멀리 있는 베이스의 상세 인벤토리/작업 상태를 항상 모든 클라이언트에 복제하지 않는다.
- 배치 팰 Actor는 표현용이며, 작업의 진실 상태는 `UPW_BaseWorkSimulationComponent`가 가진다.
- 플레이어가 멀리 있으면 Tick 없이 `LastSimulatedTime` 기준으로 Lazy 정산한다.
- 배치 팰 슬롯, 아이템 스택, 작업 상태 배열은 추후 `FFastArraySerializer`로 바꿀 수 있게 구조화한다.

## 구현 항목

1. 베이스 공통 타입 정의
   - `FPW_BaseCampId`
   - `EPW_BaseOwnerType`
   - `FPW_BaseOwnerId`
   - `FPW_ItemStack`
   - `FPW_BaseWorkState`
2. `APW_BaseCampActor` 추가
   - `BaseCampId`, `OwnerId`, `CampRadius`, `CampLevel` 복제
   - BeginPlay/EndPlay에서 Subsystem 등록/해제
   - `CampRadius` 파란색 디버그 Sphere 표시
3. `UPW_BaseCampSubsystem` 추가
   - 등록/해제
   - 위치 기반 조회
   - 소유자 기반 조회
   - 반경 겹침 설치 검증
4. 베이스 컴포넌트 추가
   - Ownership
   - Inventory Aggregator
   - Pal Assignment
   - Work Target Registry
   - Work Simulation
5. Storage/Work 기본 Actor 추가
   - `APW_StorageBoxActor`
   - `APW_WorkBuildingBase`
   - `UPW_InventoryComponent`

## 테스트 기준

- `PalWorldEditor` C++ 빌드 성공
- PIE에서 `APW_BaseCampActor`를 배치하면 `CampRadius`가 파란색으로 보임
- 서버에서 생성된 베이스가 클라이언트에 복제됨
- 겹치는 `CampRadius` 위치에는 새 베이스 설치 검증이 실패함
- 같은 캠프 안 보관함의 아이템이 공용 인벤토리로 합산됨
- 작업대가 캠프 공용 인벤토리 재료를 소비할 수 있음
- 플레이어가 멀리 있어도 `UPW_BaseWorkSimulationComponent`가 시간 차이 기반으로 작업을 정산함

## 제외 범위와 가정

- UI, 설치 프리뷰, Item DataAsset, 길드 권한 테이블은 후속 작업이다.
- 1차 `ItemId`는 `FName`이다.
- 1차 소유권은 Player만 실제 사용하고, Guild는 타입/API만 열어둔다.
- 작업의 진실 상태는 팰 Actor가 아니라 `UPW_BaseWorkSimulationComponent`가 가진다.
