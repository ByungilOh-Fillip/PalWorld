#include "PWBTTask_FindRandomLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "AIController.h"

UPWBTTask_FindRandomLocation::UPWBTTask_FindRandomLocation()
{
    // 비헤이비어 트리 에디터에서 보일 이름 설정
    NodeName = TEXT("Find Random Location");
    SearchRadius = 1500.0f; // 기본 탐색 반경
}

EBTNodeResult::Type UPWBTTask_FindRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // AI 컨트롤러와 폰(팰) 가져오기
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn) return EBTNodeResult::Failed;

    // 현재 팰의 위치를 탐색 기준점(Origin)으로 설정
    FVector Origin = ControlledPawn->GetActorLocation();
    FNavLocation RandomLocation;

    // 현재 월드의 네비게이션 시스템 가져오기
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    
    // 네비게이션 메쉬 위에서 탐색 반경 내의 무작위 위치를 하나 찾아냄
    if (NavSystem && NavSystem->GetRandomPointInNavigableRadius(Origin, SearchRadius, RandomLocation))
    {
        // 찾아낸 좌표를 블랙보드의 지정된 Key (TargetLocation)에 덮어쓰기
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), RandomLocation.Location);
        
        return EBTNodeResult::Succeeded;
    }

    // 길을 못 찾으면 실패 반환 (배회 시퀀스 리트라이 등 처리 가능)
    return EBTNodeResult::Failed;
}
