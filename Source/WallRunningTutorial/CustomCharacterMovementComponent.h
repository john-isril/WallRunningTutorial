// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomCharacterMovementComponent.generated.h"

/** Enum describing where is the wall relative to the character. Is the wall that the character's running on on the left or right side of the character? */
UENUM(BlueprintType, DisplayName = "Wall Run Side")
enum EWallRunSide : uint8
{
	EWRS_None		UMETA(DisplayName = "None"),
	EWRS_LeftSide	UMETA(DisplayName = "Left Side"),
	EWRS_RightSide	UMETA(DisplayName = "Right Side"),

	EWRS_MAX		UMETA(Hidden),
};

/** Enum describing what type of corner the character is at. Is it an inner or outer corner? */
enum ECornerType : uint8
{
	ECT_Inner	UMETA(DisplayName = "Inner"),
	ECT_Outer	UMETA(DisplayName = "Outer"),
	
	ECT_MAX		UMETA(Hidden),
};

/** Non-dynamic single delegate signature used to notify when the character is beginning to turn around a corner. The first parameter is a vector representing the direction of the corner turn. The second parameter is the corner type that the character is at. */
DECLARE_DELEGATE_TwoParams(FOnCornerTurnBeginSignature, const FVector& CornerTurnDirection, const ECornerType CornerType);
/** Non-dynamic single delegate signature used to notify when the character has completed turning around a corner. */
DECLARE_DELEGATE(FOnCornerTurnEndSignature);

/**
 * UCustomCharacterMovementComponent is an extension of UCharacterMovementComponent that includes a movement mode for wall running.
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

	/** Returns true if the character is in the wall running movement mode. */
	UFUNCTION(BlueprintCallable)
	bool IsWallRunning() const;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE EWallRunSide GetWallRunSide() const { return WallRunSide; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsTurningAroundCorner() const { return bIsTurningAroundCorner; }

	/** Enables the character to enter a wall run. */
	void WallRunStart();

	/* Terminates a wall run if one is in progress, and prevents the character from initiating another wall run. */
	void WallRunStop();

public:
	/** Delegate used to notify when the character is beginning to turn around a corner. Should only be subscribed to by the owning character. */
	FOnCornerTurnBeginSignature OnCornerTurnBegin;

	/** Delegate used to notify when the character has completed turning around a corner. Should only be subscribed to by the owning character. */
	FOnCornerTurnEndSignature OnCornerTurnEnd;

private:

	/** FHitResult storing the hit info for wall run specific line traces. */
	FHitResult WallRunHitResult{};

	/** World space FVector that stores the characters input while wall running and is set with the AddInputVector function. Used to detect if the character wants to turn around a corner. This will be zeroed out after every Tick. */
	FVector WallRunControlInputVector{};

	/** Actor pointer used to store the previous Actor that is not wall runnable. Primarily used for optimization. */
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> PrevNonWallrunnableActor{ nullptr };

	/** Timer used to temporarily disable wall running after one has completed. */
	FTimerHandle WallRunCooldownTimer;

	/** The distance for line traces that search for walls to run on. */
	double WallSearchTraceDistance = 0.0;

	/** Speed that the character can wall run. */
	UPROPERTY(EditAnywhere, Category = Movement, meta = (DisplayName = "Wall Run Speed"))
	float WallRunSpeed = 550.0f;

	/** The interpolation speed for rotating the character when wall running. */
	UPROPERTY(EditAnywhere, Category = Movement, meta = (DisplayName = "Wall Run Rotation Interpolation Speed"))
	float WallRunRotationInterpSpeed = 5.0f;
	
	/** Time to temporarily disable wall running after one has completed. */
	UPROPERTY(EditAnywhere, Category = Movement, meta = (DisplayName = "Wall Run Cooldown Duration"))
	float WallRunCooldownDuration = 0.7f;

	/** The time it takes to turn around a corner for wall running. */
	UPROPERTY(EditAnywhere, Category = Movement, meta = (DisplayName = "Wall Run Corner Turn Duration"))
	float WallRunCornerTurnDuration = 0.3f;

	/** If true, the character can automatically wall run if they are close enough to a wall without requiring calls to WallRunStart or WallRunStop. */
	UPROPERTY(EditAnywhere, Category = Movement, meta = (DisplayName = "Auto Wall Run"))
	bool bAutoWallRun = true;

	/** If true, the character/player is attempting to wall run. This is also set with a call to WallRunStart or WallRunStop. */
	bool bWantsToWallRun = false;

	/** If true, the character's wall run initiation is complete. If false, the wall run initiation is in progress or hasn't started. */
	bool bWallRunInitiated = false;

	/** If true, the character is currently turning around a corner. */
	bool bIsTurningAroundCorner = false;

	/** Enum describing where is the wall relative to the character. Is the wall that the character's running on on the left or right side of the character? */
	EWallRunSide WallRunSide{ EWRS_None };

protected:

	/** Called when the character's capsule component hit another object. */
	UFUNCTION()
	virtual void OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/**
	 * Check if the character has met the requirements to wall run.
	 *
	 * @return		True if the character can start wall running.
	 */
	virtual bool CanWallRun() const;

	/** Rotates the character to the initial orienation for wall runs to align with the wall. */
	virtual void InitWallRun();

	/**
	 * Find the rotation that the character needs to have to match the walls orientation for wall running.
	 *
	 * @param OutWallRunRotation:		[Out] The FRotator calculated for the wall run.
	 */
	virtual void CalcWallRunRotation(FRotator& OutWallRunRotation);

	/** Called when the character completed rotating to the initial wall run rotation. */
	UFUNCTION()
	virtual void OnWallRunInitComplete();

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	/** Called every frame to move and rotate the character along the wall when wall running. */
	virtual void PhysWallRunning(float deltaTime, int32 Iterations);

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	/**
	 * Check if the cooldown period for wall running is still in progress.
	 *
	 * @return		True if the cooldown period for wall running is still in progress.
	 */
	bool IsWallRunCooldownActive() const;

	/**
	 * Turns the character around a previously detected corner while wall running.
	 *
	 * @param TurnDirection:		The vector representing the direction to turn around the corner.
	 */
	virtual void HandleWallRunCorner(const ECornerType CornerType);

	/** Called once the character has completed turning around a corner while wall running. */
	UFUNCTION()
	virtual void OnTurnedAroundCorner();
};
