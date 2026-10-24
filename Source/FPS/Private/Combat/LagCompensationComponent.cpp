#include "Combat/LagCompensationComponent.h"

#include "Attributes/HealthComponent.h"
#include "Character/ShooterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "KismetTraceUtils.h"
#include "FPS/FPS.h"
#include "GameFramework/Character.h"
#include "Interfaces/PlayerInterface.h"
#include "Weapon/Weapon.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	SetIsReplicatedByDefault(false);
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!IsValid(GetOwner()) || !GetOwner()->HasAuthority())
	{
		SetComponentTickEnabled(false);
		return;
	}

	ClearHistory();
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValid(GetOwner()) || !GetOwner()->HasAuthority()) { return; }

	CaptureSnapshot();
}

void ULagCompensationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearHistory();
	Super::EndPlay(EndPlayReason);
}

void ULagCompensationComponent::CaptureSnapshot()
{
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const UCapsuleComponent* Capsule = IsValid(Character) ? Character->GetCapsuleComponent() : nullptr;
	if (!IsValid(Capsule)) { return; }

	if (const UHealthComponent* HealthComponent = UHealthComponent::FindHealthComponent(GetOwner()))
	{
		if (HealthComponent->GetDeathState() != EDeathState::NotDead)
		{
			ClearHistory();
			return;
		}
	}
	
	const FVector CapsuleCenter = Capsule->GetComponentLocation();

	FHitboxSnapshot Snapshot;
	Snapshot.TimeSeconds = GetWorld()->GetTimeSeconds();
	Snapshot.ActorTransform = Character->GetActorTransform();
	Snapshot.HeadRadius = HeadRadius;
	Snapshot.HeadCenter = CapsuleCenter + FVector(
		0.0,
		0.0,
		FMath::Max(0.0f, Capsule->GetScaledCapsuleHalfHeight() - HeadRadius));

	History.PushLast(Snapshot);

	const double OldestAllowedTime = Snapshot.TimeSeconds - MaxHistorySeconds;
	while (History.Num() > 0 && History.First().TimeSeconds < OldestAllowedTime)
	{
		History.PopFirst();
	}
	
	while (History.Num() > MaxHistorySamples)
	{
		History.PopFirst();
	}
}

bool ULagCompensationComponent::GetSnapshotAtTime(const double TimeSeconds, FHitboxSnapshot& OutSnapshot) const
{
	if (History.Num() == 0) { return false; }

	int32 BestIndex = 0;
	double BestDelta = FMath::Abs(History.First().TimeSeconds - TimeSeconds);
	for (int32 Index = 1; Index < History.Num(); ++Index)
	{
		const double Delta = FMath::Abs(History[Index].TimeSeconds - TimeSeconds);
		if (Delta < BestDelta)
		{
			BestDelta = Delta;
			BestIndex = Index;
		}
	}

	OutSnapshot = History[BestIndex];
	return true;
}

void ULagCompensationComponent::ClearHistory()
{
	History.Reset();
}

