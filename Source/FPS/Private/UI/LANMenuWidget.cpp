// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LANMenuWidget.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Kismet/GameplayStatics.h"

void ULANMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	FInputModeUIOnly InputModeUIOnly;
	GetOwningPlayer()->SetInputMode(InputModeUIOnly);
	GetOwningPlayer()->SetShowMouseCursor(true);
	
	Button_Host->OnClicked.AddUniqueDynamic(this, &ThisClass::HostButtonClicked);
	Button_Join->OnClicked.AddUniqueDynamic(this, &ThisClass::JoinButtonClicked);
	Button_Quit->OnClicked.AddUniqueDynamic(this, &ThisClass::QuitButtonClicked);
}

void ULANMenuWidget::HostButtonClicked()
{
	FInputModeGameOnly InputModeGameOnly;
	GetOwningPlayer()->SetInputMode(InputModeGameOnly);
	GetOwningPlayer()->SetShowMouseCursor(false);
	
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, HostingLevel, true, TEXT("Listen"));	
}

void ULANMenuWidget::JoinButtonClicked()
{
	FInputModeGameOnly InputModeGameOnly;
	GetOwningPlayer()->SetInputMode(InputModeGameOnly);
	GetOwningPlayer()->SetShowMouseCursor(false);
	
	const FString Address = TextBox_IpAddress->GetText().ToString().TrimStartAndEnd();
	UGameplayStatics::OpenLevel(this, *Address);
}

void ULANMenuWidget::QuitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
