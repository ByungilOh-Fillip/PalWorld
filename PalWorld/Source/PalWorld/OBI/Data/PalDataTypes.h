#pragma once

/**
 * @file PalDataTypes.h
 * @brief Pal 시스템에서 사용되는 핵심 데이터 타입 정의 (Enum 및 Struct)
 * 
 * @details
 * - EPartnerSkillType : 파트너 스킬의 분류 (전투, 탑승, 유틸)
 * - EMountType : 탑승 유형 (지상, 공중, 수상)
 * - EWorkType : 거점 내 작업 적성 분류 (불, 파종, 관개 등 12종)
 * - FPalPartnerSkill : 팰 고유 파트너 스킬 데이터 모델
 * - FWorkAttitude : 거점 내 작업 적성 레벨 및 속도 데이터 모델
 */

#include "CoreMinimal.h"
#include "PalDataTypes.generated.h"

// ---------------------------------------------------------
// Enums
// ---------------------------------------------------------

UENUM(BlueprintType)
enum class EPartnerSkillType : uint8
{
    None UMETA(DisplayName = "None"),
    Combat UMETA(DisplayName = "Combat"),
    Mount UMETA(DisplayName = "Mount"),
    Utility UMETA(DisplayName = "Utility")
};

UENUM(BlueprintType)
enum class EMountType : uint8
{
    None UMETA(DisplayName = "None"),
    Ground UMETA(DisplayName = "Ground"),
    Flying UMETA(DisplayName = "Flying"),
    Water UMETA(DisplayName = "Water")
};

UENUM(BlueprintType)
enum class EWorkType : uint8
{
    None UMETA(DisplayName = "None"),
    Kindling UMETA(DisplayName = "Kindling"),
    Watering UMETA(DisplayName = "Watering"),
    Planting UMETA(DisplayName = "Planting"),
    GeneratingElectricity UMETA(DisplayName = "Generating Electricity"),
    Handiwork UMETA(DisplayName = "Handiwork"),
    Gathering UMETA(DisplayName = "Gathering"),
    Lumbering UMETA(DisplayName = "Lumbering"),
    Mining UMETA(DisplayName = "Mining"),
    MedicineProduction UMETA(DisplayName = "Medicine Production"),
    Cooling UMETA(DisplayName = "Cooling"),
    Transporting UMETA(DisplayName = "Transporting"),
    Farming UMETA(DisplayName = "Farming")
};

// ---------------------------------------------------------
// Structs
// ---------------------------------------------------------

USTRUCT(BlueprintType)
struct PALWORLD_API FPalPartnerSkill
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Skill")
    EPartnerSkillType PartnerSkillType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Skill")
    EMountType MountType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Skill")
    float SkillPower;

    FPalPartnerSkill()
        : PartnerSkillType(EPartnerSkillType::None)
        , MountType(EMountType::None)
        , SkillPower(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct PALWORLD_API FWorkAttitude
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Work")
    EWorkType WorkType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Work")
    float WorkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Work")
    int32 WorkLevel;

    FWorkAttitude()
        : WorkType(EWorkType::None)
        , WorkSpeed(1.0f)
        , WorkLevel(1)
    {}
};
