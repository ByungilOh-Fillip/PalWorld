#include "PW_FindAtkEvaluator.h"
#include "PW_ST_Log.h"
#include "PWPalCharacter.h"
#include "PWPalAIController.h"

const UStruct* FPWStateTreeEvaluator_FindAttacker::GetInstanceDataType() const
{
    return InstanceDataType::StaticStruct();
}

void FPWStateTreeEvaluator_FindAttacker::TreeStart(FStateTreeExecutionContext& Context) const
{
    UE_LOG(LogPalStateTree, Log, TEXT("[PW_FindAtkEvaluator] TreeStart 호출 (상태 트리 시작)"));
}

void FPWStateTreeEvaluator_FindAttacker::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FPW_FindAtkEvaluatorInstanceData& InstanceData = Context.GetInstanceData<FPW_FindAtkEvaluatorInstanceData>(*this);

    if (InstanceData.OwnerActor)
    {
         APWPalCharacter* Pal = Cast<APWPalCharacter>(InstanceData.OwnerActor);
         if (Pal)
         {
              if (APWPalAIController* AICon = Cast<APWPalAIController>(Pal->GetController()))
              {
                  InstanceData.AttackerActor = AICon->CurrentTargetActor;
              }
              else
              {
                  UE_LOG(LogTemp, Error, TEXT("[FindAttacker] AIController 캐스팅 실패! OwnerActor: %s"), *Pal->GetName());
              }
         }
         else
         {
              UE_LOG(LogTemp, Error, TEXT("[FindAttacker] OwnerActor Cast<APWPalCharacter> 실패! 타입: %s"),
                  *InstanceData.OwnerActor->GetClass()->GetName());
         }
    }
}
