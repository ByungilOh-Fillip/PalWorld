#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PWPalDataTypes.h"
#include "PWPalDataAsset.generated.h"

// 특정 레벨에 배우는 스킬을 매핑하기 위한 구조체
USTRUCT(BlueprintType)
struct FPWPalLearnableSkill
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 RequiredLevel;

    // 실제 스킬의 데이터나 클래스 (추후 UPalSkillBase 등 추가 시 태그나 클래스로 확장)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Skill"))
    FGameplayTag SkillTag;

    FPWPalLearnableSkill()
        : RequiredLevel(1)
        , SkillTag(FGameplayTag::EmptyTag)
    {}
};

/**
 * @class UPWPalDataAsset
 * @brief 팰(Pal) 종족별 고유 데이터를 담는 Primary Data Asset
 * 주의: 레벨업으로 변하거나 현재 상태를 나타내는 값(CurrentHP 등)은 절대 넣지 않습니다.
 */
UCLASS(BlueprintType)
class PALWORLD_API UPWPalDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:

    /* ========================================
     * Basic Info
     * ======================================== */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Info")
    FName PalName;

    // 속성을 GameplayTag로 관리 (Element.Water 등, 듀얼 속성을 위해 Container 사용)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Info", meta = (Categories = "Element"))
    FGameplayTagContainer ElementTypes;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Info")
    int32 Rarity; // Rare (희귀도)

    /* ========================================
     * Base Stats (1레벨 기준 또는 계수)
     * ======================================== */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Stats")
    float BaseMaxHP;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Stats")
    float BaseMaxSP;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Stats")
    float BaseMaxHunger;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Stats")
    float BaseMaxSanity; // 스트레스(SAN) 수치

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Stats")
    float BaseAttack;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Stats")
    float BaseDefense;

    /* ========================================
     * Life & Work
     * ======================================== */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Work & Life")
    int32 FeedPerDay; // FPD (하루 식사량)

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Work & Life")
    float BaseCarryWeight;

    // 배고픔을 느끼는 기준 (예: MaxHunger의 30% 이하일 때 배고픔 상태로 전이)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Work & Life", meta=(ClampMin="0.0", ClampMax="1.0"))
    float HungryThresholdRatio = 0.3f;

    /* ========================================
     * Tags (Work & Skills)
     * ======================================== */
    // 이 팰이 할 수 있는 작업 적성 목록 (불 피우기 Lv.2 등 개별 설정)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
    TArray<FWorkAttitude> WorkAttitudes;

    // 파트너 스킬 태그
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
    FPalPartnerSkill PartnerSkill;

    /* ========================================
     * Skills
     * ======================================== */
    // 레벨업에 따라 배울 수 있는 스킬 풀
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills")
    TArray<FPWPalLearnableSkill> LearnableSkillsPool;

    /* ========================================
     * Visual & Anim
     * ======================================== */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    class UAnimMontage* HitMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    class UAnimMontage* DeathMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    class UAnimMontage* StartleMontage; // 놀람

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations")
    class UAnimMontage* KitingMontage; // 백점프 등 카이팅 모션

    // 전투 시 적과 거리 유지를 위한 기본 카이팅 거리 설정
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat AI")
    float KitingDistance = 200.0f;

    /* ========================================
     * Sounds
     * ======================================== */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sounds")
    class USoundBase* SpawnRoarSound; // 소환 시 포효 소리

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sounds")
    class USoundBase* HitSound; // 피격 소리

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sounds")
    class USoundBase* DeathSound; // 사망 소리

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sounds")
    class USoundBase* WorkSound; // 작업 시 낼 소리 (망치질, 물 뿜기 등)
};
