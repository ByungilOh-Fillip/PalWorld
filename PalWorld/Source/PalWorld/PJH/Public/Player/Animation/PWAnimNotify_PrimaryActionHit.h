// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PWAnimNotify_PrimaryActionHit.generated.h"

UCLASS(meta = (DisplayName = "PW Primary Action Hit"))
class PALWORLD_API UPWAnimNotify_PrimaryActionHit : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
