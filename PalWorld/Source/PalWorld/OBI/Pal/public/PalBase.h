#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PalBase.generated.h"

class UStatusComponent;
class USkillComponent;

UCLASS()
class PALWORLD_API APalBase : public AActor
{
    GENERATED_BODY()

public:
    APalBase();

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
    USkillComponent* SkillComponent;
};