FHitResult ULagCompensationComponent::RewindTrace(
	const FVector& ViewLocation,
	const FVector& ViewDirection,
	const float MaxDistance,
	const float TraceRadius,
	const double RewindTime,
	const FName HeadBoneName) const
{
	struct FMovedTarget
	{
		AShooterCharacter* Target = nullptr;
		FTransform OriginalTransform = FTransform::Identity;
		FHitboxSnapshot Snapshot;
	};

	const FVector Direction = ViewDirection.GetSafeNormal();
	const FVector TraceEnd = ViewLocation + Direction * MaxDistance;
	
	const AActor* Player = GetOwner();
	FCollisionQueryParams QueryParams(FName(TEXT("LagCompensationTrace")), false);
	QueryParams.AddIgnoredActor(Player);

	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse.SetAllChannels(ECR_Ignore);
	ResponseParams.CollisionResponse.SetResponse(ECC_Pawn, ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECC_PhysicsBody, ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECC_WorldStatic, ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECC_WorldDynamic, ECR_Block);

	TArray<FMovedTarget> MovedTargets;

	for (TActorIterator<AShooterCharacter> It(GetWorld()); It; ++It)
	{
		AShooterCharacter* Target = *It;
		if (!IsValid(Target) || Target == Player) { continue; }
		if (!Target->Implements<UPlayerInterface>()) { continue; }

		if (const UHealthComponent* TargetHealth = UHealthComponent::FindHealthComponent(Target))
		{
			if (TargetHealth->GetDeathState() != EDeathState::NotDead) { continue; }
		}

		const ULagCompensationComponent* TargetLagCompensation = Target->GetLagCompensationComponent();
		if (!IsValid(TargetLagCompensation)) { continue; }

		FHitboxSnapshot Snapshot;
		if (!TargetLagCompensation->GetSnapshotAtTime(RewindTime, Snapshot))
		{
			QueryParams.AddIgnoredActor(Target);
			continue;
		}

		FMovedTarget MovedTarget;
		MovedTarget.Target = Target;
		MovedTarget.OriginalTransform = Target->GetActorTransform();
		MovedTarget.Snapshot = Snapshot;
		MovedTargets.Add(MovedTarget);

		Target->SetActorTransform(
			Snapshot.ActorTransform,
			false,
			nullptr,
			ETeleportType::TeleportPhysics);

		if (bDrawDebug)
		{
			if (const USkeletalMeshComponent* Mesh = Target->GetMesh())
			{
				const FBoxSphereBounds MeshBounds = Mesh->Bounds;
				DrawDebugBox(
					GetWorld(),
					MeshBounds.Origin,
					MeshBounds.BoxExtent,
					FQuat::Identity,
					FColor::Cyan,
					false,
					5.0f);
			}
			DrawDebugSphere(
				GetWorld(),
				Snapshot.HeadCenter,
				Snapshot.HeadRadius,
				12,
				FColor::Red,
				false,
				5.0f);
		}
	}

	FHitResult HitResult;
	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		ViewLocation,
		TraceEnd,
		FQuat::Identity,
		FPSTraceChannels::ECC_Weapon,
		FCollisionShape::MakeSphere(TraceRadius),
		QueryParams,
		ResponseParams);
	
	// 防止没有命中时子弹轨迹朝向世界原点
	if (!bHit)
	{
		HitResult.ImpactPoint = TraceEnd;
	}

	// 爆头只由服务器头部球判定，不采信真实扫掠返回的骨骼名
	HitResult.BoneName = NAME_None;

	if (AShooterCharacter* HitTarget = Cast<AShooterCharacter>(HitResult.GetActor()))
	{
		for (const FMovedTarget& MovedTarget : MovedTargets)
		{
			if (MovedTarget.Target != HitTarget) { continue; }

			const float HeadRadiusWithTrace = MovedTarget.Snapshot.HeadRadius + TraceRadius;
			if (FVector::DistSquared(HitResult.ImpactPoint, MovedTarget.Snapshot.HeadCenter)
				<= FMath::Square(HeadRadiusWithTrace))
			{
				HitResult.BoneName = HeadBoneName;
			}
			break;
		}
	}

	for (const FMovedTarget& MovedTarget : MovedTargets)
	{
		if (!IsValid(MovedTarget.Target)) { continue; }

		MovedTarget.Target->SetActorTransform(
			MovedTarget.OriginalTransform,
			false,
			nullptr,
			ETeleportType::TeleportPhysics);
	}
	
	if (bDrawDebug)
	{
		DrawDebugSphereTraceSingle(
			GetWorld(),
			ViewLocation,
			TraceEnd,
			TraceRadius,
			EDrawDebugTrace::ForDuration,
			bHit,
			HitResult,
			FLinearColor::Yellow,
			FLinearColor::Red,
			5.0f
		);	
	}

	return HitResult;
}
