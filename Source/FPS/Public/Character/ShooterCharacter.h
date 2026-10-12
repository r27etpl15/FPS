// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/PlayerInterface.h"
#include "ShooterCharacter.generated.h"

class UHealthComponent;
class UInputAction;
class UCombatComponent;
class UCameraComponent;
class USpringArmComponent;
enum class ETurningInPlace : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponFirstReplicated, AWeapon*, Weapon, bool, bCurrentlyTargetingPlayer);

UCLASS()
class FPS_API AShooterCharacter : public ACharacter, public IPlayerInterface
{
	GENERATED_BODY()

public:
	AShooterCharacter();
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	
	//~ Begin UPlayerInterface Interface
	virtual FName GetWeaponAttachPoint_Implementation(const FGameplayTag& WeaponTypeTag) const override;
	virtual USkeletalMeshComponent* GetMesh1P_Implementation() const override;
	virtual USkeletalMeshComponent* GetMesh3P_Implementation() const override;
	virtual void WeaponReplicated_Implementation() override;
	virtual AWeapon* GetCurrentWeapon_Implementation() const override;
	virtual int32 GetReserveAmmo_Implementation() const override;
	virtual void Notify_CycleWeapon_Implementation() override;
	virtual void Notify_ReloadWeapon_Implementation() override;
	virtual void AddAmmo_Implementation(const FGameplayTag& WeaponType, int32 AmmoAmount) override;
	virtual bool DoDamage_Implementation(float DamageAmount, AActor* DamageInstigator) override;
	//~ End UPlayerInterface Interface
	
	bool HasWeaponFirstReplicated() const { return bWeaponFirstReplicated; }
	
	// 修复 GetBaseAimRotation() 压缩后数值范围问题
	UFUNCTION(BlueprintCallable)
	FRotator GetFixedAimRotation() const;
	
	UFUNCTION(BlueprintCallable)
	bool HasCurrentWeapon() const;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|FABRIK")
	FTransform FABRIKSocketTransform;
	
	UPROPERTY(BlueprintAssignable)
	FWeaponFirstReplicated OnWeaponFirstReplicated;

protected:
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnAim(bool bIsAiming);
	
	UFUNCTION()
	void OnDeathStarted();
	
	UFUNCTION(BlueprintImplementableEvent)
	void DeathEffects();
	
private:
	void Input_CycleWeapon();
	void Input_ReloadWeapon();
	void Input_FireWeaponPressed();
	void Input_FireWeaponReleased();
	void Input_AimPressed();
	void Input_AimReleased();
	
	void CalculateFABRIKSocketTransform();
	// 原地转身
	void CalculateTurnInPlaceParameters(float DeltaTime);
	void TurnInPlace(float DeltaTime);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HitReact(int32 MontageIndex);
	
	void DeathTimerFinished();
	
	FRotator StartingAimRotation;
	float InterpAO_Yaw;
	bool bWeaponFirstReplicated = false;
	FTimerHandle DeathTimer;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|TurnInPlace", meta=(AllowPrivateAccess = "true"))
	float AO_YAW;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|TurnInPlace", meta=(AllowPrivateAccess = "true"))
	ETurningInPlace TurningStatus;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|Strafing", meta=(AllowPrivateAccess = "true"))
	float MovementOffsetYaw;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Respawn", meta=(AllowPrivateAccess = "true"))
	float RespawnTime = 5.0f;
	
	// 第一人称视角
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh1P;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS|Camera", meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCamera;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Aiming", meta=(AllowPrivateAccess = "true"))
	float DefaultFOV;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS|Combat", meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UCombatComponent> CombatComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS|Health", meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;
	
	// 输入
	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> CycleWeaponAction;
	
	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> FireWeaponAction;
	
	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> ReloadWeaponAction;
	
	UPROPERTY(EditAnywhere, Category="FPS|Input")
	TObjectPtr<UInputAction> AimWeaponAction;
	
	UPROPERTY(EditAnywhere, Category="FPS|HitReact")
	TArray<TObjectPtr<UAnimMontage>> HitReacts;
};
