// Fill out your copyright notice in the Description page of Project Settings.


#include "Elimination/EliminationComponent.h"

#include "Game/ShooterGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShooterPlayerState.h"
#include "ShooterTypes/ShooterTypes.h"

UEliminationComponent::UEliminationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
}

void UEliminationComponent::OnRoundReported(AActor* Attacker, AActor* Victim, bool bHit, bool bHeadShot, bool bLethal)
{
	AShooterPlayerState* AttackerPlayerState = GetShooterPlayerStateFromActor(Attacker);
	if (!IsValid(AttackerPlayerState)) { return; }
	
	ProcessHitOrMiss(bHit, AttackerPlayerState);
	
	// 未命中就提前返回
	if (!bHit) { return; }
	
	AShooterPlayerState* VictimPlayerState = GetShooterPlayerStateFromActor(Victim);
	if (!IsValid(VictimPlayerState)) { return; }
	
	if (bLethal)
	{
		ProcessElimination(bHeadShot, AttackerPlayerState, VictimPlayerState);
	}
}


void UEliminationComponent::BeginPlay()
{
	Super::BeginPlay();
	
	
}

AShooterPlayerState* UEliminationComponent::GetShooterPlayerStateFromActor(AActor* Actor)
{
	APawn* Pawn = Cast<APawn>(Actor);
	if (IsValid(Pawn))
	{
		return Pawn->GetPlayerState<AShooterPlayerState>();
	}
	
	return nullptr;
}

void UEliminationComponent::ProcessHitOrMiss(bool bHit, AShooterPlayerState* AttackerPlayerState)
{
	if (bHit)
	{
		AttackerPlayerState->AddHit();
	}
	else
	{
		AttackerPlayerState->AddMiss();
	}
}

void UEliminationComponent::ProcessElimination(bool bHeadShot, AShooterPlayerState* AttackerPlayerState,
	AShooterPlayerState* VictimPlayerState)
{
	AttackerPlayerState->AddScoredElim();
	VictimPlayerState->AddDefeat();
	
	ESpecialElimType SpecialElimType {};
	
	ProcessHeadShot(bHeadShot, SpecialElimType, AttackerPlayerState);
	ProcessSequentialEliminations(SpecialElimType, AttackerPlayerState);
	ProcessStreaks(SpecialElimType, AttackerPlayerState, VictimPlayerState);
	
	AShooterGameStateBase* GameState = Cast<AShooterGameStateBase>(UGameplayStatics::GetGameState(AttackerPlayerState));
	if (IsValid(GameState))
	{
		HandleFirstBlood(GameState, SpecialElimType, AttackerPlayerState);
		UpdateLeaderStatus(GameState, SpecialElimType, AttackerPlayerState, VictimPlayerState);
	}
	
}

void UEliminationComponent::ProcessHeadShot(bool bHeadShot, ESpecialElimType& OutSpecialElimType,
	AShooterPlayerState* AttackerPlayerState)
{
	if (bHeadShot)
	{
		OutSpecialElimType |= ESpecialElimType::HeadShot;
		AttackerPlayerState->AddHeadShotElim();
	}
}

void UEliminationComponent::ProcessSequentialEliminations(ESpecialElimType& OutSpecialElimType,
	AShooterPlayerState* AttackerPlayerState)
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	
	if (CurrentTime - LastElimTime <= SequentialElimInterval)
	{
		SequentialElims++;
	}
	else
	{
		SequentialElims = 1;
	}
	
	LastElimTime = CurrentTime;
	
	if (SequentialElims > 1)
	{
		OutSpecialElimType |= ESpecialElimType::Sequential;
		AttackerPlayerState->AddSequentialElim(SequentialElims);
	}
}

void UEliminationComponent::ProcessStreaks(ESpecialElimType& OutSpecialElimType,
	AShooterPlayerState* AttackerPlayerState, AShooterPlayerState* VictimPlayerState)
{
	Streak++;
	if (Streak >= ElimsNeededForStreak)
	{
		OutSpecialElimType |= ESpecialElimType::Streak;
		AttackerPlayerState->SetOnStreak(true);
		AttackerPlayerState->UpdateHighestStreak(Streak);
	}
	
	if (VictimPlayerState->IsOnStreak())
	{
		OutSpecialElimType |= ESpecialElimType::ShowStopper;
		AttackerPlayerState->AddShowStopperElim();
		VictimPlayerState->SetOnStreak(false);
	}
	
	if (AttackerPlayerState->GetLastAttacker() == VictimPlayerState)
	{
		OutSpecialElimType |= ESpecialElimType::Revenge;
		AttackerPlayerState->AddRevengeElim();
		AttackerPlayerState->SetLastAttacker(nullptr);
	}
	VictimPlayerState->SetLastAttacker(AttackerPlayerState);
}

void UEliminationComponent::HandleFirstBlood(AShooterGameStateBase* GameState, ESpecialElimType& OutSpecialElimType,
	AShooterPlayerState* AttackerPlayerState)
{
	if (!GameState->HasFirstBloodBeenHad())
	{
		OutSpecialElimType |= ESpecialElimType::FirstBlood;
		AttackerPlayerState->GotFirstBlood();
	}
}

void UEliminationComponent::UpdateLeaderStatus(AShooterGameStateBase* GameState, ESpecialElimType& OutSpecialElimType,
	AShooterPlayerState* AttackerPlayerState, AShooterPlayerState* VictimPlayerState)
{
	AShooterPlayerState* LastSoleLeader = GameState->GetSoleLeader();
	const bool bAttackerWasTiedForTheLead = GameState->IsTiedForTheLead(AttackerPlayerState);
	
	GameState->UpdateLeader();
	if (!bAttackerWasTiedForTheLead && GameState->IsTiedForTheLead(AttackerPlayerState))
	{
		// 之前没有领先，现在领先了
		OutSpecialElimType |= ESpecialElimType::TiedTheLeader;
	}
	
	if (IsValid(LastSoleLeader) && LastSoleLeader != GameState->GetSoleLeader())
	{
		// 上一个领先的玩家失去了领先的地位，通知客户端显示失去领先
		// ESpecialElimType 是传给 AttackerPlayerState 客户端的，所以对于失去领先的玩家要发送 RPC
		LastSoleLeader->Client_LostTheLead();
		
		if (VictimPlayerState == LastSoleLeader)
		{
			OutSpecialElimType |= ESpecialElimType::Dethrone;
			AttackerPlayerState->AddDethroneElim();
		}
	}
	
	if (AttackerPlayerState != LastSoleLeader && AttackerPlayerState == GameState->GetSoleLeader())
	{
		OutSpecialElimType |= ESpecialElimType::TiedTheLeader;
	}
}
