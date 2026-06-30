#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PW_PlayerBasePlacementComponent.generated.h"

class APW_BaseCampActor;

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_PlayerBasePlacementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_PlayerBasePlacementComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Placement")
	void TogglePlacementMode();

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Placement")
	void ConfirmPlacement();

	UFUNCTION(BlueprintCallable, Category = "PW|Base|Placement")
	void CancelPlacement();

	UFUNCTION(BlueprintPure, Category = "PW|Base|Placement")
	FTransform GetCurrentPlacementTransform() const { return CurrentPlacementTransform; }

	UFUNCTION(BlueprintPure, Category = "PW|Base|Placement")
	bool CanPlaceAtCurrentPreview() const { return bCanPlaceAtCurrentPreview; }

	UFUNCTION(BlueprintPure, Category = "PW|Base|Placement")
	bool IsPlacementModeActive() const { return bPlacementModeActive; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Base|Placement")
	TSubclassOf<APW_BaseCampActor> BaseCampClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Base|Placement", meta = (ClampMin = "1.0"))
	float PlacementTraceDistance = 6000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Base|Placement", meta = (ClampMin = "1.0"))
	float PreviewCampRadius = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Base|Placement")
	TEnumAsByte<ECollisionChannel> PlacementTraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Base|Placement|Debug")
	float PreviewDrawLifetime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Base|Placement|Debug")
	float PreviewDrawThickness = 4.0f;

private:
	bool bPlacementModeActive = false;
	bool bCanPlaceAtCurrentPreview = false;
	FTransform CurrentPlacementTransform = FTransform::Identity;

	UFUNCTION(Server, Reliable)
	void ServerRequestPlaceBaseCamp(FTransform PlacementTransform);

	void SetPlacementModeActive(bool bNewActive);
	void UpdatePlacementPreview();
	bool ResolvePlacementView(FVector& OutViewLocation, FVector& OutViewDirection) const;
	bool ResolvePlacementTransform(FTransform& OutPlacementTransform) const;
	bool CanPlaceAtTransform(const FTransform& PlacementTransform) const;
	void DrawPlacementPreview() const;
	APW_BaseCampActor* SpawnBaseCampAuthority(const FTransform& PlacementTransform);
	FString ResolveOwnerIdString() const;
};
