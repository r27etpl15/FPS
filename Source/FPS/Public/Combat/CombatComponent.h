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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRoundFired, int32, RoundsCurrent, int32, RoundsMax, int32, RoundsInReserve);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAimingStatusChanged, bool, bIsAiming);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTargetingPlayerStatusChanged, bool, bIsAiming);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCurrentReserveAmmoChanged, int32, RoundsInReserve, int32, RoundsInWeapon, UMaterialInterface*, WeaponIconMaterial);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FRoundReported, AActor*, Attacker, AActor*, Victim, bool, bHit, bool, bHeadShot, bool, bLethal);

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
	
	UPROPERTY(BlueprintAssignable)
	FRoundReported OnRoundReported;
	
	void SpawnInventory();
	void DestroyInventory();
	void Equip(AWeapon* InWeapon);
	void EquipWeapon(AWeapon* InWeapon);
	
	bool IsAiming() const { return bAiming; }
	bool IsHitPlayer() const { return bHitPlayer; }
	AWeapon* GetCurrentWeapon() const { return CurrentWeapon; }
	int32 GetCurrentReserveAmmo() const { return CurrentReserveAmmo; }
	
	void InitializeWeaponWidgets() const;
	
	void Notify_CycleWeapon();
	void Notify_ReloadWeapon();
	
	void AddAmmo(const FGameplayTag& WeaponType, int32 AmmoAmount);
	
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void BlendOut_CycleWeapon(UAnimMontage* Montage, bool bInterrupted);
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon")
	float TraceLength = 20000;
	
	// 反作弊（服务器专用）
	UPROPERTY(EditDefaultsOnly, Category="FPS|AntiCheat")
	float FireRateTolerance = 0.8f;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|AntiCheat")
	float ShotDistanceTolerance = 1.1f;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|AntiCheat")
	FName HeadBoneName = FName("head");
	
private:
	bool bTriggerPressed = false;
	bool bHitPlayer = false;
	bool bHitPlayerLastFrame = false;
	FTimerHandle FireTimer;
	
	float AccumulatedRecoilPitch = 0.0f;
	float AccumulatedRecoilYaw = 0.0f;
	
	float LastFireTime = -1000.0f;
	
	bool IsShotDistanceValid(const FHitResult& HitResult) const;
	bool IsHeadShot(const FHitResult& HitResult) const;
	
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
	
	int32 Local_WeaponIndex = 0;
	
	void FireTimerFinished();
	
	void ApplyRecoil();
	void RecoverRecoil(float DeltaTime);
	
	void SetCurrentWeapon(AWeapon* NewWeapon, AWeapon* LastWeapon);
	
	void Local_Aim(bool bPressed);
	void Local_FireWeapon();
	void Local_CycleWeapon(int32 WeaponIndex);
	void Local_ReloadWeapon();
	
	int32 AdvanceWeaponIndex();
	
	AWeapon* SpawnWeapon(TSubclassOf<AWeapon> WeaponClass) const;
	
	UFUNCTION(Server, Reliable)
	void Server_Aim(bool bPressed);
	
	UFUNCTION(Server, Reliable)
	void Server_FireWeapon(const FHitResult& HitResult);
	
	UFUNCTION(Server, Reliable)
	void Server_DryFire();
	
	UFUNCTION(Server, Reliable)
	void Server_CycleWeapon(int32 WeaponIndex);
	
	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon(AWeapon* InWeapon);
	
	UFUNCTION(Server, Reliable)
	void Server_ReloadWeapon();
	
	UFUNCTION(Client, Reliable)
	void Client_ReloadWeapon(int32 NewWeaponAmmo, int32 NewCarriedAmmo);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FireWeapon(const FHitResult& HitResult, int32 AuthAmmo);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DryFire();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CycleWeapon(int32 WeaponIndex);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ReloadWeapon();
	
	UFUNCTION()
	void OnRep_CurrentWeapon(AWeapon* LastWeapon);
	
	UFUNCTION()
	void OnRep_CurrentReserveAmmo();
};