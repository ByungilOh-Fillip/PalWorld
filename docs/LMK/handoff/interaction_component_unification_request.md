# 플레이어 상호작용 컴포넌트 통합 요청

이 문서는 병합 이후 `F` 키 상호작용에서 상인 거래 UI가 열리지 않는 문제의 원인과, 플레이어 상호작용 경로를 하나로 합치는 수정 방향을 정리한다.

목표는 `UPWPlayerInteractionComponent`와 `UPWInteractionScannerComponent`로 나뉜 플레이어 상호작용 책임을 `UPWInteractionScannerComponent` 기준으로 통합하는 것이다.

## 문제 요약

현재 에디터에서 상호작용 가이드는 정상 표시된다.
즉 대상 Actor의 `UPWInteractableTargetComponent`, `UPWInteractionGuideWidget`, `UPWInteractionScannerComponent` 기반 UI 갱신은 살아 있다.

하지만 상인에게 `F` 키로 상호작용하면 `WBP_TradePanel`이 열리지 않는다.

현재 증상:

- 상인 근처에서 Interaction Guide는 보인다.
- `F` 키 바인딩 자체는 동작한다.
- 상인 `Interact()` 계열 로그 또는 PrintString은 찍힐 수 있다.
- 하지만 `APW_MerchantPalCharacter::LocalInteract_Implementation()`에 있는 `OpenTradePanel()` 경로가 타지 않아 거래 UI가 열리지 않는다.

## 현재 구조

현재 프로젝트에는 플레이어 상호작용 컴포넌트가 두 계열로 존재한다.

| 컴포넌트 | 위치 | 현재 책임 |
|---|---|---|
| `UPWPlayerInteractionComponent` | `PJH/Public/Player/Components` | 주변 overlap 검색, 기존 단발 `Interact()` 실행, 하이라이트 |
| `UPWInteractionScannerComponent` | `Global/Components` | 대상 스캔, Interaction Guide 표시, `LocalInteract()`, Hold, Progress, MultiAction 실행 |

두 컴포넌트가 모두 "현재 상호작용 대상 찾기"와 "`F` 입력 처리"를 일부씩 담당하고 있어 병합 후 입력 실행 경로와 UI 표시 경로가 분리되었다.

## 문제 원인

상인 거래 UI는 `IPWInteractable::Interact()`가 아니라 `IPWLocalInteractable::LocalInteract()`에서 열린다.

상인 코드의 일반 `Interact()`는 UI를 열지 않는다.

```cpp
bool APW_MerchantPalCharacter::Interact_Implementation(AActor* Interactor)
{
	return CanInteract_Implementation(Interactor);
}
```

거래 UI 생성은 로컬 상호작용 경로에 있다.

```cpp
bool APW_MerchantPalCharacter::LocalInteract_Implementation(AActor* Interactor)
{
	if (!CanLocalInteract_Implementation(Interactor))
	{
		return false;
	}

	if (UPWPlayerTradeComponent* TradeComponent = Interactor->FindComponentByClass<UPWPlayerTradeComponent>())
	{
		if (TradeComponent->OpenTrade(this))
		{
			OpenTradePanel(Interactor, TradeComponent);
			BP_OnMerchantLocalInteracted(Interactor);
			return true;
		}
	}

	return false;
}
```

하지만 현재 `F` 키는 `UPWPlayerInteractionComponent` 쪽으로 들어간다.

현재 흐름:

```text
F Press
-> APWPlayerController::HandleInteractPressed()
-> APWPlayerCharacter::Interact()
-> UPWPlayerInteractionComponent::TryInteract()
-> IPWInteractable::Execute_Interact(...)
```

이 경로는 `IPWLocalInteractable::Execute_LocalInteract(...)`를 호출하지 않는다.
따라서 상인에게 일반 `Interact()`만 호출되고, 실제 UI 오픈 함수인 `OpenTradePanel()`은 실행되지 않는다.

## 병합 이후 문제가 드러난 이유

기존 `generic interaction system` 구현은 플레이어 C++을 직접 수정하지 않고, 플레이어 BP에 `UPWInteractionScannerComponent`를 붙여 Enhanced Input에서 호출하는 방식이었다.

