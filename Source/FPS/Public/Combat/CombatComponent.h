// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


class AWeapon;
class UWeaponData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 切换下一把武器
	void Initiate_CycleWeapon();
	void Initiate_FireWeaponPressed();
	void Initiate_FireWeaponReleased();
	void Initiate_ReloadWeapon();
	void Initiate_AimPressed();
	void Initiate_AimReleased();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon")
	TObjectPtr<UWeaponData> WeaponData;
	
	void SpawnInventory();
	void DestroyInventory();
	void EquipWeapon(AWeapon* InWeapon);
	
	bool IsAiming() const { return bAiming; }
	void SetAiming(bool bPressed) { bAiming = bPressed; }
	AWeapon* GetCurrentWeapon() const { return CurrentWeapon; }
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon")
	float TraceLength = 20000;
	
private:
	void FireTimerFinished();
	
	bool bTriggerPressed = false;
	FTimerHandle FireTimer;
	
	UPROPERTY(Transient, Replicated)
	TArray<TObjectPtr<AWeapon>> Inventory;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon")
	TArray<TSubclassOf<AWeapon>> DefaultWeaponClass;
	
	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing=OnRep_CurrentWeapon, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<AWeapon> CurrentWeapon;
	
	UPROPERTY(BlueprintReadOnly, Replicated, meta=(AllowPrivateAccess = "true"))
	bool bAiming = false;
	
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
	
	void Local_FireWeapon();
};