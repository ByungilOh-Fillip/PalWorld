#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "PWInteractionGuideTypes.generated.h"

USTRUCT(BlueprintType)
struct PALWORLD_API FPWInteractionGuideAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Interaction")
	FName ActionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Interaction")
	FKey Key = EKeys::F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Interaction")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Interaction")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Interaction", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Progress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Interaction")
	bool bShowProgress = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Interaction", meta = (ClampMin = "0.01", EditCondition = "bShowProgress"))
	float ProgressDurationSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PW|Interaction")
	int32 SortOrder = 0;

	bool IsValidGuide() const
	{
		return !ActionId.IsNone() && !Label.IsEmpty();
	}
};
