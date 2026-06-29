# 구현 순서와 검증 기준

이 문서는 LMK 월드 시스템의 권장 구현 순서와 검증 기준을 정리한다.

## 구현 순서

1. `APW_WorldGameState`에 복제될 월드 상태 정의
2. `UPW_WorldTimeSubsystem`에서 서버 시간 진행 구현
3. `UPW_WeatherSubsystem`에서 서버 날씨 변경 구현
4. 공통 인터페이스 정의
5. Temperature Volume과 상태 태그 적용 요청 구현
6. `UPW_WildPalSpawnSubsystem`과 `APW_WildPalSpawner` 구현
7. 스포너에 낮/밤, 날씨, 온도 조건 연결
8. `UPW_WorkTargetSubsystem`, `APW_WorkBuildingBase`, 작업 슬롯 구현
9. 작업 건물에 `Work.*` 태그 요구 조건 연결
10. Save hook 정리

## Phase 1: 월드 상태

- [ ] 시간 값 서버 진행
- [ ] `UPW_WorldTimeSubsystem` 구현
- [ ] 낮/밤 판정
- [ ] 시간 값 Replication
- [ ] `UPW_WeatherSubsystem` 구현
- [ ] 날씨 값 Replication

검증:

- [ ] PIE 2인 이상 환경에서 시간이 서버 기준으로 동기화됨
- [ ] PIE 2인 이상 환경에서 날씨가 서버 기준으로 동기화됨

## Phase 2: 온도

- [ ] 온도 볼륨 배치
- [ ] 시간/날씨 보정 적용
- [ ] 추위/더위 태그 적용 요청

검증:

- [ ] 온도 볼륨에 들어간 대상에게 `Status.Explo.Cold` 적용 요청이 발생함
- [ ] 온도 볼륨에 들어간 대상에게 `Status.Explo.Heat` 적용 요청이 발생함

## Phase 3: 야생 팰 스폰

- [ ] 서버 권한 스폰
- [ ] 낮/밤 조건
- [ ] 날씨 조건
- [ ] 온도 조건

검증:

- [ ] 서버에서만 야생 팰이 스폰됨
- [ ] 클라이언트에서 야생 팰 스폰 결과가 보임
- [ ] 낮/밤 전환에 따라 스폰 조건 결과가 달라짐

## Phase 4: 작업 건물

- [ ] 작업 요구 태그 제공
- [ ] 작업 슬롯 예약/해제
- [ ] 작업 진행 상태 Replication

검증:

- [ ] 작업 건물이 요구 `Work.*` 태그를 제공함
- [ ] 팰 파트가 인터페이스로 작업 슬롯을 예약할 수 있음
- [ ] 작업 슬롯 점유 상태가 서버 기준으로 유지됨

## 최종 확인

- [ ] LMK 코드가 플레이어/팰 구체 구현체를 직접 참조하지 않음
- [ ] 새 구현 파일이 `PalWorld/Source/PalWorld/LMK/Public`, `PalWorld/Source/PalWorld/LMK/Private` 아래에 있음
- [ ] 실제 호출자가 없는 추상화를 만들지 않았음
