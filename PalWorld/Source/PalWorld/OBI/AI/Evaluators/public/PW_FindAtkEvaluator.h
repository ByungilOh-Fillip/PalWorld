#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "Blueprint/StateTreeEvaluatorBlueprintBase.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Actor.h"
#include "PW_FindAtkEvaluator.generated.h"

/* ========================================================
 * 1. 데이터 컨테이너 (Instance Data)
 * StateTree 에디터의 세부 창에 노출될 Input / Output 변수들을 정의합니다.
 * ======================================================== */
USTRUCT(BlueprintType)
struct FPW_FindAtkEvaluatorInstanceData
{
    GENERATED_BODY()

    // [Input] 나 자신 (StateTree 에디터의 Context 영역에서 이쪽으로 바인딩해 줍니다)
    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<AActor> OwnerActor = nullptr;

    // [Output] 나를 때린 적 (에디터의 Condition 노드에서 이 변수가 Null인지 아닌지 검사하게 됩니다)
    UPROPERTY(EditAnywhere, Category = "Output")
    TObjectPtr<AActor> AttackerActor = nullptr;
};

/* ========================================================
 * 2. 실제 평가자 로직 (Evaluator)
 * 매 프레임 상황을 판단하여 위의 Instance Data를 업데이트합니다.
 * ======================================================== */
USTRUCT(meta = (DisplayName = "Find Attacker (최근 피격 대상 찾기)", Category = "Pal|AI"))
struct PALWORLD_API FPWStateTreeEvaluator_FindAttacker : public FStateTreeEvaluatorCommonBase
{
    GENERATED_BODY()

    // 내가 사용할 데이터 구조체를 시스템에 알려줍니다.
    typedef FPW_FindAtkEvaluatorInstanceData InstanceDataType;
    
    virtual const UStruct* GetInstanceDataType() const override;

    // 매 틱(Tick)마다 실행되는 정찰 로직
    virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
