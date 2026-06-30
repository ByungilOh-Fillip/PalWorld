# 2026-06-29_harvestable_resources_v1 구현 계획

## 구현 목표

첫 구현 범위는 `HP 기반 나무/돌 자원 + Third Person BP 테스트 진입점`이다.
자원은 서버 권한으로 HP가 감소하고, 고갈/리스폰 상태를 모든 클라이언트에 복제한다.

## 서버-클라이언트 통신 구조

```text
Client BP input
    ↓
Harvest debug component local trace start/direction
    ↓
Harvest debug component Server RPC
    ↓
Server trace + IPW_HarvestDamageTarget call
    ↓
Harvestable resource component state change
    ↓
Replication / OnRep
    ↓
Client actor visibility and collision update
```

- 클라이언트는 직접 자원 HP를 변경하지 않고, 조준 트레이스 시작점/방향만 서버에 요청값으로 보낸다.
- Listen Server 기준으로 먼저 검증하되 Dedicated Server에서도 입력과 권한 로직이 분리되도록 한다.
- 자원 Actor는 표시와 충돌을 담당하고, HP/고갈/리스폰 규칙은 컴포넌트가 담당한다.

## 책임 분리

| 위치 | 책임 |
|---|---|
| `IPW_HarvestDamageTarget` | 자원 데미지 대상 최소 인터페이스 |
| `APW_HarvestableResourceActor` | 레벨 배치 단위, 메시/충돌 상태 반영 |
| `UPW_HarvestableResourceComponent` | HP, 고갈, 리스폰, 보상 이벤트, Replication |
| `APW_HarvestableTree` | 나무 기본값 |
| `APW_HarvestableRock` | 돌 기본값 |
| `UPW_HarvestDebugTraceComponent` | Third Person BP 테스트용 서버 트레이스 요청 |

## 구현 항목

1. `Work.Lumbering`, `Work.Mining` Native GameplayTag 등록
2. `IPW_HarvestDamageTarget` 추가
3. `UPW_HarvestableResourceComponent` 추가
4. `APW_HarvestableResourceActor` 추가
5. `APW_HarvestableTree`, `APW_HarvestableRock` 추가
6. `UPW_HarvestDebugTraceComponent` 추가
7. BP 연결용 `TryHarvestForward()` BlueprintCallable 제공

## 테스트 기준

- `PalWorldEditor` 빌드 성공
- Tree/Rock Actor를 레벨에 배치 가능
- Third Person BP에 `PW_HarvestDebugTraceComponent`를 수동 추가하고 키 입력에서 `TryHarvestForward()` 호출 가능
- 클라이언트 호출도 서버 RPC를 통해서만 HP 감소
- 한 플레이어가 자원을 고갈시키면 모든 플레이어에게 숨김/충돌 해제가 동기화
- `RespawnDelay` 이후 모든 플레이어에게 표시/충돌 복구가 동기화
- 로그 또는 Blueprint delegate로 `Wood`/`Stone` 보상 이름과 수량 확인

## 제외 범위와 가정

- Third Person BP 자산은 자동 수정하지 않는다.
- 인벤토리 지급, 아이템 DataAsset, 정식 플레이어 채집 입력, 팰 작업 AI는 제외한다.
- 파괴 애니메이션, Niagara, 사운드는 구현하지 않고 이벤트 훅만 제공한다.
- 기존 레벨, 솔루션, PJH 플레이어 정식 컴포넌트는 수정하지 않는다.
