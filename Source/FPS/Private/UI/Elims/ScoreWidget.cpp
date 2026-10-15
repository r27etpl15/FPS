// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Elims/ScoreWidget.h"

#include "Components/TextBlock.h"
#include "Player/ShooterPlayerController.h"
#include "Player/ShooterPlayerState.h"

void UScoreWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	AShooterPlayerState* PlayerState = GetPlayerState();
	if (IsValid(PlayerState))
	{
		PlayerState->OnScoreChanged.AddDynamic(this, &ThisClass::OnScoreChanged);
	}
	else
	{
		AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetOwningPlayer());
		if (IsValid(PlayerController))
		{
			PlayerController->OnPlayerStateReplicated.AddUniqueDynamic(this, &ThisClass::OnPlayerStateReplicated);
		}
	}
}

AShooterPlayerState* UScoreWidget::GetPlayerState() const
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (IsValid(PlayerController))
	{
		return PlayerController->GetPlayerState<AShooterPlayerState>();
	}
	
	return nullptr;
}

void UScoreWidget::OnScoreChanged(int32 Score)
{
	if (IsValid(Text_Score))
	{
		Text_Score->SetText(FText::AsNumber(Score));
	}
}

void UScoreWidget::OnPlayerStateReplicated()
{
	AShooterPlayerState* PlayerState = GetPlayerState();
	if (IsValid(PlayerState))
	{
		PlayerState->OnScoreChanged.AddDynamic(this, &ThisClass::OnScoreChanged);
		OnScoreChanged(PlayerState->GetScoredElims());
	}
	
	AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetOwningPlayer());
	if (IsValid(PlayerController))
	{
		PlayerController->OnPlayerStateReplicated.RemoveDynamic(this, &ThisClass::OnPlayerStateReplicated);
	}
}