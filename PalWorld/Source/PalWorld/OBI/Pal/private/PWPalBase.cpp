#include "PWPalBase.h"
#include "StatusComponent.h"
#include "PWSkillComponent.h"

APWPalBase::APWPalBase()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // 멀티플레이어 통신을 위한 Actor 리플리케이션 켜기
    bReplicates = true;

    // 컴포넌트 생성 및 부착
    StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
    SkillComponent = CreateDefaultSubobject<UPWSkillComponent>(TEXT("SkillComponent"));
}

void APWPalBase::BeginPlay()
{
    Super::BeginPlay();
}

void APWPalBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
