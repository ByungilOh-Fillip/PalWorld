#include "PW_Task_CalcFleeLocation.h"
#include "PW_ST_Log.h"
#include "NavigationSystem.h"

const UStruct* FPWStateTreeTask_CalcFleeLocation::GetInstanceDataType() const
{
    return InstanceDataType::StaticStruct();
}

EStateTreeRunStatus FPWStateTreeTask_CalcFleeLocation::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    UE_LOG(LogPalStateTree, Log, TEXT("[PW_Task_CalcFleeLocation] EnterState 진입"));

    FPW_Task_CalcFleeLocationInstanceData& InstanceData = Context.GetInstanceData<FPW_Task_CalcFleeLocationInstanceData>(*this);

    if (InstanceData.OwnerActor && InstanceData.AttackerActor)
    {
        FVector OwnerLoc = InstanceData.OwnerActor->GetActorLocation();
        FVector AttackerLoc = InstanceData.AttackerActor->GetActorLocation();
        
        // 공격자 반대 방향으로 벡터 계산
        FVector FleeDir = (OwnerLoc - AttackerLoc).GetSafeNormal();
        if (FleeDir.IsNearlyZero())
        {
            // 위치가 완전히 같다면 임의의 방향으로
            FleeDir = FVector(1, 0, 0); 
        }

        FVector FleeTarget = OwnerLoc + (FleeDir * InstanceData.FleeDistance);

        // 네비게이션 시스템을 통해 실제 이동 가능한 가까운 점 찾기
        if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Context.GetWorld()))
        {
            FNavLocation NavLoc;
            if (NavSys->ProjectPointToNavigation(FleeTarget, NavLoc))
            {
                InstanceData.TargetLocation = NavLoc.Location;
                return EStateTreeRunStatus::Succeeded;
            }
        }
    }
    
    // 계산 실패 시 제자리
    if (InstanceData.OwnerActor)
    {
        InstanceData.TargetLocation = InstanceData.OwnerActor->GetActorLocation();
    }
    return EStateTreeRunStatus::Failed;
}
