#include "PW_Eval_WorkFinder.h"
#include "PW_ST_Log.h"
#include "PWPalCharacter.h"
#include "PWSkillComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

const UStruct* FPWStateTreeEvaluator_WorkFinder::GetInstanceDataType() const
{
    return InstanceDataType::StaticStruct();
}

void FPWStateTreeEvaluator_WorkFinder::TreeStart(FStateTreeExecutionContext& Context) const
{
    UE_LOG(LogPalStateTree, Log, TEXT("[PW_Eval_WorkFinder] TreeStart 호출 (상태 트리 시작)"));
}

// 태그 기반으로 가장 가까운 액터 탐색 헬퍼 함수
static AActor* FindNearestActorWithTag(UWorld* World, const FVector& Origin, FName Tag)
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithTag(World, Tag, FoundActors);

    AActor* NearestActor = nullptr;
    float MinDistSq = MAX_flt;

    for (AActor* Actor : FoundActors)
    {
        if (Actor)
        {
            float DistSq = FVector::DistSquared(Origin, Actor->GetActorLocation());
            if (DistSq < MinDistSq)
            {
                MinDistSq = DistSq;
                NearestActor = Actor;
            }
        }
    }
    return NearestActor;
}

void FPWStateTreeEvaluator_WorkFinder::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FPW_Eval_WorkFinderInstanceData& InstanceData = Context.GetInstanceData<FPW_Eval_WorkFinderInstanceData>(*this);

    if (InstanceData.OwnerActor)
    {
        if (APWPalCharacter* Pal = Cast<APWPalCharacter>(InstanceData.OwnerActor))
        {
            // 화면에 StateTree가 돌아가고 있는지 디버그 메시지를 띄웁니다.
            if (GEngine)
            {
                FString DebugMsg = FString::Printf(TEXT("[StateTree Running!] Pal: %s / IsNight: %s / IsHungry: %s"),
                    *Pal->GetName(),
                    InstanceData.bIsNight ? TEXT("True") : TEXT("False"),
                    InstanceData.bIsHungry ? TEXT("True") : TEXT("False"));

                GEngine->AddOnScreenDebugMessage(1001, 0.5f, FColor::Green, DebugMsg);
            }

            UWorld* World = Pal->GetWorld();
            if (!World) return;

            FVector PalLocation = Pal->GetActorLocation();

            // Priority Queue (가중치 기반 타겟 탐색) 로직
            
            // 1순위: 밤 상태 확인 (수면 우선)
            if (InstanceData.bIsNight)
            {
                InstanceData.TargetWorkActor = FindNearestActorWithTag(World, PalLocation, FName("Bed"));
                InstanceData.bHasValidWork = (InstanceData.TargetWorkActor != nullptr);
                return; // 최우선순위를 찾았으므로 (혹은 시도했으므로) 종료
            }

            // 2순위: 배고픔 상태 확인
            if (InstanceData.bIsHungry)
            {
                InstanceData.TargetWorkActor = FindNearestActorWithTag(World, PalLocation, FName("FoodBox"));
                InstanceData.bHasValidWork = (InstanceData.TargetWorkActor != nullptr);
                return; 
            }

            // 3순위: 일반 거점 작업 탐색
            InstanceData.TargetWorkActor = FindNearestActorWithTag(World, PalLocation, FName("WorkStation"));
            InstanceData.bHasValidWork = (InstanceData.TargetWorkActor != nullptr);
            
            // 향후 적성에 맞는 몽타주 세팅 등을 여기서 진행
            // if (InstanceData.bHasValidWork && Pal->GetSkillComponent()) { ... }
        }
    }
}
