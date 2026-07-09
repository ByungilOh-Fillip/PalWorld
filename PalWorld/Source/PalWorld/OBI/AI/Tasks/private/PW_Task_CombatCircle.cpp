#include "PW_Task_CombatCircle.h"
#include "PWPalCharacter.h"
#include "AIController.h"

const UStruct* FPWStateTreeTask_CombatCircle::GetInstanceDataType() const
{
    return InstanceDataType::StaticStruct();
}

EStateTreeRunStatus FPWStateTreeTask_CombatCircle::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FPW_Task_CombatCircleInstanceData& InstanceData = Context.GetInstanceData<FPW_Task_CombatCircleInstanceData>(*this);
    InstanceData.TimeElapsed = 0.0f;
    InstanceData.RandomDuration = FMath::RandRange(InstanceData.MinDuration, InstanceData.MaxDuration);
    
    // 시작 시 한 번의 타겟 계산 및 이동 명령
    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FPWStateTreeTask_CombatCircle::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FPW_Task_CombatCircleInstanceData& InstanceData = Context.GetInstanceData<FPW_Task_CombatCircleInstanceData>(*this);

    InstanceData.TimeElapsed += DeltaTime;
    
    if (InstanceData.TimeElapsed >= InstanceData.RandomDuration)
    {
        return EStateTreeRunStatus::Succeeded;
    }

    // TODO: AIController의 MoveTo()나 PathFollowingComponent를 사용하여 TargetActor 주위를 원형으로 돌게 합니다.
    return EStateTreeRunStatus::Running;
}

void FPWStateTreeTask_CombatCircle::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    // 정리 작업 (예: 이동 멈춤)
}
