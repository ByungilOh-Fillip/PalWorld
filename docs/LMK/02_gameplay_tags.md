# GameplayTag 사용 기준

공유 GameplayTag 문서를 Enum 대체 기준으로 사용한다.
LMK 시스템은 태그를 판정 기준으로 사용하고, 태그 효과 자체는 각 담당 시스템에 위임한다.

## 작업 적성 태그

작업 건물은 `Work.*` 태그로 요구 작업을 표현한다.

| 태그 | LMK 사용처 |
|---|---|
| `Work.Kindling` | 화로, 조리대, 온열 장치 |
| `Work.Watering` | 농장 물 주기, 제분기, 물 공급 장치 |
| `Work.Planting` | 농장 파종 건물 |
| `Work.GeneratingElectricity` | 발전기, 전력 공급 건물 |
| `Work.Handiwork` | 수작업 제작대, 수리대 |
| `Work.Gathering` | 농작물 수확 지점 |
| `Work.Lumbering` | 벌목장, 나무 자원 |
| `Work.Mining` | 채석장, 광석 자원 |
| `Work.MedicineProduction` | 제약대 |
| `Work.Cooling` | 냉장고, 냉각 장치 |
| `Work.Transporting` | 운반 목적지, 보관함 연결 |
| `Work.Farming` | 목장, 방목 건물 |

## 환경 상태 태그

온도 시스템은 직접 HP를 변경하지 않는다.
대상에게 상태 태그 적용/해제 요청만 보낸다.

| 태그 | 발생 조건 |
|---|---|
| `Status.Explo.Cold` | 대상 위치의 체감 온도가 추위 임계값 이하 |
| `Status.Explo.Heat` | 대상 위치의 체감 온도가 더위 임계값 이상 |
| `Status.Explo.Drowning` | 수중/산소 시스템과 연동될 때 사용 |

## 팰 건강 태그

작업 건물은 팰 건강 상태를 직접 변경하지 않는다.
팰 파트가 제공하는 상태 조회 인터페이스를 통해 작업 가능 여부 판단에만 참고한다.

| 태그 | LMK 영향 |
|---|---|
| `Status.Health.Starvation` | 작업 배정 불가 또는 작업 중단 요청 |
| `Status.Health.Depression` | 작업 배정 불가 |
| `Status.Health.Weakness` | 작업 속도 감소 후보 |
| `Status.Health.Fracture` | 이동/작업 속도 감소 후보 |

## 사용 원칙

- `Work.*`는 작업 가능 여부와 건물 요구 조건에 사용한다.
- `Status.Explo.*`는 온도/탐험 제약 시스템에서 적용 요청에 사용한다.
- `Status.Health.*`는 작업 가능 여부 판단에만 사용한다.
- LMK는 상태 효과 수치와 지속 피해를 직접 구현하지 않는다.
