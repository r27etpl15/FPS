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

TArray<ESpecialElimType> AShooterPlayerState::DecodeSpecialElimBitmask(ESpecialElimType SpecialElimTypeBitmask)
{
	TArray<ESpecialElimType> ValidElims;
	
	uint16 BitmaskValue = static_cast<uint16>(SpecialElimTypeBitmask);
	
	for (uint16 i = 0; i < 16; i++)
	{
		if (BitmaskValue & (1 << i))
		{
			ValidElims.Add(static_cast<ESpecialElimType>(1 << i));
		}
	}
	
	return ValidElims;
}

void AShooterPlayerState::Client_ScoredElim_Implementation(int32 ElimScore)
{
	OnScoreChanged.Broadcast(ElimScore);
}

void AShooterPlayerState::Client_SpecialElim_Implementation(const ESpecialElimType& SpecialElim,
	int32 SequentialElimCount, int32 StreakCount, int32 ElimScore)
{
	ensure(IsValid(SpecialElimData));
	
	OnScoreChanged.Broadcast(ElimScore);
	
	TArray<ESpecialElimType> SpecialElimTypes = DecodeSpecialElimBitmask(SpecialElim);
	for (ESpecialElimType ElimType : SpecialElimTypes)
	{
		FSpecialElimInfo& SpecialElimInfo = SpecialElimData->SpecialElimInfo.FindChecked(ElimType);
		if (ElimType == ESpecialElimType::Sequential)
		{
			SpecialElimInfo.SequentialElimCount = SequentialElimCount;
		}
		if (ElimType == ESpecialElimType::Streak)
		{
			SpecialElimInfo.StreakCount = StreakCount;
		}
		SpecialElimInfo.ElimType = ElimType;
		
		SpecialElimQueue.Enqueue(SpecialElimInfo);
	}
	
	if (!bIsProcessingQueue)
	{
		ProcessNextSpecialElim();
	}
}

void AShooterPlayerState::ProcessNextSpecialElim()
{
	FSpecialElimInfo SpecialElimInfo;
	if (SpecialElimQueue.Dequeue(SpecialElimInfo))
	{
		bIsProcessingQueue = true;
		ShowSpecialElim(SpecialElimInfo);
		
		GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(TimerHandle, this, &ThisClass::ProcessNextSpecialElim, ElimDisplayTime);
		});
	}
	else
	{
		bIsProcessingQueue = false;
	}
}

void AShooterPlayerState::ShowSpecialElim(const FSpecialElimInfo& SpecialElimInfo)
{
	FString ElimMessageString = SpecialElimInfo.ElimMessage;
	if (SpecialElimInfo.ElimType == ESpecialElimType::Sequential)
	{
		if (SpecialElimInfo.SequentialElimCount == 2)
		{
			ElimMessageString = FString(TEXT("双杀！"));
		}
		else if (SpecialElimInfo.SequentialElimCount == 3)
		{
			ElimMessageString = FString(TEXT("三杀!"));
		}
		else if (SpecialElimInfo.SequentialElimCount == 4)
		{
			ElimMessageString = FString(TEXT("四杀!"));
		}
		else if (SpecialElimInfo.SequentialElimCount > 4)
		{
			ElimMessageString = FString::Printf(TEXT("连杀 x %d！"), SpecialElimInfo.SequentialElimCount);
		}
	}
	if (SpecialElimInfo.ElimType == ESpecialElimType::Streak)
	{
		ElimMessageString = FString::Printf(TEXT("天神下凡 x %d！"), SpecialElimInfo.StreakCount);
	}
	
	if (IsValid(SpecialElimWidgetClass))
	{
		USpecialElimWidget* SpecialElimWidget = CreateWidget<USpecialElimWidget>(GetPlayerController(), SpecialElimWidgetClass);
		if (IsValid(SpecialElimWidget))
		{
			SpecialElimWidget->InitializeWidget(ElimMessageString, SpecialElimInfo.ElimIcon);
			SpecialElimWidget->AddToViewport();
		}
	}
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
