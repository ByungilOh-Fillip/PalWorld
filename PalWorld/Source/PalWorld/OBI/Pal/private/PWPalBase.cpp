#include "PWPalBase.h"
#include "StatusComponent.h"
#include "PWSkillComponent.h"
#include "Net/UnrealNetwork.h"
#include "PW_ST_EventsTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PWPalDataAsset.h"

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

    // 태그 변화에 따른 내부 속도 동기화 이벤트 바인딩
    OnStateTagChangedDelegate.AddDynamic(this, &APWPalBase::HandleStateTagChanged);

    // 기본 이동 속도를 배회(Wander) 속도로 초기화 (StateTree가 태그로 속도를 바꾸기 전까지의 기본값)
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = 250.f;
    }
}

void APWPalBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APWPalBase::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    TagContainer.AppendTags(ActiveTags);
}

void APWPalBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APWPalBase, ActiveTags);
}

void APWPalBase::OnRep_ActiveTags(const FGameplayTagContainer& PreviousTags)
{
    // 1. 추가된 태그 찾기 (현재 ActiveTags에는 있는데, 이전 PreviousTags에는 없는 것)
    for (auto It = ActiveTags.CreateConstIterator(); It; ++It)
    {
        const FGameplayTag& Tag = *It;
        if (!PreviousTags.HasTagExact(Tag))
        {
            OnStateTagChangedDelegate.Broadcast(Tag, true);
        }
    }

    // 2. 삭제된 태그 찾기 (이전 PreviousTags에는 있었는데, 현재 ActiveTags에는 없는 것)
    for (auto It = PreviousTags.CreateConstIterator(); It; ++It)
    {
        const FGameplayTag& Tag = *It;
        if (!ActiveTags.HasTagExact(Tag))
        {
            OnStateTagChangedDelegate.Broadcast(Tag, false);
        }
    }
}

// StateTree가 특정 State에 진입할 때 호출할 함수 (서버 전용 권장)
void APWPalBase::AddStateTag(FGameplayTag TagToAdd)
{
    if (TagToAdd.IsValid() && !ActiveTags.HasTagExact(TagToAdd))
    {
        ActiveTags.AddTag(TagToAdd);

        // 서버에서는 OnRep이 호출되지 않으므로 수동으로 Broadcast 해줍니다.
        if (HasAuthority())
        {
            OnStateTagChangedDelegate.Broadcast(TagToAdd, true);
        }
    }
}

// StateTree가 특정 State에서 빠져나올 때 호출할 함수 (서버 전용 권장)
void APWPalBase::RemoveStateTag(FGameplayTag TagToRemove)
{
    if (TagToRemove.IsValid() && ActiveTags.HasTagExact(TagToRemove))
    {
        ActiveTags.RemoveTag(TagToRemove);

        // 서버에서는 OnRep이 호출되지 않으므로 수동으로 Broadcast 해줍니다.
        if (HasAuthority())
        {
            OnStateTagChangedDelegate.Broadcast(TagToRemove, false);
        }
    }
}

void APWPalBase::PlayHitReaction()
{
    if (PalDataAsset && PalDataAsset->HitMontage)
    {
        PlayAnimMontage(PalDataAsset->HitMontage);
    }
}

void APWPalBase::HandleStateTagChanged(FGameplayTag Tag, bool bAdded)
{
    // 추가(진입)될 때만 속도 조절
    if (!bAdded) return;

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp) return;

    // 기획(에디터 세팅)에 맞춰 디테일하게 값을 뽑아 쓰려면 UObject로 매핑 테이블을 만들 수도 있습니다.
    // 임시 하드코딩 예시 (원하시는 속도로 튜닝 가능)
    if (Tag == PW_ST_EventsTags::State_Runaway)
    {
        MoveComp->MaxWalkSpeed = 800.f; // 도주 (전력질주)
    }
    else if (Tag == PW_ST_EventsTags::State_Peaceful_Wander)
    {
        MoveComp->MaxWalkSpeed = 250.f; // 걷기 (평화)
    }
    else if (Tag == PW_ST_EventsTags::State_Combat_Circle)
    {
        MoveComp->MaxWalkSpeed = 400.f; // 경계 (조심스러운 걸음)
    }
    else if (Tag == PW_ST_EventsTags::State_Work_DoWork)
    {
        MoveComp->MaxWalkSpeed = 300.f; // 작업장 이동
    }
}
