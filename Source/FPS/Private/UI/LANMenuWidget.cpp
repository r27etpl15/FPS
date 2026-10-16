// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LANMenuWidget.h"

#include "ShooterGameInstance.h"
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
	
	UShooterGameInstance* GameInstance = Cast<UShooterGameInstance>(GetGameInstance());
	if (IsValid(GameInstance))
	{
		GameInstance->SetPlayerName(TextBox_PlayerName->GetText().ToString().TrimStartAndEnd());
	}
	
	const FString Name = IsValid(GameInstance) ? GameInstance->GetPlayerName() : FString();
	const FString NameOption = Name.IsEmpty() ? FString() : FString::Printf(TEXT("?Name=%s"), *Name);
	
	FString Options = TEXT("Listen");
	if (!NameOption.IsEmpty())
	{
		Options += NameOption;
	}
	
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, HostingLevel, true, Options);	
}

void ULANMenuWidget::JoinButtonClicked()
{
	FInputModeGameOnly InputModeGameOnly;
	GetOwningPlayer()->SetInputMode(InputModeGameOnly);
	GetOwningPlayer()->SetShowMouseCursor(false);
	
	UShooterGameInstance* GameInstance = Cast<UShooterGameInstance>(GetGameInstance());
	if (IsValid(GameInstance))
	{
		GameInstance->SetPlayerName(TextBox_PlayerName->GetText().ToString().TrimStartAndEnd());
	}
	
	const FString Name = IsValid(GameInstance) ? GameInstance->GetPlayerName() : FString();
	const FString NameOption = Name.IsEmpty() ? FString() : FString::Printf(TEXT("Name=%s"), *Name);
	
	const FString Address = TextBox_IpAddress->GetText().ToString().TrimStartAndEnd();
	UGameplayStatics::OpenLevel(this, *Address, false, NameOption);
}

void ULANMenuWidget::QuitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
