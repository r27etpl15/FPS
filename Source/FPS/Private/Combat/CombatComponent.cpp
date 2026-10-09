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
	if (IsValid(CurrentWeapon))
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
	if (!IsValid(CurrentWeapon)) { return; }
	if (CurrentWeapon->GetWeaponStatus() == EWeaponStatus::Cycling) { return; }
	
	AdvanceWeaponIndex();
	Local_CycleWeapon(Local_WeaponIndex);
}

void UCombatComponent::Local_CycleWeapon(int32 WeaponIndex)
{
	AWeapon* NextWeapon = Inventory[WeaponIndex];
	if (!IsValid(NextWeapon) || !IsValid(WeaponData)) { return; }
	
	CurrentWeapon->SetWeaponStatus(EWeaponStatus::Cycling);
	NextWeapon->SetWeaponStatus(EWeaponStatus::Cycling);
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	const bool bIsLocal = IsValid(OwningPawn) && OwningPawn->IsLocallyControlled();
	
	const FMontageData MontageData = bIsLocal ? WeaponData->FirstPersonMontages.FindChecked(NextWeapon->GetWeaponTypeTag())
		: WeaponData->ThirdPersonMontages.FindChecked(NextWeapon->GetWeaponTypeTag());
	USkeletalMeshComponent* MeshComponent = bIsLocal ? IPlayerInterface::Execute_GetMesh1P(OwningPawn) : IPlayerInterface::Execute_GetMesh3P(OwningPawn);
	if (IsValid(MeshComponent) && IsValid(MontageData.EquipMontage))
	{
		MeshComponent->GetAnimInstance()->Montage_Play(MontageData.EquipMontage);
	}
	
	if (bIsLocal)
	{
		Server_CycleWeapon(WeaponIndex);
		MeshComponent->GetAnimInstance()->OnMontageBlendingOut.AddDynamic(this, &ThisClass::BlendOut_CycleWeapon);
	}
}

void UCombatComponent::Server_CycleWeapon_Implementation(int32 WeaponIndex)
{
	Local_WeaponIndex = WeaponIndex;
	Multicast_CycleWeapon(WeaponIndex);
}

void UCombatComponent::Multicast_CycleWeapon_Implementation(int32 WeaponIndex)
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwningPawn)) { return; }
	
	if (!OwningPawn->IsLocallyControlled())
	{
		Local_WeaponIndex = WeaponIndex;
		Local_CycleWeapon(WeaponIndex);
	}
}

void UCombatComponent::Notify_CycleWeapon()
{
	if (!IsValid(CurrentWeapon)) { return; }
	
	AWeapon* NewWeapon = Inventory[Local_WeaponIndex];
	if (IsValid(NewWeapon))
	{
		EquipWeapon(NewWeapon);
	}
}

void UCombatComponent::BlendOut_CycleWeapon(UAnimMontage* Montage, bool bInterrupted)
{
	UAnimInstance* AnimInstance = IPlayerInterface::Execute_GetMesh1P(GetOwner())->GetAnimInstance();
	if (IsValid(AnimInstance) && AnimInstance->OnMontageBlendingOut.IsAlreadyBound(this, &ThisClass::BlendOut_CycleWeapon))
	{
		AnimInstance->OnMontageBlendingOut.RemoveDynamic(this, &ThisClass::BlendOut_CycleWeapon);
	}
	
	CurrentWeapon->SetWeaponStatus(EWeaponStatus::Idle);
	
	// 只需要在本地更新 UI
	OnReticleChanged.Broadcast(CurrentWeapon->GetReticleDynamicMaterialInstance(), CurrentWeapon->GetReticleParams(), bHitPlayer);
	OnAmmoCounterChanged.Broadcast(CurrentWeapon->GetAmmoCounterDynamicMaterialInstance(), CurrentWeapon->GetAmmo(), CurrentWeapon->GetMagCapacity());
	OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, CurrentWeapon->GetAmmo(), CurrentWeapon->GetWeaponIcon());
	
	// 切换武器后按着开火键，并且是自动武器就直接射击
	if (bTriggerPressed && CurrentWeapon->GetFireType() == EFireType::Auto && CurrentWeapon->GetAmmo() > 0)
	{
		Local_FireWeapon();
	}
}

