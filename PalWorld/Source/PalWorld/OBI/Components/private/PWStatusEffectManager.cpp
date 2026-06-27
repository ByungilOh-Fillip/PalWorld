#include "PWStatusEffectManager.h"
#include "StatusComponent.h"
#include "GameFramework/Actor.h"

UPWStatusEffectManager::UPWStatusEffectManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UPWStatusEffectManager::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        OwnerStatusComponent = Owner->FindComponentByClass<UStatusComponent>();
    }
}

void UPWStatusEffectManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // TODO: 매 틱마다 OwnerStatusComponent->ActiveStatusEffects 를 순회하며 도트 데미지 등을 처리
}

void UPWStatusEffectManager::ApplyStatusEffect(FGameplayTag EffectTag)
{
    if (!OwnerStatusComponent || !GetOwner()->HasAuthority()) return;

    // TODO: 대상이 Player인지 Pal인지 검사하여 무효화(Reject) 규칙 적용 (예: 향수병은 NPC에게만)

    OwnerStatusComponent->ActiveStatusEffects.AddTag(EffectTag);
}

void UPWStatusEffectManager::RemoveStatusEffect(FGameplayTag EffectTag)
{
    if (!OwnerStatusComponent || !GetOwner()->HasAuthority()) return;

    OwnerStatusComponent->ActiveStatusEffects.RemoveTag(EffectTag);
}

bool UPWStatusEffectManager::HasStatusEffect(FGameplayTag EffectTag) const
{
    if (!OwnerStatusComponent) return false;

    return OwnerStatusComponent->ActiveStatusEffects.HasTag(EffectTag);
}
