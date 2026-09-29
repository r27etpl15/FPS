// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/CombatComponent.h"

#include "Data/WeaponData.h"
#include "Interfaces/PlayerInterface.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, Inventory);
	DOREPLIFETIME(ThisClass, CurrentWeapon);
	DOREPLIFETIME_CONDITION(ThisClass, bAiming, COND_SkipOwner);
}

void UCombatComponent::Initiate_CycleWeapon()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("CycleWeapon"));
}

void UCombatComponent::Initiate_FireWeaponPressed()
{
	if (!IsValid(CurrentWeapon)) { return; }
	
	bTriggerPressed = true;
	
	if (CurrentWeapon->GetAmmo() > 0)
	{
		Local_FireWeapon();	
	}
}

void UCombatComponent::Initiate_FireWeaponReleased()
{
	bTriggerPressed = false;
}

void UCombatComponent::Server_FireWeapon_Implementation(const FHitResult& HitResult)
{
	if (!IsValid(CurrentWeapon)) { return; }
	
	if (GetNetMode() != NM_ListenServer || !Cast<APawn>(GetOwner())->IsLocallyControlled())
	{
		CurrentWeapon->Auth_Fire();
	}
	
	MultiCast_FireWeapon(HitResult, CurrentWeapon->GetAmmo());
}

void UCombatComponent::MultiCast_FireWeapon_Implementation(const FHitResult& HitResult, int32 AuthAmmo)
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (OwningPawn->IsLocallyControlled())
	{
		CurrentWeapon->Rep_fire(AuthAmmo);
	}
	else
	{
		ensure(IsValid(WeaponData));
		
		EPhysicalSurface ImpactSurfaceType = HitResult.PhysMaterial.IsValid(false) ? HitResult.PhysMaterial->SurfaceType.GetValue() : SurfaceType1;
		CurrentWeapon->Local_Fire(HitResult.ImpactPoint, HitResult.ImpactNormal, ImpactSurfaceType, false);
	
		UAnimMontage* Montage3P = WeaponData->ThirdPersonMontages.FindChecked(CurrentWeapon->GetWeaponTypeTag()).FireMontage;
		USkeletalMeshComponent* Mesh3P = IPlayerInterface::Execute_GetMesh3P(GetOwner());
		if (IsValid(Montage3P) && IsValid(Mesh3P))
		{
			// 播放的是角色蒙太奇，不是武器的
			Mesh3P->GetAnimInstance()->Montage_Play(Montage3P);
		}
	}
}

void UCombatComponent::Local_FireWeapon()
{
	if (!IsValid(CurrentWeapon)) { return; }
	ensure(IsValid(WeaponData));
	
	UAnimMontage* Montage1P = WeaponData->FirstPersonMontages.FindChecked(CurrentWeapon->GetWeaponTypeTag()).FireMontage;
	USkeletalMeshComponent* Mesh1P = IPlayerInterface::Execute_GetMesh1P(GetOwner());
	if (IsValid(Montage1P) && IsValid(Mesh1P))
	{
		// 播放的是角色蒙太奇，不是武器的
		Mesh1P->GetAnimInstance()->Montage_Play(Montage1P);
	}
	
	FHitResult HitResult;
	CurrentWeapon->WeaponTrace(HitResult, TraceLength);
	
	EPhysicalSurface ImpactSurfaceType = HitResult.PhysMaterial.IsValid(false) ? HitResult.PhysMaterial->SurfaceType.GetValue() : SurfaceType1;
	CurrentWeapon->Local_Fire(HitResult.ImpactPoint, HitResult.ImpactNormal, ImpactSurfaceType, true);
	
	GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &ThisClass::FireTimerFinished, CurrentWeapon->GetFireTime());
	
	Server_FireWeapon(HitResult);
}

void UCombatComponent::FireTimerFinished()
{
	if (!IsValid(CurrentWeapon)) { return; }
	
	if (bTriggerPressed && CurrentWeapon->GetFireType() == EFireType::Auto && CurrentWeapon->GetAmmo() > 0)
	{
		Local_FireWeapon();
	}
}

void UCombatComponent::Initiate_ReloadWeapon()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("ReloadWeapon"));
}

void UCombatComponent::Initiate_AimPressed()
{
	SetAiming(true);
	Server_Aim(bAiming);
}

void UCombatComponent::Initiate_AimReleased()
{
	SetAiming(false);
	Server_Aim(bAiming);
}

void UCombatComponent::Server_Aim_Implementation(bool bPressed)
{
	SetAiming(bPressed);
}

void UCombatComponent::SpawnInventory()
{
	if (!GetOwner()->HasAuthority()) { return; }
	
	for (const TSubclassOf<AWeapon>& WeaponClass : DefaultWeaponClass)
	{
		AWeapon* Weapon = SpawnWeapon(WeaponClass);
		Inventory.AddUnique(Weapon);
	}
	
	if (Inventory.Num() > 0)
	{
		EquipWeapon(Inventory[0]);
	}
}

void UCombatComponent::DestroyInventory()
{
	for (TObjectPtr<AWeapon> Weapon : Inventory)
	{
		if (IsValid(Weapon))
		{
			Weapon->Destroy();
		}
	}
}

void UCombatComponent::EquipWeapon(AWeapon* InWeapon)
{
	CurrentWeapon = InWeapon;
	CurrentWeapon->AttachToOwningPawn();
}

AWeapon* UCombatComponent::SpawnWeapon(TSubclassOf<AWeapon> WeaponClass) const
{
	AActor* OwningActor = GetOwner();
	
	if (!IsValid(OwningActor)) { return nullptr;}
	if (!OwningActor->HasAuthority()) { return nullptr; }
	
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Instigator = Cast<APawn>(OwningActor);
	SpawnParameters.Owner = OwningActor;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	return GetWorld()->SpawnActor<AWeapon>(WeaponClass, SpawnParameters);
}

void UCombatComponent::OnRep_CurrentWeapon(AWeapon* LastWeapon)
{
	if (!IsValid(CurrentWeapon)) { return; }
	
	CurrentWeapon->AttachToOwningPawn();
}
