// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Animation/PWAnimNotify_PrimaryActionHit.h"

#include "Components/SkeletalMeshComponent.h"
#include "Player/Components/PWPlayerPrimaryActionComponent.h"
#include "Player/Core/PWPlayerCharacter.h"

FString UPWAnimNotify_PrimaryActionHit::GetNotifyName_Implementation() const
{
	return TEXT("PrimaryActionHit");
}

void UPWAnimNotify_PrimaryActionHit::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	APWPlayerCharacter* PlayerCharacter = MeshComp ? Cast<APWPlayerCharacter>(MeshComp->GetOwner()) : nullptr;
	UPWPlayerPrimaryActionComponent* PrimaryActionComponent = PlayerCharacter ? PlayerCharacter->GetPrimaryActionComponent() : nullptr;
	if (!PrimaryActionComponent)
	{
		return;
	}

	PrimaryActionComponent->HandlePrimaryActionHitNotify();
}
