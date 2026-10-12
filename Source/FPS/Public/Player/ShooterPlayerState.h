// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ShooterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class FPS_API AShooterPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AShooterPlayerState();
	
	void GotFirstBlood();
	void IsNowWinner();
	
	void AddScoredElims();
	void AddHeadShotElims();
	void AddDefeats();
	void AddHits();
	void AddMisses();
	void UpdateHighestStreak(int32 StreakCount);
	void AddRevengeElims();
	void AddDethroneElims();
	void AddShowStopperElims();
	void AddSequentialElims(int32 SequenceCount);
	
private:
	bool bWinner = false;
	bool bOnStreak = false;		
	bool bFirstBlood = false;				// 是否在连杀中
	
	int32 ScoredElims = 0;
	int32 HeadShotElims = 0;
	int32 Defeats = 0;
	int32 Hits = 0;
	int32 Misses = 0;
	int32 HighestStreak = 0;
	int32 RevengeElims = 0;
	int32 DethroneElims = 0;				// 击杀排名第一的玩家
	int32 ShowStopperElims = 0;				// 终结正在连杀的玩家
	
	TMap<int32, int32> SequentialElims;		// 连杀数 / 次数， 例如 2 / 4，一局中两连杀有过 4 次
};
