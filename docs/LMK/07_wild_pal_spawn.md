# Wild Pal Spawn 시스템

목표는 서버 권한으로 야생 팰을 스폰하고 월드 상태에 따라 스폰 조건을 제어하는 것이다.

## 추천 클래스

- `APW_WildPalSpawner`
- `UPW_WildPalSpawnSubsystem`
- `FPW_WildPalSpawnEntry`
- `FPW_WildPalSpawnCondition`

## 책임 분리

| 위치 | 책임 |
|---|---|
| `UPW_WildPalSpawnSubsystem` | 스포너 등록, 전체 활성 수 관리, 월드 조건 조회 |
| `APW_WildPalSpawner` | 스폰 위치/반경, 개별 스폰 설정, 리스폰 타이머 |
| Spawned Pal Actor | Actor Replication과 팰 자체 동작 |

## 구현 항목

- [ ] 스폰할 팰 클래스 설정
- [ ] 스폰 가중치 설정
- [ ] 최대 활성 수 설정
- [ ] 스폰 반경 또는 스폰 포인트 설정
- [ ] 리스폰 시간 설정
- [ ] 낮/밤 조건 설정
- [ ] 날씨 조건 설정
- [ ] 온도 범위 조건 설정
- [ ] 플레이어 거리 조건 설정
- [ ] 서버에서만 스폰 실행
- [ ] 스포너를 `UPW_WildPalSpawnSubsystem`에 등록
- [ ] 활성 스폰 목록 관리
- [ ] 팰 사망/디스폰 시 목록 정리
- [ ] 리스폰 타이머 관리
- [ ] 스폰/디스폰 델리게이트 제공
- [ ] 스폰된 팰에 lifecycle 인터페이스 호출

## 스폰 조건

| 조건 | 설명 |
|---|---|
| Time | 낮 전용, 밤 전용, 항상 스폰 |
| Weather | 특정 날씨에서만 스폰 |
| Temperature | 특정 온도 범위에서만 스폰 |
| PlayerDistance | 플레이어가 너무 가깝거나 멀면 스폰 제한 |
| ActiveCount | 최대 활성 수 제한 |

## 제외 항목

- 팰 AI 의사결정
- 팰 전투 로직
- 포획 로직
- 팰 스탯과 성장
- 개별 야생 팰 저장

## 완료 기준

- [ ] 서버에서만 야생 팰이 생성됨
- [ ] 클라이언트는 Replication으로 스폰 결과를 봄
- [ ] 낮/밤 조건에 따라 스폰 여부가 달라짐
- [ ] 날씨 조건에 따라 스폰 여부가 달라짐
- [ ] 온도 조건에 따라 스폰 여부가 달라짐
- [ ] 팰 AI/전투 구현체를 직접 참조하지 않음
