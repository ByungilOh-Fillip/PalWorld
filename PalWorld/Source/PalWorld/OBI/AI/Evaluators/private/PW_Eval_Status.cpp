#include "PW_Eval_Status.h"
#include "PWPalCharacter.h"
#include "StatusComponent.h"

const UStruct* FPWStateTreeEvaluator_Status::GetInstanceDataType() const
{
    return InstanceDataType::StaticStruct();
}

void FPWStateTreeEvaluator_Status::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FPW_Eval_StatusInstanceData& InstanceData = Context.GetInstanceData<FPW_Eval_StatusInstanceData>(*this);

    if (InstanceData.OwnerActor)
    {
        if (APWPalCharacter* Pal = Cast<APWPalCharacter>(InstanceData.OwnerActor))
        {
            // TODO: UStatusComponent에서 실제 배고픔 상태를 가져옵니다.
            // if (UStatusComponent* StatusComp = Pal->GetStatusComponent())
            // {
            //     InstanceData.bIsHungry = StatusComp->IsHungry();
            // }
        }
    }
}
