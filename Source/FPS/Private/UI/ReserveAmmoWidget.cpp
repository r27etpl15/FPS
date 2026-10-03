// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ReserveAmmoWidget.h"

#include "Character/ShooterCharacter.h"
#include "Combat/CombatComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Weapon/Weapon.h"

void UReserveAmmoWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	Image_WeaponIcon->SetRenderOpacity(0.0f);
	Text_Ammo->SetRenderOpacity(0.0f);
	
	GetOwningPlayer()->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);
	
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetOwningPlayer()->GetPawn());
	if (!IsValid(ShooterCharacter)) { return; }
	
	OnPossessedPawnChanged(nullptr, ShooterCharacter);
	
	if (ShooterCharacter->HasWeaponFirstReplicated())
	{
		AWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(ShooterCharacter);
		if (IsValid(Weapon))
		{
			OnCurrentReserveAmmoChanged(IPlayerInterface::Execute_GetReserveAmmo(ShooterCharacter), Weapon->GetAmmo());
		}
	}
	else
	{
		ShooterCharacter->OnWeaponFirstReplicated.AddDynamic(this, &ThisClass::OnWeaponFirstReplicated);
	}
}

void UReserveAmmoWidget::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	UCombatComponent* OldPawnComponent = UCombatComponent::FindCombatComponent(OldPawn);
	if (IsValid(OldPawnComponent))
	{
		OldPawnComponent->OnCurrentReserveAmmoChanged.RemoveDynamic(this, &ThisClass::OnCurrentReserveAmmoChanged);
		OldPawnComponent->OnRoundFired.RemoveDynamic(this, &ThisClass::UReserveAmmoWidget::OnRoundFired);
	}
	
	UCombatComponent* NewPawnComponent = UCombatComponent::FindCombatComponent(NewPawn);
	if (IsValid(NewPawnComponent))
	{
		Image_WeaponIcon->SetRenderOpacity(1.0f);
		Text_Ammo->SetRenderOpacity(1.0f);
		
		OldPawnComponent->OnCurrentReserveAmmoChanged.AddDynamic(this, &ThisClass::OnCurrentReserveAmmoChanged);
		OldPawnComponent->OnRoundFired.AddDynamic(this, &ThisClass::UReserveAmmoWidget::OnRoundFired);
	}
}

void UReserveAmmoWidget::OnCurrentReserveAmmoChanged(int32 RoundsInReserve, int32 RoundsInWeapon)
{
	// 当前弹匣弹药数 / 备用弹药数（剩余弹药数）
	
	if (IsValid(Text_Ammo))
	{
		FText AmmoText = FText::Format(NSLOCTEXT("AmmoText", "AmmoKey", "{0}/{1}"), RoundsInWeapon, RoundsInReserve);
		Text_Ammo->SetText(AmmoText);
	}
}

void UReserveAmmoWidget::OnRoundFired(int32 RoundsCurrent, int32 RoundsMax, int32 RoundsInReserve)
{
	if (IsValid(Text_Ammo))
	{
		FText AmmoText = FText::Format(NSLOCTEXT("AmmoText", "AmmoKey", "{0}/{1}"), RoundsCurrent, RoundsInReserve);
		Text_Ammo->SetText(AmmoText);
	}
}

void UReserveAmmoWidget::OnWeaponFirstReplicated(AWeapon* Weapon, bool bCurrentlyTargetingPlayer)
{
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetOwningPlayer()->GetPawn());
	if (!IsValid(ShooterCharacter)) { return; }
	
	OnCurrentReserveAmmoChanged(IPlayerInterface::Execute_GetReserveAmmo(ShooterCharacter), Weapon->GetAmmo());
}
