#include "PW_Task_Rest.h"
#include "PWPalCharacter.h"

const UStruct* FPWStateTreeTask_Rest::GetInstanceDataType() const
{
    return InstanceDataType::StaticStruct();
}

EStateTreeRunStatus FPWStateTreeTask_Rest::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FPW_Task_RestInstanceData& InstanceData = Context.GetInstanceData<FPW_Task_RestInstanceData>(*this);
    
    InstanceData.TimeElapsed = 0.0f;
    InstanceData.bIsEnding = false;
    InstanceData.RandomDuration = FMath::RandRange(InstanceData.MinRestTime, InstanceData.MaxRestTime);

    if (InstanceData.OwnerActor)
    {
        if (APWPalCharacter* Pal = Cast<APWPalCharacter>(InstanceData.OwnerActor))
        {
            if (InstanceData.RestMontage)
            {
                Pal->PlayAnimMontage(InstanceData.RestMontage);
            }
        }
    }

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FPWStateTreeTask_Rest::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FPW_Task_RestInstanceData& InstanceData = Context.GetInstanceData<FPW_Task_RestInstanceData>(*this);

    // 2. 이미 End 섹션으로 넘어간 경우 (기지개 켜고 일어나는 중)
    if (InstanceData.bIsEnding)
    {
        // 몽타주가 완전히 끝났는지 확인 (현재 재생 중인 몽타주가 내 몽타주가 아니면 끝난 것)
        if (APWPalCharacter* Pal = Cast<APWPalCharacter>(InstanceData.OwnerActor))
        {
            if (Pal->GetCurrentMontage() != InstanceData.RestMontage)
            {
                return EStateTreeRunStatus::Succeeded; // 진짜로 휴식 완전 종료!
            }
        }
        return EStateTreeRunStatus::Running; // 아직 일어나는 모션 재생 중
    }

    // 1. 아직 자고 있는 경우 (Start -> Loop 반복 중)
    InstanceData.TimeElapsed += DeltaTime;

    if (InstanceData.TimeElapsed >= InstanceData.RandomDuration)
    {
        // 잘 시간이 다 되었으므로 End 섹션으로 점프!
        InstanceData.bIsEnding = true;
        
        if (APWPalCharacter* Pal = Cast<APWPalCharacter>(InstanceData.OwnerActor))
        {
            if (UAnimInstance* AnimInst = Pal->GetMesh()->GetAnimInstance())
            {
                // 강제로 "End" 섹션으로 넘어가게 합니다.
                AnimInst->Montage_JumpToSection(FName("End"), InstanceData.RestMontage);
            }
        }
        
        // 당장 종료하지 않고, End 애니메이션이 끝날 때까지 기다리기 위해 이번 틱은 Running 반환
        return EStateTreeRunStatus::Running;
    }

    return EStateTreeRunStatus::Running;
}

void FPWStateTreeTask_Rest::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FPW_Task_RestInstanceData& InstanceData = Context.GetInstanceData<FPW_Task_RestInstanceData>(*this);

    // 정상적으로 End 모션까지 끝나서 종료되었을 때는 굳이 Stop을 부르지 않아도 됨.
    // 하지만 도중에 공격을 받거나 맞아서 억지로 State가 종료되는 경우를 대비해 Stop 호출.
    if (InstanceData.OwnerActor)
    {
        if (APWPalCharacter* Pal = Cast<APWPalCharacter>(InstanceData.OwnerActor))
        {
            if (InstanceData.RestMontage && Pal->GetCurrentMontage() == InstanceData.RestMontage)
            {
                Pal->StopAnimMontage(InstanceData.RestMontage);
            }
        }
    }
}
