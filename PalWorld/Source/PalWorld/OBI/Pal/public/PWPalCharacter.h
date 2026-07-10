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

    // 애니메이션에서 읽어갈 현재 상태 태그
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Pal|State")
    FGameplayTag CurrentActionTag;

protected:
    // 사망 시 재생할 몽타주
    UPROPERTY(EditDefaultsOnly, Category = "Pal|Animation")
    class UAnimMontage* DeathMontage;

    // 사망 처리 함수
    virtual void Die();
};
