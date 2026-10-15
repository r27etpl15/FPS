// Fill out your copyright notice in the Description page of Project Settings.


#include "Attributes/HealthComponent.h"

#include "Net/UnrealNetwork.h"


UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UHealthComponent, DeathState);
	DOREPLIFETIME_CONDITION(UHealthComponent, Health, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UHealthComponent, MaxHealth, COND_OwnerOnly);
}


float UHealthComponent::GetHealthNormalized() const
{
	return (MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f;
}

bool UHealthComponent::ChangeHealthByAmount(float Amount, AActor* Instigator)
{
	// 反作弊：血量仅允许服务器修改
	if (!GetOwner() || !GetOwner()->HasAuthority()) { return false; }
	
	float OldValue = Health;
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	
	OnHealthChanged.Broadcast(this, OldValue, Health, Instigator);
	
	if (Health <= 0.0f)
	{
		StartDeath();
		return true;
	}
	
	return false;
}

void UHealthComponent::StartDeath()
{
	if (DeathState != EDeathState::NotDead) { return; }
	
	DeathState = EDeathState::DeathStarted;
	
	// 服务器广播
	OnDeathStarted.Broadcast();
	
	GetOwner()->ForceNetUpdate();
}

void UHealthComponent::ChangeMaxHealthByAmount(float Amount, AActor* Instigator)
{
	// 反作弊：最大血量仅允许服务器修改
	if (!GetOwner() || !GetOwner()->HasAuthority()) { return; }
	
	float OldValue = MaxHealth;
	MaxHealth += Amount;
	
	OnMaxHealthChanged.Broadcast(this, OldValue, MaxHealth, Instigator);
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void UHealthComponent::OnRep_DeathState(EDeathState OldDeathState)
{
	if (DeathState == EDeathState::DeathStarted)
	{
		// 客户端广播
		OnDeathStarted.Broadcast();
	}
}

void UHealthComponent::OnRep_Health(float OldValue)
{
	OnHealthChanged.Broadcast(this, OldValue, Health, nullptr);
}

void UHealthComponent::OnRep_MaxHealth(float OldValue)
{
	OnMaxHealthChanged.Broadcast(this, OldValue, MaxHealth, nullptr);
}
