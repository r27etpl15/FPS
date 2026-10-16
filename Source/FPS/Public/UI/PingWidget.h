// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PingWidget.generated.h"

class UTextBlock;
class AShooterPlayerState;
/**
 * 
 */
UCLASS()
class FPS_API UPingWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Ping;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Ping")
	float PingUpdateInterval = 0.5f;
	
private:
	AShooterPlayerState* GetPlayerState() const;
	void StartPingTimer();
	void StopPingTimer();
	
	FTimerHandle PingTimerHandle;
	
	UFUNCTION()
	void UpdateTextPing();
	
	UFUNCTION()
	void OnPlayerStateReplicated();
};
