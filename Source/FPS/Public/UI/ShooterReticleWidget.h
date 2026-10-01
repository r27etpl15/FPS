// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterTypes/ShooterTypes.h"
#include "ShooterReticleWidget.generated.h"

class AWeapon;
class UImage;
class UMaterialInstanceDynamic;

/**
 * 
 */
UCLASS()
class FPS_API UShooterReticleWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Reticle;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_AmmoCounter;
	
private:
	TWeakObjectPtr<UMaterialInstanceDynamic> CurrentReticle_DynamicMaterialInstance;
	TWeakObjectPtr<UMaterialInstanceDynamic> CurrentAmmoCounter_DynamicMaterialInstance;
	
	FReticleParams CurrentReticleParams;
	float BaseCornerScaleFactor;
	float BaseShapeCutFactor;
	float BaseCornerScaleFactor_RoundFired;
	float BaseShapeCutFactor_RoundFired;
	float BaseCornerScaleFactor_Aiming;
	float BaseShapeCutFactor_Aiming;
	float BaseCornerScaleFactor_TargetingPlayer;
	
	bool bAiming;
	bool bTargetingPlayer;
	
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	
	UFUNCTION()
	void OnWeaponFirstReplicated(AWeapon* Weapon);
	
	UFUNCTION()
	void OnReticleChanged(UMaterialInstanceDynamic* ReticleDynamicMaterialInstance, const FReticleParams& ReticleParams, bool bCurrentlyTargetingPlayer);
	
	UFUNCTION()
	void OnAmmoCounterChanged(UMaterialInstanceDynamic* AmmoCounterDynamicMaterialInstance, int32 RoundsCurrent, int32 RoundsMax);
	
	UFUNCTION()
	void OnRoundFired(int32 RoundsCurrent, int32 RoundsMax);
	
	UFUNCTION()
	void OnAimingStatusChanged(bool bIsAiming);
	
	UFUNCTION()
	void OnTargetingPlayerStatusChanged(bool bIsTargetingPlayer);
};

