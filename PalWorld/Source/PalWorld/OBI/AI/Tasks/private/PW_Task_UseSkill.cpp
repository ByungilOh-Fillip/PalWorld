#include "PW_Task_UseSkill.h"
#include "PWPalCharacter.h"
#include "PWSkillComponent.h"
#include "Animation/AnimMontage.h"

const UStruct* FPWStateTreeTask_UseSkill::GetInstanceDataType() const
{
    return InstanceDataType::StaticStruct();
}

EStateTreeRunStatus FPWStateTreeTask_UseSkill::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FPW_Task_UseSkillInstanceData& InstanceData = Context.GetInstanceData<FPW_Task_UseSkillInstanceData>(*this);
    InstanceData.TimeElapsed = 0.0f;
    InstanceData.MontageDuration = 0.0f;

    if (APWPalCharacter* Pal = Cast<APWPalCharacter>(InstanceData.OwnerActor))
    {
        if (UPWSkillComponent* SkillComp = Pal->GetSkillComponent())
        {
            FPWSkillData SelectedSkill;
            int32 SlotIndex = 0;
            if (SkillComp->GetAvailableRandomSkill(SelectedSkill, SlotIndex))
            {
                if (SelectedSkill.SkillMontage)
                {
                    InstanceData.MontageDuration = Pal->PlayAnimMontage(SelectedSkill.SkillMontage);
                    SkillComp->MarkSkillAsUsed(SlotIndex);
                    
                    // 만약 몽타주 재생을 실패했거나 길이가 없다면 즉시 종료
                    if (InstanceData.MontageDuration <= 0.0f)
                    {
                        return EStateTreeRunStatus::Succeeded;
                    }
                    return EStateTreeRunStatus::Running;
                }
            }
        }
    }
    
    // 사용 가능한 스킬이 없으면 바로 종료하여 다음 Task(Kite 등)로 넘어가게 함
    return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FPWStateTreeTask_UseSkill::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FPW_Task_UseSkillInstanceData& InstanceData = Context.GetInstanceData<FPW_Task_UseSkillInstanceData>(*this);

    InstanceData.TimeElapsed += DeltaTime;

    // 몽타주가 끝날 때까지 대기
    if (InstanceData.TimeElapsed >= InstanceData.MontageDuration)
    {
        return EStateTreeRunStatus::Succeeded;
    }

    return EStateTreeRunStatus::Running;
}
