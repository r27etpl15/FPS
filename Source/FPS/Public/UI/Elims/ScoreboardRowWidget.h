// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreboardRowWidget.generated.h"

class AShooterPlayerState;
class UTextBlock;
/**
 * 
 */
UCLASS()
class FPS_API UScoreboardRowWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetScoreboardRowText(APlayerState* PlayerState);
	
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Name;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Kills;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Deaths;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_HeadShots;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_HighestStreak;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Accuracy;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Ping;
};
