// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ShooterPlayerState.h"

#include "Data/SpecialElimData.h"
#include "UI/Elims/SpecialElimWidget.h"

AShooterPlayerState::AShooterPlayerState()
{
	SetNetUpdateFrequency(100.0f);
}

void AShooterPlayerState::GotFirstBlood()
{
	bFirstBlood = true;
}

void AShooterPlayerState::IsNowWinner()
{
	bWinner = true;
}

void AShooterPlayerState::SetOnStreak(bool bIsOnStreak)
{
	bOnStreak = bIsOnStreak;
}

void AShooterPlayerState::AddScoredElim()
{
	ScoredElims++;
}

void AShooterPlayerState::AddHeadShotElim()
{
	HeadShotElims++;
}

void AShooterPlayerState::AddDefeat()
{
	Defeats++;
}

void AShooterPlayerState::AddHit()
{
	Hits++;
}

void AShooterPlayerState::AddMiss()
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

void AShooterPlayerState::AddRevengeElim()
{
	RevengeElims++;
}

void AShooterPlayerState::AddDethroneElim()
{
	DethroneElims++;
}

void AShooterPlayerState::AddShowStopperElim()
{
	ShowStopperElims++;
}

void AShooterPlayerState::AddSequentialElim(int32 SequenceCount)
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

void AShooterPlayerState::SetLastAttacker(APlayerState* Attacker)
{
	LastAttacker = Attacker;
}

void AShooterPlayerState::Client_ScoredElim_Implementation(int32 ElimScore)
{
}

void AShooterPlayerState::Client_SpecialElim_Implementation(const ESpecialElimType& SpecialElim,
	int32 SequentialElimCount, int32 StreakCount, int32 ElimScore)
{
}

void AShooterPlayerState::Client_LostTheLead_Implementation()
{
	ensure(IsValid(SpecialElimData));
	
	FSpecialElimInfo& SpecialElimInfo = SpecialElimData->SpecialElimInfo.FindChecked(ESpecialElimType::LostTheLead);
	if (IsValid(SpecialElimWidgetClass))
	{
		USpecialElimWidget* SpecialElimWidget = CreateWidget<USpecialElimWidget>(GetPlayerController(), SpecialElimWidgetClass);
		if (IsValid(SpecialElimWidget))
		{
			SpecialElimWidget->InitializeWidget(SpecialElimInfo.ElimMessage, SpecialElimInfo.ElimIcon);
			SpecialElimWidget->AddToViewport();
		}
	}
}
