// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "ShooterTypes/ShooterTypes.h"
#include "CombatComponent.generated.h"

class AWeapon;
class UWeaponData;
class UMaterialInstanceDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FReticleChanged, UMaterialInstanceDynamic*, ReticleDynamicInstance, const FReticleParams&, ReticleParams, bool, bCurrentlyTargetingPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAmmoCounterChanged, UMaterialInstanceDynamic*, AmmoCounterDynamicInstance, int32, RoundsCurrent, int32, RoundsMax);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRoundFired, int32, RoundsCurrent, int32, RoundsMax);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAimingStatusChanged, bool, bIsAiming);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTargetingPlayerStatusChanged, bool, bIsAiming);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCurrentReserveAmmoChanged, int32, RoundsInReserve, int32, RoundsInWeapon);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintPure, Category="FPS|Combat")
	static UCombatComponent* FindCombatComponent(const AActor* Actor) { return IsValid(Actor) ? Actor->FindComponentByClass<UCombatComponent>() : nullptr; }
	
	// 切换下一把武器
	void Initiate_CycleWeapon();
	void Initiate_FireWeaponPressed();
	void Initiate_FireWeaponReleased();
	void Initiate_ReloadWeapon();
	void Initiate_AimPressed();
	void Initiate_AimReleased();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon")
	TObjectPtr<UWeaponData> WeaponData;
	
	UPROPERTY(BlueprintAssignable)
	FReticleChanged OnReticleChanged;
	
	UPROPERTY(BlueprintAssignable)
	FAmmoCounterChanged OnAmmoCounterChanged;
	
	UPROPERTY(BlueprintAssignable)
	FRoundFired OnRoundFired;
	
	UPROPERTY(BlueprintAssignable)
	FAimingStatusChanged OnAimingStatusChanged;
	
	UPROPERTY(BlueprintAssignable)
	FTargetingPlayerStatusChanged OnTargetingPlayerStatusChanged;
	
	UPROPERTY(BlueprintAssignable)
	FCurrentReserveAmmoChanged OnCurrentReserveAmmoChanged;
	
	void SpawnInventory();
	void DestroyInventory();
	void EquipWeapon(AWeapon* InWeapon);
	
	bool IsAiming() const { return bAiming; }
	bool IsHitPlayer() const { return bHitPlayer; }
	
	AWeapon* GetCurrentWeapon() const { return CurrentWeapon; }
	
	void InitializeWeaponWidgets() const;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon")
	float TraceLength = 20000;
	
private:
	void FireTimerFinished();
	
	bool bTriggerPressed = false;
	bool bHitPlayer = false;
	bool bHitPlayerLastFrame = false;
	FTimerHandle FireTimer;
	
	UPROPERTY(Transient, Replicated)
	TArray<TObjectPtr<AWeapon>> Inventory;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon")
	TArray<TSubclassOf<AWeapon>> DefaultWeaponClass;
	
	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing=OnRep_CurrentWeapon, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<AWeapon> CurrentWeapon;
	
	UPROPERTY(BlueprintReadOnly, Replicated, meta=(AllowPrivateAccess = "true"))
	bool bAiming = false;
	
	UPROPERTY(ReplicatedUsing=OnRep_CurrentReserveAmmo)
	int32 CurrentReserveAmmo;
	
	TMap<FGameplayTag, int32> ReserveAmmo;
	
	void Local_Aim(bool bPressed);
	void Local_FireWeapon();
	
	UFUNCTION()
	void OnRep_CurrentWeapon(AWeapon* LastWeapon);
	
	AWeapon* SpawnWeapon(TSubclassOf<AWeapon> WeaponClass) const;
	
	UFUNCTION(Server, Reliable)
	void Server_Aim(bool bPressed);
	
	UFUNCTION(Server, Reliable)
	void Server_FireWeapon(const FHitResult& HitResult);
	
	UFUNCTION(Server, Reliable)
	void Server_DryFire();
	
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_FireWeapon(const FHitResult& HitResult, int32 AuthAmmo);
	
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_DryFire();
	
	UFUNCTION()
	void OnRep_CurrentReserveAmmo();
};