기존 문서의 의도:

```text
플레이어 BP
-> IA_Interact Started
-> UPWInteractionScannerComponent::TryInteract()
```

이후 Interaction Guide / Hold / LocalInteract / MultiAction 기능이 `UPWInteractionScannerComponent`에 추가되었다.

반면 현재 병합된 플레이어 C++은 별도 직접 바인딩을 가지고 있다.

```cpp
InputComponent->BindKey(EKeys::F, IE_Pressed, this, &APWPlayerController::HandleInteractPressed);
```

그리고 이 함수는 `APWPlayerCharacter::Interact()`만 호출한다.

즉 병합 이후 실행 경로가 아래처럼 갈라졌다.

```text
표시 경로:
UPWInteractionScannerComponent
-> 현재 대상 탐색
-> Interaction Guide 표시

실행 경로:
APWPlayerController::HandleInteractPressed()
-> APWPlayerCharacter::Interact()
-> UPWPlayerInteractionComponent::TryInteract()
-> 일반 Interact만 실행
```

그래서 "가이드는 보이는데 실제 상점 UI는 안 열리는" 상태가 된다.

## 통합 방향

기준 컴포넌트는 `UPWInteractionScannerComponent`로 잡는 것이 맞다.

이유:

- `UPWInteractionScannerComponent`는 이미 `LocalInteract()`를 알고 있다.
- Interaction Guide 표시/숨김과 대상 선택을 이미 담당한다.
- Hold 상호작용과 ProgressBar 갱신을 이미 담당한다.
- `UPWMultiInteractable` 기반 다중 액션 실행을 이미 담당한다.
- 상인 거래 UI처럼 로컬 플레이어 UI를 열어야 하는 기능과 구조가 맞다.

반대로 `UPWPlayerInteractionComponent`는 기존 단발 `Interact()`와 하이라이트 중심의 구형 경로다.
여기에 `LocalInteract`, Hold, Progress, MultiAction을 다시 붙이면 기능이 중복된다.

권장 구조:

```text
UPWInteractionScannerComponent = 플레이어 상호작용 최종 진입점
UPWPlayerInteractionComponent = 제거 예정 또는 임시 fallback
```

최종 입력 흐름:

```text
F Press
-> UPWInteractionScannerComponent::TryInteractPressed()
   1. Hold 가능하면 Hold 시작
   2. 아니면 LocalInteract 가능하면 LocalInteract 실행
   3. 아니면 일반 Interact 실행

F Release
-> UPWInteractionScannerComponent::InteractReleased()
   1. 진행 중인 Hold 종료
```

## 실제 수정 코드

아래 코드는 최소 통합 1단계다.

목표:

- 플레이어 `F` 입력이 `UPWInteractionScannerComponent`를 우선 사용한다.
- 상인 거래 UI가 `LocalInteract()` 경로로 열린다.
- 기존 `UPWPlayerInteractionComponent`는 당장 제거하지 않고 fallback으로 유지한다.
- 빌드/에디터 리스크를 줄이기 위해 큰 삭제는 후속 작업으로 미룬다.

### 1. `PWPlayerController.h`

파일:

```text
PalWorld/Source/PalWorld/PJH/Public/Player/Core/PWPlayerController.h
```

수정:

```diff
	void HandlePrimaryActionStarted(const FInputActionValue& Value);
	void HandlePrimaryActionCompleted(const FInputActionValue& Value);
	void HandleInteractPressed();
+	void HandleInteractReleased();
	void HandleAimStarted(const FInputActionValue& Value);
```

### 2. `PWPlayerController.cpp`

파일:

```text
PalWorld/Source/PalWorld/PJH/Private/Player/Core/PWPlayerController.cpp
```

`F` 키 Release 바인딩을 추가한다.

```diff
	if (InputComponent)
	{
		// 메뉴 입력은 지금 단계에서 확실히 동작해야 하므로 IMC와 별도로 직접 바인딩한다.
		InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &APWPlayerController::ToggleInventoryMenu);
		InputComponent->BindKey(EKeys::F, IE_Pressed, this, &APWPlayerController::HandleInteractPressed);
+		InputComponent->BindKey(EKeys::F, IE_Released, this, &APWPlayerController::HandleInteractReleased);

		if (bEnableDebugStatHotkeys)
		{
```

