// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ShooterPlayerState.h"

AShooterPlayerState::AShooterPlayerState()
{
	NetUpdateFrequency = 100.0f;
}

void AShooterPlayerState::GotFirstBlood()
{
	bFirstBlood = true;
}

void AShooterPlayerState::IsNowWinner()
{
	bWinner = true;
}

void AShooterPlayerState::AddScoredElims()
{
	ScoredElims++;
}

void AShooterPlayerState::AddHeadShotElims()
{
	HeadShotElims++;
}

void AShooterPlayerState::AddDefeats()
{
	Defeats++;
}

void AShooterPlayerState::AddHits()
{
	Hits++;
}

void AShooterPlayerState::AddMisses()
{
	Misses++;
}

void AShooterPlayerState::UpdateHighestStreak(int32 StreakCount)
{
	if (StreakCount > HighestStreak)
	{
		HighestStreak = StreakCount;
	}
}

void AShooterPlayerState::AddRevengeElims()
{
	RevengeElims++;
}

void AShooterPlayerState::AddDethroneElims()
{
	DethroneElims++;
}

void AShooterPlayerState::AddShowStopperElims()
{
	ShowStopperElims++;
}

void AShooterPlayerState::AddSequentialElims(int32 SequenceCount)
{
	if (SequentialElims.Contains(SequenceCount))
	{
		SequentialElims[SequenceCount]++;
	}
	else
	{
		SequentialElims.Add(SequenceCount, 1);
	}
	
	// 一条命双杀之后再三杀需要让双杀数量减一
	for (auto& Elem : SequentialElims)
	{
		if (Elem.Key < SequenceCount && Elem.Value > 0)
		{
			Elem.Value--;
		}
	}
}
