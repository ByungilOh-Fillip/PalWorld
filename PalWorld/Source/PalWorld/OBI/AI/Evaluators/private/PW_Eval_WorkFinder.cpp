#include "PW_Eval_WorkFinder.h"
#include "PWPalCharacter.h"
#include "PWSkillComponent.h"

const UStruct* FPWStateTreeEvaluator_WorkFinder::GetInstanceDataType() const
{
    return InstanceDataType::StaticStruct();
}

void FPWStateTreeEvaluator_WorkFinder::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FPW_Eval_WorkFinderInstanceData& InstanceData = Context.GetInstanceData<FPW_Eval_WorkFinderInstanceData>(*this);

    if (InstanceData.OwnerActor)
    {
        if (APWPalCharacter* Pal = Cast<APWPalCharacter>(InstanceData.OwnerActor))
        {
            // TODO: 거점 매니저(BaseCampManager)에 접근하여 이 팰의 적성과 일치하는 최우선순위 작업을 찾습니다.
            // Priority Queue (가중치 기반 타겟 탐색) 로직
            // 1. 배고픔 상태 확인 (최우선순위 100)
            if (InstanceData.bIsHungry)
            {
                // TODO: 밥통 액터를 찾아서 InstanceData.TargetWorkActor에 넣고, 먹기 몽타주 세팅
                // return;
            }

            // 2. 밤 상태 확인 (차순위 90)
            if (InstanceData.bIsNight)
            {
                // TODO: 침대 액터를 찾아서 세팅
                // return;
            }

            // 3. 일반 거점 작업 탐색 (가중치 50 등)
            // if (UPWSkillComponent* SkillComp = Pal->GetSkillComponent())
            // {
            //      TODO: 거점 매니저에서 적성(WorkAttitude)에 맞는 작업 탐색
            //      InstanceData.TargetWorkActor = FoundWork;
            //      InstanceData.FoundWorkMontage = FoundAttitude.WorkMontage;
            //      InstanceData.bHasValidWork = (FoundWork != nullptr);
            // }
        }
    }
}
