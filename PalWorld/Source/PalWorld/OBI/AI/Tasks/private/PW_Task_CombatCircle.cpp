#include "PW_Task_CombatCircle.h"
#include "PW_ST_Log.h"
#include "PWPalCharacter.h"
#include "AIController.h"
#include "NavigationSystem.h"

const UStruct* FPWStateTreeTask_CombatCircle::GetInstanceDataType() const
{
    return InstanceDataType::StaticStruct();
}

EStateTreeRunStatus FPWStateTreeTask_CombatCircle::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    UE_LOG(LogPalStateTree, Log, TEXT("[PW_Task_CombatCircle] EnterState 진입"));

    FPW_Task_CombatCircleInstanceData& InstanceData = Context.GetInstanceData<FPW_Task_CombatCircleInstanceData>(*this);

    UE_LOG(LogTemp, Error, TEXT("[CombatCircle::EnterState] 진입! OwnerActor: %s / TargetActor: %s"),
        InstanceData.OwnerActor ? *InstanceData.OwnerActor->GetName() : TEXT("NULL"),
        InstanceData.TargetActor ? *InstanceData.TargetActor->GetName() : TEXT("NULL"));

    InstanceData.TimeElapsed = 0.0f;
    InstanceData.RandomDuration = FMath::RandRange(InstanceData.MinDuration, InstanceData.MaxDuration);
    
    // 타겟 주위의 랜덤한 원형 좌표를 구해서 이동 명령을 내립니다.
    if (InstanceData.OwnerActor && InstanceData.TargetActor)
    {
        if (AAIController* AICon = Cast<AAIController>(Cast<APawn>(InstanceData.OwnerActor)->GetController()))
        {
            FVector TargetLoc = InstanceData.TargetActor->GetActorLocation();
            
            // 타겟을 중심으로 360도 중 임의의 방향으로 반경(CircleRadius)만큼 떨어진 점을 구함
            FVector2D RandomDir = FMath::RandPointInCircle(1.0f);
            FVector Destination = TargetLoc + FVector(RandomDir.X, RandomDir.Y, 0.0f).GetSafeNormal() * InstanceData.CircleRadius;

            // 목적지로 이동 (내비게이션 메시 활용)
            AICon->MoveToLocation(Destination, 50.0f);
        }
    }

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FPWStateTreeTask_CombatCircle::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FPW_Task_CombatCircleInstanceData& InstanceData = Context.GetInstanceData<FPW_Task_CombatCircleInstanceData>(*this);

    InstanceData.TimeElapsed += DeltaTime;
    
    if (InstanceData.TimeElapsed >= InstanceData.RandomDuration)
    {
        return EStateTreeRunStatus::Succeeded;
    }

    // 대상(플레이어)을 계속 바라보도록 회전 처리
    if (InstanceData.OwnerActor && InstanceData.TargetActor)
    {
        if (AAIController* AICon = Cast<AAIController>(Cast<APawn>(InstanceData.OwnerActor)->GetController()))
        {
            AICon->SetFocus(InstanceData.TargetActor);
        }
    }

    return EStateTreeRunStatus::Running;
}

void FPWStateTreeTask_CombatCircle::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FPW_Task_CombatCircleInstanceData& InstanceData = Context.GetInstanceData<FPW_Task_CombatCircleInstanceData>(*this);
    
    // 상태를 빠져나갈 때 이동과 포커스 중지
    if (InstanceData.OwnerActor)
    {
        if (AAIController* AICon = Cast<AAIController>(Cast<APawn>(InstanceData.OwnerActor)->GetController()))
        {
            AICon->StopMovement();
            AICon->ClearFocus(EAIFocusPriority::Gameplay);
        }
    }
}
