// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Elims/ScoreboardWidget.h"

#include "Components/VerticalBox.h"
#include "GameFramework/GameStateBase.h"
#include "Player/ShooterPlayerController.h"
#include "Player/ShooterPlayerState.h"
#include "UI/Elims/ScoreboardRowWidget.h"

void UScoreboardWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	AShooterPlayerState* PlayerState = GetPlayerState();
	if (IsValid(PlayerState))
	{
		StartRefreshRowsTimer();
	}
	else
	{
		AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetOwningPlayer());
		if (IsValid(PlayerController))
		{
			PlayerController->OnPlayerStateReplicated.AddDynamic(this, &ThisClass::OnPlayerStateReplicated);
		}
	}
}

void UScoreboardWidget::NativeDestruct()
{
	StopRefreshRowsTimer();
	
	if (AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetOwningPlayer()))
	{
		PlayerController->OnPlayerStateReplicated.RemoveDynamic(this, &ThisClass::OnPlayerStateReplicated);
	}
	
	Super::NativeDestruct();
}

AShooterPlayerState* UScoreboardWidget::GetPlayerState() const
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (IsValid(PlayerController))
	{
		return PlayerController->GetPlayerState<AShooterPlayerState>();
	}
	
	return nullptr;
}

void UScoreboardWidget::RefreshRows()
{
	Box_PlayerInfos->ClearChildren();
	
	if (IsValid(Box_PlayerInfos))
	{
		TArray<APlayerState*> PlayerStates = GetWorld()->GetGameState()->PlayerArray;
		PlayerStates.Sort([](const APlayerState& A, const APlayerState& B)
		{
			const AShooterPlayerState* PlayerStateA = Cast<AShooterPlayerState>(&A);
			const AShooterPlayerState* PlayerStateB = Cast<AShooterPlayerState>(&B);
			
			return PlayerStateA->GetScoredElims() > PlayerStateB->GetScoredElims();
		});
		
		for (APlayerState* PlayerState : PlayerStates)
		{
			UScoreboardRowWidget* ScoreboardRowWidget = CreateWidget<UScoreboardRowWidget>(GetOwningPlayer(), ScoreboardRowClass);
			ScoreboardRowWidget->SetScoreboardRowText(PlayerState);
			
			Box_PlayerInfos->AddChildToVerticalBox(ScoreboardRowWidget);
		}
	}
}

void UScoreboardWidget::StartRefreshRowsTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RefreshRowsTimerHandle, this, &ThisClass::RefreshRows, RefreshRowsInterval, true);
	}
		
	RefreshRows();
}

void UScoreboardWidget::StopRefreshRowsTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshRowsTimerHandle);
	}
}

void UScoreboardWidget::OnPlayerStateReplicated()
{
	if (!IsValid(GetPlayerState())) { return; }
	
	StartRefreshRowsTimer();
	
	AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetOwningPlayer());
	if (IsValid(PlayerController))
	{
		PlayerController->OnPlayerStateReplicated.RemoveDynamic(this, &ThisClass::OnPlayerStateReplicated);
	}
}
