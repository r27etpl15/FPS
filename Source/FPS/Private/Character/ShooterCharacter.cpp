// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ShooterCharacter.h"

#include "EnhancedInputComponent.h"
#include "Attributes/HealthComponent.h"
#include "Camera/CameraComponent.h"
#include "Combat/CombatComponent.h"
#include "Combat/LagCompensationComponent.h"
#include "Components/CapsuleComponent.h"
#include "Data/WeaponData.h"
#include "Elimination/EliminationComponent.h"
#include "FPS/FPS.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Game/ShooterGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/ShooterPlayerController.h"
#include "ShooterTypes/ShooterTypes.h"
#include "Weapon/Weapon.h"


AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 0.0f;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 15.0f;
	SpringArm->bUsePawnControlRotation = true;
	
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(SpringArm);
	FirstPersonCamera->bUsePawnControlRotation = false;
	FirstPersonCamera->SetOrthoNearClipPlane(1.0f);
	
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh1P");
	Mesh1P->SetupAttachment(FirstPersonCamera);
	Mesh1P->bOnlyOwnerSee = true;
	Mesh1P->bOwnerNoSee = false;
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->bReceivesDecals = false;
	Mesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered;
	Mesh1P->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	
	GetMesh()->bOnlyOwnerSee = false;
	GetMesh()->bOwnerNoSee = true;
	GetMesh()->bReceivesDecals = false;
	if (GetNetMode() == NM_DedicatedServer)
	{
		GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}
	else
	{
		GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered;
	}
	
	GetCharacterMovement()->MovementState.bCanCrouch = true;
	
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	EliminationComponent = CreateDefaultSubobject<UEliminationComponent>(TEXT("EliminationComponent"));
	EliminationComponent->SetIsReplicated(false);
	LagCompensationComponent = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent"));
	
	DefaultFOV = 90.0f;
	
	TurningStatus = ETurningInPlace::NotTurning;
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	FirstPersonCamera->SetFieldOfView(DefaultFOV);
	
	StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
	
	HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
	
	if (AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetController()))
	{
		PlayerController->SetPawnAlive(true);
	}
	
	if (HasAuthority())
	{
		CombatComponent->OnRoundReported.AddDynamic(EliminationComponent, &UEliminationComponent::OnRoundReported);
	}
	
	if (GetNetMode() == NM_DedicatedServer)
	{
		GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}
}

void AShooterCharacter::BeginDestroy()
{
	Super::BeginDestroy();
	
	if (CombatComponent)
	{
		CombatComponent->DestroyInventory();	
	}
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	CalculateTurnInPlaceParameters(DeltaTime);
	CalculateFABRIKSocketTransform();
}

void AShooterCharacter::CalculateTurnInPlaceParameters(float DeltaTime)
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	
	if (Speed == 0.0f && !bIsInAir)
	{
		FRotator CurrentAimRotation(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		// 在 BeginPlay里面初始化StartingAimRotation
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_YAW = DeltaAimRotation.Yaw;
		
		
		if (TurningStatus == ETurningInPlace::NotTurning)
		{
			InterpAO_Yaw = AO_YAW;
		}
		
		TurnInPlace(DeltaTime);
	}
	
	if (Speed > 0.0f || bIsInAir)
	{
		StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		AO_YAW = 0.0f;
		
		FRotator AimRotation = GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(GetVelocity());
		
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		TurningStatus = ETurningInPlace::NotTurning;
	}
	
	AO_YAW *= -1.0f;
}

void AShooterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_YAW > 90.0f)
	{
		TurningStatus = ETurningInPlace::Right;
	}
	else if (AO_YAW < -90.0f)
	{
		TurningStatus = ETurningInPlace::Left;
	}
	
	// 在转向中
	if (TurningStatus != ETurningInPlace::NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.0f, DeltaTime, 4.0f);
		AO_YAW = InterpAO_Yaw;
		if (FMath::Abs(AO_YAW) < 5.0f)
		{
			TurningStatus = ETurningInPlace::NotTurning;
			StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		}
	}
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* ShooterInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	
	ShooterInputComponent->BindAction(CycleWeaponAction, ETriggerEvent::Started, this, &ThisClass::Input_CycleWeapon);
	ShooterInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Started, this, &ThisClass::Input_FireWeaponPressed);
	ShooterInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Completed, this, &ThisClass::Input_FireWeaponReleased);
	ShooterInputComponent->BindAction(AimWeaponAction, ETriggerEvent::Started, this, &ThisClass::Input_AimPressed);
	ShooterInputComponent->BindAction(AimWeaponAction, ETriggerEvent::Completed, this, &ThisClass::Input_AimReleased);
	ShooterInputComponent->BindAction(ReloadWeaponAction, ETriggerEvent::Started, this, &ThisClass::Input_ReloadWeapon);
}

void AShooterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (IsValid(CombatComponent))
	{
		CombatComponent->SpawnInventory();
		
		// SpawnInventory 的 UI 广播发生在 PossessedBy 期间，早于 UI 重绑（OnPossessedPawnChanged）；
		// 下一帧再广播一次，确保 UI 已绑到新组件（修复监听主机复活后准星残留）
		TWeakObjectPtr<UCombatComponent> WeakCombatComponent = CombatComponent;
		GetWorldTimerManager().SetTimerForNextTick([WeakCombatComponent]()
		{
			if (WeakCombatComponent.IsValid())
			{
				WeakCombatComponent->InitializeWeaponWidgets();
			}
		});
	}
}

void AShooterCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}

FName AShooterCharacter::GetWeaponAttachPoint_Implementation(const FGameplayTag& WeaponTypeTag) const
{
	checkf(CombatComponent->WeaponData, TEXT("没有Weapon Data Asset，请在角色类里面填写"));
	
	return CombatComponent->WeaponData->GripPoints.FindChecked(WeaponTypeTag);
}

USkeletalMeshComponent* AShooterCharacter::GetMesh1P_Implementation() const
{
	return Mesh1P;
}

USkeletalMeshComponent* AShooterCharacter::GetMesh3P_Implementation() const
{
	return GetMesh();
}

void AShooterCharacter::WeaponReplicated_Implementation()
{
	if (!bWeaponFirstReplicated)
	{
		bWeaponFirstReplicated = true;
		OnWeaponFirstReplicated.Broadcast(CombatComponent->GetCurrentWeapon(), CombatComponent->IsHitPlayer());
	}
}

AWeapon* AShooterCharacter::GetCurrentWeapon_Implementation() const
{
	return CombatComponent->GetCurrentWeapon();
}

int32 AShooterCharacter::GetReserveAmmo_Implementation() const
{
	return CombatComponent->GetCurrentReserveAmmo();
}

void AShooterCharacter::Notify_CycleWeapon_Implementation()
{
	CombatComponent->Notify_CycleWeapon();
}

void AShooterCharacter::Notify_ReloadWeapon_Implementation()
{
	CombatComponent->Notify_ReloadWeapon();
}

void AShooterCharacter::AddAmmo_Implementation(const FGameplayTag& WeaponType, int32 AmmoAmount)
{
	if (HasAuthority() && IsValid(CombatComponent))
	{
		CombatComponent->AddAmmo(WeaponType, AmmoAmount);
	}
}

bool AShooterCharacter::DoDamage_Implementation(float DamageAmount, AActor* DamageInstigator)
{
	// 反作弊：伤害结算仅允许在服务器执行
	if (!HasAuthority()) { return false; }
	if (!IsValid(HealthComponent)) { return false; }
	
	// 致命攻击则不播放受击动画
	if (HealthComponent->ChangeHealthByAmount(-DamageAmount, DamageInstigator))
	{
		return true;
	}
	
	const int32 MontageSelection = FMath::RandRange(0, HitReacts.Num() - 1);
	
	Multicast_HitReact(MontageSelection);
	
	return false;
}

