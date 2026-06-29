# 플레이어 파트 채집 연동 인수인계서

이 문서는 공식 LMK 시스템 목차에 포함되는 설계 문서가 아니라, 플레이어 파트 개발자에게 전달하기 위한 인수인계 문서다.
목적은 LMK 채집 자원 시스템의 책임 경계를 설명하고, Third Person 테스트 구현을 정식 플레이어 채집 기능으로 옮길 때 필요한 연동 지점을 구체적으로 전달하는 것이다.

## 문서 성격

| 항목 | 내용 |
|---|---|
| 문서 유형 | 인수인계 |
| 대상 | 플레이어 파트 개발자 |
| 범위 | 플레이어 입력, Trace, 서버 RPC, LMK 자원 인터페이스 호출 방법 |
| 비범위 | LMK 자원 내부 구현 변경, 인벤토리 최종 설계, 팰 AI 작업 구현 |
| 참고 구현 | `UPW_HarvestDebugTraceComponent` |

이 문서는 구현 방향을 제안하지만, 플레이어 파트의 최종 클래스명과 입력 매핑은 플레이어 파트 컨벤션에 맞춰 조정해도 된다.

## 현재 구현 상태 요약

LMK 자원 시스템은 두 종류의 채집 대상을 제공한다.

| 대상 | 클래스 | 용도 |
|---|---|---|
| 단일 자원 Actor | `APW_HarvestableResourceActor`, `APW_HarvestableTree`, `APW_HarvestableRock` | 튜토리얼, 퀘스트, 특수 자원, 소수 테스트 자원 |
| Cluster 자원 | `APW_HarvestableResourceCluster` | 오픈월드의 일반 나무/돌/광석 대량 배치 |

플레이어 파트는 자원이 단일 Actor인지 Cluster인지 몰라도 된다.
라인트레이스 결과에 따라 아래 인터페이스 중 하나를 호출하면 된다.

| 상황 | 인터페이스 | 호출 함수 |
|---|---|---|
| Hit Actor가 단일 자원 | `IPW_HarvestDamageTarget` | `ApplyHarvestDamage(DamageAmount, InstigatorActor)` |
| Hit Actor가 Cluster이고 `HitResult.Item`이 유효 | `IPW_HarvestInstanceDamageTarget` | `ApplyHarvestDamageToInstance(InstanceIndex, DamageAmount, InstigatorActor)` |

현재 테스트용으로 `UPW_HarvestDebugTraceComponent`가 구현되어 있다.
이 컴포넌트는 정식 플레이어 시스템이 아니라, 플레이어 파트가 참고할 수 있는 최소 샘플이다.

## 플레이어가 가져가야 하는 핵심 책임

플레이어 파트는 아래 책임을 가진다.

1. 입력을 받는다.
2. 채집 가능한 상태인지 검사한다.
3. 시점 또는 무기 판정 기준으로 Trace를 수행한다.
4. Trace 결과가 채집 대상인지 검사한다.
5. 서버 RPC로 채집 요청을 보낸다.
6. 서버에서 다시 Trace 또는 검증을 수행한다.
7. 최종 채집 데미지를 계산한다.
8. LMK 자원 인터페이스를 호출한다.
9. 성공 시 플레이어 애니메이션, 사운드, UI를 갱신한다.
10. 실제 아이템 지급은 인벤토리 파트와 연결한다.

LMK 자원 시스템은 아래 책임만 가진다.

- 서버 권한으로 자원 HP 감소
- 자원 고갈 처리
- 자원 리스폰 처리
- 고갈/리스폰 상태 Replication
- Cluster 인스턴스 숨김/복구
- 임시 고갈 Actor 스폰

플레이어 입력, 장비, 스태미나, 인벤토리, 애니메이션 상태는 LMK 자원 시스템이 직접 처리하지 않는다.

## 현재 참고 코드

### 테스트 컴포넌트

위치:

```text
PalWorld/Source/PalWorld/LMK/Public/Resource/PW_HarvestDebugTraceComponent.h
PalWorld/Source/PalWorld/LMK/Private/Resource/PW_HarvestDebugTraceComponent.cpp
```

역할:

- BP에서 `TryHarvestForward()` 호출 가능
- 클라이언트가 호출하면 서버 RPC로 전달
- 서버가 Trace를 다시 수행
- 단일 자원 또는 Cluster 인스턴스 인터페이스 호출

