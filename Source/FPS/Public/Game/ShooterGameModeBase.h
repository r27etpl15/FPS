// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class FPS_API AShooterGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = L"") override;
	
	void RequestRespawn(ACharacter* Character, AController* Controller);
	
private:
	int32 PlayerNameCount = 0;
};
