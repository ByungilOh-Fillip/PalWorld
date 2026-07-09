#include "PWSkillComponent.h"

UPWSkillComponent::UPWSkillComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    // 스킬 관련 데이터가 네트워크로 동기화되어야 할 경우를 대비해 Replication 활성화
    SetIsReplicatedByDefault(true); 
}

#include "PWPalBase.h"
#include "PWPalDataAsset.h"

void UPWSkillComponent::BeginPlay()
{
    Super::BeginPlay();

    // 부모 팰의 PDA에서 기본 장착 스킬 데이터를 복사해옵니다. (쿨타임은 기본값 -9999.0f로 세팅됨)
    if (APWPalBase* PalOwner = Cast<APWPalBase>(GetOwner()))
    {
        if (UPWPalDataAsset* PDA = PalOwner->PalDataAsset)
        {
            LightSkill = PDA->DefaultLightSkill;
            MediumSkill = PDA->DefaultMediumSkill;
            HeavySkill = PDA->DefaultHeavySkill;
        }
    }
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

    APWPalBase* PalOwner = Cast<APWPalBase>(GetOwner());
    if (!PalOwner || !PalOwner->PalDataAsset)
    {
        return false; // 원본 데이터가 없으면 적성을 찾을 수 없음
    }

    for (const FWorkAttitude& WorkAttitude : PalOwner->PalDataAsset->WorkAttitudes)
    {
        if (WorkAttitude.WorkTypeTag.MatchesTagExact(WorkTag))
        {
            OutWorkAttitude = WorkAttitude;
            return true;
        }
    }

    return false;
}

bool UPWSkillComponent::GetAvailableRandomSkill(FPWSkillData& OutSkillData, int32& OutSkillSlot)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    TArray<int32> ReadySlots;

    if (LightSkill.IsReady(CurrentTime))
    {
        ReadySlots.Add(0);
    }
    if (MediumSkill.IsReady(CurrentTime))
    {
        ReadySlots.Add(1);
    }
    if (HeavySkill.IsReady(CurrentTime))
    {
        ReadySlots.Add(2);
    }

    if (ReadySlots.Num() == 0)
    {
        return false;
    }

    // 사용 가능한 스킬 중 무작위 하나 선택
    int32 RandomIndex = FMath::RandRange(0, ReadySlots.Num() - 1);
    OutSkillSlot = ReadySlots[RandomIndex];

    switch (OutSkillSlot)
    {
        case 0: OutSkillData = LightSkill; break;
        case 1: OutSkillData = MediumSkill; break;
        case 2: OutSkillData = HeavySkill; break;
    }

    return true;
}

void UPWSkillComponent::MarkSkillAsUsed(int32 SkillSlot)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    switch (SkillSlot)
    {
        case 0: LightSkill.LastUseTime = CurrentTime; break;
        case 1: MediumSkill.LastUseTime = CurrentTime; break;
        case 2: HeavySkill.LastUseTime = CurrentTime; break;
    }
}
