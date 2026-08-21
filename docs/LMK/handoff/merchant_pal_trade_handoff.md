# 상인 Pal 거래 구조 인수인계

이 문서는 상인 Pal NPC와 거래 UI/거래 요청 구조 v1 구현 내용을 정리한다.
이번 구현은 실제 구매/판매를 처리하지 않고, 추후 거래 기능을 붙이기 위한 구조만 제공한다.

## 구현 내용

- `APW_MerchantPalCharacter`
  - `APWPalCharacter`를 상속한 상인 Pal 캐릭터다.
  - `IPWInteractable`, `IPWLocalInteractable`을 구현한다.
  - `UPWInteractableTargetComponent`와 `UPW_MerchantComponent`를 기본 컴포넌트로 가진다.
  - F 상호작용 시 상호작용한 Actor에 `UPWPlayerTradeComponent`가 붙어 있으면 `OpenTrade()`를 호출한다.
  - 구매, 판매, 화폐, 인벤토리 변경은 직접 처리하지 않는다.

- `UPW_MerchantComponent`
  - 상인의 표시 이름, 인사말, 판매 목록을 보관한다.
  - 판매 목록은 `FPW_MerchantTradeEntry`로 구성된다.
  - 항목에는 `ItemData`, `ItemId`, `BuyPrice`, `SellPrice`, `StockCount`, `bInfiniteStock`이 있다.
  - v1에서는 가격과 재고를 표시/검증 준비용 데이터로만 사용한다.
  - 플레이어 인벤토리, 화폐, UI를 직접 참조하지 않는다.

- `UPWPlayerTradeComponent`
  - 플레이어 BP에 붙여 쓰는 `BlueprintSpawnableComponent`다.
  - 현재 거래 중인 상인을 로컬 상태로 보관한다.
  - `OpenTrade()`, `CloseTrade()`, `CanTradeWithMerchant()`를 제공한다.
  - 후속 구현을 위한 `RequestBuyItem()`, `RequestSellItem()`과 서버 RPC placeholder를 제공한다.
  - v1에서는 서버 RPC가 실제 아이템/화폐를 변경하지 않고, 유효성 확인 후 로그만 남긴다.

- `UPWTradePanelWidget`
  - 로컬 전용 거래 UI 기반 위젯이다.
  - `UPWPlayerTradeComponent`에 바인딩하고, 거래 상태 변경 델리게이트를 받아 갱신한다.
  - 현재 상인의 `UPW_MerchantComponent`에서 판매 목록을 읽어 UI에 표시할 수 있다.
  - 구매/판매 버튼은 상인이나 인벤토리를 직접 수정하지 않고 `UPWPlayerTradeComponent` 요청 함수만 호출해야 한다.

## BP 연결 방법

1. 플레이어 BP에 `UPWPlayerTradeComponent`를 추가한다.
2. `UPWTradePanelWidget`을 상속한 `WBP_TradePanel`을 만든다.
3. 로컬 플레이어 UI 생성 시 `WBP_TradePanel`에 플레이어의 `UPWPlayerTradeComponent`를 전달해 `InitializeWithTradeComponent()`를 호출한다.
4. `WBP_TradePanel`은 `GetTradeEntries()`로 판매 목록을 구성한다.
5. 구매/판매 버튼은 `RequestBuyItem()` 또는 `RequestSellItem()`만 호출한다.

## 주의할 점

- 다른 사람 파트의 C++ 파일은 직접 수정하지 않는다.
  - 이번 구조는 플레이어 BP가 컴포넌트를 추가해서 opt-in하는 방식이다.
  - PlayerCharacter, PlayerController, HUD C++에 강제로 연결하지 않는다.

- 상인 Actor에 Client -> Server RPC를 두지 않는다.
  - 오픈월드 멀티플레이에서 상인 Actor는 보통 클라이언트 소유 Actor가 아니다.
  - 서버 RPC는 플레이어가 소유한 `UPWPlayerTradeComponent`에 둔다.

- 클라이언트 UI 값을 신뢰하지 않는다.
  - 후속 실제 거래 구현 시 서버는 상인 유효성, 거리, 판매 목록, 가격, 재고, 인벤토리, 화폐를 다시 검증해야 한다.
  - UI에 표시된 가격이나 수량을 그대로 권한 데이터로 사용하면 안 된다.

- 상인 컴포넌트는 데이터 원본 역할만 한다.
  - 플레이어 인벤토리 제거, 아이템 지급, 화폐 차감은 상인 컴포넌트 책임이 아니다.
  - 실제 거래 처리는 플레이어 소유 거래 컴포넌트와 인벤토리/화폐 시스템의 협업으로 구현한다.

- 상인 despawn, capture, streaming out, destroy 상황을 고려한다.
  - `UPWPlayerTradeComponent`는 현재 상인이 유효하지 않거나 거래 거리 밖이면 거래 상태를 닫는다.
  - UI는 거래 상태 변경 이벤트를 받아 닫히거나 숨겨져야 한다.

## 추후 추가 구현 계획

1. 거래 UI 완성
   - 판매 목록 슬롯 위젯 제작
   - 아이템 이름, 아이콘, 구매가, 판매가, 재고 표시
   - 구매/판매 수량 선택 UI 추가
   - 거래 종료 버튼과 ESC 닫기 처리 추가

2. 실제 구매 구현
   - `UPWPlayerTradeComponent::ServerBuyItem()`에서 서버 권한 검증 추가
   - 상인 판매 목록에 해당 아이템이 있는지 확인
   - 가격과 수량을 서버 데이터 기준으로 계산
   - 플레이어 화폐 보유량 확인
   - 플레이어 인벤토리 공간 확인
   - 성공 시 화폐 차감, 아이템 지급, 필요하면 상인 재고 차감

3. 실제 판매 구현
   - `UPWPlayerTradeComponent::ServerSellItem()`에서 플레이어 인벤토리 보유량 확인
   - 상인이 매입 가능한 항목인지 확인
   - 서버 기준 판매가 계산
   - 성공 시 플레이어 아이템 제거, 화폐 지급

4. 화폐 시스템 결정
   - 화폐를 일반 아이템으로 처리할지, 별도 Player Wallet 컴포넌트로 처리할지 결정한다.
   - 결정 전까지 거래 구조는 화폐 구현체에 직접 의존하지 않는다.

5. 포획 상인 정책 구현
   - 포획 후에도 대화/거래 기능을 유지한다.
   - 포획 상태에 따라 상점 목록, 가격, 대화 내용이 달라질지 별도 정책으로 정한다.

6. 상인 전용 AI 추가
   - 일반 Pal AI와 분리된 상인용 AIController 또는 BehaviorTree를 준비한다.
   - 대기, 배회, 피격 반응, 거래 중 정지 같은 동작은 후속 작업으로 구현한다.

## v1 제외 범위

- 실제 화폐 시스템
- 실제 인벤토리 변경
- 상인 재고 차감
- 거래 UI 디자인 완성
- 포획 후 상인 정책 구현
- 상인 전용 AI 구현
- Player C++ 직접 수정
