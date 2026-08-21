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

    // 해당 작업을 수행할 때 재생할 애니메이션 몽타주 (채굴 모션, 물뿌리기 모션 등)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Work")
    class UAnimMontage* WorkMontage;

    FWorkAttitude()
        : WorkTypeTag(FGameplayTag::EmptyTag)
        , WorkSpeed(1.0f)
        , WorkLevel(1)
        , WorkMontage(nullptr)
    {}
};

USTRUCT(BlueprintType)
struct PALWORLD_API FPWSkillData
{
    GENERATED_BODY()

    // 시전할 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    class UAnimMontage* SkillMontage = nullptr;

    // 사거리 (이 거리 이내로 접근해야 시전 가능)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float MaxRange = 1000.0f;

    // 스킬 액터 클래스 (실제 스킬의 콜리전, 이펙트, 투사체 로직을 담당하는 액터)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    TSubclassOf<class APWSkillBase> SkillActorClass;

    // 재사용 대기시간 (초) - 기획자 조정 (시전 시간/위력에 비례)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float Cooldown = 5.0f;

    // 시전 시간 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float CastTime = 2.0f;

    // 위력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    float Power = 10.0f;

    // 내부 관리용: 마지막 사용 시간 (런타임 전용)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Skill")
    float LastUseTime = -9999.0f;

    bool IsReady(float CurrentTime) const
    {
        return (CurrentTime - LastUseTime) >= Cooldown;
    }
};
