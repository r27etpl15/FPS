// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ShooterGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class FPS_API UShooterGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	void SetPlayerName(const FString& InPlayerName) { PlayerName = InPlayerName; }
	FString GetPlayerName() const { return PlayerName; }
	
private:
	UPROPERTY()
	FString PlayerName;
};
