// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ShooterPlayerState.generated.h"

class USpecialElimWidget;
class USpecialElimData;
enum class ESpecialElimType : uint16;
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
	void SetOnStreak(bool bIsOnStreak);
	bool IsOnStreak() const { return bOnStreak; }
	
	void AddScoredElim();
	void AddHeadShotElim();
	void AddDefeat();
	void AddHit();
	void AddMiss();
	void UpdateHighestStreak(int32 StreakCount);
	void AddRevengeElim();
	void AddDethroneElim();
	void AddShowStopperElim();
	void AddSequentialElim(int32 SequenceCount);
	void SetLastAttacker(APlayerState* Attacker);
	APlayerState* GetLastAttacker() const { return LastAttacker.IsValid() ? LastAttacker.Get() : nullptr; }
	int32 GetScoredElims() const { return ScoredElims; }
	
	UFUNCTION(Client, Reliable)
	void Client_LostTheLead();
	
	UFUNCTION(Client, Reliable)
	void Client_ScoredElim(int32 ElimScore);
	
	UFUNCTION(Client, Reliable)
	void Client_SpecialElim(const ESpecialElimType& SpecialElim, int32 SequentialElimCount, int32 StreakCount, int32 ElimScore);
	
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
	
	TWeakObjectPtr<APlayerState> LastAttacker;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|SpecialElims", meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USpecialElimData> SpecialElimData;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|SpecialElims", meta=(AllowPrivateAccess = "true"))
	TSubclassOf<USpecialElimWidget> SpecialElimWidgetClass;
	
	// 解码特殊淘汰位掩码
	TArray<ESpecialElimType> DecodeSpecialElimBitmask(ESpecialElimType SpecialElimTypeBitmask);
};
