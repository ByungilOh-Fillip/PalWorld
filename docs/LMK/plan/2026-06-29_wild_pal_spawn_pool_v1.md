# 2026-06-29_wild_pal_spawn_pool_v1 구현 계획

## 구현 목표

첫 구현 범위는 `스폰 지역 Actor + 서버 권한 Subsystem + 오브젝트 풀 기반 야생 팰 스폰`이다.
오픈월드/멀티플레이 환경에서 플레이어 주변 스폰만 활성화하고, 지역/전역 예산으로 서버 부하를 제한한다.

## 서버-클라이언트 통신 구조

```text
Server Wild Pal Spawn Subsystem
    플레이어 위치와 전역 예산 평가
    ↓
Server Wild Pal Spawner
    지역 조건, 낮/밤 조건, 지역 예산 평가
    ↓
Server Pal Spawn Pool Subsystem
    풀 재사용 또는 새 Actor Spawn
    ↓
Replicated Pal Actor
    ↓
Relevant Client가 동일 Actor 관찰
```

- 스폰/활성화/반납은 서버에서만 수행한다.
- 클라이언트는 Replicated Actor를 관찰만 한다.
- 맵 반대편 플레이어에게 모든 야생 팰을 항상 복제하지 않고, Unreal relevancy / Net Cull 거리로 근처 플레이어에게만 복제한다.

## 책임 분리

| 위치 | 책임 |
|---|---|
| `APW_WildPalSpawner` | 레벨 배치 스폰 지역, 후보 팰/가중치/낮밤/지역 예산 설정 |
| `UPW_WildPalSpawnSubsystem` | 서버 전용 스포너 등록, 플레이어 거리 조회, 전역 활성 수, 평가 루프 |
| `UPW_PalSpawnPoolSubsystem` | 클래스별 비활성 팰 풀, 활성/반납 상태 전환 |
| `IPW_PooledSpawnActor` | 풀 활성/반납 이벤트를 필요한 팰 구현체에 선택적으로 전달 |
| `APWPalBase` | 스폰 가능한 팰 베이스 Actor |

## 구현 항목

1. 계획 파일 작성
2. 야생 팰 스폰 문서에서 Weather 조건 제거
3. `APWPalBase`를 Blueprintable로 열고 상속/AI TODO 주석 추가
4. `IPW_PooledSpawnActor` 추가
5. `UPW_PalSpawnPoolSubsystem` 추가
6. `UPW_WildPalSpawnSubsystem` 추가
7. `APW_WildPalSpawner`, `FPW_WildPalSpawnEntry`, `EPW_WildPalSpawnTimeRule` 추가
8. NavMesh 기반 위치 선택, 지역/전역 예산, 플레이어 거리 평가 연결

## 기본값

- 지역 최대 활성 팰: `8`
- 전체 월드 최대 활성 팰: `70`
- 풀 보관 수: `70`
- 스포너 평가 간격: `2초`
- 스폰 활성 거리: `8000uu`
- 풀 반납 거리: `10000uu`
- 플레이어 최소 스폰 거리: `1200uu`
- 스폰 위치 탐색 시도: `10회`

## 테스트 기준

- `PalWorldEditor` 빌드 성공
- PIE Listen Server + 클라이언트 2명 이상에서 검증
- 스폰 액터에서 `APWPalBase` 파생 팰 클래스를 선택할 수 있음
- 플레이어 A와 B가 서로 다른 스폰 지역에 있으면 두 지역 모두 서버에서 활성화됨
- 한 스폰 지역 활성 팰이 `8마리`를 넘지 않음
- 전체 월드 활성 팰이 `70마리`를 넘지 않음
- 모든 플레이어가 특정 스폰 지역에서 `100m` 밖으로 나가면 해당 지역 팰이 풀로 반납됨
- 다시 접근하면 풀에 있던 팰이 재사용됨
- 풀 반납된 팰은 보이지 않고, 충돌하지 않고, Tick 비용을 만들지 않음
- 같은 팰 근처의 여러 클라이언트가 동일한 Replicated Actor를 봄

## 제외 범위와 가정

- Weather는 원작 기준에서 제외한다.
- 팰 AI, 전투, 포획, 스탯, 이동 세부 동작은 스폰 시스템에서 구현하지 않는다.
- 개별 야생 팰 상태 저장은 v1에서 제외한다.
- DataAsset 기반 공유 스폰 테이블은 반복 설정이 실제로 필요해진 뒤 추가한다.
- Replication Graph는 v1에서 사용하지 않는다.
