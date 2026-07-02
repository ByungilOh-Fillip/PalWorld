# LMK 월드 시스템 문서 목차

LMK 파트는 플레이어 파트와 팰 파트가 같은 월드 규칙 위에서 동작하도록 서버 권한 기반 시스템을 제공한다.

## AI Agent Read Order

AI 에이전트는 작업 시작 전에 아래 순서로 문서를 읽는다.

1. [01_scope.md](01_scope.md) - 담당 범위, 제외 범위, 초기 성공 기준 확인
2. [00_architecture.md](00_architecture.md) - GameState, WorldSubsystem, Actor 책임 기준 확인
3. [10_implementation_order.md](10_implementation_order.md) - 현재 구현 순서와 검증 기준 확인
4. 작업 대상 시스템 문서 확인
5. 연동이 필요하면 [02_gameplay_tags.md](02_gameplay_tags.md), [03_interfaces.md](03_interfaces.md) 확인
6. 저장 연동이 필요할 때만 [09_save_hooks.md](09_save_hooks.md) 확인

## Task Routing

| 요청 키워드 | 먼저 읽을 문서 |
|---|---|
| 구조, 아키텍처, Subsystem, GameState | [00_architecture.md](00_architecture.md) |
| 범위, 담당, 제외 | [01_scope.md](01_scope.md) |
| 태그, GameplayTag, 상태 | [02_gameplay_tags.md](02_gameplay_tags.md) |
| 인터페이스, 팰/플레이어 연동 | [03_interfaces.md](03_interfaces.md) |
| 시간, 낮, 밤 | [04_world_time.md](04_world_time.md) |
| 온도, 추위, 더위 | [06_temperature.md](06_temperature.md) |
| 야생 팰, 스폰, 리스폰 | [07_wild_pal_spawn.md](07_wild_pal_spawn.md) |
| 작업 건물, 작업 슬롯 | [08_work_building.md](08_work_building.md) |
| 저장, 로드 | [09_save_hooks.md](09_save_hooks.md) |
| 채집 자원, 나무, 돌, 오픈월드 최적화 | [11_harvestable_resource_optimization.md](11_harvestable_resource_optimization.md) |
| 구현 순서, 검증 | [10_implementation_order.md](10_implementation_order.md) |

## 문서 목록

| 문서 | 내용 |
|---|---|
| [00_architecture.md](00_architecture.md) | GameState, WorldSubsystem, Actor 책임 분리 |
| [01_scope.md](01_scope.md) | 담당 범위, 제외 범위, 초기 성공 기준 |
| [02_gameplay_tags.md](02_gameplay_tags.md) | GameplayTag 사용 기준 |
| [03_interfaces.md](03_interfaces.md) | 플레이어/팰/월드 연결 인터페이스 |
| [04_world_time.md](04_world_time.md) | 낮/밤, 시간 동기화 |
| [06_temperature.md](06_temperature.md) | 온도 계산과 탐험 상태 태그 |
| [07_wild_pal_spawn.md](07_wild_pal_spawn.md) | 야생 팰 스폰 조건과 생명주기 |
| [08_work_building.md](08_work_building.md) | 작업 건물과 팰 작업 적성 연결 |
| [09_save_hooks.md](09_save_hooks.md) | 저장 시스템 연동 기준 |
| [10_implementation_order.md](10_implementation_order.md) | 구현 순서와 검증 기준 |
| [11_harvestable_resource_optimization.md](11_harvestable_resource_optimization.md) | 채집 자원 오픈월드 최적화 기준 |

## 설계 원칙

- Listen Server 기준으로 먼저 구현한다.
- Dedicated Server 확장이 막히지 않도록 서버 권한 로직과 로컬 표시 로직을 분리한다.
- 복제되는 월드 상태는 `GameState`, 서버 실행 로직은 `WorldSubsystem`, 배치 단위는 `Actor`에 둔다.
- 플레이어, 팰, 건물 구체 구현체를 직접 참조하지 않는다.
- Enum 대신 공유 GameplayTag 문서의 `Work.*`, `Status.*` 태그를 우선 사용한다.
- 실제 호출자가 없는 추상화는 만들지 않는다.

## 우선순위 요약

1. 월드 시간과 낮/밤 동기화
2. 온도 볼륨과 상태 태그 적용 요청
3. 낮/밤 조건 기반 야생 팰 스폰
4. 작업 건물의 `Work.*` 요구 태그와 작업 슬롯

## Agent Working Rules

- 먼저 범위 문서를 확인하고 제외 범위를 구현하지 않는다.
- 구현 전에 [00_architecture.md](00_architecture.md)의 책임 분리 기준을 확인한다.
- 실제 호출자가 없는 인터페이스, Subsystem, DataAsset은 만들지 않는다.
- 서버 권한 변경과 클라이언트 표시 변경을 같은 책임에 섞지 않는다.
- 플레이어/팰 파트와 연결할 때는 구체 클래스 대신 인터페이스나 GameplayTag를 사용한다.
- 작업 후 관련 문서의 완료 기준을 기준으로 검증한다.
