#include "PW_Task_Kite.h"
#include "PWPalCharacter.h"
#include "Animation/AnimMontage.h"

const UStruct* FPWStateTreeTask_Kite::GetInstanceDataType() const
{
    return InstanceDataType::StaticStruct();
}

EStateTreeRunStatus FPWStateTreeTask_Kite::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FPW_Task_KiteInstanceData& InstanceData = Context.GetInstanceData<FPW_Task_KiteInstanceData>(*this);
    InstanceData.TimeSinceLastJump = 0.0f;
    InstanceData.MontageDuration = 0.0f;
    InstanceData.JumpsPerformed = 0;

    if (APWPalCharacter* Pal = Cast<APWPalCharacter>(InstanceData.OwnerActor))
    {
        if (InstanceData.KiteMontage && InstanceData.NumberOfJumps > 0)
        {
            // 1번째 점프와 몽타주 시작
            InstanceData.MontageDuration = Pal->PlayAnimMontage(InstanceData.KiteMontage);
            
            FVector JumpDir = (-Pal->GetActorForwardVector() * InstanceData.JumpBackwardForce) + FVector(0, 0, InstanceData.JumpUpwardForce);
            Pal->LaunchCharacter(JumpDir, true, true);
            InstanceData.JumpsPerformed = 1;
            
            if (InstanceData.MontageDuration > 0.0f)
            {
                return EStateTreeRunStatus::Running;
            }
        }
    }
    
    // 몽타주가 없거나 실패 시 바로 종료
    return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FPWStateTreeTask_Kite::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FPW_Task_KiteInstanceData& InstanceData = Context.GetInstanceData<FPW_Task_KiteInstanceData>(*this);

    InstanceData.TimeSinceLastJump += DeltaTime;

    // 하나의 몽타주 재생(점프 한 번)이 끝났을 때
    if (InstanceData.TimeSinceLastJump >= InstanceData.MontageDuration)
    {
        // 목표 횟수만큼 뛰지 않았다면 한 번 더 뛰기!
        if (InstanceData.JumpsPerformed < InstanceData.NumberOfJumps)
        {
            if (APWPalCharacter* Pal = Cast<APWPalCharacter>(InstanceData.OwnerActor))
            {
                InstanceData.MontageDuration = Pal->PlayAnimMontage(InstanceData.KiteMontage);
                
                FVector JumpDir = (-Pal->GetActorForwardVector() * InstanceData.JumpBackwardForce) + FVector(0, 0, InstanceData.JumpUpwardForce);
                Pal->LaunchCharacter(JumpDir, true, true);
                
                InstanceData.JumpsPerformed++;
                InstanceData.TimeSinceLastJump = 0.0f; // 타이머 초기화
                return EStateTreeRunStatus::Running;
            }
        }
        else
        {
            // 목표 점프 횟수를 모두 채우면 성공 종료
            return EStateTreeRunStatus::Succeeded;
        }
    }

    return EStateTreeRunStatus::Running;
}
