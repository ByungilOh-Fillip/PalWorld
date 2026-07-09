# PalWorld 건축 시스템 인수인계 문서

이 문서는 새롭게 프로젝트에 이식된 **건축 시스템(Building System)**의 구조와 흐름, 그리고 플레이어 폰(Pawn)이나 컨트롤러(Controller)에서 시스템을 설정하고 커스텀하는 방법을 설명합니다.

---

## 1. 시스템 개요
본 건축 시스템은 네트워크 멀티플레이어(Replication)를 완벽히 지원하며, 다음과 같은 특징을 가집니다:
- **모듈형 구조:** 플레이어 캐릭터나 컨트롤러에 '컴포넌트(Component)' 형태로 부착하여 사용합니다.
- **스냅(Snap) 기반 배치:** 토대를 중심으로 벽과 지붕이 정해진 위치에 자석처럼 달라붙는 시스템입니다.
- **서버 검증(Server Authority):** 클라이언트는 미리보기만 처리하며, 실제 건설 및 해체 처리는 서버의 충돌/거리 검증을 통과해야만 이루어집니다.

---

## 2. 핵심 컴포넌트 및 클래스 역할

### 2.1 `UPW_BuildingInputComponent` (입력 처리 컴포넌트)
- **역할:** 사용자의 키보드/마우스 입력을 받아 건축 로직 컴포넌트(Placement Component)나 UI로 전달합니다.
- **주요 기능:**
  - `B 키`: 건축 방사형 메뉴(Radial Menu) 열기/닫기
  - `C 키`: 건설 모드 중 **해체 모드**로 전환 (에임에 위치한 건축물 하이라이트)
  - `좌클릭`: 건축 배치 확정 또는 건축물 해체 확정
  - `우클릭`: 건축/해체 모드 취소
  - `마우스 휠`: 건축물 배치 전 미리보기 상태에서 회전 (토대는 자유 회전, 벽/지붕은 90도 단위 스냅 회전)
- **설정:** `bAutoBindInput` 플래그를 통해 위 키들을 자동으로 바인딩할지, 아니면 프로젝트의 Enhanced Input 등을 통해 수동으로 함수를 호출할지 결정할 수 있습니다.

### 2.2 `UPW_PlayerBuildingPlacementComponent` (건축 로직 컴포넌트)
- **역할:** 건축 미리보기 액터(Preview Actor) 생성 및 트레이스(Raycast)를 통한 스냅 위치 계산, 서버로의 배치/해체 RPC 요청을 전담합니다.
- **주요 설정 (블루프린트 디테일 패널):**
  - **Piece Definitions:** 배치할 수 있는 건축물 종류(Foundation, Wall, Roof), 각 건축물의 클래스, 겹침 검사 박스 크기(`PlacementOverlapExtent`), 유효/비유효 상태의 미리보기 마테리얼을 정의하는 배열입니다.
  - **Material Profiles:** 나무, 돌, 철 등 건축물에 씌울 재질 정보와 해당 재질의 최대 내구도(Durability)를 정의합니다.
  - **Placement Trace Distance:** 플레이어가 에임을 둔 곳으로부터 몇 언리얼 유닛까지 건축을 허용할 것인지(사거리)를 설정합니다.

### 2.3 `APW_BuildingPieceActor` (건축물 기본 액터)
- **역할:** 맵에 실제로 배치되는 토대(`Foundation`), 벽(`Wall`), 지붕(`Roof`)의 베이스 클래스입니다.
- **주요 기능:**
  - 내구도(`CurrentDurability`, `MaxDurability`), 마테리얼(`RuntimeMaterialOverride`), 부모/자식 스냅 관계를 서버-클라이언트 간 동기화합니다.
  - 파괴 시(`DestroyPiece`) 자신에게 의존하여 스냅되어 있던 자식 건축물들도 연쇄적으로 파괴됩니다.
  - 런타임에 에셋 경로 하드코딩 없이 `Piece Definitions`에 등록된 클래스가 스폰됩니다. 현재 에셋들은 `/Game/_Private/LMK/Asset/EasyBuildingSystem/` 내에 존재합니다.

---

## 3. 플레이어(Player) 블루프린트 연동 가이드

새로운 플레이어 캐릭터나 컨트롤러에 건축 시스템을 붙이려면 다음 순서를 따릅니다:

1. **컴포넌트 부착:** 
   - `BP_PlayerCharacter` (또는 `BP_PlayerController`)를 열고 `PW_BuildingInputComponent`와 `PW_PlayerBuildingPlacementComponent`를 Add Component 합니다.
2. **입력 설정:**
   - 기본적으로 Input Component의 `Auto Bind Input` 옵션이 켜져 있으면 구형 입력 시스템을 통해 B, C, 마우스 버튼들이 자동 연결됩니다.
   - (선택 사항) 만약 Enhanced Input System (향상된 입력)을 사용 중이라면 `Auto Bind Input`을 끄고, 블루프린트 이벤트 그래프에서 키보드/마우스 입력 노드를 꺼내 Input Component의 함수(`ToggleBuildingMenu()`, `ToggleDismantleMode()`, `HandlePrimaryActionPressed()` 등)를 직접 호출해 주면 됩니다.
3. **데이터 등록 (매우 중요):**
   - Placement Component의 디테일 창을 보면 `Piece Definitions`와 `Material Profiles` 배열이 있습니다.
   - C++ 생성자에서 기본적으로 나무, 돌, 철 마테리얼과 토대, 벽, 지붕 클래스를 로드하여 배열에 채워줍니다.
   - **기획이 변경되어 새로운 등급(예: 크리스탈 벽)이나 모양을 추가하고 싶다면,** C++을 건드릴 필요 없이 블루프린트 디테일 창에서 배열 원소를 추가하여 새 클래스와 마테리얼을 지정해주면 즉시 인게임에 반영됩니다.

---

## 4. 알려진 이슈 및 유의사항

- **토대 겹침 검사:**
  - 토대 위에 토대를 겹쳐서 건설하는 어뷰징을 막기 위해, 스냅되지 않는 자유 배치 상태에서는 `PlacementOverlapChannel` (기본: WorldStatic)을 피해 다른 `PW_BuildingPieceActor`와 겹치는지(OverlapMultiByChannel) 검사합니다.
  - 만약 지형이나 바위 등 환경 오브젝트 내부로 토대를 파묻고 싶게 만들려면 오버랩 검사 크기(`PlacementOverlapExtent`)를 조절하거나 콜리전 채널을 분리해야 합니다.
  - 현재 오버랩 크기나 사거리 설정은 모두 `PW_PlayerBuildingPlacementComponent`의 디테일 패널에서 수정할 수 있습니다.
- **해체 하이라이트 머티리얼:**
  - 해체 모드 시 조준된 건축물이 붉은색으로 빛나는 것은 `Piece Definitions`에 등록된 `Preview Invalid Material`을 그대로 재활용한 것입니다. 별도의 해체 전용 마테리얼이 필요하다면 C++의 `UpdateDismantlePreview()` 로직을 수정하거나 Piece Definition 구조체에 새 마테리얼 슬롯을 추가해야 합니다.
