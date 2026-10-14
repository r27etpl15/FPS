// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ShooterGameStateBase.h"

#include "Player/ShooterPlayerState.h"

AShooterGameStateBase::AShooterGameStateBase()
{
	
}

void AShooterGameStateBase::UpdateLeader()
{
	TArray<APlayerState*> LocalSortedPlayerStates = PlayerArray;
	LocalSortedPlayerStates.Sort([](const APlayerState& A, const APlayerState& B)
	{
		const AShooterPlayerState* PlayerStateA = Cast<AShooterPlayerState>(&A);
		const AShooterPlayerState* PlayerStateB = Cast<AShooterPlayerState>(&B);
		
		return PlayerStateA->GetScoredElims() > PlayerStateB->GetScoredElims();
	});
	
	Leaders.Empty();
	
	if (LocalSortedPlayerStates.Num() > 0)
	{
		int32 HighestScore = 0;
		for (APlayerState* PlayerState : LocalSortedPlayerStates)
		{	
			AShooterPlayerState* ShooterPlayerState = Cast<AShooterPlayerState>(PlayerState);
			if (IsValid(ShooterPlayerState))
			{
				int32 PlayerScore = ShooterPlayerState->GetScoredElims();
			
				if (Leaders.Num() == 0)
				{
					HighestScore = PlayerScore;
					Leaders.Add(ShooterPlayerState);
				}
				else if (HighestScore == PlayerScore)	// 分数相同也加入领先玩家数组
				{
					Leaders.Add(ShooterPlayerState);
				}
				else
				{
					break;
				}	
			}
		}
	}
	
	bHasFirstBloodBeenHad = true;
}

AShooterPlayerState* AShooterGameStateBase::GetSoleLeader() const
{
	if (Leaders.Num() == 1)
	{
		return Leaders[0];
	}
	
	return nullptr;
}

bool AShooterGameStateBase::IsTiedForTheLead(AShooterPlayerState* PlayerState)
{
	return Leaders.Contains(PlayerState) && Leaders.Num() > 1;
}
