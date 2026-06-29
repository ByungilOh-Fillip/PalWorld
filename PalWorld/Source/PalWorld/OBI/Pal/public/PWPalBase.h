#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PWPalBase.generated.h"

class UStatusComponent;
class UPWSkillComponent;

UCLASS(Blueprintable)
class PALWORLD_API APWPalBase : public AActor
{
    GENERATED_BODY()

public:
    APWPalBase();

    // TODO: Pal movement/AI integration may require APWPalBase to become ACharacter or own a movement component.

protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;

protected:
    // 팰 상태 관리 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pal|Components")
    UStatusComponent* StatusComponent;

    // 팰 스킬 및 적성 관리 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pal|Components")
    UPWSkillComponent* SkillComponent;
};
