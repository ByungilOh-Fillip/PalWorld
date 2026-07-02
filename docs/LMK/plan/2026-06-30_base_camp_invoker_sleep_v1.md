# 2026-06-30_base_camp_invoker_sleep_v1 구현 계획

## 구현 목표

베이스캠프 주변에 방문자가 없으면 `NavigationInvokerComponent`를 비활성화하고, 작업은 `UPW_BaseWorkSimulationComponent`의 서버 시간 기반 Offline Simulation으로 계속 정산한다.

## 구현 항목

1. `UPW_BaseNavigationComponent`
   - `SetBaseNavigationActive(bool)` 추가
   - `IsBaseNavigationActive()` 추가
   - 비활성 상태에서는 NavMesh 질의 함수가 실패하도록 처리
2. `APW_BaseCampActor`
   - 서버 전용 방문자 체크 타이머 추가
   - `VisitorActivationExtraRadius`
   - `VisitorCheckIntervalSeconds`
   - `VisitorKeepAliveSeconds`
   - 방문자가 있으면 Invoker 활성
   - 유예시간 동안 방문자가 없으면 Invoker 비활성
   - Active/Offline 전환 시 `SimulateUntilNow()` 호출
3. `UPW_BasePalAssignmentComponent`
   - Base Navigation이 비활성 상태면 표현용 팰 Actor 스폰을 보류
   - 슬롯 데이터는 유지

## 테스트 기준

- 플레이어가 베이스 근처에 있으면 Base Navigation Invoker가 활성 상태를 유지한다.
- 플레이어가 베이스를 떠나고 `VisitorKeepAliveSeconds`가 지나면 Invoker가 비활성화된다.
- Invoker가 비활성화되어도 작업 상태는 `SimulateUntilNow()`로 시간 차이만큼 정산된다.
- 플레이어가 다시 접근하면 Invoker가 재활성화된다.

## 제외 범위

- 팰 Actor Despawn/Restore 전체 정책은 후속 작업이다.
- 방문자 체크 공간 분할 최적화는 베이스 수가 늘어난 뒤 적용한다.
