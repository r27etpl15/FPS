// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/**
 * 
 */
UCLASS()
class FPS_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AShooterPlayerController();
	
	bool IsPawnAlive() const { return bPawnAlive; }
	void SetPawnAlive(bool bInPawnAlive) { bPawnAlive = bInPawnAlive;}
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
private:
	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputMappingContext> ShooterIMC;
	
	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> CrouchAction;
	
	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> JumpAction;
	
	bool bPawnAlive = true;
	
	void InputCrouch();
	void InputJump();
	void InputMove(const FInputActionValue& InputActionValue);
	void InputLook(const FInputActionValue& InputActionValue);
};
