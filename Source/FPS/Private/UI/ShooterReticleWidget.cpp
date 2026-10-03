// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ShooterReticleWidget.h"

#include "Character/ShooterCharacter.h"
#include "Combat/CombatComponent.h"
#include "Components/Image.h"
#include "Weapon/Weapon.h"

namespace Ammo
{
	const FName Rounds_Current = FName("Rounds_Current");
	const FName Rounds_Max = FName("Rounds_Max");
}

namespace Reticle
{
	const FName RoundedCornerScale = FName("RoundedCornerScale");
	const FName ShapeCutThickness = FName("ShapeCutThickness");
	const FName Inner_RGBA = FName("Inner_RGBA");
}

void UShooterReticleWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	Image_Reticle->SetRenderOpacity(0.0f);
	Image_AmmoCounter->SetRenderOpacity(0.0f);
	BaseCornerScaleFactor_RoundFired = 0.0f;
	BaseShapeCutFactor_RoundFired = 0.0f;
	BaseCornerScaleFactor_Aiming = 0.0f;
	BaseShapeCutFactor_Aiming = 0.0f;
	
	bAiming = false;
	bTargetingPlayer = false;
	
	GetOwningPlayer()->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);
	
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetOwningPlayer()->GetPawn());
	if (!IsValid(ShooterCharacter)) { return; }
	UCombatComponent* CombatComponent = UCombatComponent::FindCombatComponent(ShooterCharacter);
	if (!IsValid(CombatComponent)) { return; }
	
	OnPossessedPawnChanged(nullptr, ShooterCharacter);
	
	if (ShooterCharacter->HasWeaponFirstReplicated())
	{
		AWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(ShooterCharacter);
		if (IsValid(Weapon))
		{
			OnReticleChanged(Weapon->GetReticleDynamicMaterialInstance(), Weapon->GetReticleParams(), CombatComponent->IsHitPlayer());
			OnAmmoCounterChanged(Weapon->GetAmmoCounterDynamicMaterialInstance(), Weapon->GetAmmo(), Weapon->GetMagCapacity());
		}
	}
	else
	{
		ShooterCharacter->OnWeaponFirstReplicated.AddDynamic(this, &ThisClass::OnWeaponFirstReplicated);
	}
	
	if (ShooterCharacter->HasAuthority())
	{
		AWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(ShooterCharacter);
		if (IsValid(Weapon))
		{
			OnReticleChanged(Weapon->GetReticleDynamicMaterialInstance(), Weapon->GetReticleParams(), CombatComponent->IsHitPlayer());
			OnAmmoCounterChanged(Weapon->GetAmmoCounterDynamicMaterialInstance(), Weapon->GetAmmo(), Weapon->GetMagCapacity());
		}
	}
}

void UShooterReticleWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	BaseCornerScaleFactor_RoundFired = FMath::FInterpTo(
		BaseCornerScaleFactor_RoundFired, 
		0.0f,
		InDeltaTime, 
		CurrentReticleParams.RoundFiredInterpSpeed
	);
	BaseShapeCutFactor_RoundFired = FMath::FInterpTo(
		BaseShapeCutFactor_RoundFired,
		0.0f,
		InDeltaTime,
		CurrentReticleParams.RoundFiredInterpSpeed
	);
	BaseCornerScaleFactor_Aiming = FMath::FInterpTo(
		BaseCornerScaleFactor_Aiming, 
		bAiming ? CurrentReticleParams.ScaleFactor_Aiming : CurrentReticleParams.ScaleFactor_NotAiming,
		InDeltaTime, 
		CurrentReticleParams.AimingInterpSpeed
	);
	BaseShapeCutFactor_Aiming = FMath::FInterpTo(
		BaseShapeCutFactor_Aiming,
		bAiming ? CurrentReticleParams.ShapeCutFactor_Aiming : CurrentReticleParams.ShapeCutFactor_NotAiming,
		InDeltaTime,
		CurrentReticleParams.AimingInterpSpeed
	);
	BaseCornerScaleFactor_TargetingPlayer = FMath::FInterpTo(
		BaseCornerScaleFactor_TargetingPlayer,
		bTargetingPlayer ? CurrentReticleParams.ScaleFactor_Targeting : CurrentReticleParams.ScaleFactor_NotTargeting,
		InDeltaTime,
		CurrentReticleParams.TargetingPlayerInterpSpeed
	);
	
	BaseCornerScaleFactor = BaseCornerScaleFactor_RoundFired + BaseCornerScaleFactor_Aiming + BaseCornerScaleFactor_TargetingPlayer ;
	BaseShapeCutFactor = BaseShapeCutFactor_RoundFired + BaseShapeCutFactor_Aiming;	
	
	if (CurrentReticle_DynamicMaterialInstance.IsValid())
	{
		CurrentReticle_DynamicMaterialInstance->SetScalarParameterValue(Reticle::RoundedCornerScale, BaseCornerScaleFactor);
		CurrentReticle_DynamicMaterialInstance->SetScalarParameterValue(Reticle::ShapeCutThickness, BaseShapeCutFactor);
	}
}