void AShooterCharacter::Multicast_HitReact_Implementation(int32 MontageIndex)
{
	if (GetNetMode() != NM_DedicatedServer && !IsLocallyControlled())
	{
		if (HitReacts[MontageIndex])
		{
			GetMesh()->GetAnimInstance()->Montage_Play(HitReacts[MontageIndex]);	
		}
	}
}

FRotator AShooterCharacter::GetFixedAimRotation() const
{
	FRotator AimRotation = GetBaseAimRotation();
	if (AimRotation.Pitch > 90.0f && !IsLocallyControlled())
	{
		// 把俯仰在 [270, 360) 映射到 [-90, 0]，默认 Y 轴 (上下瞄准) 方向是 360 度的	
		const FVector2D InRange(270.0f, 360.0f);
		const FVector2D OutRange(-90.0f, 0.0f);
		AimRotation.Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AimRotation.Pitch);
	}
	
	return AimRotation;
}

bool AShooterCharacter::HasCurrentWeapon() const
{
	return IsValid(CombatComponent) && CombatComponent->GetCurrentWeapon() != nullptr;
}

void AShooterCharacter::OnDeathStarted()
{
	if (HasAuthority())
	{
		if (IsValid(LagCompensationComponent))
		{
			LagCompensationComponent->ClearHistory();
		}
		
		CombatComponent->DestroyInventory();
		GetWorld()->GetTimerManager().SetTimer(DeathTimer, this, &ThisClass::DeathTimerFinished, RespawnTime);
	}
	
	if (GetNetMode() != NM_DedicatedServer)
	{
		DeathEffects();
		if (AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetController()); IsValid(PlayerController))
		{
			DisableInput(PlayerController);
			if (PlayerController->IsLocalController())
			{
				PlayerController->SetPawnAlive(false);
			}
		}
	}
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(FPSTraceChannels::ECC_Weapon, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(FPSTraceChannels::ECC_Weapon, ECR_Ignore);
}

void AShooterCharacter::DeathTimerFinished()
{
	AShooterGameModeBase* GameMode = Cast<AShooterGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (IsValid(GameMode))
	{
		GameMode->RequestRespawn(this, GetController());
	}
}

void AShooterCharacter::Input_CycleWeapon()
{
	CombatComponent->Initiate_CycleWeapon();
}

void AShooterCharacter::Input_ReloadWeapon()
{
	CombatComponent->Initiate_ReloadWeapon();
}

void AShooterCharacter::Input_FireWeaponPressed()
{
	CombatComponent->Initiate_FireWeaponPressed();
}

void AShooterCharacter::Input_FireWeaponReleased()
{
	CombatComponent->Initiate_FireWeaponReleased();
}

void AShooterCharacter::Input_AimPressed()
{
	CombatComponent->Initiate_AimPressed();
	OnAim(true);
}

void AShooterCharacter::Input_AimReleased()
{
	CombatComponent->Initiate_AimReleased();
	OnAim(false);
}

void AShooterCharacter::CalculateFABRIKSocketTransform()
{
	if (IsValid(CombatComponent) && IsValid(CombatComponent->GetCurrentWeapon()) && IsValid(CombatComponent->GetCurrentWeapon()->GetMesh3P()))
	{
		FABRIKSocketTransform = CombatComponent->GetCurrentWeapon()->GetMesh3P()->GetSocketTransform("FABRIK_Socket", RTS_World);
		
		FVector OutLocation;
		FRotator OutRotation;
		GetMesh()->TransformToBoneSpace(
			"hand_r",
			FABRIKSocketTransform.GetLocation(),
			FABRIKSocketTransform.GetRotation().Rotator(),
			OutLocation,
			OutRotation
		);
		
		FABRIKSocketTransform.SetLocation(OutLocation);
		FABRIKSocketTransform.SetRotation(OutRotation.Quaternion());
	}
}