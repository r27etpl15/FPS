// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Elims/ScoreboardRowWidget.h"

#include "Components/TextBlock.h"
#include "Player/ShooterPlayerState.h"

void UScoreboardRowWidget::SetScoreboardRowText(APlayerState* PlayerState)
{
	AShooterPlayerState* ShooterPlayerState = Cast<AShooterPlayerState>(PlayerState);
	if (!IsValid(ShooterPlayerState)) { return; }
	
	if (IsValid(Text_Name))
	{
		Text_Name->SetText(FText::FromString(ShooterPlayerState->GetPlayerName()));
	}
	if (IsValid(Text_Kills))
	{
		Text_Kills->SetText(FText::AsNumber(ShooterPlayerState->GetScoredElims()));
	}
	if (IsValid(Text_Deaths))
	{
		Text_Deaths->SetText(FText::AsNumber(ShooterPlayerState->GetDefeats()));
	}
	if (IsValid(Text_HeadShots))
	{
		Text_HeadShots->SetText(FText::AsNumber(ShooterPlayerState->GetHeadShotElims()));
	}
	if (IsValid(Text_HighestStreak))
	{
		Text_HighestStreak->SetText(FText::AsNumber(ShooterPlayerState->GetHighestStreak()));
	}
	if (IsValid(Text_Accuracy))
	{
		Text_Accuracy->SetText(FText::AsNumber(ShooterPlayerState->GetAccuracy()));
	}
	if (IsValid(Text_Ping))
	{
		const float PingMilliseconds = ShooterPlayerState->GetPingInMilliseconds();
		if (PingMilliseconds <= 0.0f)
		{
			// 独立/监听主机自身，或尚未测得
			Text_Ping->SetText(FText::FromString(TEXT("--")));
		}
		else
		{
			Text_Ping->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::RoundToInt(PingMilliseconds))));	
		}
	}
}
