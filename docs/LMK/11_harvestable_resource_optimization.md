# 채집 자원 오픈월드 최적화 기준

이 문서는 2km x 2km 오픈월드에서 나무/돌 같은 채집 자원을 배치할 때의 서버, 렌더링, Replication 부하 기준을 정리한다.

## 기본 판단

채집 가능한 자원은 두 방식으로 나눈다.

| 방식 | 사용처 | 특징 |
|---|---|---|
| 단일 Actor 자원 | 특수 자원, 튜토리얼, 퀘스트 자원, 소수 테스트 자원 | 구현과 연출이 쉽지만 대량 배치에 부적합 |
| HISM Cluster 자원 | 일반 필드 나무, 돌, 광석 같은 대량 자원 | Actor 수와 Replication 대상을 줄여 오픈월드에 적합 |

일반 숲이나 돌 무더기를 `APW_HarvestableTree`, `APW_HarvestableRock` Actor로 수천 개 배치하지 않는다.
대량 자원은 `APW_HarvestableResourceCluster`를 사용한다.

## Actor 방식의 부하

단일 Actor 방식은 자원 하나가 Actor 하나다.

```text
나무 5,000개
    = Actor 5,000개
    = StaticMeshComponent 5,000개
    = Replication 후보 5,000개
    = BeginPlay / 컴포넌트 등록 / 충돌 관리 5,000개
```

이 방식은 개별 BP 이벤트, 애니메이션, 이펙트, 퀘스트 연동이 쉽다.
하지만 오픈월드 대량 배치에서는 서버와 에디터, 월드 파티션, 네트워크 복제 후보 수가 모두 늘어난다.

따라서 단일 Actor 자원은 아래 경우에만 사용한다.

- 퀘스트용 큰 나무나 특수 광맥
- 튜토리얼 또는 시작 지점의 소수 자원
- 개별 Blueprint 이벤트와 연출이 중요한 자원
- 디버그와 기능 검증용 자원

## Cluster 방식의 부하

Cluster 방식은 같은 종류의 자원을 HISM 인스턴스로 묶는다.

```text
나무 5,000개
    = Cluster Actor 50개
    = HISM Component 50개
    = Instance Transform 5,000개
    = Replication 후보 50개
```

HISM 인스턴스는 Actor가 아니다.
개별 인스턴스는 컴포넌트 내부의 Transform 데이터에 가깝기 때문에 Actor보다 훨씬 가볍다.

Cluster 방식의 핵심은 아래와 같다.

- 렌더링은 HISM이 같은 메시를 묶어서 처리한다.
- 서버는 Cluster Actor 단위로만 Replication 후보를 관리한다.
- 자원 하나가 고갈되어도 Actor 하나를 복제하지 않고 인스턴스 인덱스만 상태로 관리한다.
- 고갈/리스폰처럼 실제로 바뀐 인스턴스 상태만 클라이언트에 동기화한다.

## 서버 권한과 Replication 기준

채집 판정과 자원 상태 변경은 서버에서만 처리한다.

```text
Client 입력
    ↓
Server RPC
    ↓
Server Trace
    ↓
Cluster + InstanceIndex 판정
    ↓
Server HP 감소
    ↓
고갈 시 DepletedInstanceIndices 복제
    ↓
Client HISM 인스턴스 숨김/복구
```

중간 HP는 v1에서 복제하지 않는다.
모든 인스턴스의 HP를 계속 복제하면 네트워크 부하가 자원 수에 비례해서 증가한다.

v1에서 복제하는 값은 고갈된 인스턴스 인덱스 목록이다.

```text
Cluster_05
    DepletedInstanceIndices = [12, 37, 88]
```

클라이언트는 이 목록을 받아 해당 인스턴스만 숨긴다.
리스폰되면 서버가 목록에서 인덱스를 제거하고, 클라이언트는 해당 인스턴스를 다시 표시한다.

## 임시 고갈 Actor

HISM 인스턴스에는 개별 Blueprint 이벤트나 Timeline이 없다.
따라서 나무 쓰러짐, 돌 부서짐 같은 개별 연출은 자동으로 실행되지 않는다.

고갈 연출은 임시 Actor 방식으로 처리한다.

```text
평소
    HISM Instance

HP 0
    Instance 숨김
    같은 위치에 DepletionActorClass 서버 스폰

연출 완료
    임시 Actor LifeSpan 종료

RespawnDelay 이후
    HISM Instance 다시 표시
```

임시 Actor는 서버에서 스폰하고 Replication을 켠다.
그러면 Listen Server와 클라이언트, Dedicated Server 클라이언트 모두 같은 고갈 연출을 본다.

임시 Actor BP는 아래 연출을 담당한다.

- 나무 쓰러짐 애니메이션
- 돌 부서짐 메시
- Niagara 이펙트
- 사운드
- 짧은 물리 연출

Cluster C++는 특정 이펙트나 사운드 에셋에 직접 의존하지 않는다.

## Cluster 크기 기준

Cluster는 너무 작아도, 너무 커도 좋지 않다.

나쁜 예:

```text
Cluster 1개당 자원 3개
Cluster 1,000개
```

이 경우 Actor 수가 많아져 Cluster의 장점이 줄어든다.

나쁜 예:

```text
월드 전체 나무 5,000개를 Cluster 1개에 몰아넣기
```

이 경우 월드 파티션, 컬링, 상태 관리 단위가 너무 커진다.

권장 기준:

```text
Cluster 하나 = 같은 종류 자원 50~200개
Cluster 크기 = 약 50m~150m 구역
자원 타입별 분리 = TreeCluster, RockCluster, OreCluster
```

2km x 2km 월드에서는 자원이 있는 구역에만 Cluster를 배치한다.
모든 격자에 강제로 Cluster를 두지 않는다.

