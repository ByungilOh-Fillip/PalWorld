# LMK 월드 시스템 아키텍처

이 문서는 GameState, WorldSubsystem, Actor의 책임을 나눈다.
기본 기준은 "복제 상태는 GameState, 실행 로직은 WorldSubsystem, 배치 단위는 Actor"다.

## 책임 분리

| 위치 | 책임 | 예시 |
|---|---|---|
| `APW_WorldGameState` | 클라이언트에 복제될 월드 상태 | 시간, 낮/밤 |
| `UWorldSubsystem` | 월드 단위 서버 실행 로직 | 시간 진행, 스폰 관리 |
| `AActor` | 레벨에 배치되는 대상 | 스포너, 온도 볼륨, 작업 건물 |
| `UActorComponent` | Actor 내부 기능 분리 | 작업 슬롯, 온도 영향, 상호작용 |
| Interface | 파트 간 직접 참조 방지 | 작업 수행자, 상태 태그 대상 |

## 기본 흐름

```text
WorldSubsystem
    서버에서 규칙 계산
    ↓
GameState
    복제될 상태 저장
    ↓
Client
    OnRep으로 표시와 반응 처리
```

## 적용 예시

| 시스템 | 실행 로직 | 복제 상태 | 배치 대상 |
|---|---|---|---|
| 시간 | `UPW_WorldTimeSubsystem` | `APW_WorldGameState` | 없음 |
| 온도 | `UPW_TemperatureSubsystem` | 필요 시 GameState | `APW_TemperatureVolume` |
| 야생 팰 스폰 | `UPW_WildPalSpawnSubsystem` | 스폰된 Actor Replication | `APW_WildPalSpawner` |
| 작업 건물 | 필요 시 `UPW_WorkTargetSubsystem` | 건물 Actor Replication | `APW_WorkBuildingBase` |

## 금지 구조

- 하나의 `UPW_WorldSubsystem`에 시간, 온도, 스폰, 작업 건물을 모두 넣지 않는다.
- `GameState`에 스폰 루프, 온도 샘플링, 작업 탐색 같은 실행 로직을 몰아넣지 않는다.
- 클라이언트가 월드 권한 상태를 직접 변경하지 않는다.
- 플레이어/팰 구체 클래스를 LMK 시스템이 직접 참조하지 않는다.

## 구현 판단 기준

- 여러 클라이언트가 같은 값을 봐야 하면 `GameState`에 둔다.
- 서버에서 주기적으로 계산하거나 액터들을 관리하면 `WorldSubsystem`에 둔다.
- 레벨에 직접 놓고 위치/범위가 중요하면 `Actor`에 둔다.
- Actor 내부 기능이 커지면 `ActorComponent`로 분리한다.
- 다른 파트 구현체와 연결되면 Interface 또는 GameplayTag를 먼저 고려한다.
