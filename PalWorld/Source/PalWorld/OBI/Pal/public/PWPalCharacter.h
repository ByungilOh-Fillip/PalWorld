#pragma once

#include "CoreMinimal.h"
#include "PWPalBase.h"
#include "PWPalCharacter.generated.h"

UCLASS()
class PALWORLD_API APWPalCharacter : public APWPalBase
{
    GENERATED_BODY()

public:
    APWPalCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // 데미지 처리 로직 (피격 판정)
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
};
