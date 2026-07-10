#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PW_MenuPlayerController.generated.h"

class UPW_MainMenuWidget;

UCLASS()
class PALWORLD_API APW_MenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PW|Menu")
	void ShowMainMenu();

	UFUNCTION(BlueprintCallable, Category = "PW|Menu")
	void HideMainMenu();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|Menu")
	TSubclassOf<UPW_MainMenuWidget> MainMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PW|Menu")
	int32 MainMenuZOrder = 0;

private:
	UPROPERTY(Transient)
	TObjectPtr<UPW_MainMenuWidget> MainMenuWidget;

	void ApplyMenuInputMode();
};
