// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ShooterGameModeBase.h"

#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShooterPlayerController.h"

FString AShooterGameModeBase::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
	const FString& Options, const FString& Portal)
{
	// 先调用 Super 不然会把设置的名称给覆盖了
	const FString ErrorMessage = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
	
	// 用自定义 key 取名字：引擎会用本地平台昵称覆盖 URL 里的 Name，所以不能用 Name
	FString InName = UGameplayStatics::ParseOption(Options, TEXT("PlayerAlias"));
	if (InName.IsEmpty())
	{
		InName = FString::Printf(TEXT("Player%d"), ++PlayerNameCount);
	}
	ChangeName(NewPlayerController, InName, false);
	
	return ErrorMessage;
}

void AShooterGameModeBase::RequestRespawn(ACharacter* Character, AController* Controller)
{
	if (!IsValid(Character) || !IsValid(Controller)) { return; }
	
	Character->Reset();
	Character->Destroy();
	
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
	ensure(PlayerStarts.Num() > 0);
	int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
	
	RestartPlayerAtPlayerStart(Controller, PlayerStarts[Selection]);
	
	// 重生后恢复输入
	if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(Controller))
	{
		PC->SetPawnAlive(true);
	}
}
