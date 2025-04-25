// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomCharacterMovementComponent.generated.h"

UENUM(BlueprintType, DisplayName = "Wall Run Side")
enum EWallRunSide : uint8
{
	EWRS_None		UMETA(DisplayName = "None"),
	EWRS_LeftSide	UMETA(DisplayName = "Left Side"),
	EWRS_RightSide	UMETA(DisplayName = "Right Side"),

	EWRS_MAX		UMETA(Hidden),
};

enum ECornerType : uint8
{
	ECT_Inner	UMETA(DisplayName = "Inner"),
	ECT_Outer	UMETA(DisplayName = "Outer"),
	
	ECT_MAX		UMETA(Hidden),
};

DECLARE_DELEGATE_TwoParams(FOnCornerTurnBeginSignature, const FVector& CornerTurnDirection, const ECornerType CornerType);
DECLARE_DELEGATE(FOnCornerTurnEndSignature);

/**
 * 
 */
UCLASS()
class WALLRUNNINGTUTORIAL_API UCustomCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void AddInputVector(FVector WorldVector, bool bForce = false) override;

	virtual bool CanAttemptJump() const override;

	UFUNCTION(BlueprintCallable)
	bool IsWallRunning() const;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE EWallRunSide GetWallRunSide() const { return WallRunSide; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsTurningAroundCorner() const { return bIsTurningAroundCorner; }

	void WallRunStart();

	void WallRunStop();

public:
	FOnCornerTurnBeginSignature OnCornerTurnBegin;

	FOnCornerTurnEndSignature OnCornerTurnEnd;

private:
	FHitResult WallRunHitResult{};

	FVector WallRunControlInputVector{};

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> PrevNonWallrunnableActor{ nullptr };

	FTimerHandle WallRunCooldownTimer;

	double WallSearchTraceDistance = 0.0;

	UPROPERTY(EditAnywhere, Category = Movement, meta = (DisplayName = "Wall Run Speed"))
	float WallRunSpeed = 550.0f;

	UPROPERTY(EditAnywhere, Category = Movement, meta = (DisplayName = "Wall Run Rotation Interpolation Speed"))
	float WallRunRotationInterpSpeed = 5.0f;
	
	UPROPERTY(EditAnywhere, Category = Movement, meta = (DisplayName = "Wall Run Cooldown Duration"))
	float WallRunCooldownDuration = 0.7f;

	UPROPERTY(EditAnywhere, Category = Movement, meta = (DisplayName = "Wall Run Corner Turn Duration"))
	float WallRunCornerTurnDuration = 0.3f;

	UPROPERTY(EditAnywhere, Category = Movement, meta = (DisplayName = "Auto Wall Run"))
	bool bAutoWallRun = true;

	bool bWantsToWallRun = false;

	bool bWallRunInitiated = false;

	bool bIsTurningAroundCorner = false;

	EWallRunSide WallRunSide{ EWRS_None };

protected:
	UFUNCTION()
	virtual void OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual bool CanWallRun() const;

	virtual void InitWallRun();

	virtual void CalcWallRunRotation(FRotator& OutWallRunRotation);

	UFUNCTION()
	virtual void OnWallRunInitComplete();

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	virtual void PhysWallRunning(float deltaTime, int32 Iterations);

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	bool IsWallRunCooldownActive() const;

	virtual void HandleWallRunCorner(const ECornerType CornerType);

	UFUNCTION()

	virtual void OnTurnedAroundCorner();
};
