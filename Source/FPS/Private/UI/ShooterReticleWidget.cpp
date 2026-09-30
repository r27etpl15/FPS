// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ShooterReticleWidget.h"

#include "Character/ShooterCharacter.h"
#include "Combat/CombatComponent.h"
#include "Weapon/Weapon.h"

void UShooterReticleWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	GetOwningPlayer()->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);
	
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetOwningPlayer()->GetPawn());
	if (!IsValid(ShooterCharacter)) { return; }
	
	OnPossessedPawnChanged(nullptr, ShooterCharacter);
	
	if (ShooterCharacter->HasWeaponFirstReplicated())
	{
		AWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(ShooterCharacter);
		if (IsValid(Weapon))
		{
			OnReticleChanged(Weapon->GetReticleDynamicMaterialInstance());
			OnAmmoCounterChanged(Weapon->GetAmmoCounterDynamicMaterialInstance(), Weapon->GetAmmo(), Weapon->GetMagCapacity());
		}
	}
	else
	{
		ShooterCharacter->OnWeaponFirstReplicated.AddDynamic(this, &ThisClass::OnWeaponFirstReplicated);
	}
}

void UShooterReticleWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UShooterReticleWidget::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{	
	// 从 OldPawn CombatComponent 解绑委托
	// 绑定在 NewPawn CombatComponent 里面的委托
	
	UCombatComponent* OldPawnComponent = UCombatComponent::FindCombatComponent(OldPawn);
	if (IsValid(OldPawnComponent))
	{
		OldPawnComponent->OnReticleChanged.RemoveDynamic(this, &ThisClass::OnReticleChanged);
		OldPawnComponent->OnAmmoCounterChanged.RemoveDynamic(this, &ThisClass::OnAmmoCounterChanged);
	}
	
	UCombatComponent* NewPawnComponent = UCombatComponent::FindCombatComponent(NewPawn);
	if (IsValid(NewPawnComponent))
	{
		NewPawnComponent->OnReticleChanged.AddDynamic(this, &ThisClass::OnReticleChanged);
		NewPawnComponent->OnAmmoCounterChanged.AddDynamic(this, &ThisClass::OnAmmoCounterChanged);
	}
}

void UShooterReticleWidget::OnWeaponFirstReplicated(AWeapon* Weapon)
{
	OnReticleChanged(Weapon->GetReticleDynamicMaterialInstance());
	OnAmmoCounterChanged(Weapon->GetAmmoCounterDynamicMaterialInstance(), Weapon->GetAmmo(), Weapon->GetMagCapacity());
}

void UShooterReticleWidget::OnReticleChanged(UMaterialInstanceDynamic* ReticleDynamicMaterialInstance)
{
	// 将控件上的材质设置为动态材质实例
}

void UShooterReticleWidget::OnAmmoCounterChanged(UMaterialInstanceDynamic* AmmoCounterDynamicMaterialInstance,
	int32 RoundsCurrent, int32 RoundsMax)
{
	// 将控件上的材质设置为动态材质实例
}
