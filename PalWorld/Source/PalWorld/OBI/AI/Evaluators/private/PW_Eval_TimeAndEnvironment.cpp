#include "PW_Eval_TimeAndEnvironment.h"

const UStruct* FPWStateTreeEvaluator_TimeAndEnv::GetInstanceDataType() const
{
    return InstanceDataType::StaticStruct();
}

void FPWStateTreeEvaluator_TimeAndEnv::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FPW_Eval_TimeAndEnvironmentInstanceData& InstanceData = Context.GetInstanceData<FPW_Eval_TimeAndEnvironmentInstanceData>(*this);

    // TODO: 전역 게임 시간 매니저나 월드 타임에 접근하여 시간대를 가져옵니다.
    // 임시로 현재 시간 기준 로직 작성 (추후 TimeManager 연동 요망)
    if (UWorld* World = Context.GetWorld())
    {
        // 예시: 게임 시간이 언리얼 자체 시간 시스템(0~24)을 쓴다고 가정
        // InstanceData.bIsNight = ...;
    }
}
