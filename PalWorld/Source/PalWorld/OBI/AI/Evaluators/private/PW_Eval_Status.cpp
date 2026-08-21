#include "PW_Eval_Status.h"
#include "PW_ST_Log.h"
#include "PWPalCharacter.h"
#include "StatusComponent.h"

const UStruct* FPWStateTreeEvaluator_Status::GetInstanceDataType() const
{
    return InstanceDataType::StaticStruct();
}

void FPWStateTreeEvaluator_Status::TreeStart(FStateTreeExecutionContext& Context) const
{
    UE_LOG(LogPalStateTree, Log, TEXT("[PW_Eval_Status] TreeStart 호출 (상태 트리 시작)"));
}

void FPWStateTreeEvaluator_Status::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FPW_Eval_StatusInstanceData& InstanceData = Context.GetInstanceData<FPW_Eval_StatusInstanceData>(*this);

    if (InstanceData.OwnerActor)
    {
        if (APWPalCharacter* Pal = Cast<APWPalCharacter>(InstanceData.OwnerActor))
        {
            if (UStatusComponent* StatusComp = Pal->GetStatusComponent())
            {
                // 배고픔 수치가 30 이하일 때 허기짐 상태로 판별 (절대값 비교로 최적화)
                InstanceData.bIsHungry = (StatusComp->CurrentHunger <= 30.0f);
            }
        }
    }
}
