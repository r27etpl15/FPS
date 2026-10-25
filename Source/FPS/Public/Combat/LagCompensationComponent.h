#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/Deque.h"
#include "Engine/HitResult.h"
#include "LagCompensationComponent.generated.h"

class AShooterCharacter;

struct FLagCompSnapshot
{
	double TimeSeconds = 0.0;
	FTransform ActorTransform = FTransform::Identity;
};

struct FMovedTarget
{
	AShooterCharacter* Target = nullptr;
	FTransform OriginalTransform = FTransform::Identity;
};


/**
 * 服务延迟补偿：保存玩家历史 Transform，按开火时刻回滚目标后做真实 Trace。
 * 只在服务器启用 Tick，快照不会复制到客户端。
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPS_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULagCompensationComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool GetSnapshotAtTime(double TimeSeconds, FLagCompSnapshot& OutSnapshot) const;
	void ClearHistory();

	FHitResult RewindTrace(
		const FVector& ViewLocation,
		const FVector& ViewDirection,
		float MaxDistance,
		float TraceRadius,
		double RewindTime
	) const;

	UPROPERTY(EditDefaultsOnly, Category="FPS|LagCompensation")
	float MaxHistorySeconds = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category="FPS|LagCompensation")
	int32 MaxHistorySamples = 120;

	UPROPERTY(EditDefaultsOnly, Category="FPS|LagCompensation")
	bool bDrawDebug = false;

private:
	TDeque<FLagCompSnapshot> History;

	void CaptureSnapshot();
};
