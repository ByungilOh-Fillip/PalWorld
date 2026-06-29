#pragma once

/**
 * @file StatusComponent.h
 * @brief 팰 및 캐릭터의 생존/전투 상태(Status) 관리 컴포넌트
 * 
 * @details
 * 멀티플레이어 환경(Replication)을 지원하며, 캐릭터의 주요 스탯을 관리합니다.
 * 
 * @property Level 현재 레벨
 * @property CurrentHP 현재 체력 (네트워크 동기화)
 * @property MaxHP 최대 체력
 * @property CurrentSP 현재 스태미나 (네트워크 동기화)
 * @property MaxSP 최대 스태미나
 * @property CurrentHunger 배고픔 지수 (네트워크 동기화)
 * @property MaxHunger 최대 배고픔 지수
 * @property CurrentSanity 스트레스/SAN 수치 (네트워크 동기화)
 * @property MaxSanity 최대 스트레스 수치
 * @property Attack 공격력
 * @property Defense 방어력
 * @property WorkSpeed 작업 속도 베이스 배율
 * @property FeedPerDay 하루 식사 필요량 (FPD)
 * @property CurrentCarryWeight 현재 소지 중량 (네트워크 동기화)
 * @property MaxCarryWeight 최대 소지 가능 중량
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "StatusComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PALWORLD_API UStatusComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UStatusComponent();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Status")
    int32 Level;

    // HP
    UPROPERTY(ReplicatedUsing = OnRep_CurrentHP, BlueprintReadWrite, Category = "Pal|Status")
    float CurrentHP;

    UFUNCTION()
    void OnRep_CurrentHP();

    UPROPERTY(EditDefaultsOnly, Category = "Pal|Status")
    float MaxHP;

    // SP
    UPROPERTY(ReplicatedUsing = OnRep_CurrentSP, BlueprintReadWrite, Category = "Pal|Status")
    float CurrentSP;

    UFUNCTION()
    void OnRep_CurrentSP();

    UPROPERTY(EditDefaultsOnly, Category = "Pal|Status")
    float MaxSP;

    // Hunger
    UPROPERTY(ReplicatedUsing = OnRep_CurrentHunger, BlueprintReadWrite, Category = "Pal|Status")
    float CurrentHunger;

    UFUNCTION()
    void OnRep_CurrentHunger();

    UPROPERTY(EditDefaultsOnly, Category = "Pal|Status")
    float MaxHunger;

    // Sanity (Stress)
    UPROPERTY(ReplicatedUsing = OnRep_CurrentSanity, BlueprintReadWrite, Category = "Pal|Status")
    float CurrentSanity;

    UFUNCTION()
    void OnRep_CurrentSanity();

    UPROPERTY(EditDefaultsOnly, Category = "Pal|Status")
    float MaxSanity;

    // Combat Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Status")
    float Attack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Status")
    float Defense;

    // Work Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Status")
    float WorkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pal|Status")
    int32 FeedPerDay;

    // Carry Weight
    UPROPERTY(ReplicatedUsing = OnRep_CurrentCarryWeight, BlueprintReadWrite, Category = "Pal|Status")
    float CurrentCarryWeight;

    UFUNCTION()
    void OnRep_CurrentCarryWeight();

    UPROPERTY(EditDefaultsOnly, Category = "Pal|Status")
    float MaxCarryWeight;

    // Status Effects (GameplayTags)
    UPROPERTY(ReplicatedUsing = OnRep_ActiveStatusEffects, BlueprintReadWrite, Category = "Pal|Status")
    FGameplayTagContainer ActiveStatusEffects;

    UFUNCTION()
    void OnRep_ActiveStatusEffects();
};
