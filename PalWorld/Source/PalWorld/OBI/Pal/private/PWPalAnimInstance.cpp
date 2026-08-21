#include "PWPalAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "PWPalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPWPalAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // 폰 소유자를 가져와서 캐싱합니다.
    OwnerCharacter = Cast<APWPalCharacter>(TryGetPawnOwner());
}

void UPWPalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter)
    {
        OwnerCharacter = Cast<APWPalCharacter>(TryGetPawnOwner());
        if (!OwnerCharacter) return;
    }

    // 1. 속도 (Speed)
    FVector Velocity = OwnerCharacter->GetVelocity();
    Velocity.Z = 0.0f; // Z축(수직) 속도를 무시한 평면 이동 속도
    Speed = Velocity.Size();

    // 2. 방향 (Direction)
    Direction = UKismetAnimationLibrary::CalculateDirection(OwnerCharacter->GetVelocity(), OwnerCharacter->GetActorRotation());

    // 3. 점프/낙하 여부 (bIsFalling)
    MovementComp = OwnerCharacter->GetCharacterMovement();
    if (MovementComp != nullptr)
    {
        bIsFalling = MovementComp->IsFalling();
    }

    // 4. 상태 태그 (CurrentActionTag)
    if (OwnerCharacter)
    {
        CurrentActionTag = OwnerCharacter->CurrentActionTag;
    }
}
