// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/CombatComponent.h"

#include "Data/WeaponData.h"
#include "FPS/FPS.h"
#include "Interfaces/PlayerInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::InitializeWeaponWidgets() const
{
	if (!IsValid(CurrentWeapon))
	{
		OnReticleChanged.Broadcast(CurrentWeapon->GetReticleDynamicMaterialInstance(), CurrentWeapon->GetReticleParams(), bHitPlayer);
		OnAmmoCounterChanged.Broadcast(CurrentWeapon->GetAmmoCounterDynamicMaterialInstance(), CurrentWeapon->GetAmmo(), CurrentWeapon->GetMagCapacity());
	}
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwningPawn) || !OwningPawn->IsLocallyControlled()) { return; }
	
	APlayerController* PlayerController = Cast<APlayerController>(OwningPawn->GetController());
	if (!IsValid(PlayerController)) { return; }
	
	FVector EyesWorldLocation;
	FRotator EyesWorldRotation;
	PlayerController->GetActorEyesViewPoint(EyesWorldLocation, EyesWorldRotation);
	const FVector EyesWorldDirection = UKismetMathLibrary::GetForwardVector(EyesWorldRotation);
	
	const FVector Start = EyesWorldLocation;
	const FVector End = EyesWorldLocation + EyesWorldDirection * TraceLength;
	
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	
	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse.SetAllChannels(ECR_Ignore);
	ResponseParams.CollisionResponse.SetResponse(ECC_Pawn, ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECC_PhysicsBody, ECR_Block);
	
	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		FPSTraceChannels::ECC_Weapon,
		QueryParams,
		ResponseParams
	);
	
	bHitPlayer = IsValid(HitResult.GetActor()) && HitResult.GetActor()->Implements<UPlayerInterface>();
	
	if (bHitPlayer != bHitPlayerLastFrame)
	{
		OnTargetingPlayerStatusChanged.Broadcast(bHitPlayer);
	}
	
	bHitPlayerLastFrame = bHitPlayer;
	
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, Inventory);
	DOREPLIFETIME(ThisClass, CurrentWeapon);
	DOREPLIFETIME_CONDITION(ThisClass, bAiming, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ThisClass, CurrentReserveAmmo, COND_OwnerOnly);
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
	else
	{
		CurrentWeapon->OnDryFire(true);
		
		Server_DryFire();
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

void UCombatComponent::Server_DryFire_Implementation()
{
	MultiCast_DryFire();
}

void UCombatComponent::MultiCast_DryFire_Implementation()
{
	if (!IsValid(CurrentWeapon)) { return; }
	
	if (!Cast<APawn>(GetOwner())->IsLocallyControlled())
	{
		CurrentWeapon->OnDryFire(false);	
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
	
	OnRoundFired.Broadcast(CurrentWeapon->GetAmmo(), CurrentWeapon->GetMagCapacity());
	
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
	Local_Aim(true);
	Server_Aim(bAiming);
}

void UCombatComponent::Initiate_AimReleased()
{
	Local_Aim(false);
	Server_Aim(bAiming);
}

void UCombatComponent::Server_Aim_Implementation(bool bPressed)
{
	Local_Aim(bPressed);
}

void UCombatComponent::SpawnInventory()
{
	if (!GetOwner()->HasAuthority()) { return; }
	
	for (const TSubclassOf<AWeapon>& WeaponClass : DefaultWeaponClass)
	{
		AWeapon* Weapon = SpawnWeapon(WeaponClass);
		Inventory.AddUnique(Weapon);
		ReserveAmmo.Add(Weapon->GetWeaponTypeTag(), Weapon->GetStartingCarriedAmmo());
	}
	
	if (Inventory.Num() > 0)
	{
		EquipWeapon(Inventory[0]);
		InitializeWeaponWidgets();
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
	
	CurrentReserveAmmo = ReserveAmmo.FindChecked(CurrentWeapon->GetWeaponTypeTag());
	OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, CurrentWeapon->GetAmmo());
}

void UCombatComponent::Local_Aim(bool bPressed)
{
	bAiming = bPressed;
	
	OnAimingStatusChanged.Broadcast(bAiming);
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

void UCombatComponent::OnRep_CurrentReserveAmmo()
{
	if (IsValid(CurrentWeapon))
	{
		OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, CurrentWeapon->GetAmmo());
	}
}

void UCombatComponent::OnRep_CurrentWeapon(AWeapon* LastWeapon)
{
	if (!IsValid(CurrentWeapon)) { return; }
	
	CurrentWeapon->AttachToOwningPawn();
	IPlayerInterface::Execute_WeaponReplicated(GetOwner());
	InitializeWeaponWidgets();
}
