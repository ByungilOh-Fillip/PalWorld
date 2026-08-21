#include "PWSkillBase.h"
#include "Engine/DamageEvents.h"

APWSkillBase::APWSkillBase()
{
    PrimaryActorTick.bCanEverTick = true;
    CasterActor = nullptr;
    TargetActor = nullptr;
    SkillPower = 0.0f;
}

void APWSkillBase::BeginPlay()
{
    Super::BeginPlay();
}

void APWSkillBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APWSkillBase::ExecuteSkill(AActor* InCaster, AActor* InTarget, float InPower)
{
    CasterActor = InCaster;
    TargetActor = InTarget;
    SkillPower = InPower;
    
    // 블루프린트 자식 클래스에서 이벤트(ReceiveExecuteSkill) 등을 오버라이드하여 
    // 투사체 이동, 장판 생성, 근접 콜리전 활성화 등을 처리합니다.
}

void APWSkillBase::FinishSkill()
{
    Destroy();
}

void APWSkillBase::ApplySkillDamage(AActor* HitActor)
{
    if (HitActor && CasterActor)
    {
        // 언리얼 기본 데미지 시스템 호출
        FDamageEvent DamageEvent;
        HitActor->TakeDamage(SkillPower, DamageEvent, CasterActor->GetInstigatorController(), CasterActor);
    }
}