입력 핸들러를 Press / Release로 나눈다.

```diff
 void APWPlayerController::HandleInteractPressed()
 {
	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
	{
-		PlayerCharacter->Interact();
+		PlayerCharacter->StartInteract();
	}
 }
+
+void APWPlayerController::HandleInteractReleased()
+{
+	if (APWPlayerCharacter* PlayerCharacter = GetPWPlayerCharacter())
+	{
+		PlayerCharacter->StopInteract();
+	}
+}
```

### 3. `PWPlayerCharacter.h`

파일:

```text
PalWorld/Source/PalWorld/PJH/Public/Player/Core/PWPlayerCharacter.h
```

기존 `Interact()`는 호환용으로 유지하고, Press / Release용 함수를 추가한다.

```diff
	void StartPrimaryAction();
	void StopPrimaryAction();
	void Interact();
+	void StartInteract();
+	void StopInteract();
	bool StartAim();
```

### 4. `PWPlayerCharacter.cpp`

파일:

```text
PalWorld/Source/PalWorld/PJH/Private/Player/Core/PWPlayerCharacter.cpp
```

`UPWInteractionScannerComponent` include를 추가한다.

프로젝트 include 경로에 따라 아래 둘 중 하나를 사용한다.
현재 `Global/Components/public`가 include path에 잡혀 있으므로 기존 파일들과 같은 방식이면 첫 번째가 가능하다.

```diff
 #include "Player/Components/PWPlayerInteractionComponent.h"
+#include "PWInteractionScannerComponent.h"
 #include "Player/Components/PWPlayerInventoryLinkComponent.h"
```

만약 include 경로가 잡히지 않으면 아래 경로형 include를 사용한다.

```cpp
#include "Global/Components/public/PWInteractionScannerComponent.h"
```

`Interact()`를 호환 래퍼로 바꾸고, 실제 처리는 `StartInteract()` / `StopInteract()`로 분리한다.

```diff
 void APWPlayerCharacter::Interact()
 {
+	StartInteract();
+}
+
+void APWPlayerCharacter::StartInteract()
+{
+	if (UPWInteractionScannerComponent* ScannerComponent = FindComponentByClass<UPWInteractionScannerComponent>())
+	{
+		if (ScannerComponent->TryBeginHoldInteraction())
+		{
+			return;
+		}
+	}
+
	if (InteractionComponent)
	{
		InteractionComponent->TryInteract();
	}
 }
+
+void APWPlayerCharacter::StopInteract()
+{
+	if (UPWInteractionScannerComponent* ScannerComponent = FindComponentByClass<UPWInteractionScannerComponent>())
+	{
+		ScannerComponent->EndHoldInteraction();
+	}
+}
```

이 수정 후 실행 흐름:

```text
F Press
-> PlayerController::HandleInteractPressed()
-> PlayerCharacter::StartInteract()
-> UPWInteractionScannerComponent::TryBeginHoldInteraction()
   -> Hold 가능 시 Hold 시작
   -> Hold 아니면 LocalInteract 먼저 실행
   -> LocalInteract 실패 시 일반 Interact fallback

F Release
-> PlayerController::HandleInteractReleased()
-> PlayerCharacter::StopInteract()
-> UPWInteractionScannerComponent::EndHoldInteraction()
```

## 더 좋은 2단계 리팩토링

위 1단계 코드는 이름이 조금 어색하다.
`TryBeginHoldInteraction()`은 실제로 Hold 전용 함수처럼 보이지만, 내부에서는 아래 처리를 모두 한다.

```cpp
return ExecuteLocalInteraction(InteractableActor) || TryInteract();
```

즉 현재 함수는 사실상 "`F` Press 처리" 역할이다.
장기적으로는 함수명을 실제 책임에 맞게 바꾸는 것이 좋다.

권장 이름:

```cpp
bool TryInteractPressed();
void InteractReleased();
```

또는:

```cpp
bool TryPrimaryInteractionPressed();
void PrimaryInteractionReleased();
```

