#include "PWPalAIController.h"

#include "Components/StateTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"


APWPalAIController::APWPalAIController()
{
    PalPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PalPerceptionComponent"));
    SetPerceptionComponent(*PalPerceptionComponent);
    
    StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));

    // 시각(Sight) 센서 설정
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
    SightConfig->SightRadius = 1000.0f; // 시야 반경
    SightConfig->LoseSightRadius = 1200.0f; // 시야에서 벗어나는 반경
    SightConfig->PeripheralVisionAngleDegrees = 90.0f; // 시야각 (전방 180도)
    SightConfig->SetMaxAge(5.0f); // 기억 유지 시간 (초)
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 900.0f;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    

    // 청각(Hearing) 센서 설정
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing Config"));
    HearingConfig->HearingRange = 2000.0f; // 소리를 들을 수 있는 반경
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

    // 센서 부착 및 주력 센서(Dominant Sense)를 시각으로 설정
    PalPerceptionComponent->ConfigureSense(*SightConfig);
    PalPerceptionComponent->ConfigureSense(*HearingConfig);
    PalPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void APWPalAIController::BeginPlay()
{
    Super::BeginPlay();

    // 퍼셉션 업데이트 이벤트 연결
    if (PalPerceptionComponent)
    {
        PalPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &APWPalAIController::OnTargetDetected);
    }
}

void APWPalAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 팰에 빙의(Possess)하면, 할당된 StateTree를 가동합니다.
    if (StateTreeComponent)
    {
        StateTreeComponent->StartLogic();
    }
}

void APWPalAIController::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus)
{
    // 대상이 시야/청각에 성공적으로 들어왔는지 확인
    if (Stimulus.WasSuccessfullySensed())
    {
         // TODO: StateTree 파라미터나 컨텍스트에 TargetActor 전달 로직 추가
         // StateTree에서는 자체 컨텍스트 바인딩을 사용하므로 Blackboard 세팅을 제거합니다.
    }
    else
    {
        // TODO : 시야에서 사라졌을 때의 처리 (TargetActor 초기화 등)
    }
}
