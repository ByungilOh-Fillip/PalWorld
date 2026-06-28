# Work Building 시스템

목표는 작업 건물이 요구 작업 태그를 제공하고 팰 파트가 이를 기준으로 작업을 배정할 수 있게 하는 것이다.

## 추천 클래스

- `APW_WorkBuildingBase`
- `UPW_WorkBuildingComponent`
- `UPW_WorkTargetSubsystem`
- `FPW_WorkSlot`
- `FPW_WorkRequirement`

## 책임 분리

| 위치 | 책임 |
|---|---|
| `UPW_WorkTargetSubsystem` | 월드 내 작업 대상 등록/검색 |
| `APW_WorkBuildingBase` | 건물 위치, Replication, 상호작용 진입점 |
| `UPW_WorkBuildingComponent` | 요구 태그, 작업 슬롯, 진행률 |
| Pal Part | 작업 대상 선택과 AI 의사결정 |

## 구현 항목

- [ ] 건물별 요구 작업 태그 설정
- [ ] 작업 건물을 `UPW_WorkTargetSubsystem`에 등록
- [ ] 작업 슬롯 수 설정
- [ ] 슬롯 점유 상태 관리
- [ ] 작업 가능 거리 또는 반경 설정
- [ ] 작업 진행률 관리
- [ ] 서버 권한으로 작업 슬롯 예약
- [ ] 서버 권한으로 작업 시작
- [ ] 서버 권한으로 작업 중단
- [ ] 서버 권한으로 작업 완료
- [ ] 작업 수행자의 `Work.*` 태그 보유 여부 검사
- [ ] 작업 수행자의 건강/상태 태그 검사 hook
- [ ] 작업 속도 보정 hook
- [ ] 작업 결과 전달 hook
- [ ] 작업 상태 Replication

## 1차 건물 후보

| 건물 유형 | 요구 태그 | 1차 목표 |
|---|---|---|
| 벌목장 | `Work.Lumbering` | 팰이 작업 슬롯을 예약하고 진행률을 채움 |
| 채석장 | `Work.Mining` | 벌목장과 같은 구조로 다른 태그 검증 |
| 발전기 | `Work.GeneratingElectricity` | 작업 중일 때 건물 활성 상태 유지 |
| 냉장고 | `Work.Cooling` | 작업 중일 때 온도 보정 hook 검증 |

## 작업 흐름

```text
Pal AI -> WorkTarget 검색
WorkBuilding -> 요구 Work 태그 제공
Pal AI -> 작업 가능 여부 판단
Pal AI -> 슬롯 예약 요청
WorkBuilding -> 서버에서 예약/진행/완료 처리
```

## 완료 기준

- [ ] 팰 파트가 건물의 요구 작업 태그를 조회할 수 있음
- [ ] 작업 건물이 팰 구체 클래스에 직접 의존하지 않음
- [ ] 슬롯 예약/해제가 서버 기준으로 일관되게 처리됨
- [ ] 작업 진행 상태가 클라이언트에 동기화됨
- [ ] 최소 2개 이상의 서로 다른 `Work.*` 태그 건물이 같은 기반 구조를 사용함