void UCombatComponent::Initiate_FireWeaponPressed()
{
	if (!IsValid(CurrentWeapon)) { return; }
	
	bTriggerPressed = true;
	
	if (CurrentWeapon->GetWeaponStatus() == EWeaponStatus::Idle && CurrentWeapon->GetAmmo() > 0)
	{
		Local_FireWeapon();	
	}
	else if (CurrentWeapon->GetWeaponStatus() == EWeaponStatus::Idle)
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
	if (CurrentWeapon->GetAmmo() <= 0) { return; }
	
	if (GetNetMode() != NM_ListenServer || !Cast<APawn>(GetOwner())->IsLocallyControlled())
	{
		CurrentWeapon->Auth_Fire();
	}
	
	Multicast_FireWeapon(HitResult, CurrentWeapon->GetAmmo());
}

void UCombatComponent::Multicast_FireWeapon_Implementation(const FHitResult& HitResult, int32 AuthAmmo)
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
	Multicast_DryFire();
}

void UCombatComponent::Multicast_DryFire_Implementation()
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
	
	CurrentWeapon->SetWeaponStatus(EWeaponStatus::Firing);
	
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
	
	OnRoundFired.Broadcast(CurrentWeapon->GetAmmo(), CurrentWeapon->GetMagCapacity(), CurrentReserveAmmo);
	
	GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &ThisClass::FireTimerFinished, CurrentWeapon->GetFireTime());
	
	Server_FireWeapon(HitResult);
}

int32 UCombatComponent::AdvanceWeaponIndex()
{
	if (Inventory.Num() > 1)
	{
		Local_WeaponIndex = (Local_WeaponIndex + 1) % Inventory.Num();
	}
	
	return Local_WeaponIndex;
}

void UCombatComponent::FireTimerFinished()
{
	if (!IsValid(CurrentWeapon)) { return; }
	
	if (CurrentWeapon->GetWeaponStatus() == EWeaponStatus::Firing)
	{
		CurrentWeapon->SetWeaponStatus(EWeaponStatus::Idle);
	}
	
	if (bTriggerPressed && CurrentWeapon->GetFireType() == EFireType::Auto && CurrentWeapon->GetAmmo() > 0)
	{
		Local_FireWeapon();
	}
	else if (bTriggerPressed && CurrentWeapon->GetFireType() == EFireType::Auto && CurrentWeapon->GetAmmo() == 0)
	{
		CurrentWeapon->OnDryFire(true);
		
		Server_DryFire();
	}
}

void UCombatComponent::Initiate_ReloadWeapon()
{
	if (!IsValid(CurrentWeapon)) { return; }
	if (CurrentWeapon->GetWeaponStatus() == EWeaponStatus::Cycling || CurrentWeapon->GetWeaponStatus() == EWeaponStatus::Reloading) { return; }
	if (CurrentWeapon->GetAmmo() == CurrentWeapon->GetMagCapacity()) { return; }
	if (CurrentReserveAmmo == 0) { return; }
	
	Local_ReloadWeapon();
	Server_ReloadWeapon();
}

