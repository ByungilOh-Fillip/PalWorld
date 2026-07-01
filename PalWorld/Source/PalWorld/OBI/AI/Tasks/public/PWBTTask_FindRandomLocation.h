#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "PWBTTask_FindRandomLocation.generated.h"

/**
 * @class UPWBTTask_FindRandomLocation
 * @brief NavMesh를 기반으로 반경 내 무작위 위치를 찾아 블랙보드(TargetLocation)에 기록하는 태스크
 */

UCLASS()
class PALWORLD_API UPWBTTask_FindRandomLocation : public UBTTask_BlackboardBase
{
    GENERATED_BODY()

public:
    UPWBTTask_FindRandomLocation();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    // 무작위 위치를 찾을 반경 (블루프린트 에디터 노드에서 수정 가능)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float SearchRadius;
};
