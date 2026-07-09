#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "PWPalAnimInstance.generated.h"

class UCharacterMovementComponent;
class APWPalCharacter;

/**
 * @class UPWPalAnimInstance
 * @brief 팰의 애니메이션을 구동하는 베이스 클래스 (속도, 방향, 점프, 상태 태그 연동)
 */
UCLASS()
class PALWORLD_API UPWPalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
    // 캐릭터의 이동 속도 (Locomotion 용)
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Pal|Animation")
    float Speed;

    // 캐릭터의 이동 방향 (조준/스트레이핑 용, -180 ~ 180)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pal|Animation")
    float Direction;

    // 공중에 떠있는지 여부 (점프/낙하)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pal|Animation")
    bool bIsFalling;

    // 현재 수행 중인 행동 태그 (작업, 전투 등 분기용)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pal|Animation")
    FGameplayTag CurrentActionTag;

private:
    UPROPERTY()
    TObjectPtr<APWPalCharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<UCharacterMovementComponent> MovementComp;
};
