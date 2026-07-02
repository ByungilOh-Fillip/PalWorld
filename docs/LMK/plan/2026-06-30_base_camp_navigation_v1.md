# 2026-06-30_base_camp_navigation_v1 구현 계획

## 구현 목표

베이스캠프 안에서 배치 팰이 작업대, 보관함, 자원 사이를 이동할 수 있도록 베이스 단위 Navigation Invoker 기반 Dynamic NavMesh를 사용한다.
월드 전체 Dynamic NavMesh 비용을 피하기 위해 `Generate Navigation Only Around Navigation Invokers`를 기본 설정으로 둔다.

## 책임 분리

| 위치 | 책임 |
|---|---|
| `UPW_BaseNavigationComponent` | 베이스 반경 기준 NavMesh 조회, 경로 검증, reachable point 제공 |
| `UNavigationInvokerComponent` | 베이스 주변 NavMesh 생성/제거 반경 제공 |
| `APW_BaseCampActor` | Navigation 컴포넌트 소유 및 `CampRadius` 기준 Invoker 설정 |
| `UPW_BasePalAssignmentComponent` | 배치 팰 표현 Actor를 NavMesh reachable point에 스폰 |
| Storage/Work Actor Mesh | 필요 시 NavMesh 장애물로 반영 |

## 구현 항목

1. `UPW_BaseNavigationComponent` 추가
   - `ConfigureNavigationInvoker(CampRadius)`
   - `ProjectPointToBaseNavigation`
   - `FindPathToLocation`
   - `GetRandomReachablePointInBase`
   - `IsNavigationReadyForBase`
   - `DrawDebugNavigationSample`
2. `APW_BaseCampActor`에 `UPW_BaseNavigationComponent`와 `UNavigationInvokerComponent` 추가
3. `CampRadius` 변경 시 Invoker 생성 반경을 갱신
4. 배치 팰 스폰을 랜덤 오프셋 대신 NavMesh reachable point 우선으로 변경
5. 보관함/작업대 StaticMesh가 Navigation에 영향을 줄 수 있게 설정
6. `DefaultEngine.ini`에 Dynamic NavMesh와 Navigation Invoker 기반 생성 설정 추가

## 에디터 설정과 테스트

- 레벨에는 `NavMeshBoundsVolume`이 필요하다. Invoker를 써도 Bounds 밖에는 NavMesh가 생성되지 않는다.
- PIE에서 `P` 키로 베이스 주변 NavMesh가 생성되는지 확인한다.
- 베이스 Actor의 파란 `CampRadius`와 NavMesh 생성 범위가 맞는지 확인한다.
- 배치 팰 스폰 시 NavMesh 위 reachable point에 스폰되는지 확인한다.
- 작업대/보관함 설치 후 길막 오브젝트가 NavMesh에 반영되는지 확인한다.

## 제외 범위와 가정

- 문, 계단, 토대 등 건축물별 세부 NavModifier 정책은 후속 작업이다.
- NavMesh가 아직 준비되지 않아도 작업 시뮬레이션은 멈추지 않는다.
- 실제 AI MoveTo 작업 연결은 후속 작업이며, 이번 범위는 베이스 내 NavMesh 준비와 스폰 위치 보정이다.
