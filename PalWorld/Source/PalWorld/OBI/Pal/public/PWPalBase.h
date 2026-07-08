#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PWPalBase.generated.h"

class UStatusComponent;
class UPWSkillComponent;

UCLASS()
class PALWORLD_API APWPalBase : public ACharacter
{
    GENERATED_BODY()

public:
    APWPalBase();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintPure, Category = "Pal|Components")
    UPWSkillComponent* GetSkillComponent() const { return SkillComponent; }

    UFUNCTION(BlueprintPure, Category = "Pal|Capture")
    bool IsCaptureInteractionDisabled() const { return bCaptureInteractionDisabled; }

    void SetCaptureInteractionDisabled(bool bDisabled);

protected:
    // 팰 상태 관리 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pal|Components")
    TObjectPtr<UStatusComponent> StatusComponent;

    // 팰 스킬 및 적성 관리 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pal|Components")
    TObjectPtr<UPWSkillComponent> SkillComponent;

private:
    UPROPERTY(ReplicatedUsing = OnRep_CaptureInteractionDisabled)
    bool bCaptureInteractionDisabled = false;

    UFUNCTION()
    void OnRep_CaptureInteractionDisabled();

    void ApplyCaptureInteractionDisabled();
};
