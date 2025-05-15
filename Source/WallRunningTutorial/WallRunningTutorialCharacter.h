// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "WallRunningTutorialCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
enum ECornerType : uint8;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AWallRunningTutorialCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Rotation for the camera to lock onto. */
	FRotator TargetCameraRotation{};

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* WallRunAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Interpolation speed for rotating the CameraBoom to the target rotation for camera lock-ons. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float CameraLockOnInterpSpeed = 7.0f;

	/** True if the camera should lock onto a target. */
	bool bCameraLockOnActive = false;

public:

	AWallRunningTutorialCharacter(const FObjectInitializer& ObjectInitializer);	

	virtual void PostInitializeComponents() override;

	virtual void Tick(float DeltaTime) override;

	/** Returns the characters movement component casted to a UCustomCharacterMovementComponent. */
	class UCustomCharacterMovementComponent* GetCustomCharacterMovement() const;
		
protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

protected:

	virtual void BeginPlay() override;

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called when the character is beginning to turn around a corner for wall running. */
	virtual void OnCornerTurnBegin(const FVector& CornerTurnDirection, const ECornerType CornerType);

	/** Called when the character completed turning around a corner for wall running. */
	virtual void OnCornerTurnEnd();

	/** Rotates the CameraBoom when the character's camera should lock onto a target. */
	virtual void RotateCameraToTarget(float DeltaTime);

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

