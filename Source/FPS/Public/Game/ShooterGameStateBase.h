// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ShooterGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class FPS_API AShooterGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AShooterGameStateBase();
	
	bool HasFirstBloodBeenHad() const { return bHasFirstBloodBeenHad; }
	void UpdateLeader();
	
private:
	bool bHasFirstBloodBeenHad = false;
};
