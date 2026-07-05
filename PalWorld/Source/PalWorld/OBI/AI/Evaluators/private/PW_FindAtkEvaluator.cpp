

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "GameFramework/Actor.h"
#include "PW_FindAtkEvaluator.generated.h"

/* ========================================================
 * 1. 데이터 컨테이너 (Instance Data)
 * StateTree 에디터의 세부 창에 노출될 Input / Output 변수들을 정의합니다.
 * ======================================================== */
USTRUCT()
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
USTRUCT(meta = (DisplayName = "Find Attacker (최근 피격 대상 찾기)"))
struct PALWORLD_API FPWStateTreeEvaluator_FindAttacker : public FStateTreeEvaluatorBase
{
    GENERATED_BODY()

    // 내가 사용할 데이터 구조체를 시스템에 알려줍니다.
    using FInstanceDataType = FPW_FindAtkEvaluatorInstanceData;
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    // 매 틱(Tick)마다 실행되는 정찰 로직
    virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override
    {
        // 1. 컨텍스트에서 우리가 정의한 인스턴스 데이터를 꺼내옵니다.
        FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

        // 2. 주인이 누군지 확인 (Input으로 들어온 값)
        if (InstanceData.OwnerActor)
        {
            // 3. 실제 게임 로직: 팰 캐릭터로 형변환해서 '최근 데미지 준 적'을 알아냅니다.
            /*
             * APWPalCharacter* Pal = Cast<APWPalCharacter>(InstanceData.OwnerActor);
             * if (Pal)
             * {
             *     // Output 변수에 값을 채워 넣습니다! (이제 StateTree가 이 값을 볼 수 있음)
             *     InstanceData.AttackerActor = Pal->GetLastAttacker();
             * }
             */
        }
    }
};
