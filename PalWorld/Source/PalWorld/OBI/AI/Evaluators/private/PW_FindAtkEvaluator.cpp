#include "PW_FindAtkEvaluator.h"

#include "PWPalCharacter.h"

const UStruct* FPWStateTreeEvaluator_FindAttacker::GetInstanceDataType() const
{
    return InstanceDataType::StaticStruct();
}

void FPWStateTreeEvaluator_FindAttacker::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    // 1. 컨텍스트에서 우리가 정의한 인스턴스 데이터를 꺼내옵니다.
    FPW_FindAtkEvaluatorInstanceData& InstanceData = Context.GetInstanceData<FPW_FindAtkEvaluatorInstanceData>(*this);

    // 2. 주인이 누군지 확인 (Input으로 들어온 값)
    if (InstanceData.OwnerActor)
    {

         APWPalCharacter* Pal = Cast<APWPalCharacter>(InstanceData.OwnerActor);

         if (Pal)
         {
              // Output 변수에 값을 채워 넣습니다! (이제 StateTree가 이 값을 볼 수 있음)
              // InstanceData.AttackerActor = Pal->GetLastAttacker();
         }

    }
}
