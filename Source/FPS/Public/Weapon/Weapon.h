// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EFireType : uint8
{
	Auto,
	SemiAuto
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
	
	virtual void OnRep_Instigator() override;
	
	void AttachToOwningPawn() const;
	void WeaponTrace(FHitResult& OutHit, float TraceLength);
	void Local_Fire(const FVector& ImpactPoint, const FVector& ImpactNormal, TEnumAsByte<EPhysicalSurface> ImpactSurfaceType, bool bIsFirstPerson);

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
	
	void SetMeshVisibilities(APawn* OwningPawn) const;
};
