# Interaction Guide UI 인수인계

이 문서는 상호작용 대상에 여러 키 가이드 UI를 붙이고, 홀드 작업 진행도를 ProgressBar로 표시하기 위한 구현 내용을 정리한다.

## 구현 범위

| 구성 | 책임 |
|---|---|
| `FPWInteractionGuideAction` | UI에 표시할 액션 데이터 |
| `UPWInteractableTargetComponent` | 대상 Actor가 보여줄 가이드 액션 목록과 WidgetComponent 관리 |
| `UPWInteractionGuideWidget` | 여러 가이드 엔트리를 생성하고 표시 |
| `UPWInteractionGuideEntryWidget` | 키, 라벨, ProgressBar 한 줄 표시 |
| `UPWInteractionScannerComponent` | 현재 대상 탐색, 가이드 표시/숨김, 액션 실행, 홀드 진행도 갱신 |
| `UPWMultiInteractable` | `ActionId` 기반 다중 액션 실행 인터페이스 |
| `UPW_WorkBuildingComponent` | 실제 작업 진행도를 Interaction Guide Progress에 동기화 |

기본 UI 방식은 대상 Actor에 붙은 `Screen Space WidgetComponent`다.
따라서 대상 위치를 따라가지만 화면 UI처럼 그려지고, 캐릭터나 월드 메시가 중간에 있어도 가려지지 않는다.

## 액션 데이터

`FPWInteractionGuideAction`은 UI와 실행에 필요한 최소 데이터를 담는다.

| 필드 | 의미 |
|---|---|
| `ActionId` | 실행할 기능을 구분하는 ID |
| `Key` | 화면에 표시하고 입력 매칭에 사용할 키 |
| `Label` | UI에 표시할 설명 |
| `bEnabled` | 비활성 액션 표시/실행 여부 |
| `Progress` | ProgressBar 현재 값, `0.0 ~ 1.0` |
| `bShowProgress` | ProgressBar 표시 여부 |
| `ProgressDurationSeconds` | 홀드 UI가 0에서 1까지 차는 데 걸리는 시간 |
| `SortOrder` | UI 정렬 순서 |

`InteractionGuideActions` 배열이 비어 있으면 기존 방식처럼 `DefaultActionId`, `DefaultActionKey`, `PromptText`로 단일 액션을 만든다.
배열에 하나라도 들어 있으면 배열이 우선이며, 기본 액션은 자동으로 추가되지 않는다.

## Widget 구조

### `WBP_InteractionGuide`

부모 클래스는 `UPWInteractionGuideWidget`이다.

필수 바인딩:

- `GuideList`: `VerticalBox`

설정:

- `EntryWidgetClass`에 `WBP_InteractionGuideEntry`를 지정한다.

### `WBP_InteractionGuideEntry`

부모 클래스는 `UPWInteractionGuideEntryWidget`이다.

권장 바인딩:

- `KeyText`: `TextBlock`
- `LabelText`: `TextBlock`
- `ProgressBar`: `ProgressBar`

기존 이름 호환:

- `HoldProgressBar`도 동작한다.
- 새로 만들 때는 `ProgressBar` 이름을 권장한다.

WBP Graph에서 액션 배열을 직접 처리하지 않는다.
WBP는 표시만 담당하고, 어떤 정보를 몇 개 표시할지는 `UPWInteractableTargetComponent`가 결정한다.

## 입력과 실행 흐름

### 단일 기본 상호작용

기존 Actor는 `IPWInteractable`만 구현해도 동작한다.
`InteractionGuideActions`가 비어 있으면 `F + PromptText` 단일 액션으로 표시된다.

### 다중 액션

대상 Actor가 `UPWMultiInteractable`을 구현하면 `ActionId` 기반으로 실행된다.

흐름:

1. `UPWInteractionScannerComponent`가 현재 대상의 `InteractionGuideActions`를 읽는다.
2. 입력 키가 눌리면 `Key`로 액션을 찾는다.
3. 찾은 액션의 `ActionId`를 `InteractAction(Interactor, ActionId)`에 전달한다.
4. 대상 Actor가 `ActionId`별 실제 기능을 실행한다.

`ActionId`는 UI 텍스트가 아니라 기능 식별자다.
예를 들어 `Default`, `OpenMenu`, `Upgrade`, `Repair`, `FastTravel`처럼 사용한다.

## 홀드 진행도

홀드 UI는 시간 기반으로 동작한다.

```cpp
Progress = ElapsedTime / ProgressDurationSeconds;
Progress = FMath::Clamp(Progress, 0.0f, 1.0f);
```

홀드 시작 시 현재 `GuideAction.Progress`를 읽어서 시작 위치를 정한다.
따라서 작업이 40% 진행된 상태에서 다시 누르면 ProgressBar도 40% 위치부터 찬다.

홀드를 중간에 떼도 `UPWInteractionScannerComponent`는 Progress를 지우지 않는다.
진행도를 초기화할지 유지할지는 작업 시스템이 결정해야 한다.

## WorkBuilding 연동

`APW_WorkBuildingBase` 계열은 `UPW_WorkBuildingComponent`의 작업량 모델과 자동 동기화된다.

실제 작업 시간:

```cpp
RequiredPlayerWorkSeconds = RequiredWorkProgress / PlayerWorkRate;
```

예시:

| RequiredWorkProgress | PlayerWorkRate | 완료 시간 |
|---:|---:|---:|
| `100` | `10` | `10초` |
| `300` | `10` | `30초` |
| `100` | `20` | `5초` |

