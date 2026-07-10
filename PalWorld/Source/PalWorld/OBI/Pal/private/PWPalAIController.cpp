#include "PWPalAIController.h"

#include "PW_ST_EventsTags.h"
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
        PalPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &APWPalAIController::OnTargetPerceptionUpdated);
    }
}

void APWPalAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    UE_LOG(LogTemp, Warning, TEXT("[PWPalAIController::OnPossess] 팰 빙의 완료: %s, StateTreeComp: %s"),
        InPawn ? *InPawn->GetName() : TEXT("NULL"),
        StateTreeComponent ? TEXT("존재함") : TEXT("NULL ← 문제!"));

    // 팰에 빙의(Possess)하면, 할당된 StateTree를 가동합니다.
    if (StateTreeComponent)
    {
        StateTreeComponent->StartLogic();
        UE_LOG(LogTemp, Warning, TEXT("[PWPalAIController::OnPossess] StartLogic() 호출 완료"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[PWPalAIController::OnPossess] StateTreeComponent가 없음! Blueprint에서 StateTree 에셋이 할당됐는지 확인!"));
    }
}

void APWPalAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus const Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        CurrentTargetActor = Actor; // 감지된 대상을 현재 타겟으로 저장

        FStateTreeEvent TreeEvent;
        TreeEvent.Tag = PW_ST_EventsTags::Event_SenseThreat;

        FST_PerceptionPayload Payload;
        Payload.TargetActor = Actor;
        TreeEvent.Payload = FInstancedStruct::Make(Payload);

        if (StateTreeComponent)
        {
            StateTreeComponent->SendStateTreeEvent(TreeEvent);
        }
    }
    else
    {
        if (CurrentTargetActor == Actor)
        {
            CurrentTargetActor = nullptr; // 시야에서 벗어나면 타겟 초기화
        }

        // 시야에서 사라졌을 때 타겟 상실 이벤트 전송
        FStateTreeEvent TreeEvent;
        TreeEvent.Tag = PW_ST_EventsTags::Event_TargetLost;

        FST_PerceptionPayload Payload;
        Payload.TargetActor = Actor;
        TreeEvent.Payload = FInstancedStruct::Make(Payload);

        if (StateTreeComponent)
        {
            StateTreeComponent->SendStateTreeEvent(TreeEvent);
        }
    }
}