권장 2단계 코드 방향:

```cpp
bool UPWInteractionScannerComponent::TryInteractPressed()
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return false;
	}

	AActor* InteractableActor = Owner->HasAuthority() ? FindBestInteractable() : CurrentInteractableActor.Get();
	if (InteractableActor == nullptr)
	{
		return false;
	}

	const bool bCanBeginHold = InteractableActor->GetClass()->ImplementsInterface(UPWHoldInteractable::StaticClass())
		&& IPWHoldInteractable::Execute_CanBeginHoldInteraction(InteractableActor, Owner);

	if (bCanBeginHold)
	{
		StartHoldGuideProgress(InteractableActor);

		if (Owner->HasAuthority())
		{
			return ExecuteBeginHoldInteraction(InteractableActor);
		}

		ServerTryBeginHoldInteraction();
		return true;
	}

	if (ExecuteLocalInteraction(InteractableActor))
	{
		return true;
	}

	if (Owner->HasAuthority())
	{
		return ExecuteInteraction(InteractableActor);
	}

	ServerTryInteract();
	return true;
}

void UPWInteractionScannerComponent::InteractReleased()
{
	EndHoldInteraction();
}
```

이름 변경 시 기존 BP 연결이 깨질 수 있으므로, 과도기에는 기존 함수도 남겨서 래핑한다.

```cpp
bool UPWInteractionScannerComponent::TryBeginHoldInteraction()
{
	return TryInteractPressed();
}
```

## 이후 제거 방향

1단계 통합이 안정화되면 `UPWPlayerInteractionComponent`의 책임을 정리한다.

### 유지할 기능

`UPWPlayerInteractionComponent`에 있던 하이라이트 기능이 필요하면 `UPWInteractionScannerComponent`로 옮긴다.

현재 하이라이트 책임:

```cpp
void UPWPlayerInteractionComponent::SetActorHighlighted(AActor* TargetActor, bool bHighlighted) const;
```

이 기능은 `UPWInteractionScannerComponent::SetCurrentInteractableActor()` 안에서 이전 대상/새 대상 변경 시 처리하는 것이 자연스럽다.

권장 이동 위치:

```cpp
void UPWInteractionScannerComponent::SetCurrentInteractableActor(AActor* NewInteractableActor)
{
	AActor* PreviousInteractableActor = CurrentInteractableActor.Get();
	if (PreviousInteractableActor == NewInteractableActor)
	{
		return;
	}

	SetActorHighlighted(PreviousInteractableActor, false);
	HideInteractionGuide(PreviousInteractableActor);

	CurrentInteractableActor = NewInteractableActor;

	SetActorHighlighted(NewInteractableActor, true);
	ShowInteractionGuide(NewInteractableActor);
}
```

### 제거할 기능

`UPWPlayerInteractionComponent`가 더 이상 입력 실행에 사용되지 않으면 아래를 제거한다.

- `APWPlayerCharacter`의 `UPWPlayerInteractionComponent* InteractionComponent`
- `CreateDefaultSubobject<UPWPlayerInteractionComponent>(TEXT("InteractionComponent"))`
- `PWPlayerInteractionComponent.h/.cpp` 파일 또는 사용처

단, 삭제 전에 월드 아이템 줍기, StorageBox, BaseCamp, Merchant, WorkBuilding이 모두 `UPWInteractionScannerComponent` 경로에서 정상 동작하는지 확인해야 한다.

## 병합 결과

1단계 수정 후 기대 결과:

- 상인 가이드가 보인다.
- `F` Press 시 `UPWInteractionScannerComponent::TryBeginHoldInteraction()` 경로를 탄다.
- 상인이 `IPWLocalInteractable`을 구현하므로 `ExecuteLocalInteraction()`이 실행된다.
- `APW_MerchantPalCharacter::LocalInteract_Implementation()`에서 `OpenTradePanel()`이 호출된다.
- BP에 붙인 `UPWPlayerTradeComponent`와 상인 `TradePanelWidgetClass`가 유효하면 거래 UI가 열린다.
- Hold 상호작용 대상은 Press에서 진행도가 시작되고 Release에서 종료된다.
- 기존 단발 상호작용 대상은 fallback으로 일반 `Interact()`가 호출된다.

