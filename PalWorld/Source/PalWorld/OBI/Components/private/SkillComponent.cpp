#include "SkillComponent.h"

USkillComponent::USkillComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    // 스킬 관련 데이터가 네트워크로 동기화되어야 할 경우를 대비해 Replication 활성화
    SetIsReplicatedByDefault(true); 
}

void USkillComponent::BeginPlay()
{
    Super::BeginPlay();
}

void USkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