void UShooterReticleWidget::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{	
	UCombatComponent* OldPawnComponent = UCombatComponent::FindCombatComponent(OldPawn);
	if (IsValid(OldPawnComponent))
	{
		OldPawnComponent->OnReticleChanged.RemoveDynamic(this, &ThisClass::OnReticleChanged);
		OldPawnComponent->OnAmmoCounterChanged.RemoveDynamic(this, &ThisClass::OnAmmoCounterChanged);
		OldPawnComponent->OnRoundFired.RemoveDynamic(this, &ThisClass::OnRoundFired);
		OldPawnComponent->OnAimingStatusChanged.RemoveDynamic(this, &ThisClass::OnAimingStatusChanged);
		OldPawnComponent->OnTargetingPlayerStatusChanged.RemoveDynamic(this, &ThisClass::OnTargetingPlayerStatusChanged);
	}
	
	UCombatComponent* NewPawnComponent = UCombatComponent::FindCombatComponent(NewPawn);
	if (IsValid(NewPawnComponent))
	{
		Image_Reticle->SetRenderOpacity(1.0f);
		Image_AmmoCounter->SetRenderOpacity(1.0f);
		
		NewPawnComponent->OnReticleChanged.AddDynamic(this, &ThisClass::OnReticleChanged);
		NewPawnComponent->OnAmmoCounterChanged.AddDynamic(this, &ThisClass::OnAmmoCounterChanged);
		NewPawnComponent->OnRoundFired.AddDynamic(this, &ThisClass::OnRoundFired);
		NewPawnComponent->OnAimingStatusChanged.AddDynamic(this, &ThisClass::OnAimingStatusChanged);
		NewPawnComponent->OnTargetingPlayerStatusChanged.AddDynamic(this, &ThisClass::OnTargetingPlayerStatusChanged);
	}
}

void UShooterReticleWidget::OnWeaponFirstReplicated(AWeapon* Weapon, bool bCurrentlyTargetingPlayer)
{
	OnReticleChanged(Weapon->GetReticleDynamicMaterialInstance(), Weapon->GetReticleParams(), bCurrentlyTargetingPlayer);
	OnAmmoCounterChanged(Weapon->GetAmmoCounterDynamicMaterialInstance(), Weapon->GetAmmo(), Weapon->GetMagCapacity());
}

void UShooterReticleWidget::OnReticleChanged(UMaterialInstanceDynamic* ReticleDynamicMaterialInstance, const FReticleParams& ReticleParams
	, bool bCurrentlyTargetingPlayer)
{
	CurrentReticleParams = ReticleParams;
	CurrentReticle_DynamicMaterialInstance = ReticleDynamicMaterialInstance;
	
	FSlateBrush Brush;
	Brush.SetResourceObject(ReticleDynamicMaterialInstance);
	if (IsValid(Image_Reticle))
	{
		Image_Reticle->SetBrush(Brush);
	}
	
	OnTargetingPlayerStatusChanged(bCurrentlyTargetingPlayer);
}

void UShooterReticleWidget::OnAmmoCounterChanged(UMaterialInstanceDynamic* AmmoCounterDynamicMaterialInstance,
	int32 RoundsCurrent, int32 RoundsMax)
{
	CurrentAmmoCounter_DynamicMaterialInstance = AmmoCounterDynamicMaterialInstance;
	CurrentAmmoCounter_DynamicMaterialInstance->SetScalarParameterValue(Ammo::Rounds_Current, RoundsCurrent);
	CurrentAmmoCounter_DynamicMaterialInstance->SetScalarParameterValue(Ammo::Rounds_Max, RoundsMax);
	
	FSlateBrush Brush;
	Brush.SetResourceObject(AmmoCounterDynamicMaterialInstance);
	if (IsValid(Image_AmmoCounter))
	{
		Image_AmmoCounter->SetBrush(Brush);
	}
}

void UShooterReticleWidget::OnRoundFired(int32 RoundsCurrent, int32 RoundsMax, int32 RoundsInReserve)
{
	BaseCornerScaleFactor_RoundFired += CurrentReticleParams.ScaleFactor_RoundFired;
	BaseShapeCutFactor_RoundFired += CurrentReticleParams.ShapeCutFactor_RoundFired;
	
	if (CurrentAmmoCounter_DynamicMaterialInstance.IsValid())
	{
		CurrentAmmoCounter_DynamicMaterialInstance->SetScalarParameterValue(Ammo::Rounds_Current, RoundsCurrent);
		CurrentAmmoCounter_DynamicMaterialInstance->SetScalarParameterValue(Ammo::Rounds_Max, RoundsMax);
	}
}

void UShooterReticleWidget::OnAimingStatusChanged(bool bIsAiming)
{
	bAiming = bIsAiming;
}

void UShooterReticleWidget::OnTargetingPlayerStatusChanged(bool bIsTargetingPlayer)
{
	bTargetingPlayer = bIsTargetingPlayer;
	if (CurrentReticle_DynamicMaterialInstance.IsValid())
	{
		FLinearColor ReticleColor = bTargetingPlayer ? FLinearColor::Red : FLinearColor::White;
		CurrentReticle_DynamicMaterialInstance->SetVectorParameterValue(Reticle::Inner_RGBA, ReticleColor);
	}
}
