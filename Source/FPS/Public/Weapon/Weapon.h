// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UCLASS()
class FPS_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();
	
	FGameplayTag GetWeaponTypeTag() const { return WeaponTypeTag; }
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }
	
	virtual void OnRep_Instigator() override;
	
	void AttachToOwningPawn() const;

protected:
	virtual void BeginPlay() override;
	
private:
	// 面相第一人称
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> Mesh1P;
	
	// 面相第三人称
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> Mesh3P;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|WeaponType", meta=(AllowPrivateAccess = "true"))
	FGameplayTag WeaponTypeTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Aiming", meta=(AllowPrivateAccess = "true"))
	float AimFOV;
	
	void SetMeshVisibilities(APawn* OwningPawn) const;
};
