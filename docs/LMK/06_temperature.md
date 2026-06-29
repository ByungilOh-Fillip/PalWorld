# Temperature 시스템

목표는 지역, 시간을 기준으로 체감 온도를 계산하고 탐험 상태 태그 적용을 요청하는 것이다.

## 추천 클래스

- `APW_TemperatureVolume`
- `UPW_TemperatureSubsystem`
- `FPW_TemperatureSample`

## 구현 항목

- [ ] 온도 볼륨 Actor 정의
- [ ] 기본 지역 온도 설정
- [ ] 낮/밤 온도 보정
- [ ] 건물 또는 장비에 의한 보정 hook 준비
- [ ] 대상 위치 기준 체감 온도 계산
- [ ] 추위 임계값 설정
- [ ] 더위 임계값 설정
- [ ] `Status.Explo.Cold` 적용 요청
- [ ] `Status.Explo.Heat` 적용 요청
- [ ] 정상 온도 복귀 시 상태 해제 요청
- [ ] 서버 권한 기준으로 상태 적용 판단

## 계산 입력

| 입력 | 설명 |
|---|---|
| BaseTemperature | 온도 볼륨의 기본 온도 |
| TimeModifier | 낮/밤 또는 시간대 보정 |
| BuildingModifier | 냉각/온열 건물 보정 hook |
| EquipmentModifier | 장비 보정 hook, 실제 구현은 플레이어 파트 |

## 상태 적용 원칙

- LMK는 상태 태그 적용/해제 요청만 보낸다.
- HP 감소, 이동 제한, UI 표시는 상태 시스템이나 플레이어/팰 파트가 처리한다.
- 대상은 `IPW_GameplayTagStatusTarget` 같은 인터페이스로 연결한다.

## 완료 기준

- [ ] 같은 위치의 플레이어/팰에게 같은 온도 규칙이 적용됨
- [ ] 온도 시스템이 플레이어/팰 구체 클래스에 직접 의존하지 않음
- [ ] `Status.Explo.Cold` 적용/해제 요청이 동작함
- [ ] `Status.Explo.Heat` 적용/해제 요청이 동작함
