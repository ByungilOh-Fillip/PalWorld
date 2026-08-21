#include "Temperature/PW_TemperatureVolume.h"

#include "Components/BoxComponent.h"
#include "PW_GameplayTags.h"
#include "Interfaces/PW_GameplayTagStatusTarget.h"
#include "World/PW_WorldGameState.h"

APW_TemperatureVolume::APW_TemperatureVolume()
{
	bReplicates = true;

	TemperatureBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("TemperatureBounds"));
	SetRootComponent(TemperatureBounds);

	TemperatureBounds->SetBoxExtent(FVector(500.0f, 500.0f, 200.0f));
	TemperatureBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TemperatureBounds->SetCollisionResponseToAllChannels(ECR_Overlap);
}

void APW_TemperatureVolume::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		TemperatureBounds->OnComponentBeginOverlap.AddDynamic(this, &APW_TemperatureVolume::OnTemperatureBeginOverlap);
		TemperatureBounds->OnComponentEndOverlap.AddDynamic(this, &APW_TemperatureVolume::OnTemperatureEndOverlap);
	}
}

void APW_TemperatureVolume::OnTemperatureBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (OtherActor == nullptr || OtherActor == this)
	{
		return;
	}

	ApplyTemperatureToActor(OtherActor);
}

void APW_TemperatureVolume::OnTemperatureEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex)
{
	if (OtherActor == nullptr)
	{
		return;
	}

	ClearTemperatureFromActor(OtherActor);
}

float APW_TemperatureVolume::CalculateEffectiveTemperature() const
{
	float EffectiveTemperature = BaseTemperature;

	const UWorld* World = GetWorld();
	const APW_WorldGameState* WorldGameState = World != nullptr ? World->GetGameState<APW_WorldGameState>() : nullptr;
	if (WorldGameState != nullptr && IPW_WorldStateProvider::Execute_IsNight(WorldGameState))
	{
		EffectiveTemperature += NightTemperatureModifier;
	}

	return EffectiveTemperature;
}

void APW_TemperatureVolume::ApplyTemperatureToActor(AActor* TargetActor) const
{
	if (TargetActor == nullptr || !TargetActor->GetClass()->ImplementsInterface(UPW_GameplayTagStatusTarget::StaticClass()))
	{
		return;
	}

	const float EffectiveTemperature = CalculateEffectiveTemperature();
	if (EffectiveTemperature <= ColdThreshold)
	{
		IPW_GameplayTagStatusTarget::Execute_ApplyStatusTag(TargetActor, PW_GameplayTags::Status_Explo_Cold);
		IPW_GameplayTagStatusTarget::Execute_RemoveStatusTag(TargetActor, PW_GameplayTags::Status_Explo_Heat);
		return;
	}

	if (EffectiveTemperature >= HeatThreshold)
	{
		IPW_GameplayTagStatusTarget::Execute_ApplyStatusTag(TargetActor, PW_GameplayTags::Status_Explo_Heat);
		IPW_GameplayTagStatusTarget::Execute_RemoveStatusTag(TargetActor, PW_GameplayTags::Status_Explo_Cold);
		return;
	}

	ClearTemperatureFromActor(TargetActor);
}

void APW_TemperatureVolume::ClearTemperatureFromActor(AActor* TargetActor) const
{
	if (TargetActor == nullptr || !TargetActor->GetClass()->ImplementsInterface(UPW_GameplayTagStatusTarget::StaticClass()))
	{
		return;
	}

	IPW_GameplayTagStatusTarget::Execute_RemoveStatusTag(TargetActor, PW_GameplayTags::Status_Explo_Cold);
	IPW_GameplayTagStatusTarget::Execute_RemoveStatusTag(TargetActor, PW_GameplayTags::Status_Explo_Heat);
}