현재 기본값:

```text
TraceDistance = 300
HarvestDamage = 25
TraceChannel = Visibility
```

이 컴포넌트는 실제 플레이어 기능으로 그대로 쓰기보다, 정식 컴포넌트 구현 시 참고용으로 사용하는 것이 좋다.

### 인터페이스 호출 예시

`UPW_HarvestDebugTraceComponent::ApplyHarvestDamageToHit()`의 현재 흐름은 아래와 같다.

```cpp
AActor* HitActor = HitResult.GetActor();
if (HitActor == nullptr)
{
    return false;
}

if (HitResult.Item != INDEX_NONE
    && HitActor->GetClass()->ImplementsInterface(UPW_HarvestInstanceDamageTarget::StaticClass()))
{
    return IPW_HarvestInstanceDamageTarget::Execute_ApplyHarvestDamageToInstance(
        HitActor,
        HitResult.Item,
        HarvestDamage,
        GetOwner());
}

if (!HitActor->GetClass()->ImplementsInterface(UPW_HarvestDamageTarget::StaticClass()))
{
    return false;
}

return IPW_HarvestDamageTarget::Execute_ApplyHarvestDamage(HitActor, HarvestDamage, GetOwner());
```

정식 플레이어 채집 코드도 이 판정 순서를 유지하는 것이 좋다.

Cluster 인스턴스는 `HitResult.Item`이 핵심이다.
HISM을 Trace로 맞추면 Unreal이 맞은 인스턴스 인덱스를 `HitResult.Item`에 넣어준다.

## 추천 플레이어 컴포넌트 구조

플레이어 파트에서 정식 구현을 할 때는 새 컴포넌트를 만드는 것을 권장한다.

예시 이름:

```text
UPWPlayerHarvestComponent
```

추천 위치:

```text
PalWorld/Source/PalWorld/PJH/Public/Player/Components/PWPlayerHarvestComponent.h
PalWorld/Source/PalWorld/PJH/Private/Player/Components/PWPlayerHarvestComponent.cpp
```

권장 책임:

| 함수 | 책임 |
|---|---|
| `TryStartHarvest()` | 로컬 입력 진입점 |
| `CanStartHarvest()` | 현재 액션 상태, 장비, 스태미나 검사 |
| `BuildHarvestTrace()` | Trace 시작점/방향/거리 계산 |
| `ServerRequestHarvest()` | 클라이언트 요청을 서버로 전달 |
| `ValidateHarvestRequest()` | 서버에서 거리, 방향, 대상 유효성 검사 |
| `TraceHarvestTarget()` | 서버 Trace 수행 |
| `CalculateHarvestDamage()` | 도구/스탯/자원 타입 기반 데미지 계산 |
| `ApplyHarvestDamageToHit()` | LMK 자원 인터페이스 호출 |
| `HandleHarvestSuccess()` | 애니메이션, 사운드, UI, 인벤토리 hook |

정식 구현 시 `UPW_HarvestDebugTraceComponent`를 플레이어 캐릭터에 계속 붙이는 것은 추천하지 않는다.
디버그 컴포넌트는 테스트용으로 남겨두고, 실제 플레이어 기능은 플레이어 파트 컴포넌트에 둔다.

## 네트워크 흐름

채집은 반드시 서버 권한으로 처리한다.
클라이언트가 직접 자원 HP를 줄이면 안 된다.

권장 흐름:

```text
Local Player Input
    ↓
TryStartHarvest()
    ↓
로컬 CanStartHarvest()
    ↓
TraceStart / TraceDirection 계산
    ↓
ServerRequestHarvest(TraceStart, TraceDirection)
    ↓
서버 CanStartHarvest()
    ↓
서버 Trace
    ↓
Hit 대상 검증
    ↓
채집 데미지 계산
    ↓
LMK Harvest Interface 호출
    ↓
자원 시스템이 HP/고갈/리스폰 처리
    ↓
자원 상태 Replication
```

Listen Server에서는 서버 플레이어가 로컬에서 바로 서버 권한을 가진다.
그래도 코드 구조는 클라이언트와 같은 경로를 타게 만드는 것이 좋다.

Dedicated Server 확장을 위해 아래 원칙을 지킨다.

