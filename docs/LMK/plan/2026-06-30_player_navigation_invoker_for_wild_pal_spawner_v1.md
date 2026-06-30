# 2026-06-30_player_navigation_invoker_for_wild_pal_spawner_v1 구현 계획

## 구현 목표

플레이어 클래스를 직접 수정하지 않고, 플레이어 BP 또는 추후 플레이어 C++ 생성자에 붙일 수 있는 `UPW_PlayerNavigationInvokerComponent`를 추가한다.
기본 반경은 현재 `APW_WildPalSpawner` 기본값인 `ActivationDistance 8000 + SpawnRadius 3000`에 맞춰 `GenerationRadius 11000`, `RemovalRadius 13000`으로 둔다.

## 구현 항목

1. `UPW_PlayerNavigationInvokerComponent`
   - `BlueprintSpawnableComponent`
   - 같은 Actor의 `UNavigationInvokerComponent`를 찾아 반경 설정
   - 없으면 경고 로그만 출력
2. 기본값
   - `GenerationRadius = 11000`
   - `RemovalRadius = 13000`
   - `MinGenerationRadius = 8000`
   - `MaxGenerationRadius = 15000`
3. API
   - `ConfigureNavigationInvoker`
   - `SetPlayerNavigationInvokerActive`
   - `IsPlayerNavigationInvokerReady`
   - `GetRecommendedGenerationRadiusForWildPalSpawner`
   - `ApplyWildPalSpawnerCompatibleDefaults`

## 에디터 적용

- 플레이어 BP에 `NavigationInvokerComponent`를 추가한다.
- 같은 플레이어 BP에 `PW_PlayerNavigationInvokerComponent`를 추가한다.
- 레벨에는 큰 `NavMeshBoundsVolume`이 필요하다.
- 프로젝트 설정은 `Generate Navigation Only Around Invokers`와 `RuntimeGeneration=Dynamic`을 유지한다.

## 테스트 기준

- `P` 키로 플레이어 주변 NavMesh가 생성되는지 확인한다.
- 플레이어가 이동하면 NavMesh 생성 영역이 따라오는지 확인한다.
- Wild Pal Spawner가 플레이어 근처에서 NavMesh 기반 스폰 위치를 찾는지 확인한다.
- Base Invoker가 꺼져도 플레이어 주변 야생 팰 스폰은 유지되는지 확인한다.
