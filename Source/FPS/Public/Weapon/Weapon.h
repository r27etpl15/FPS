// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "ShooterTypes/ShooterTypes.h"
#include "Weapon.generated.h"

class UMaterialInstanceDynamic;

UENUM(BlueprintType)
enum class EFireType : uint8
{
	Auto,
	SemiAuto
};

UENUM(BlueprintType)
enum class EWeaponStatus : uint8
{
	Idle,	// 武器待机状态，可开火 / 换弹 / 循环切换武器
	Firing,
	Reloading,
	Cycling,
	Unequipped
};

UCLASS()
class FPS_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();
	
	FGameplayTag GetWeaponTypeTag() const { return WeaponTypeTag; }
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }
	float GetFireTime() const { return FireTime; }
	EFireType GetFireType() const { return FireType; }
	int32 GetAmmo() const { return Ammo; }
	int32 GetMagCapacity() const { return MagCapacity; }
	UMaterialInstanceDynamic* GetReticleDynamicMaterialInstance();
	UMaterialInstanceDynamic* GetAmmoCounterDynamicMaterialInstance();
	FReticleParams GetReticleParams() const { return ReticleParams; }
	int32 GetStartingCarriedAmmo() const { return StartingCarriedAmmo; }
	UMaterialInterface* GetWeaponIcon() const { return WeaponIcon; }
	EWeaponStatus GetWeaponStatus() const { return WeaponStatus; }
	float GetDamage() const { return Damage; }
	float GetHeadShotDamageMultiplier() const { return HeadShotDamageMultiplier; }
	float GetRecoilPitch() const { return RecoilPitch; }
	float GetRecoilYaw() const { return RecoilYaw; }
	float GetRecoilRecoverySpeed() const { return RecoilRecoverySpeed; }
	
	void SetWeaponStatus(const EWeaponStatus InWeaponStatus) { WeaponStatus = InWeaponStatus; }
	void SetAmmo(const int32 InAmmo) { Ammo = InAmmo; }
	
	void AttachToOwningPawn(APawn* Pawn) const;
	void DetachFromOwningPawn();
	void WeaponTrace(FHitResult& OutHit, float TraceLength);
	void Local_Fire(const FVector& ImpactPoint, const FVector& ImpactNormal, TEnumAsByte<EPhysicalSurface> ImpactSurfaceType, bool bIsFirstPerson);
	void Auth_Fire();
	void Rep_fire(int32 AuthAmmo);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnDryFire(bool bIsFirstPerson);
	
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void FireEffects(const FVector& ImpactPoint, const FVector& ImpactNormal, EPhysicalSurface ImpactSurfaceType, bool bIsFirstPerson);
	
private:
	// 面相第一人称
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS|Weapon", meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh1P;
	
	// 面相第三人称
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS|Weapon", meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh3P;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|WeaponType", meta=(AllowPrivateAccess = "true"))
	FGameplayTag WeaponTypeTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Aiming", meta=(AllowPrivateAccess = "true"))
	float AimFOV = 65.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Trace", meta=(AllowPrivateAccess = "true"))
	float TraceRadius = 3.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Trace", meta=(AllowPrivateAccess = "true"))
	bool bShowDebugSphereTrace = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|FireType", meta=(AllowPrivateAccess = "true"))
	EFireType FireType = EFireType::SemiAuto;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|FireType", meta=(AllowPrivateAccess = "true"))
	float FireTime = 0.1f;
	
	UPROPERTY(EditAnywhere, Category="FPS|Ammo")
	int32 MagCapacity = 10;
	
	UPROPERTY(EditAnywhere, Category="FPS|Ammo")
	int32 Ammo = 5;
	
	UPROPERTY(EditAnywhere, Category="FPS|Ammo")
	int32 StartingCarriedAmmo = 10;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|Damage", meta=(AllowPrivateAccess = "true"))
	float Damage = 15.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|Damage", meta=(AllowPrivateAccess = "true"))
	float HeadShotDamageMultiplier = 3.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|Recoil", meta=(AllowPrivateAccess = "true"))
	float RecoilPitch = 0.8f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|Recoil", meta=(AllowPrivateAccess = "true"))
	float RecoilYaw = 0.3f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|Recoil", meta=(AllowPrivateAccess = "true"))
	float RecoilRecoverySpeed = 6.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon")
	TObjectPtr<UMaterialInterface> ReticleMaterialInterface;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon")
	TObjectPtr<UMaterialInterface> AmmoCounterMaterialInterface;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Reticle")
	FReticleParams ReticleParams;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Icon")
	TObjectPtr<UMaterialInterface> WeaponIcon;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterialInstance_Reticle;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterialInstance_AmmoCounter;
	
	EWeaponStatus WeaponStatus = EWeaponStatus::Idle;
	
	int32 Sequence = 0;
	
	void SetMeshVisibilities(APawn* OwningPawn) const;
};
