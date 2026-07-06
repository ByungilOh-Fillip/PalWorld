#include "StatusComponent.h"
#include "Net/UnrealNetwork.h"

UStatusComponent::UStatusComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true); // 멀티플레이어 MVP를 위한 리플리케이션 활성화
    
    Level = 1;
    
    MaxHP = 100.0f;
    CurrentHP = MaxHP;

    MaxSP = 100.0f;
    CurrentSP = MaxSP;

    MaxHunger = 100.0f;
    CurrentHunger = MaxHunger;

    MaxSanity = 100.0f;
    CurrentSanity = MaxSanity;

    Attack = 10.0f;
    Defense = 10.0f;
    WorkSpeed = 100.0f;
    FeedPerDay = 1;

    MaxCarryWeight = 300.0f;
    CurrentCarryWeight = 0.0f;
}

void UStatusComponent::BeginPlay()
{
    Super::BeginPlay();

    if (GetOwner() && GetOwner()->HasAuthority())
    {
        CurrentHP = MaxHP;
        CurrentSP = MaxSP;
        CurrentHunger = MaxHunger;
        CurrentSanity = MaxSanity;
    }
}

void UStatusComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UStatusComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UStatusComponent, CurrentHP);
    DOREPLIFETIME(UStatusComponent, CurrentSP);
    DOREPLIFETIME(UStatusComponent, CurrentHunger);
    DOREPLIFETIME(UStatusComponent, CurrentSanity);
    DOREPLIFETIME(UStatusComponent, CurrentCarryWeight);
    DOREPLIFETIME(UStatusComponent, ActiveStatusEffects);
}

void UStatusComponent::OnRep_CurrentHP()
{
    HandleCurrentHPChanged();
}

void UStatusComponent::OnRep_CurrentSP()
{
    HandleCurrentSPChanged();
}

void UStatusComponent::OnRep_CurrentHunger()
{
    HandleCurrentHungerChanged();
}

void UStatusComponent::OnRep_CurrentSanity()
{
    HandleCurrentSanityChanged();
}

void UStatusComponent::OnRep_CurrentCarryWeight()
{
    HandleCurrentCarryWeightChanged();
}

void UStatusComponent::OnRep_ActiveStatusEffects()
{
}

void UStatusComponent::HandleCurrentSPChanged()
{
}

void UStatusComponent::HandleCurrentHPChanged()
{
}

void UStatusComponent::HandleCurrentHungerChanged()
{
}

void UStatusComponent::HandleCurrentSanityChanged()
{
}

void UStatusComponent::HandleCurrentCarryWeightChanged()
{
}
