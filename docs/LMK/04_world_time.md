# World Time

서버가 월드 시간을 진행하고, 클라이언트는 복제된 시간으로 낮/밤 연출만 갱신한다.

## 구현 클래스

| 클래스 | 책임 |
|---|---|
| `UPW_WorldTimeSubsystem` | 서버 시간 진행, 테스트용 낮/밤 토글 |
| `APW_WorldGameState` | `CurrentDay`, `CurrentTimeOfDay` 복제 |
| `APW_DayNightVisualController` | 복제된 시간으로 태양/달/SkyLight 보간 |

## 현재 기준

- 하루 길이: `RealSecondsPerGameDay = 1200`
- 서버 갱신 주기: `TimeUpdateIntervalSeconds = 1`
- 기본 시작 시간: `06:00`
- 낮 판정 시작: `06:00`
- 밤 판정 시작: `18:00`
- 테스트 토글 명령: `pw.Time.ToggleDayNight`

## 테스트 토글 기준

`pw.Time.ToggleDayNight`는 시각 차이가 확실히 보이도록 아래 시간으로 이동한다.

| 현재 상태 | 이동 시간 |
|---|---|
| 낮 | `00:00` |
| 밤 | `12:00` |

낮/밤 판정 기준 자체는 `06:00`, `18:00`을 유지한다.

## 라이트 연출 기준

- `05:00 ~ 07:00`: 밤에서 낮으로 보간
- `07:00 ~ 17:00`: 완전한 낮
- `17:00 ~ 19:00`: 낮에서 밤으로 보간
- `19:00 ~ 05:00`: 완전한 밤
- 달 라이트는 태양과 별도 각도 곡선을 사용한다.
- 자정 달 각도 기본값은 `-35`로, 정오 태양처럼 머리 위에서 비추지 않는다.
- 밤에는 `SkyLight` 밝기와 색도 낮춰 지면 보조광을 줄인다.

## 서버-클라이언트 흐름

1. 서버 `UPW_WorldTimeSubsystem`이 타이머로 시간 증가
2. 서버가 `APW_WorldGameState::SetWorldTime` 호출
3. `CurrentDay`, `CurrentTimeOfDay`가 클라이언트에 복제
4. 클라이언트 `OnRep`에서 시간 변경 이벤트 발생
5. `APW_DayNightVisualController`가 로컬 라이트를 보간

## 에디터 배치

- 레벨에 `BP_PW_DayNightVisualController` 1개 배치
- `DL_Sun`, `DL_Moon` Directional Light 2개 배치
- `SkyLight` 1개 배치
- 컨트롤러에 `태양 라이트`, `달 라이트`를 직접 지정
- 컨트롤러에 `하늘 보조광`을 직접 지정하거나 자동 탐색을 사용
- 월드 파티션 사용 시 전역 액터는 `Is Spatially Loaded = false` 권장

## 테스트 기준

- Listen Server에서 시간이 증가한다.
- 클라이언트가 같은 `CurrentTimeOfDay`를 받는다.
- `pw.Time.ToggleDayNight` 실행 시 `12:00` 또는 `00:00`으로 이동한다.
- 서버와 클라이언트의 낮/밤 판정이 같다.
- 라이트 회전과 밝기가 클라이언트에서 부드럽게 보간된다.
- `00:00` 그림자가 정오처럼 수직으로 떨어지지 않는다.
- 밤에는 지면 보조광이 낮아져 낮과 명확히 구분된다.

## 제외 범위

- 저장/로드
- 야생 팰 스폰
- 작업 건물 시간 조건
- 상태 효과 시스템
