#include "PWSkillComponent.h"

UPWSkillComponent::UPWSkillComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    // 스킬 관련 데이터가 네트워크로 동기화되어야 할 경우를 대비해 Replication 활성화
    SetIsReplicatedByDefault(true); 
}

void UPWSkillComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UPWSkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
