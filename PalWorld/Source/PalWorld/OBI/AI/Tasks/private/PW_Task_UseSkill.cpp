#include "PW_Task_UseSkill.h"
#include "PW_ST_Log.h"
#include "PWPalCharacter.h"
#include "PWSkillComponent.h"
#include "PWSkillBase.h"
#include "Animation/AnimMontage.h"

const UStruct* FPWStateTreeTask_UseSkill::GetInstanceDataType() const
{
    return InstanceDataType::StaticStruct();
}

EStateTreeRunStatus FPWStateTreeTask_UseSkill::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    UE_LOG(LogPalStateTree, Log, TEXT("[PW_Task_UseSkill] EnterState 진입"));

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
                // 1. 사거리 체크
                if (InstanceData.TargetActor)
                {
                    float Distance = FVector::Dist(Pal->GetActorLocation(), InstanceData.TargetActor->GetActorLocation());
                    if (Distance > SelectedSkill.MaxRange)
                    {
                        // 사거리 밖이면 실패(Failed) 반환 -> StateTree가 Kite나 Approach 등 다른 행동을 선택하도록 유도
                        return EStateTreeRunStatus::Failed;
                    }
                }

                // 2. 몽타주 재생 및 스킬 액터 스폰
                if (SelectedSkill.SkillMontage)
                {
                    InstanceData.MontageDuration = Pal->PlayAnimMontage(SelectedSkill.SkillMontage);
                    SkillComp->MarkSkillAsUsed(SlotIndex);
                    
                    // 스킬 액터 스폰 (스킬 로직 실행)
                    if (SelectedSkill.SkillActorClass && Pal->GetWorld())
                    {
                        FActorSpawnParameters SpawnParams;
                        SpawnParams.Owner = Pal;
                        SpawnParams.Instigator = Pal;
                        // 위치는 우선 팰 위치에 스폰 (투사체, 장판 등은 각 스킬 액터 BeginPlay/ExecuteSkill에서 위치를 조정함)
                        APWSkillBase* SpawnedSkill = Pal->GetWorld()->SpawnActor<APWSkillBase>(SelectedSkill.SkillActorClass, Pal->GetActorLocation(), Pal->GetActorRotation(), SpawnParams);
                        
                        if (SpawnedSkill)
                        {
                            SpawnedSkill->ExecuteSkill(Pal, InstanceData.TargetActor, SelectedSkill.Power);
                        }
                    }

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
