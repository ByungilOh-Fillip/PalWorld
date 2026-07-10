#include "PWPalCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StatusComponent.h"
#include "PWPalAIController.h"
#include "PW_ST_EventsTags.h"
#include "Components/StateTreeComponent.h"

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

    // 기본 AI 컨트롤러 클래스를 PWPalAIController로 지정
    AIControllerClass = APWPalAIController::StaticClass();

    // 캐릭터 무브먼트 설정 (팰의 기본 이동 설정)
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 120.0f, 0.0f); // 자연스러운 회전 속도
        GetCharacterMovement()->bUseControllerDesiredRotation = false;
    }
}

void APWPalCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void APWPalCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 속도가 거의 없을 때(정지 상태)는 회전을 비활성화하여 Delay 중 빙글빙글 도는 현상 방지
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        const float SpeedSq = GetVelocity().SizeSquared2D();
        MoveComp->bOrientRotationToMovement = (SpeedSq > 100.f); // 10cm/s 이상일 때만 이동 방향으로 회전
    }
}

float APWPalCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    UE_LOG(LogTemp, Error, TEXT("[PWPalCharacter::TakeDamage] 팰 피격 발생! 데미지: %f, 가해자: %s"), DamageAmount, DamageCauser ? *DamageCauser->GetName() : TEXT("None"));

    // ACharacter의 기본 데미지 처리 호출
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // AI Controller를 통해 StateTree로 피격 이벤트(Event_Hit) 전송
    if (APWPalAIController* AIController = Cast<APWPalAIController>(GetController()))
    {
        // 피격 대상을 타겟으로 자동 지정
        AIController->CurrentTargetActor = DamageCauser;

        if (TObjectPtr<UStateTreeComponent> STComp = AIController->StateTreeComponent)
        {
            FStateTreeEvent TreeEvent;
            TreeEvent.Tag = PW_ST_EventsTags::Event_Hit;

            // 누가 때렸는지 Payload에 담아 보냅니다.
            FST_PerceptionPayload Payload;
            Payload.TargetActor = DamageCauser;
            TreeEvent.Payload = FInstancedStruct::Make(Payload);

            STComp->SendStateTreeEvent(TreeEvent);
        }
    }

    // StatusComponent에서 체력 깎기 로직 (실제 게임 로직 연동)
    if (StatusComponent && StatusComponent->CurrentHP > 0.0f)
    {
        // TODO: 방어력(Defense) 등 데미지 공식 적용 필요
        StatusComponent->CurrentHP -= ActualDamage;
        
        if (StatusComponent->CurrentHP <= 0.0f)
        {
            StatusComponent->CurrentHP = 0.0f;
            Die();
        }
    }

    return ActualDamage;
}

void APWPalCharacter::Die()
{
    // 1. 충돌 해제 (시체가 길막하는 것 방지)
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 2. AI 뇌(컨트롤러) 연결 끊기
    if (AController* AICon = GetController())
    {
        AICon->UnPossess();
    }

    // 3. 사망 애니메이션 재생
    if (DeathMontage)
    {
        PlayAnimMontage(DeathMontage);
    }

    // 4. 5초 뒤 액터 삭제 (타이머)
    SetLifeSpan(5.0f);
}
