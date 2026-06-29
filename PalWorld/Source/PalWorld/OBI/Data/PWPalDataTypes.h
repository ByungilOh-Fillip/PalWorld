#pragma once

/**
 * @file PWPalDataTypes.h
 * @brief Pal 시스템에서 사용되는 핵심 데이터 타입 정의 (FGameplayTag 구조)
 * 
 * @details
 * - 기존 Enum 방식을 대체하여 FGameplayTag를 사용합니다.
 * - FPalPartnerSkill : 팰 고유 파트너 스킬 (Skill.Combat, Skill.Mount 등)
 * - FWorkAttitude : 거점 내 작업 적성 레벨 및 속도 데이터 모델 (Work.Kindling 등)
 */

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PWPalDataTypes.generated.h"

// ---------------------------------------------------------
// Structs
// ---------------------------------------------------------

USTRUCT(BlueprintType)
struct PALWORLD_API FPalPartnerSkill
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Skill", meta = (Categories = "Skill"))
    FGameplayTag PartnerSkillTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Skill")
    float SkillPower;

    FPalPartnerSkill()
        : PartnerSkillTag(FGameplayTag::EmptyTag)
        , SkillPower(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct PALWORLD_API FWorkAttitude
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Work", meta = (Categories = "Work"))
    FGameplayTag WorkTypeTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Work")
    float WorkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Work")
    int32 WorkLevel;

    FWorkAttitude()
        : WorkTypeTag(FGameplayTag::EmptyTag)
        , WorkSpeed(1.0f)
        , WorkLevel(1)
    {}
};
