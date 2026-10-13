// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EliminationComponent.generated.h"


class AShooterGameStateBase;
enum class ESpecialElimType : uint16;
class AShooterPlayerState;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPS_API UEliminationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEliminationComponent();
	
	// 单次射击时调用
	UFUNCTION()
	void OnRoundReported(AActor* Attacker, AActor* Victim, bool bHit, bool bHeadShot, bool bLethal);
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Elimination")
	float SequentialElimInterval = 5.0f;
	
protected:
	virtual void BeginPlay() override;
	
private:
	AShooterPlayerState* GetShooterPlayerStateFromActor(AActor* Actor);
	bool HasSpecialElimTypes(const ESpecialElimType& SpecialElimType);
	
	void ProcessHitOrMiss(bool bHit, AShooterPlayerState* AttackerPlayerState);
	void ProcessElimination(bool bHeadShot, AShooterPlayerState* AttackerPlayerState, AShooterPlayerState* VictimPlayerState);
	void ProcessHeadShot(bool bHeadShot, ESpecialElimType& OutSpecialElimType, AShooterPlayerState* AttackerPlayerState);
	void ProcessSequentialEliminations(ESpecialElimType& OutSpecialElimType, AShooterPlayerState* AttackerPlayerState);
	void ProcessStreaks(ESpecialElimType& OutSpecialElimType, AShooterPlayerState* AttackerPlayerState, AShooterPlayerState* VictimPlayerState);
	void HandleFirstBlood(AShooterGameStateBase* GameState, ESpecialElimType& OutSpecialElimType, AShooterPlayerState* AttackerPlayerState);
	void UpdateLeaderStatus(AShooterGameStateBase* GameState, ESpecialElimType& OutSpecialElimType, AShooterPlayerState* AttackerPlayerState, AShooterPlayerState* VictimPlayerState);
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Elimination", meta=(AllowPrivateAccess = "true"))
	int32 ElimsNeededForStreak = 2;
	
	float LastElimTime = 0.0f;
	int32 SequentialElims = 0;
	int32 Streak = 0;
};
