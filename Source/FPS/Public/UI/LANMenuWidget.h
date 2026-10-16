// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LANMenuWidget.generated.h"

class UEditableTextBox;
class UButton;
/**
 * 
 */
UCLASS()
class FPS_API ULANMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnInitialized() override;
	
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> TextBox_IpAddress;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> TextBox_PlayerName;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Host;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Join;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Quit;
	
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> HostingLevel;
	
	UFUNCTION()
	void HostButtonClicked();
	
	UFUNCTION()
	void JoinButtonClicked();
	
	UFUNCTION()
	void QuitButtonClicked();
};
