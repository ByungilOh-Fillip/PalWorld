# Wild Pal Spawn 시스템

목표는 서버 권한으로 플레이어 주변 야생 팰을 활성화하고, 지역/전역 예산과 오브젝트 풀로 오픈월드 부하를 제한하는 것이다.

## 추천 클래스

- `APW_WildPalSpawner`
- `UPW_WildPalSpawnSubsystem`
- `UPW_PalSpawnPoolSubsystem`
- `IPW_PooledSpawnActor`
- `FPW_WildPalSpawnEntry`

## 책임 분리

| 위치 | 책임 |
|---|---|
| `UPW_WildPalSpawnSubsystem` | 스포너 등록, 플레이어 거리 조회, 전체 활성 수 관리, 평가 루프 |
| `APW_WildPalSpawner` | 스폰 위치/반경, 후보 팰 설정, 지역 활성 수, 리스폰 타이머 |
| `UPW_PalSpawnPoolSubsystem` | 클래스별 풀 재사용, 활성/비활성 상태 전환 |
| `IPW_PooledSpawnActor` | 풀 활성/반납 이벤트 hook |
| Spawned Pal Actor | Actor Replication과 팰 자체 동작 |

## 구현 항목

- [ ] 스폰할 팰 클래스 설정
- [ ] 스폰 가중치 설정
- [ ] 최대 활성 수 설정
- [ ] 스폰 반경 또는 스폰 포인트 설정
- [ ] 리스폰 시간 설정
- [ ] 낮/밤 조건 설정
- [ ] 플레이어 거리 조건 설정
- [ ] 지역 활성 팰 수 기본 8마리 제한
- [ ] 전체 월드 활성 팰 수 기본 70마리 제한
- [ ] 오브젝트 풀 반납/재사용
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
| PlayerDistance | 스폰 지역 활성/반납과 플레이어 근접 스폰 제한 |
| ActiveCount | 지역 8마리, 전체 70마리 제한 |

## 제외 항목

- 팰 AI 의사결정
- 팰 전투 로직
- 포획 로직
- 팰 스탯과 성장
- 개별 야생 팰 저장
- DataAsset 기반 공유 스폰 테이블
- Replication Graph 연동

## 완료 기준

- [ ] 서버에서만 야생 팰이 생성됨
- [ ] 클라이언트는 Replication으로 스폰 결과를 봄
- [ ] 낮/밤 조건에 따라 스폰 여부가 달라짐
- [ ] 한 스폰 지역 활성 팰이 8마리를 넘지 않음
- [ ] 전체 월드 활성 팰이 70마리를 넘지 않음
- [ ] 모든 플레이어가 반납 거리 밖으로 나가면 해당 지역 팰이 풀로 반납됨
- [ ] 재진입 시 풀에 있던 팰이 재사용됨
- [ ] 팰 AI/전투 구현체를 직접 참조하지 않음
