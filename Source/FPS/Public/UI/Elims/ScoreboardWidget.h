// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreboardWidget.generated.h"

class AShooterPlayerState;
class UScoreboardRowWidget;
class UVerticalBox;
/**
 * 
 */
UCLASS()
class FPS_API UScoreboardWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	
	void StartRefreshRowsTimer();
	void StopRefreshRowsTimer();
	
private:
	void RefreshRows();
	
	UFUNCTION()
	void OnOnScoreboardDisplayChanged(bool bDisplayScoreboard);
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> Box_PlayerInfos;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UScoreboardRowWidget> ScoreboardRowClass;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Info")
	float RefreshRowsInterval = 0.5f;
	
	FTimerHandle RefreshRowsTimerHandle;
};
