#pragma once

/**
 * @file SkillComponent.h
 * @brief 팰의 고유 스킬 및 작업 적성(Work Attitude)을 관리하는 컴포넌트
 * 
 * @details
 * 팰이 가진 파트너 스킬과 거점에서 수행 가능한 12종의 작업 적성 배열을 들고 있습니다.
 * 
 * @property PartnerSkill 팰 고유 파트너 스킬 데이터
 * @property WorkAttitudes 거점에서 수행 가능한 작업 적성 및 레벨 배열
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PWPalDataTypes.h"
#include "PWSkillComponent.generated.h"

// FPWSkillData moved to PWPalDataTypes.h

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PALWORLD_API UPWSkillComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UPWSkillComponent();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintPure, Category = "Pal|Work")
    bool CanWork(FGameplayTag WorkTag) const;

    UFUNCTION(BlueprintPure, Category = "Pal|Work")
    bool FindWorkAttitude(FGameplayTag WorkTag, FWorkAttitude& OutWorkAttitude) const;

    /* 
     * 전투 스킬 슬롯 (3가지) - 런타임에 쿨타임 관리 및 상태 저장을 위해 존재함
     * (원본 데이터는 PWPalDataAsset에서 복사되어 들어옵니다)
     */
    // 가벼운 스킬 (쿨타임 짧음)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Pal|CombatSkills")
    FPWSkillData LightSkill;

    // 중간 스킬 (쿨타임 보통)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Pal|CombatSkills")
    FPWSkillData MediumSkill;

    // 무거운 스킬 (쿨타임 길고 위력 강함)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Pal|CombatSkills")
    FPWSkillData HeavySkill;

    // 현재 쿨타임이 끝나서 사용 가능한 스킬 중 하나를 무작위로 골라 반환 (0: Light, 1: Medium, 2: Heavy)
    UFUNCTION(BlueprintCallable, Category = "Pal|CombatSkills")
    bool GetAvailableRandomSkill(FPWSkillData& OutSkillData, int32& OutSkillSlot);

    // 스킬 사용 후 쿨타임 시작 적용
    UFUNCTION(BlueprintCallable, Category = "Pal|CombatSkills")
    void MarkSkillAsUsed(int32 SkillSlot);
};