- 서버는 플레이어 카메라 컴포넌트 렌더링에 의존하지 않는다.
- 클라이언트가 넘긴 `TraceStart`, `TraceDirection`은 요청값으로만 사용한다.
- 서버는 반드시 거리, 방향, 대상 유효성을 다시 검사한다.
- 자원 HP 감소는 서버에서만 호출한다.
- 클라이언트는 애니메이션과 UI 예측은 할 수 있지만, 자원 상태 확정은 서버 복제를 따른다.

## 서버 검증 기준

서버 RPC에서 최소한 아래 검증을 해야 한다.

| 검증 | 이유 |
|---|---|
| Owner Pawn 유효성 | 죽은 Actor나 잘못된 소유자 요청 방지 |
| Controller 유효성 | 실제 플레이어 입력인지 확인 |
| Action 상태 | 구르기, 공격, 스킬 중 채집 시작 방지 |
| Trace 거리 제한 | 클라이언트가 먼 거리 좌표를 보내는 것 방지 |
| Trace 방향 제한 | 뒤쪽/말도 안 되는 방향 요청 방지 |
| 대상 인터페이스 확인 | 채집 대상이 아닌 Actor 타격 방지 |
| Cluster InstanceIndex 확인 | HISM 인스턴스 범위 밖 접근 방지 |
| 서버 Trace 재수행 | 클라이언트 HitResult를 신뢰하지 않기 |

현재 디버그 컴포넌트는 테스트용이라 이 검증이 최소화되어 있다.
정식 플레이어 구현에서는 반드시 보강해야 한다.

## Trace 기준

현재 디버그 컴포넌트는 Controller view 기준으로 Trace한다.

```text
Start = Controller->GetPlayerViewPoint() 위치
Direction = Controller->GetPlayerViewPoint() 회전의 Forward Vector
End = Start + Direction * TraceDistance
Channel = Visibility
```

정식 구현에서는 게임 기획에 따라 기준을 정해야 한다.

선택지:

| 방식 | 장점 | 단점 |
|---|---|---|
| 카메라 기준 Trace | 조준한 대상이 잘 맞음 | 근접 무기 감각과 다를 수 있음 |
| 캐릭터 전방 Trace | 액션 게임 근접 판정에 가까움 | 카메라로 보고 있는 대상과 어긋날 수 있음 |
| 무기 소켓 기준 Sweep | 도끼/곡괭이 타격감 좋음 | 애니메이션/무기 소켓 연동 필요 |

1차 정식 구현 추천:

```text
카메라 기준 LineTrace로 대상 선택
캐릭터와 대상 거리로 근접 가능 여부 검증
```

이후 애니메이션 타격 프레임이 준비되면 무기 소켓 Sweep으로 확장한다.

## 자원 대상 판정

정식 플레이어 코드에서 채집 대상 판정은 아래 순서를 따른다.

1. Trace로 `FHitResult` 획득
2. `HitActor == nullptr`이면 실패
3. `HitResult.Item != INDEX_NONE`이고 `HitActor`가 `UPW_HarvestInstanceDamageTarget` 구현이면 Cluster 자원으로 처리
4. 아니면 `HitActor`가 `UPW_HarvestDamageTarget` 구현인지 확인
5. 둘 다 아니면 채집 실패

Cluster 판정을 먼저 해야 한다.
HISM Cluster Actor가 단일 자원 인터페이스를 추가로 구현하게 되더라도, 인스턴스 인덱스가 있는 경우에는 인스턴스 데미지가 우선이어야 한다.

## 데미지 계산

LMK 자원 시스템은 `DamageAmount`만 받는다.
도끼, 곡괭이, 플레이어 스탯, 팰 작업력 같은 값은 호출자가 계산한다.

권장 계산 흐름:

```text
BaseHarvestDamage
    * ToolMultiplier
    * ResourceAffinityMultiplier
    * PlayerStatMultiplier
    * BuffDebuffMultiplier
```

예시:

```text
맨손으로 나무 타격: 5
돌도끼로 나무 타격: 25
돌도끼로 돌 타격: 8
곡괭이로 돌 타격: 30
```

자원 타입 판정은 현재 다음 정보로 가능하다.

- 단일 자원: `UPW_HarvestableResourceComponent::GetRequiredWorkTag()`
- Cluster 자원: `UPW_HarvestableResourceClusterComponent::GetRequiredWorkTag()`

플레이어 파트가 자원 컴포넌트에 직접 의존하고 싶지 않다면, 나중에 별도 조회 인터페이스를 추가할 수 있다.
1차에서는 필요한 최소 범위로 직접 컴포넌트 조회를 해도 된다.

