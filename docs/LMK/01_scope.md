# LMK 담당 범위

이 문서는 LMK 파트가 우선 구현할 월드 시스템 범위를 정의한다.
아이템, 인벤토리, 크래프팅은 플레이어 파트 담당으로 본다.

## 구현 목표

플레이어 파트와 팰 파트가 같은 월드 상태, 스폰 규칙, 작업 대상 규칙을 공유하도록 서버 권한 기반 시스템을 제공한다.

## 담당 범위

| 시스템 | 책임 |
|---|---|
| World Time | 서버 기준 시간 흐름, 낮/밤 판정, 클라이언트 동기화 |
| Weather | 날씨 상태 관리, 온도와 스폰 조건에 영향 제공 |
| Temperature | 지역/시간/날씨 기반 온도 계산, 탐험 상태 태그 적용 요청 |
| Wild Pal Spawn | 야생 팰 스폰 포인트/존, 스폰 조건, 리스폰, 디스폰 |
| Work Building | 작업 건물의 요구 작업 태그, 작업 슬롯, 팰 배정 인터페이스 |
| World Bridge | 플레이어 파트/팰 파트와 월드 시스템 사이의 인터페이스 |
| Save Hooks | 월드 상태, 스폰 상태, 작업 건물 상태 저장용 데이터 제공 |

## 제외 범위

- Inventory 내부 구조
- Item DataAsset 전체 설계
- Crafting Recipe와 제작 UI
- Player Input 처리
- Pal AI 의사결정
- Pal 스탯, 스킬, 성장 구현
- Combat Damage와 상태 이상 세부 효과

## 소스 배치

모든 LMK C++ 구현은 아래 폴더 안에 둔다.

```text
PalWorld/Source/PalWorld/LMK/
├── Public/
└── Private/
```

필요하면 하위 폴더를 나눈다.

```text
World/
Weather/
Temperature/
Spawn/
Work/
Interfaces/
```

## 초기 성공 기준

- 서버 시간이 흐르고 낮/밤 상태가 클라이언트에 동기화된다.
- 온도 볼륨이 대상에게 `Status.Explo.Cold` 또는 `Status.Explo.Heat` 적용 요청을 보낸다.
- 야생 팰 스포너가 낮/밤 조건에 따라 서버에서 팰을 스폰한다.
- 작업 건물이 `Work.Mining` 또는 `Work.Lumbering` 요구 태그를 노출한다.
- 팰 파트가 인터페이스로 작업 슬롯 예약을 시도할 수 있다.
