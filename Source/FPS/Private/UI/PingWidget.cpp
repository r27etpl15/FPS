// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PingWidget.h"

#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Player/ShooterPlayerController.h"
#include "Player/ShooterPlayerState.h"
#include "TimerManager.h"

void UPingWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (IsValid(GetPlayerState()))
	{
		StartPingTimer();
	}
	else if (AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetOwningPlayer()))
	{
		PlayerController->OnPlayerStateReplicated.AddUniqueDynamic(this, &ThisClass::OnPlayerStateReplicated);
	}
}

void UPingWidget::NativeDestruct()
{
	StopPingTimer();
	
	if (AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetOwningPlayer()))
	{
		PlayerController->OnPlayerStateReplicated.RemoveDynamic(this, &ThisClass::OnPlayerStateReplicated);
	}
	
	Super::NativeDestruct();
}

AShooterPlayerState* UPingWidget::GetPlayerState() const
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		return PlayerController->GetPlayerState<AShooterPlayerState>();
	}
	
	return nullptr;
}

void UPingWidget::StartPingTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(PingTimerHandle, this, &ThisClass::UpdateTextPing, PingUpdateInterval, true);
	}
	
	// 立即刷新一次，避免等到首个周期
	UpdateTextPing();
}

void UPingWidget::StopPingTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PingTimerHandle);
	}
}

void UPingWidget::UpdateTextPing()
{
	if (!IsValid(Text_Ping)) { return; }
	
	AShooterPlayerState* PlayerState = GetPlayerState();
	if (!IsValid(PlayerState)) { return; }
	
	const float PingMilliseconds = PlayerState->GetPingInMilliseconds();
	if (PingMilliseconds <= 0.0f)
	{
		// 独立/监听主机自身，或尚未测得
		Text_Ping->SetText(FText::FromString(TEXT("-- ms")));
		return;
	}
	
	Text_Ping->SetText(FText::FromString(FString::Printf(TEXT("%d ms"), FMath::RoundToInt(PingMilliseconds))));
}

void UPingWidget::OnPlayerStateReplicated()
{
	if (!IsValid(GetPlayerState())) { return; }
	
	StartPingTimer();
	
	if (AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetOwningPlayer()))
	{
		PlayerController->OnPlayerStateReplicated.RemoveDynamic(this, &ThisClass::OnPlayerStateReplicated);
	}
}