`APW_WorkBuildingBase::SynchronizeInteractionGuideActions()`가 `F` 또는 `Default` 액션의 `ProgressDurationSeconds`를 위 값으로 맞춘다.
`UPW_WorkBuildingComponent`는 `WorkProgress / RequiredWorkProgress`를 `Default` 액션 Progress로 계속 밀어준다.

중간에 홀드를 떼면 실제 `WorkProgress`는 유지된다.
다시 홀드를 시작하면 UI ProgressBar는 유지된 진행도에서 시작한다.

## 에디터 설정 체크리스트

### 대상 Actor

- Actor가 `IPWInteractable`을 구현한다.
- Actor에 `UPWInteractableTargetComponent`가 있다.
- `Interaction Enabled`가 켜져 있다.
- `Interaction Radius`가 플레이어 거리보다 충분히 크다.
- `Interaction Guide Widget Class`에 `WBP_InteractionGuide`가 지정되어 있다.

### 단일 F 액션

- `InteractionGuideActions` 배열을 비워 둔다.
- `PromptText`를 원하는 라벨로 설정한다.
- `DefaultActionId`는 기본값 `Default`를 사용한다.
- `DefaultActionKey`는 `F`를 사용한다.

### 직접 액션 배열 사용

- 배열에 액션을 하나 이상 넣으면 기본 액션은 무시된다.
- F 액션도 필요하면 배열에 직접 추가한다.
- `ActionId`는 `None`이 아니어야 한다.
- `Label`은 비어 있으면 안 된다.
- ProgressBar를 표시하려면 `bShowProgress`를 켠다.

### WorkBuilding 홀드 작업

- `WorkBuildingComponent`가 존재한다.
- 작업이 `Reserved` 또는 `InProgress` 상태여야 홀드가 가능하다.
- `Start with Reserved Work`를 켜거나, `ReserveWork()`를 먼저 호출한다.
- 플레이어가 `Work Interaction Radius` 안에 있어야 한다.
- F Press는 `TryBeginHoldInteraction()`에 연결한다.
- F Release는 `EndHoldInteraction()`에 연결한다.

## 문제 해결

### UI가 안 뜬다

- 플레이어에 `UPWInteractionScannerComponent`가 붙어 있는지 확인한다.
- 대상 Actor가 `IPWInteractable`을 구현했는지 확인한다.
- 대상 Actor에 `UPWInteractableTargetComponent`가 있는지 확인한다.
- `CanInteract()`가 `true`를 반환하는지 확인한다.
- `WBP_InteractionGuide` 부모가 `UPWInteractionGuideWidget`인지 확인한다.
- `WBP_InteractionGuide`에 `GuideList`가 있는지 확인한다.
- `EntryWidgetClass`가 `WBP_InteractionGuideEntry`로 지정되어 있는지 확인한다.

### UI가 너무 크게 나온다

`SetDrawAtDesiredSize(true)`는 위젯의 Desired Size를 Draw Size로 사용한다.
따라서 WBP 루트가 큰 Canvas Panel이거나 고정 크기가 크면 화면에서도 크게 보인다.

권장:

- 루트 위젯은 내용 크기에 맞게 구성한다.
- `SizeBox`, `Border`, `VerticalBox`로 원하는 폭을 제한한다.
- `Canvas Panel`을 루트로 쓰는 경우 불필요하게 큰 슬롯 크기를 피한다.

### Show Progress를 켰는데 홀드가 안 된다

`bShowProgress`는 UI ProgressBar 표시 옵션일 뿐이다.
홀드 가능 여부는 `IPWHoldInteractable::CanBeginHoldInteraction()`과 대상 작업 상태가 결정한다.

`APW_WorkBuildingBase`는 `WorkBuildingComponent->CanBeginWork(Interactor)`가 true일 때만 홀드가 가능하다.
즉 작업이 예약되어 있지 않으면 기본 인터랙션만 가능하다.

## 테스트 체크리스트

- 기존 단일 F 상호작용 Actor가 수정 없이 `F + PromptText`로 표시되는지 확인.
- `InteractionGuideActions` 배열을 사용한 Actor에서 여러 액션이 정렬되어 표시되는지 확인.
- `ActionId` 기반 실행이 대상 Actor의 `InteractAction()`으로 들어오는지 확인.
- 대상 변경 시 이전 대상 UI가 숨겨지는지 확인.
- 거리 이탈 시 UI가 숨겨지는지 확인.
- 홀드 중 ProgressBar가 시간에 따라 증가하는지 확인.
- 홀드 중간에 떼고 다시 눌렀을 때 기존 진행도 위치부터 시작하는지 확인.
- WorkBuilding에서 `RequiredWorkProgress / PlayerWorkRate`와 UI 완료 시간이 일치하는지 확인.
- PIE 2인 환경에서 클라이언트 입력도 서버 검증 후 작업 진행되는지 확인.

## 주의 사항

- `ProgressDurationSeconds`는 UI 홀드 시간이며, 실제 작업 시간과 동기화하려면 대상 시스템이 값을 맞춰야 한다.
- WorkBuilding은 자동 동기화되지만, 다른 `IPWHoldInteractable` Actor는 직접 동기화해야 한다.
- `UPWInteractionScannerComponent`는 로컬 플레이어의 UI만 갱신한다.
- 실제 기능 실행은 서버 검증을 거친다.
- WBP는 표시만 담당하고 액션 목록을 직접 만들지 않는다.
