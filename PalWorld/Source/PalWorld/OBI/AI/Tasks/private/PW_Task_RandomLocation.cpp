#include "PW_Task_RandomLocation.h"
#include "PW_ST_Log.h"
#include "NavigationSystem.h"
#include "PWPalBase.h"

const UStruct* FPWStateTreeTask_RandomLocation::GetInstanceDataType() const
{
    return InstanceDataType::StaticStruct();
}

EStateTreeRunStatus FPWStateTreeTask_RandomLocation::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    UE_LOG(LogPalStateTree, Log, TEXT("[PW_Task_RandomLocation] EnterState 진입"));

    FPW_Task_RandomLocationInstanceData& InstanceData = Context.GetInstanceData<FPW_Task_RandomLocationInstanceData>(*this);

    if (GEngine) GEngine->AddOnScreenDebugMessage(2001, 5.f, FColor::Yellow,
        FString::Printf(TEXT("[RandomLocation] 시작! OwnerActor: %s"),
            InstanceData.OwnerActor ? *InstanceData.OwnerActor->GetName() : TEXT("NULL ← 바인딩 문제!")));

    if (APWPalBase* Pal = Cast<APWPalBase>(InstanceData.OwnerActor))
    {
        if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Context.GetWorld()))
        {
            FNavLocation RandomNavLoc;
            FVector OriginLocation = Pal->GetActorLocation();
            float SearchRadius = InstanceData.SearchRadius;

            // 팰이 거점에 소속되어 있다면 거점을 중심으로 반경을 제한함
            if (Pal->IsAssignedToBaseCamp() && Pal->BaseCampActor)
            {
                OriginLocation = Pal->BaseCampActor->GetActorLocation();
                SearchRadius = Pal->BaseCampRadius;
            }

            // NavMesh에서 유효한 위치를 최대 5회 시도
            const int32 MaxRetries = 5;
            for (int32 i = 0; i < MaxRetries; ++i)
            {
                if (NavSys->GetRandomReachablePointInRadius(OriginLocation, SearchRadius, RandomNavLoc))
                {
                    InstanceData.TargetLocation = RandomNavLoc.Location;
                    return EStateTreeRunStatus::Succeeded;
                }
            }

            // 5회 실패 시 현재 위치 근처 소반경에서 재시도
            if (NavSys->GetRandomReachablePointInRadius(OriginLocation, 100.0f, RandomNavLoc))
            {
                InstanceData.TargetLocation = RandomNavLoc.Location;
                return EStateTreeRunStatus::Succeeded;
            }
        }
    }
    
    // 계산 실패 시 원래 위치 그대로 유지
    if (InstanceData.OwnerActor)
    {
        InstanceData.TargetLocation = InstanceData.OwnerActor->GetActorLocation();
    }
    return EStateTreeRunStatus::Failed;
}
