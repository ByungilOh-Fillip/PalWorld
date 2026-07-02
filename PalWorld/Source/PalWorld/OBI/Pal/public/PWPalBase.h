#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PWPalBase.generated.h"

class UStatusComponent;
class UPWSkillComponent;

UCLASS()
class PALWORLD_API APWPalBase : public ACharacter
{
    GENERATED_BODY()

public:
    APWPalBase();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintPure, Category = "Pal|Components")
    UPWSkillComponent* GetSkillComponent() const { return SkillComponent; }

protected:
    // 팰 상태 관리 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pal|Components")
    TObjectPtr<UStatusComponent> StatusComponent;

    // 팰 스킬 및 적성 관리 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pal|Components")
    TObjectPtr<UPWSkillComponent> SkillComponent;
};