예시:

```text
Forest_01_TreeCluster
Forest_01_RockCluster
Forest_02_TreeCluster
Hill_01_RockCluster
```

## 플레이어 경험 기준

플레이어 입장에서는 단일 Actor 방식과 Cluster 방식의 차이가 없어야 한다.

동일하게 보여야 하는 것:

- 때리면 자원이 닳는다.
- 다 캐면 해당 나무/돌만 사라진다.
- 고갈 연출이 모든 플레이어에게 보인다.
- 시간이 지나면 같은 위치에 리스폰된다.
- 다른 플레이어도 같은 상태를 본다.

내부 구현만 다르다.

```text
단일 Actor 자원
    자원 하나가 직접 HP와 표시 상태를 가짐

Cluster 자원
    Cluster가 InstanceIndex별 HP와 표시 상태를 관리함
```

플레이어/팰 파트는 구체 구현을 몰라도 된다.
채집 대상 인터페이스만 호출하고, LMK 자원 시스템이 단일 Actor인지 Cluster인지에 맞게 처리한다.

## 향후 확장

v1 이후 필요해질 수 있는 확장은 아래와 같다.

- Foliage 또는 PCG 배치를 Cluster 인스턴스로 변환
- 저장/로드 시 고갈 인스턴스와 리스폰 남은 시간 저장
- 팰 작업 AI가 Cluster 인스턴스를 작업 대상으로 선택
- 중간 HP를 UI나 머티리얼에 표시
- Instance Custom Data로 균열, 색 변화, 흔들림 표현
- Cluster 배치용 에디터 유틸리티 추가

이 확장들은 기본 Cluster 채집, 고갈, 리스폰, Replication이 안정화된 뒤 진행한다.

## 권장 개선 방향

현재 `APW_HarvestableResourceCluster`는 런타임 구조 검증용으로 적합하다.
하지만 `InstanceTransforms`를 사람이 직접 입력하는 방식은 최종 제작 워크플로우로 적합하지 않다.

최종 권장 방향은 아래와 같다.

```text
디자이너 배치
    Foliage 또는 PCG로 나무/돌 배치
    ↓
에디터 변환 도구
    배치된 인스턴스 Transform 추출
    ↓
Cluster 생성
    자원 타입별 `APW_HarvestableResourceCluster` 생성
    ↓
런타임
    Cluster가 HP, 고갈, 리스폰, Replication 관리
```

중요한 점은 Foliage를 런타임 채집 대상으로 직접 쓰는 것이 아니라, Foliage/PCG 배치 데이터를 Cluster 데이터로 변환해 사용하는 것이다.

### Foliage 직접 채집을 우선하지 않는 이유

Foliage Mode로 칠한 인스턴스는 `APW_HarvestableResourceCluster`가 아니며, 현재 채집 인터페이스도 구현하지 않는다.
따라서 현재 구조에서는 Foliage 브러쉬로 배치한 나무/돌이 자동으로 채집 대상이 되지 않는다.

Foliage를 런타임 채집 대상으로 직접 사용하려면 아래 문제가 생긴다.

- `InstancedFoliageActor` 내부 구조에 의존해야 한다.
- 자원별 HP, 고갈, 리스폰 상태를 별도 Manager가 추적해야 한다.
- 네트워크 복제와 저장/로드 기준을 Foliage 구조 위에 새로 얹어야 한다.
- 임시 고갈 Actor 연출, 인스턴스 숨김, 리스폰 복구 흐름이 복잡해진다.
- 엔진 버전이나 Foliage 내부 변경에 영향을 받을 가능성이 커진다.

따라서 우선순위는 Foliage 직접 채집이 아니라 `Foliage/PCG -> Cluster 변환`이다.

### 단계별 개선 로드맵

1. Cluster 수동 배치로 런타임 동작을 안정화한다.
   - 서버 권한 채집
   - 고갈 인스턴스 숨김
   - 임시 고갈 Actor 스폰
   - 리스폰
   - Listen Server 2인 동기화

2. Cluster 배치 보조 기능을 만든다.
   - 선택한 Actor들의 Transform을 `InstanceTransforms`로 복사
   - 선택한 StaticMeshActor들을 Cluster 인스턴스로 변환
   - 자원 타입별 Cluster를 자동 생성

3. Foliage 변환 도구를 만든다.
   - 특정 FoliageType 또는 Static Mesh 기준으로 인스턴스 Transform 추출
   - TreeCluster, RockCluster 등 자원 타입별 Cluster 생성
   - 원본 Foliage를 제거하거나 비채집 배경 Foliage로 유지할지 선택

4. PCG 변환 또는 생성 파이프라인을 검토한다.
   - Biome, 경사, 고도, 노이즈 기반 후보 위치 생성
   - 생성 결과를 Cluster 인스턴스로 저장
   - 런타임에는 PCG가 아니라 Cluster가 채집 상태를 관리

5. 저장/로드와 팰 AI 연동을 붙인다.
   - `ClusterId + InstanceIndex` 기준으로 고갈 상태 저장
   - 리스폰 남은 시간 저장
   - 팰 작업 AI가 Cluster 인스턴스를 작업 대상으로 예약

### 최종 목표 구조

```text
일반 필드 자원
    배치: Foliage/PCG
    런타임: HarvestableResourceCluster

특수 자원
    배치: 개별 HarvestableResourceActor
    런타임: 개별 Actor HP/리스폰

플레이어/팰
    자원 구현체를 직접 알지 않고 Harvest 인터페이스만 호출
```

이 구조를 유지하면 제작자는 Foliage/PCG처럼 편하게 배치하고, 런타임은 서버 권한 Cluster 시스템으로 통제할 수 있다.
