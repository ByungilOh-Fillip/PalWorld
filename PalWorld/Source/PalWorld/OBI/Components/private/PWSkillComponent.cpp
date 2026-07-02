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

bool UPWSkillComponent::CanWork(FGameplayTag WorkTag) const
{
    FWorkAttitude WorkAttitude;
    return FindWorkAttitude(WorkTag, WorkAttitude);
}

bool UPWSkillComponent::FindWorkAttitude(FGameplayTag WorkTag, FWorkAttitude& OutWorkAttitude) const
{
    if (!WorkTag.IsValid())
    {
        return false;
    }

    for (const FWorkAttitude& WorkAttitude : WorkAttitudes)
    {
        if (WorkAttitude.WorkTypeTag.MatchesTagExact(WorkTag))
        {
            OutWorkAttitude = WorkAttitude;
            return true;
        }
    }

    return false;
}
