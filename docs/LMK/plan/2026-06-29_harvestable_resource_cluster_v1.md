# 2026-06-29_harvestable_resource_cluster_v1 구현 계획

## 구현 목표

첫 구현 범위는 `HISM Cluster + 임시 고갈 Actor 기반 오픈월드 자원`이다.
기존 단일 자원 Actor는 유지하고, 대량 배치 자원은 Cluster Actor가 인스턴스 단위로 HP/고갈/리스폰을 관리한다.

## 서버-클라이언트 통신 구조

```text
Client BP input
    ↓
Harvest debug component local trace start/direction
    ↓
Harvest debug component Server RPC
    ↓
Server trace
    ↓
Single resource or cluster instance damage interface
    ↓
Server cluster component state change
    ↓
Depleted instance index replication
    ↓
Client HISM instance hide/show
```

- 클라이언트는 직접 자원 상태를 변경하지 않는다.
- 서버가 Cluster와 InstanceIndex를 판정하고 HP, 고갈, 리스폰을 처리한다.
- 클라이언트에는 고갈/리스폰 인스턴스 상태만 복제한다.

## 책임 분리

| 위치 | 책임 |
|---|---|
| `IPW_HarvestDamageTarget` | 단일 자원 데미지 인터페이스 |
| `IPW_HarvestInstanceDamageTarget` | Cluster 인스턴스 데미지 인터페이스 |
| `APW_HarvestableResourceCluster` | HISM 인스턴스 표시/숨김, 임시 Actor 스폰 |
| `UPW_HarvestableResourceClusterComponent` | 인스턴스별 HP, 고갈 상태, 리스폰 타이머, 상태 복제 |
| `UPW_HarvestDebugTraceComponent` | 서버 트레이스 후 단일/Cluster 대상 호출 |

## 구현 항목

1. `IPW_HarvestInstanceDamageTarget` 추가
2. `UPW_HarvestableResourceClusterComponent` 추가
3. `APW_HarvestableResourceCluster` 추가
4. 수동 배치용 `InstanceTransforms` 배열과 HISM 생성 흐름 추가
5. 고갈 시 HISM 인스턴스 숨김과 임시 Actor 스폰 추가
6. 리스폰 시 HISM 인스턴스 복구 추가
7. `UPW_HarvestDebugTraceComponent`가 Cluster 인스턴스 타격을 호출하도록 확장

## 테스트 기준

- `PalWorldEditor` 빌드 성공
- Cluster BP에 Static Mesh와 20~100개 Transform을 설정해 인스턴스 생성 가능
- Listen Server 2인 PIE에서 서버/클라이언트 모두 Cluster 인스턴스를 채집 가능
- 한 인스턴스가 고갈되면 모든 플레이어에게 해당 인스턴스만 숨김
- 고갈 위치에 임시 Actor가 서버에서 스폰되어 모든 플레이어에게 복제
- 임시 Actor는 수명 종료 후 제거
- `RespawnDelay` 후 해당 인스턴스가 모든 플레이어에게 다시 표시
- 기존 `APW_HarvestableTree`, `APW_HarvestableRock` 단일 자원 채집 동작 유지

## 제외 범위와 가정

- PCG/Foliage 자동 연동은 제외하고 Cluster BP 수동 배치로 검증한다.
- 중간 HP 복제는 제외하고 고갈/리스폰 상태만 복제한다.
- 임시 Actor의 실제 애니메이션, Niagara, 사운드는 BP가 담당한다.
- 인벤토리 지급, 저장/로드, 팰 AI 작업 대상 검색은 제외한다.