## Action 상태 연동

현재 `UPWPlayerActionComponent`에는 `EPWPlayerActionState::Gathering` 값이 이미 있다.
정식 채집 구현은 이 상태를 사용하는 것이 좋다.

권장 흐름:

```text
TryStartHarvest()
    ↓
ActionComponent->IsBusy() 확인
    ↓
서버에서 CurrentActionState = Gathering
    ↓
채집 몽타주 재생
    ↓
타격 프레임에 실제 HarvestDamage 적용
    ↓
몽타주 종료 또는 타이머 종료
    ↓
CurrentActionState = None
```

주의:

- 입력 시점에 바로 데미지를 넣을지, 애니메이션 Notify 시점에 넣을지 결정해야 한다.
- 1차는 입력 즉시 데미지 적용으로 시작할 수 있다.
- 정식 UX는 애니메이션 타격 프레임 Notify에서 데미지를 적용하는 것이 자연스럽다.

## 인벤토리 연동

현재 LMK 자원 시스템은 실제 아이템 지급을 하지 않는다.
현재는 보상 이름과 보상 수량을 이벤트/로그로만 제공한다.

단일 자원:

```text
UPW_HarvestableResourceComponent
- RewardName
- RewardAmount
- OnHarvested
```

Cluster 자원:

```text
UPW_HarvestableResourceClusterComponent
- RewardName
- RewardAmount
- OnInstanceDamaged
```

정식 인벤토리 연동 방식은 플레이어 파트/인벤토리 파트가 결정한다.

권장 방식:

1. 서버에서 채집 데미지 적용 성공
2. 자원 시스템이 성공 여부 반환
3. 플레이어 HarvestComponent가 자원 보상 정보를 조회
4. 서버 인벤토리 컴포넌트에 아이템 추가 요청
5. 인벤토리 Replication으로 UI 갱신

주의:

- 클라이언트 UI에서 먼저 아이템을 넣지 않는다.
- 서버 인벤토리 확정 전에 UI 예측을 한다면, 실패 시 롤백이 필요하다.
- `RewardName`은 임시 식별자다. 아이템 DataAsset이 생기면 `ItemId` 또는 `PrimaryAssetId`로 바꾸는 것이 좋다.

## 플레이어 정식 구현 예시 의사코드

아래는 실제 구현 방향을 보여주는 의사코드다.

```cpp
void UPWPlayerHarvestComponent::TryStartHarvest()
{
    if (!CanStartHarvest())
    {
        return;
    }

    FVector TraceStart;
    FVector TraceDirection;
    BuildHarvestTrace(TraceStart, TraceDirection);

    if (AActor* Owner = GetOwner())
    {
        if (!Owner->HasAuthority())
        {
            ServerRequestHarvest(TraceStart, TraceDirection);
            return;
        }
    }

    HandleHarvestRequestAuthority(TraceStart, TraceDirection);
}
```

```cpp
void UPWPlayerHarvestComponent::ServerRequestHarvest_Implementation(
    FVector_NetQuantize TraceStart,
    FVector_NetQuantizeNormal TraceDirection)
{
    if (!ValidateHarvestRequest(TraceStart, FVector(TraceDirection)))
    {
        return;
    }

    HandleHarvestRequestAuthority(TraceStart, FVector(TraceDirection));
}
```

```cpp
bool UPWPlayerHarvestComponent::ApplyHarvestDamageToHit(const FHitResult& HitResult)
{
    AActor* HitActor = HitResult.GetActor();
    if (!HitActor)
    {
        return false;
    }

    const float DamageAmount = CalculateHarvestDamage(HitResult);

    if (HitResult.Item != INDEX_NONE
        && HitActor->GetClass()->ImplementsInterface(UPW_HarvestInstanceDamageTarget::StaticClass()))
    {
        return IPW_HarvestInstanceDamageTarget::Execute_ApplyHarvestDamageToInstance(
            HitActor,
            HitResult.Item,
            DamageAmount,
            GetOwner());
    }

    if (HitActor->GetClass()->ImplementsInterface(UPW_HarvestDamageTarget::StaticClass()))
    {
        return IPW_HarvestDamageTarget::Execute_ApplyHarvestDamage(
            HitActor,
            DamageAmount,
            GetOwner());
    }

    return false;
}
```

## BP 테스트 방식

