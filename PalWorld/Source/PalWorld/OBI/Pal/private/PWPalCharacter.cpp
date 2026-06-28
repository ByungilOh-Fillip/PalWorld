#include "PWPalCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StatusComponent.h"

APWPalCharacter::APWPalCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 캡슐 컴포넌트 충돌체 설정 (충돌 처리)
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
        GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
    }

    // AI 설정: 스폰되거나 맵에 배치될 때 자동으로 AI Controller가 빙의(Possess)하도록 설정
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    
    // 추후 AI Controller 클래스가 완성되면 여기서 기본 컨트롤러로 지정합니다.
    // AIControllerClass = APWPalAIController::StaticClass();

    // 캐릭터 무브먼트 설정 (팰의 기본 이동 설정)
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bOrientRotationToMovement = true; // 이동 방향을 바라보게 설정
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    }
}

void APWPalCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void APWPalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

float APWPalCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    // ACharacter의 기본 데미지 처리 호출
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // StatusComponent에서 체력 깎기 로직 (실제 게임 로직 연동)
    if (StatusComponent)
    {
        // TODO: 방어력(Defense) 등 데미지 공식 적용 필요
        StatusComponent->CurrentHP -= ActualDamage;
        
        if (StatusComponent->CurrentHP <= 0.0f)
        {
            StatusComponent->CurrentHP = 0.0f;
            // TODO: 사망(Death) 로직 처리 (애니메이션 재생, 컴포넌트 비활성화, 파괴 등)
        }
    }

    return ActualDamage;
}
