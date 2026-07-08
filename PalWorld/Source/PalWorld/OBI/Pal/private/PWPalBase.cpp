#include "PWPalBase.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "StatusComponent.h"
#include "PWSkillComponent.h"

APWPalBase::APWPalBase()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // 멀티플레이어 통신을 위한 Actor 리플리케이션 켜기
    bReplicates = true;

    // 컴포넌트 생성 및 부착
    StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
    SkillComponent = CreateDefaultSubobject<UPWSkillComponent>(TEXT("SkillComponent"));
}

void APWPalBase::BeginPlay()
{
    Super::BeginPlay();
}

void APWPalBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APWPalBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APWPalBase, bCaptureInteractionDisabled);
}

void APWPalBase::SetCaptureInteractionDisabled(bool bDisabled)
{
    if (!HasAuthority() || bCaptureInteractionDisabled == bDisabled)
    {
        return;
    }

    bCaptureInteractionDisabled = bDisabled;
    ApplyCaptureInteractionDisabled();
    ForceNetUpdate();
}

void APWPalBase::OnRep_CaptureInteractionDisabled()
{
    ApplyCaptureInteractionDisabled();
}

void APWPalBase::ApplyCaptureInteractionDisabled()
{
    SetActorHiddenInGame(bCaptureInteractionDisabled);
    SetActorEnableCollision(!bCaptureInteractionDisabled);
    SetActorTickEnabled(!bCaptureInteractionDisabled);

    TArray<UActorComponent*> Components;
    GetComponents(Components);
    for (UActorComponent* Component : Components)
    {
        if (Component)
        {
            Component->SetComponentTickEnabled(!bCaptureInteractionDisabled);
        }
    }
}
