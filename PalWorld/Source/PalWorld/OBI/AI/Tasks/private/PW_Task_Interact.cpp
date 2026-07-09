#include "PW_Task_Interact.h"
#include "PWPalCharacter.h"
#include "Animation/AnimMontage.h"

const UStruct* FPWStateTreeTask_Interact::GetInstanceDataType() const
{
    return InstanceDataType::StaticStruct();
}

EStateTreeRunStatus FPWStateTreeTask_Interact::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FPW_Task_InteractInstanceData& InstanceData = Context.GetInstanceData<FPW_Task_InteractInstanceData>(*this);
    InstanceData.TimeElapsed = 0.0f;
    InstanceData.MontageDuration = 0.0f;

    if (APWPalCharacter* Pal = Cast<APWPalCharacter>(InstanceData.OwnerActor))
    {
        if (InstanceData.InteractMontage)
        {
            InstanceData.MontageDuration = Pal->PlayAnimMontage(InstanceData.InteractMontage);
            
            if (InstanceData.MontageDuration > 0.0f)
            {
                return EStateTreeRunStatus::Running;
            }
        }
    }
    
    // 몽타주가 없거나 실패 시 Succeeded 처리
    return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FPWStateTreeTask_Interact::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FPW_Task_InteractInstanceData& InstanceData = Context.GetInstanceData<FPW_Task_InteractInstanceData>(*this);

    InstanceData.TimeElapsed += DeltaTime;

    if (InstanceData.TimeElapsed >= InstanceData.MontageDuration)
    {
        // 몽타주 재생이 끝나면 완료
        return EStateTreeRunStatus::Succeeded;
    }

    return EStateTreeRunStatus::Running;
}
