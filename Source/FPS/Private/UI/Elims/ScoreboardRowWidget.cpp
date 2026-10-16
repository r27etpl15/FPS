// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Elims/ScoreboardRowWidget.h"

#include "Components/TextBlock.h"
#include "Player/ShooterPlayerState.h"

void UScoreboardRowWidget::SetScoreboardRowText(APlayerState* PlayerState)
{
	AShooterPlayerState* ShooterPlayerState = Cast<AShooterPlayerState>(PlayerState);
	if (!IsValid(ShooterPlayerState)) { return; }
	
	Text_Name->SetText(FText::FromString(ShooterPlayerState->GetPlayerName()));
	Text_Kills->SetText(FText::AsNumber(ShooterPlayerState->GetScoredElims()));
	Text_Deaths->SetText(FText::AsNumber(ShooterPlayerState->GetDefeats()));
	Text_HeadShots->SetText(FText::AsNumber(ShooterPlayerState->GetHeadShotElims()));
	Text_HighestStreak->SetText(FText::AsNumber(ShooterPlayerState->GetHighestStreak()));
	Text_Accuracy->SetText(FText::AsNumber(ShooterPlayerState->GetAccuracy()));

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
	
	if (!IsValid(GetOwningPlayer())) { return; }
	
	if (GetOwningPlayer()->PlayerState == PlayerState)
	{
		Text_Name->SetColorAndOpacity(LocalPlayerTextColor);
		Text_Kills->SetColorAndOpacity(LocalPlayerTextColor);
		Text_Deaths->SetColorAndOpacity(LocalPlayerTextColor);
		Text_HeadShots->SetColorAndOpacity(LocalPlayerTextColor);
		Text_HighestStreak->SetColorAndOpacity(LocalPlayerTextColor);
		Text_Accuracy->SetColorAndOpacity(LocalPlayerTextColor);
		Text_Ping->SetColorAndOpacity(LocalPlayerTextColor);
	}
}
