// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FHealthChanged, UHealthComponent*, HealthComponent, float, OldValue, float, NewValue, AActor*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDeathStarted);

UENUM(BlueprintType)
enum class EDeathState : uint8 
{
	NotDead,
	DeathStarted,
	DeathFinished
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPS_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	EDeathState GetDeathState() const { return DeathState; }
	float GetHealth() const { return Health; }
	float GetMaxHealth() const { return MaxHealth; }
	
	UFUNCTION(BlueprintPure, Category="FPS|Health")
	static UHealthComponent* FindHealthComponent(const AActor* Actor) { return IsValid(Actor) ? Actor->FindComponentByClass<UHealthComponent>() : nullptr; }
	
	UFUNCTION(BlueprintCallable)
	float GetHealthNormalized() const;
	
	// 返回值为 true 代表是致命攻击
	bool ChangeHealthByAmount(float Amount, AActor* Instigator);
	void ChangeMaxHealthByAmount(float Amount, AActor* Instigator);
	
	UPROPERTY(BlueprintAssignable)
	FHealthChanged OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FHealthChanged OnMaxHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FDeathStarted OnDeathStarted;
	
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnRep_DeathState(EDeathState OldDeathState);
	
	UFUNCTION()
	void OnRep_Health(float OldValue);
	
	UFUNCTION()
	void OnRep_MaxHealth(float OldValue);
	
	UPROPERTY(ReplicatedUsing=OnRep_DeathState)
	EDeathState DeathState = EDeathState::NotDead;
	
	UPROPERTY(ReplicatedUsing=OnRep_Health, EditDefaultsOnly, Category="FPS|Health")
	float Health = 100.0f;
	
	UPROPERTY(ReplicatedUsing=OnRep_MaxHealth, EditDefaultsOnly, Category="FPS|Health")
	float MaxHealth = 100.0f;
	
private:
	void StartDeath();
};