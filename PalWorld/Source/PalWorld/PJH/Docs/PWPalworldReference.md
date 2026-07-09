****# PW Palworld Reference

이 문서는 PJH 작업 기준으로 팰월드의 플레이어 UI/스탯/장비 구조를 헷갈리지 않기 위해 남긴 기준 메모다.

## HUD 기준

이미지 기준 HUD 구성은 아래 순서로 본다.

| 위치 | 의미 | 우리 프로젝트 위젯 바인딩 |
| --- | --- | --- |
| 상단 원형 아이콘 | 현재/퀵 선택 팰 UI | 아직 팰 파티/소환 시스템 없음 |
| 팰 이름/레벨/팰 HP | 현재 소환 또는 선택 팰 정보 | 추후 `PalPartyComponent` 이후 연결 |
| 파란색 바 | 플레이어 쉴드 | `Progress_Shield`, `Text_Shield` |
| 초록색 바 | 플레이어 HP | `Progress_Health`, `Text_Health` |
| 주황색 바 | 플레이어 배고픔 | `Progress_Hunger`, `Text_Hunger` |

## Shield 기준

팰월드의 Shield는 손에 들고 막는 방패라기보다 플레이어에게 파란 쉴드 게이지를 제공하는 장비/스탯 축으로 본다.

| 항목 | 기준 |
| --- | --- |
| 장비 분류 | Armor 계열 아이템이지만 UI/슬롯상 Shield 장비칸으로 분리 |
| 예시 진행 | Common Shield -> Mega Shield -> Giga Shield -> Hyper Shield -> Ultra Shield |
| 역할 | 공격 데미지를 먼저 흡수하는 추가 내구도/보호막 |
| HP 관계 | 일반 공격 데미지는 Shield가 먼저 받고, 남은 데미지만 HP로 들어간다 |
| 예외 | 굶주림/낙하/상태 이상류 데미지는 Shield가 아니라 HP에 직접 들어갈 수 있게 설계 |
| 구현 기준 | `PWPlayerStatComponent`는 현재/최대 쉴드 수치와 UI 표시를 담당 |
| 장비 DA 기준 | `DA_Item_Shield`는 MaxShield/회복 규칙/방어 보너스를 제공하는 장비 데이터로 확장 |

## HP 기준

| 항목 | 기준 |
| --- | --- |
| 역할 | 플레이어 생존의 최종 체력 |
| 데미지 순서 | 일반 공격: Shield -> HP |
| 굶주림 | Hunger가 0이면 HP를 직접 감소시키는 방향으로 구현 |
| 사망 처리 | HP가 0이 되었을 때 다운/사망/리스폰 흐름은 아직 미구현 |

## Hunger 기준

| 항목 | 기준 |
| --- | --- |
| 역할 | 플레이어 배고픔/포만도 |
| 감소 | 서버 권한으로 시간에 따라 감소 |
| 회복 | 음식 아이템 사용으로 회복 |
| 0 도달 | Starvation 상태로 보고 HP 직접 감소 |
| UI | 주황색 바로 표시 |
| 다음 구현 | `DA_Item_Food`에 HungerRestoreAmount 같은 회복량 추가 |
| 사용 방식 | 인벤토리에서 Consumable 아이템 우클릭 시 사용 |

## 데미지 함수 기준

| 함수 | 용도 |
| --- | --- |
| `ApplyHealthDamage` | 일반 공격 데미지. Shield를 먼저 깎고 남은 피해만 HP에 적용 |
| `ApplyDirectHealthDamage` | 굶주림/낙하/특수 상태처럼 Shield를 무시하고 HP에 직접 적용 |
| `ApplyStarvationDamage` | Hunger가 0일 때 서버 Tick에서 HP를 직접 감소 |

## Consumable 기준

| 항목 | 기준 |
| --- | --- |
| 데이터 | `PWItemDataAsset`의 `HealthRestoreAmount`, `HungerRestoreAmount` |
| 사용 조건 | 회복할 수 있는 수치가 실제로 부족할 때만 사용 성공 |
| 소비 처리 | 사용 성공 시 인벤토리 슬롯에서 1개 감소 |
| 실패 처리 | Hunger/HP가 이미 가득 차 있으면 아이템을 소비하지 않음 |

## 현재 프로젝트 상태

| 시스템 | 현재 상태 |
| --- | --- |
| `PWPlayerStatComponent` | HP, Stamina, Hunger, Shield 기반 있음 |
| `PWPlayerHUDWidget` | Shield/Health/Hunger 위젯 자동 바인딩 준비됨 |
| `PWItemDataAsset` | 장비 타입/슬롯/메시/아이콘은 있음, 스탯 보너스 필드는 아직 없음 |
| `PWPlayerEquipmentComponent` | Shield 슬롯은 있음 |
| `DA_Item_TestShield` | 이름과 용도를 `DA_Item_Shield`로 정리 필요 |
| 팰 UI | 아직 팰 파티/소환 데이터가 없어서 표시만 만들면 가짜 UI가 됨 |

## 다음 구현 순서

1. `DA_Item_TestShield`를 에디터에서 `DA_Item_Shield`로 Rename하고 Redirector 정리.
2. `DA_Item_Shield`의 `ItemId`를 `Shield`로 맞추고 `ItemType = Shield`, `EquipmentSlotType = Shield`로 설정.
3. `PWItemDataAsset`에 장비 스탯 보너스 필드를 추가한다.
4. 최소 필드는 `MaxShieldBonus`, `DefenseBonus`, `HungerRestoreAmount` 정도로 시작한다.
5. Shield 장착/해제 시 `PWPlayerEquipmentComponent`가 `PWPlayerStatComponent`의 MaxShield를 갱신하도록 연결한다.
6. Hunger가 0이면 일정 시간마다 HP를 직접 감소시킨다.
7. 음식 아이템 사용 시 Hunger를 회복한다.
8. 팰 UI는 Pal Party/Pal Summon 데이터가 생긴 뒤 연결한다.

## 참고한 외부 기준

- Palworld Wiki, Common Shield: https://palworld.fandom.com/wiki/Common_Shield
- Palworld Wiki, Food: https://palworld.fandom.com/wiki/Food