## 검증 체크리스트

### 상인

- 플레이어 BP에 `UPWPlayerTradeComponent`가 붙어 있는지 확인한다.
- 상인 BP에 `TradePanelWidgetClass`가 지정되어 있는지 확인한다.
- 상인 근처에서 Interaction Guide가 표시되는지 확인한다.
- `F` Press 시 `WBP_TradePanel`이 AddToViewport 되는지 확인한다.
- 거래 UI 오픈 시 마우스 커서가 보이고 `GameAndUI` 입력 모드가 적용되는지 확인한다.
- 닫기 버튼 또는 거래 종료 시 `GameOnly` 입력 모드가 복구되는지 확인한다.

### 기존 단발 상호작용

- WorldItem 줍기가 정상 동작하는지 확인한다.
- StorageBox / BaseCamp 기존 상호작용 로그 또는 UI가 정상 동작하는지 확인한다.
- 대상이 없을 때 `F`를 눌러도 오류가 없는지 확인한다.

### Hold / Progress

- WorkBuilding에서 `F` Press 시 ProgressBar가 증가하는지 확인한다.
- `F` Release 시 서버 작업이 종료되는지 확인한다.
- 다시 누르면 기존 진행도 위치부터 ProgressBar가 시작되는지 확인한다.
- 거리 이탈 시 가이드가 숨겨지고 Hold가 정리되는지 확인한다.

### 멀티플레이

- Listen Server에서 서버 플레이어와 클라이언트 플레이어 모두 검증한다.
- 클라이언트가 상인을 열 때 UI는 클라이언트 화면에만 뜨는지 확인한다.
- 클라이언트의 구매/판매 요청은 `UPWPlayerTradeComponent` 서버 RPC로 들어가는지 확인한다.

## 주의 사항

- `LocalInteract()`는 로컬 UI 오픈 용도다. 서버 상태 변경은 여기서 신뢰하면 안 된다.
- 실제 구매/판매는 반드시 `UPWPlayerTradeComponent`의 서버 RPC에서 다시 검증해야 한다.
- `TryBeginHoldInteraction()`은 현재 이름과 책임이 맞지 않는다. 2단계에서 이름 정리가 필요하다.
- `UPWPlayerInteractionComponent`를 즉시 삭제하면 하이라이트나 기존 줍기 경로가 깨질 수 있으므로, 먼저 입력 경로를 통합하고 테스트한 뒤 제거한다.
- BP에 이미 `UPWInteractionScannerComponent`가 붙어 있다면 C++에서 중복 생성하지 않는 것이 안전하다. 1단계는 `FindComponentByClass`로 기존 BP 컴포넌트를 우선 사용한다.

## 전달용 짧은 요청문

아래 내용을 작업자에게 그대로 전달하면 된다.

```text
현재 F키 입력이 PJH의 UPWPlayerInteractionComponent::TryInteract()만 호출해서 IPWLocalInteractable::LocalInteract() 경로를 타지 않습니다.
상인 거래 UI는 APW_MerchantPalCharacter::LocalInteract_Implementation() 안의 OpenTradePanel()에서 열리기 때문에, 현재 병합 상태에서는 가이드는 보이지만 상점 UI가 열리지 않습니다.

플레이어 상호작용 기준을 Global/Components의 UPWInteractionScannerComponent로 통합해 주세요.
1단계로 F Press는 ScannerComponent->TryBeginHoldInteraction(), F Release는 ScannerComponent->EndHoldInteraction()을 호출하도록 PlayerController/PlayerCharacter를 수정하고, ScannerComponent가 없거나 실패하면 기존 UPWPlayerInteractionComponent::TryInteract()로 fallback 해주세요.

이후 2단계로 TryBeginHoldInteraction() 이름을 TryInteractPressed() 같은 실제 책임에 맞는 이름으로 바꾸고, UPWPlayerInteractionComponent의 하이라이트 기능만 ScannerComponent로 옮긴 뒤 구형 컴포넌트를 제거하는 방향으로 정리하면 됩니다.
```
