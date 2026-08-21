#include "PW_Eval_TimeAndEnvironment.h"
#include "PW_ST_Log.h"
#include "Interfaces/PW_WorldStateProvider.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"

const UStruct* FPWStateTreeEvaluator_TimeAndEnv::GetInstanceDataType() const
{
    return InstanceDataType::StaticStruct();
}

void FPWStateTreeEvaluator_TimeAndEnv::TreeStart(FStateTreeExecutionContext& Context) const
{
    UE_LOG(LogPalStateTree, Log, TEXT("[PW_Eval_TimeAndEnvironment] TreeStart 호출 (상태 트리 시작)"));
}

void FPWStateTreeEvaluator_TimeAndEnv::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FPW_Eval_TimeAndEnvironmentInstanceData& InstanceData = Context.GetInstanceData<FPW_Eval_TimeAndEnvironmentInstanceData>(*this);

    if (UWorld* World = Context.GetWorld())
    {
        if (AGameStateBase* GameState = UGameplayStatics::GetGameState(World))
        {
            if (GameState->Implements<UPW_WorldStateProvider>())
            {
                InstanceData.bIsNight = IPW_WorldStateProvider::Execute_IsNight(GameState);
            }
        }
    }
}
