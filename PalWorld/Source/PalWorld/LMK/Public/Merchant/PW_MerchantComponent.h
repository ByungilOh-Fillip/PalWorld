#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PW_MerchantComponent.generated.h"

class UPWItemDataAsset;

USTRUCT(BlueprintType)
struct PALWORLD_API FPW_MerchantTradeEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Merchant")
	TObjectPtr<UPWItemDataAsset> ItemData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Merchant")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Merchant", meta = (ClampMin = "0"))
	int32 BuyPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Merchant", meta = (ClampMin = "0"))
	int32 SellPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Merchant", meta = (ClampMin = "0", EditCondition = "!bInfiniteStock"))
	int32 StockCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Merchant")
	bool bInfiniteStock = true;

	bool MatchesItem(FName InItemId) const;
};

UCLASS(ClassGroup = (PW), meta = (BlueprintSpawnableComponent))
class PALWORLD_API UPW_MerchantComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPW_MerchantComponent();

	UFUNCTION(BlueprintPure, Category = "PW|Merchant")
	FText GetMerchantDisplayName() const { return MerchantDisplayName; }

	UFUNCTION(BlueprintPure, Category = "PW|Merchant")
	FText GetGreetingText() const { return GreetingText; }

	UFUNCTION(BlueprintPure, Category = "PW|Merchant")
	const TArray<FPW_MerchantTradeEntry>& GetTradeEntries() const { return TradeEntries; }

	UFUNCTION(BlueprintPure, Category = "PW|Merchant")
	bool FindTradeEntry(FName ItemId, FPW_MerchantTradeEntry& OutTradeEntry) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Merchant")
	FText MerchantDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Merchant")
	FText GreetingText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PW|Merchant", meta = (TitleProperty = "ItemId"))
	TArray<FPW_MerchantTradeEntry> TradeEntries;
};
