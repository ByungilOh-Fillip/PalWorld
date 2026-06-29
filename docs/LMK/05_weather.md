# Weather 시스템

목표는 날씨가 온도와 야생 팰 스폰 조건에 영향을 줄 수 있게 하는 것이다.

## 추천 클래스

- `APW_WorldGameState`
- `UPW_WeatherSubsystem`

## 책임 분리

| 위치 | 책임 |
|---|---|
| `UPW_WeatherSubsystem` | 서버에서 날씨 변경 규칙과 주기 계산 |
| `APW_WorldGameState` | 현재 날씨 복제 |
| Client | `OnRep` 이후 화면 효과와 사운드 반응 처리 |

## 구현 항목

- [ ] 날씨 타입 정의
- [ ] 현재 날씨 서버 관리
- [ ] 날씨 변경 주기 설정
- [ ] 수동 날씨 변경 함수 추가
- [ ] `UPW_WeatherSubsystem`에서 날씨 변경 결정
- [ ] `APW_WorldGameState`에 현재 날씨 반영
- [ ] 날씨별 온도 보정값 제공
- [ ] 날씨별 스폰 조건 제공
- [ ] 날씨 변경 값을 `ReplicatedUsing`으로 동기화
- [ ] `GetLifetimeReplicatedProps`에 날씨 값 등록
- [ ] `OnRep`에서 날씨 변경 델리게이트 호출

## 1차 날씨 후보

| 날씨 | 용도 |
|---|---|
| Clear | 기본 상태 |
| Rain | 온도 하락, 물 관련 팰 스폰 조건 |
| Storm | 희귀 스폰 또는 위험 지역 조건 |
| HeatWave | 더위 상태 유발 후보 |
| Snow | 추위 상태 유발 후보 |

## 연동 지점

- Temperature는 날씨별 온도 보정값을 읽는다.
- Wild Pal Spawn은 특정 날씨에서만 스폰되는 조건을 읽는다.
- World Time은 날씨 변경 주기 계산에 사용될 수 있다.
- Save는 현재 날씨를 저장한다.

## 완료 기준

- [ ] 서버에서 변경한 날씨가 클라이언트에 반영됨
- [ ] 온도 시스템이 날씨 보정값을 읽음
- [ ] 스포너가 날씨 조건을 처리할 수 있음
- [ ] 날씨 상태 변경이 델리게이트로 전달됨