void UCombatComponent::Local_ReloadWeapon()
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!IsValid(CurrentWeapon) || !IsValid(OwningPawn)) { return; }
	ensure(WeaponData);
	
	const bool bIsLocal = OwningPawn->IsLocallyControlled();
	
	// 播放角色蒙太奇
	UAnimMontage* ReloadMontage = bIsLocal ? WeaponData->FirstPersonMontages.FindChecked(CurrentWeapon->GetWeaponTypeTag()).ReloadMontage
		: WeaponData->ThirdPersonMontages.FindChecked(CurrentWeapon->GetWeaponTypeTag()).ReloadMontage;
	USkeletalMeshComponent* MeshComponent = bIsLocal ? IPlayerInterface::Execute_GetMesh1P(OwningPawn) : IPlayerInterface::Execute_GetMesh3P(OwningPawn);
	if (IsValid(ReloadMontage) && IsValid(MeshComponent))
	{
		MeshComponent->GetAnimInstance()->Montage_Play(ReloadMontage);
	}
	
	// 播放武器蒙太奇
	UAnimMontage* WeaponReloadMontage = WeaponData->WeaponMontages.FindChecked(CurrentWeapon->GetWeaponTypeTag()).ReloadMontage;
	USkeletalMeshComponent* WeaponMeshComponent = bIsLocal ? CurrentWeapon->GetMesh1P() : CurrentWeapon->GetMesh3P();
	if (IsValid(WeaponReloadMontage) && IsValid(WeaponMeshComponent))
	{
		WeaponMeshComponent->GetAnimInstance()->Montage_Play(WeaponReloadMontage);
	}
	
	CurrentWeapon->SetWeaponStatus(EWeaponStatus::Reloading);
}

void UCombatComponent::Server_ReloadWeapon_Implementation()
{
	Multicast_ReloadWeapon(CurrentWeapon->GetAmmo(), CurrentReserveAmmo);
}

void UCombatComponent::Multicast_ReloadWeapon_Implementation(int32 NewWeaponAmmo, int32 NewCarriedAmmo)
{
	if (!Cast<APawn>(GetOwner())->IsLocallyControlled())
	{
		Local_ReloadWeapon();
	}
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
		Equip(Inventory[0]);
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

void UCombatComponent::Equip(AWeapon* InWeapon)
{
	CurrentWeapon = InWeapon;
	CurrentWeapon->AttachToOwningPawn(Cast<APawn>(GetOwner()));
	
	CurrentReserveAmmo = ReserveAmmo.FindChecked(CurrentWeapon->GetWeaponTypeTag());
	OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, CurrentWeapon->GetAmmo(), CurrentWeapon->GetWeaponIcon());
}

void UCombatComponent::EquipWeapon(AWeapon* InWeapon)
{
	if (!IsValid(InWeapon) || !IsValid(GetOwner())) { return; }
	
	if (GetOwner()->HasAuthority())
	{
		SetCurrentWeapon(InWeapon, CurrentWeapon);
	}
	else
	{
		Server_EquipWeapon(InWeapon);
	}
}

void UCombatComponent::Server_EquipWeapon_Implementation(AWeapon* InWeapon)
{
	EquipWeapon(InWeapon);
}

void UCombatComponent::SetCurrentWeapon(AWeapon* NewWeapon, AWeapon* LastWeapon)
{
	AWeapon* LocalLastWeapon = nullptr;
	
	if (IsValid(LastWeapon))
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != CurrentWeapon)
	{
		LocalLastWeapon = CurrentWeapon;
	}
	
	if (IsValid(LocalLastWeapon))
	{
		LocalLastWeapon->DetachFromOwningPawn();
		LocalLastWeapon->SetWeaponStatus(EWeaponStatus::Unequipped);
	}
	
	CurrentWeapon = NewWeapon;
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (IsValid(OwningPawn) && IsValid(CurrentWeapon) && OwningPawn->HasAuthority())
	{
		CurrentReserveAmmo = ReserveAmmo.FindChecked(CurrentWeapon->GetWeaponTypeTag());
	}
	
	CurrentWeapon->AttachToOwningPawn(OwningPawn);
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
		OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, CurrentWeapon->GetAmmo(), CurrentWeapon->GetWeaponIcon());
	}
}

void UCombatComponent::OnRep_CurrentWeapon(AWeapon* LastWeapon)
{
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
	
	IPlayerInterface::Execute_WeaponReplicated(GetOwner());
	InitializeWeaponWidgets();
}
