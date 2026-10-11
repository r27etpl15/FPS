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
	float OldValue = Health;
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	
	OnHealthChanged.Broadcast(this, OldValue, Health, Instigator);
	
	return false;
}

void UHealthComponent::ChangeMaxHealthByAmount(float Amount, AActor* Instigator)
{
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
	
}

void UHealthComponent::OnRep_Health(float OldValue)
{
	OnHealthChanged.Broadcast(this, OldValue, Health, nullptr);
}

void UHealthComponent::OnRep_MaxHealth(float OldValue)
{
	OnMaxHealthChanged.Broadcast(this, OldValue, MaxHealth, nullptr);
}
