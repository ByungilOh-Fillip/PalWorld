#include "PW_Task_RandomLocation.h"
#include "NavigationSystem.h"
#include "PWPalBase.h"

const UStruct* FPWStateTreeTask_RandomLocation::GetInstanceDataType() const
{
    return InstanceDataType::StaticStruct();
}

EStateTreeRunStatus FPWStateTreeTask_RandomLocation::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FPW_Task_RandomLocationInstanceData& InstanceData = Context.GetInstanceData<FPW_Task_RandomLocationInstanceData>(*this);

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

            if (NavSys->GetRandomReachablePointInRadius(OriginLocation, SearchRadius, RandomNavLoc))
            {
                InstanceData.TargetLocation = RandomNavLoc.Location;
                return EStateTreeRunStatus::Succeeded; // 목표 위치 계산 완료이므로 성공
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
