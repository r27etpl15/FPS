// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ShooterPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

AShooterPlayerController::AShooterPlayerController()
{
	bReplicates = true;
}

void AShooterPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	OnPlayerStateReplicated.Broadcast();
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (IsValid(Subsystem))
	{
		Subsystem->AddMappingContext(ShooterIMC, 0);
	}
}

void AShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	UEnhancedInputComponent* ShooterInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	ShooterInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::InputMove);
	ShooterInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::InputLook);
	ShooterInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::InputJump);
	ShooterInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ThisClass::InputCrouch);
	ShooterInputComponent->BindAction(ScoreboardAction, ETriggerEvent::Started, this, &ThisClass::InputScoreboardStarted);
	ShooterInputComponent->BindAction(ScoreboardAction, ETriggerEvent::Completed, this, &ThisClass::InputScoreboardCompleted);
}

void AShooterPlayerController::InputCrouch()
{
	if (!IsValid(GetCharacter())) return;
	if (!bPawnAlive) { return; }
	
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacter()->GetCharacterMovement();
	if (IsValid(CharacterMovementComponent))
	{
		CharacterMovementComponent->bWantsToCrouch = !CharacterMovementComponent->bWantsToCrouch;
	}
}

void AShooterPlayerController::InputJump()
{
	if (!IsValid(GetCharacter())) return;
	if (!bPawnAlive) { return; }
	
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacter()->GetCharacterMovement();
	if (IsValid(CharacterMovementComponent))
	{
		if (CharacterMovementComponent->bWantsToCrouch)
		{
			CharacterMovementComponent->bWantsToCrouch = false;
		}
		else
		{
			GetCharacter()->Jump();
		}
	}
}

void AShooterPlayerController::InputMove(const FInputActionValue& InputActionValue)
{
	if (!bPawnAlive) { return; }
	
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation = FRotator(0.0f, Rotation.Yaw, 0.0f);
	
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AShooterPlayerController::InputLook(const FInputActionValue& InputActionValue)
{
	if (!bPawnAlive) { return; }
	
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	
	AddYawInput(InputAxisVector.X);
	AddPitchInput(InputAxisVector.Y);
}

void AShooterPlayerController::InputScoreboardStarted()
{
	OnScoreboardDisplayChanged.Broadcast(true);
}

void AShooterPlayerController::InputScoreboardCompleted()
{
	OnScoreboardDisplayChanged.Broadcast(false);
}
