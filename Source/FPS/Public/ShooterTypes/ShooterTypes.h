#pragma once

#include "ShooterTypes.generated.h"

UENUM(BlueprintType)
enum class ETurningInPlace : uint8 
{
	Left UMETA(DisplayName = "TurningLeft"),
	Right UMETA(DisplayName = "TurningRight"),
	NotTurning UMETA(DisplayName = "NotTurning")
};

USTRUCT(BlueprintType)
struct FReticleParams
{
	GENERATED_BODY()
	
	// 形状裁切因子
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ShapeCutFactor_RoundFired = 0.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ShapeCutFactor_Aiming = 0.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ShapeCutFactor_NotAiming = 0.0f;
	
	// 缩放因子
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_RoundFired = 0.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_Aiming = 0.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_NotAiming = 0.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_Targeting = 0.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ScaleFactor_NotTargeting = 0.0f;
	
	// 插值速度
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RoundFiredInterpSpeed = 20.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AimingInterpSpeed = 15.0f;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TargetingPlayerInterpSpeed = 10.0f;
};

UENUM(meta=(BitFlags))
enum class ESpecialElimType : uint16
{
	None			= 0,
	HeadShot		= 1 << 0,	// 00000000 00000001
	Sequential		= 1 << 1,	// 00000000 00000010 快速连杀
	Streak			= 1 << 2,	// 00000000 00000100 连杀状态
	Revenge			= 1 << 3,	// 00000000 00001000
	Dethrone		= 1 << 4,	// 00000000 00010000 终结连杀
	ShowStopper		= 1 << 5,	// 00000000 00100000
	FirstBlood		= 1 << 6,	// 00000000 01000000
	GainedTheLead	= 1 << 7,	// 00000000 10000000
	TiedTheLeader	= 1 << 8,	// 00000001 00000000 追平领先者
	LostTheLead		= 1 << 9	// 00000010 00000000
};

ENUM_CLASS_FLAGS(ESpecialElimType)