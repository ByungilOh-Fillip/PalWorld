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
#include "OBI/Data/PalDataTypes.h"
#include "SkillComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PALWORLD_API USkillComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    USkillComponent();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 파트너 스킬
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Skill")
    FPalPartnerSkill PartnerSkill;

    // 작업 적성 배열
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Work")
    TArray<FWorkAttitude> WorkAttitudes;
};