정식 플레이어 구현 전까지는 아래 방식으로 테스트할 수 있다.

1. `BP_ThirdPersonCharacter`에 `PW_HarvestDebugTraceComponent` 추가
2. 키 입력 예: `E`
3. `E Pressed -> TryHarvestForward()` 연결
4. 자원 BP 또는 Cluster BP 배치
5. PIE Listen Server 2인 테스트

이 방식은 기능 검증용이다.
정식 플레이어 컴포넌트가 생기면 디버그 컴포넌트 의존을 제거한다.

## 자원 BP 설정 참고

### 단일 자원

나무:

```text
Parent Class: PW_HarvestableTree
ResourceMesh: 나무 메시
ResourceComponent.MaxHealth: 100
ResourceComponent.RespawnDelay: 30
```

돌:

```text
Parent Class: PW_HarvestableRock
ResourceMesh: 돌 메시
ResourceComponent.MaxHealth: 150
ResourceComponent.RespawnDelay: 45
```

### Cluster 자원

나무 Cluster:

```text
Parent Class: PW_HarvestableResourceCluster
HarvestInstances.StaticMesh: 나무 메시
ClusterComponent.MaxHealth: 100
ClusterComponent.RespawnDelay: 30
ClusterComponent.RequiredWorkTag: Work.Lumbering
ClusterComponent.RewardName: Wood
ClusterComponent.RewardAmount: 1
InstanceTransforms: 나무 위치 배열
DepletionActorClass: BP_FallingTree 또는 None
```

돌 Cluster:

```text
Parent Class: PW_HarvestableResourceCluster
HarvestInstances.StaticMesh: 돌 메시
ClusterComponent.MaxHealth: 150
ClusterComponent.RespawnDelay: 45
ClusterComponent.RequiredWorkTag: Work.Mining
ClusterComponent.RewardName: Stone
ClusterComponent.RewardAmount: 1
InstanceTransforms: 돌 위치 배열
DepletionActorClass: BP_BrokenRock 또는 None
```

## 테스트 체크리스트

플레이어 파트가 연동 후 반드시 확인해야 할 항목:

- Listen Server 2인에서 서버 플레이어가 단일 자원을 채집할 수 있다.
- Listen Server 2인에서 클라이언트 플레이어가 단일 자원을 채집할 수 있다.
- Listen Server 2인에서 서버 플레이어가 Cluster 인스턴스를 채집할 수 있다.
- Listen Server 2인에서 클라이언트 플레이어가 Cluster 인스턴스를 채집할 수 있다.
- 클라이언트가 채집한 자원이 서버와 다른 클라이언트에게도 사라진다.
- Cluster에서 맞은 인스턴스 하나만 사라진다.
- `RespawnDelay` 후 같은 인스턴스가 다시 나타난다.
- 임시 고갈 Actor가 모든 플레이어에게 보인다.
- 채집 대상이 아닌 Actor를 때렸을 때 아무 상태도 바뀌지 않는다.
- 너무 먼 거리의 요청이 서버에서 실패한다.
- 구르기, 스킬, 공격 중 채집 입력이 막힌다.
- 채집 성공 시 인벤토리 서버 지급이 한 번만 발생한다.
- 패킷 지연 상황에서도 자원 상태는 서버 복제를 따른다.

## 플레이어 파트에서 건드리지 말아야 할 것

아래는 플레이어 파트가 직접 수정하지 않는 것을 권장한다.

- 자원 Actor의 `CurrentHealth`
- Cluster의 `DepletedInstanceIndices`
- HISM 인스턴스 숨김/복구
- 자원 리스폰 타이머
- 임시 고갈 Actor 스폰

이 값들은 서버 권한 LMK 자원 시스템이 관리한다.
플레이어 파트는 채집 요청과 보상 수령만 담당한다.

## 향후 협업 지점

추가 설계가 필요한 지점:

- 아이템 DataAsset 식별자 확정
- 도구 타입과 자원 타입 매칭 규칙
- 애니메이션 Notify 기반 데미지 적용 시점
- 스태미나 소모와 실패 처리
- 팰 작업 AI가 같은 자원 인터페이스를 사용할 때의 예약 규칙
- 저장/로드 시 고갈 자원 상태 유지

이 항목들은 플레이어, 인벤토리, 팰 AI, LMK 자원 시스템 간의 경계가 있으므로 별도 문서나 이슈로 분리하는 것이 좋다.
