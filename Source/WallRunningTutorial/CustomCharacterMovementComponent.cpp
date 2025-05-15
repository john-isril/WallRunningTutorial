// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomCharacterMovementComponent.h"
#include <GameFramework/Character.h>
#include <GameFramework/SpringArmComponent.h>
#include <Components/CapsuleComponent.h>
#include "WallrunnableInterface.h"
#include "CustomMovementModes.h"
#include <Kismet/KismetSystemLibrary.h>


void UCustomCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	bWantsToWallRun = false;
	WallSearchTraceDistance = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius() * 2.0;
	PrevNonWallrunnableActor = nullptr;
	CharacterOwner->GetCapsuleComponent()->OnComponentHit.AddUniqueDynamic(this, &UCustomCharacterMovementComponent::OnCapsuleHit);
}

void UCustomCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	WallRunControlInputVector = {};
}

void UCustomCharacterMovementComponent::AddInputVector(FVector WorldVector, bool bForce)
{
	if (IsWallRunning())
	{
		WallRunControlInputVector = WorldVector;
	}
	else
	{
		Super::AddInputVector(WorldVector, bForce);
	}
}

bool UCustomCharacterMovementComponent::CanAttemptJump() const
{
	if (IsWallRunning())
	{
		return bWallRunInitiated;
	}

	return Super::CanAttemptJump();
}

bool UCustomCharacterMovementComponent::IsWallRunning() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_WallRunning;
}

void UCustomCharacterMovementComponent::WallRunStart()
{
	if (bAutoWallRun) return;

	bWantsToWallRun = true;
}

void UCustomCharacterMovementComponent::WallRunStop()
{
	if (bAutoWallRun) return;

	bWantsToWallRun = false;

	if (IsWallRunning())
	{
		SetMovementMode(EMovementMode::MOVE_Falling);
	}
}

void UCustomCharacterMovementComponent::OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	/* No need to process an Actor that we've already determined is non-wallrunnable. */
	if (OtherActor == PrevNonWallrunnableActor) return;

	if (CanWallRun())
	{
		/* Check if the hit Actor inherits from the IWallrunnableInterface. If so, the Actor can be wall run on. Store the hit result and initiate the wall run */
		if (Cast<IWallrunnableInterface>(OtherActor))
		{
			WallRunHitResult = Hit;
			InitWallRun();
		}
		else
		{
			/* Store a pointer to this Actor to check again in the future if a hit actor is non-wall runnable.*/
			PrevNonWallrunnableActor = OtherActor;
		}
	}
}

bool UCustomCharacterMovementComponent::CanWallRun() const
{
	return (bAutoWallRun || bWantsToWallRun) && IsFalling() && !IsWallRunCooldownActive();
}

void UCustomCharacterMovementComponent::InitWallRun()
{
	WallRunControlInputVector = {};
	bWallRunInitiated = false;
	bIsTurningAroundCorner = false;

	SetMovementMode(MOVE_Custom, CMOVE_WallRunning);

	/* Save what side the wall is relative to the character. */
	const double RightProjWallNormal = FVector::DotProduct(CharacterOwner->GetActorRightVector(), WallRunHitResult.ImpactNormal);
	
	WallRunSide = (RightProjWallNormal > 0.0) ? EWRS_LeftSide : EWRS_RightSide;

	FRotator TargetRotation{};

	CalcWallRunRotation(TargetRotation);
	
	/* Rotate the character to the target rotation. */

	static constexpr float MoveDuration = 0.2f;

	const FLatentActionInfo LatentActionInfo{ 0, INDEX_NONE, TEXT("OnWallRunInitComplete"), this };
	UKismetSystemLibrary::MoveComponentTo(CharacterOwner->GetRootComponent(), CharacterOwner->GetActorLocation(), TargetRotation, true, true, MoveDuration, true, EMoveComponentAction::Move, LatentActionInfo);

}

void UCustomCharacterMovementComponent::CalcWallRunRotation(FRotator& OutWallRunRotation)
{
	FVector Y{};

	if (WallRunSide == EWRS_LeftSide)
	{
		Y = WallRunHitResult.ImpactNormal;
	}
	else
	{
		Y = -WallRunHitResult.ImpactNormal;
	}

	const FVector X = FVector::CrossProduct(Y, CharacterOwner->GetActorUpVector()).GetSafeNormal();

	OutWallRunRotation = FRotationMatrix::MakeFromXY(X, Y).Rotator();
}

void UCustomCharacterMovementComponent::OnWallRunInitComplete()
{
	bWallRunInitiated = true;
}

void UCustomCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case CMOVE_WallRunning:
		PhysWallRunning(deltaTime, Iterations);
		break;

	default:
		break;
	}
}

void UCustomCharacterMovementComponent::PhysWallRunning(float deltaTime, int32 Iterations)
{
	if (!bWallRunInitiated || bIsTurningAroundCorner || deltaTime < MIN_TICK_TIME) return;

	/* Check if the character is at an inner corner, then turn them around the corner if there is one. */

	FVector TraceStart = CharacterOwner->GetActorLocation();
	FVector TraceDirection = CharacterOwner->GetActorForwardVector();
	FVector TraceEnd = TraceStart + TraceDirection * WallSearchTraceDistance;
	GetWorld()->LineTraceSingleByChannel(WallRunHitResult, TraceStart, TraceEnd, ECC_Visibility);

	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false);

	if (WallRunHitResult.bBlockingHit)
	{
		HandleWallRunCorner(ECT_Inner);
		return;
	}
	
	/* Check if a wall is besides the character, then move the character along the wall if there is one. */

	if (WallRunSide == EWRS_LeftSide)
	{
		TraceDirection = -CharacterOwner->GetActorRightVector();
	}
	else
	{
		TraceDirection = CharacterOwner->GetActorRightVector();
	}

	TraceEnd = TraceStart + TraceDirection * WallSearchTraceDistance;

	GetWorld()->LineTraceSingleByChannel(WallRunHitResult, TraceStart, TraceEnd, ECC_Visibility);

	if (WallRunHitResult.bBlockingHit)
	{
		const FVector ImpactPointToOwner{ CharacterOwner->GetActorLocation() - WallRunHitResult.ImpactPoint };
		const double ImpactPointToOwnerProjImpactNormal{ ImpactPointToOwner.Dot(WallRunHitResult.ImpactNormal) };
		SafeMoveUpdatedComponent(-WallRunHitResult.ImpactNormal * ImpactPointToOwnerProjImpactNormal, UpdatedComponent->GetComponentQuat(), true, WallRunHitResult);

		FRotator TargetRotation{};
		CalcWallRunRotation(TargetRotation);
		const FRotator InterpedTargetRotation = FMath::RInterpTo(CharacterOwner->GetActorRotation(), TargetRotation, deltaTime, WallRunRotationInterpSpeed);

		Velocity = CharacterOwner->GetActorForwardVector() * WallRunSpeed;

		const FVector AdjustedVelocity = Velocity * deltaTime;
		SafeMoveUpdatedComponent(AdjustedVelocity, InterpedTargetRotation, true, WallRunHitResult);

		return;
	}

	/* The character isn't besides a wall to run along, and they're also not at an inner corner. Now check if they're at an outer corner. */

	if (WallRunSide == EWRS_LeftSide)
	{
		TraceStart = CharacterOwner->GetActorLocation() + -CharacterOwner->GetActorRightVector() * WallSearchTraceDistance;
	}
	else
	{
		TraceStart = CharacterOwner->GetActorLocation() + CharacterOwner->GetActorRightVector() * WallSearchTraceDistance;
	}

	TraceDirection = -CharacterOwner->GetActorForwardVector();
	TraceEnd = TraceStart + TraceDirection * WallSearchTraceDistance;

	GetWorld()->LineTraceSingleByChannel(WallRunHitResult, TraceStart, TraceEnd, ECC_Visibility);

	if (WallRunHitResult.bBlockingHit)
	{
		HandleWallRunCorner(ECT_Outer);
		return;
	}

	SetMovementMode(EMovementMode::MOVE_Falling);
}

void UCustomCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (PreviousMovementMode == EMovementMode::MOVE_Custom && PreviousCustomMode == CMOVE_WallRunning)
	{
		GetWorld()->GetTimerManager().SetTimer(WallRunCooldownTimer, WallRunCooldownDuration, false);
	}
}

bool UCustomCharacterMovementComponent::IsWallRunCooldownActive() const
{
	return GetWorld()->GetTimerManager().IsTimerActive(WallRunCooldownTimer);
}

void UCustomCharacterMovementComponent::HandleWallRunCorner(const ECornerType CornerType)
{
	FRotator TargetRotation{};
	CalcWallRunRotation(TargetRotation);

	const FVector CornerTurnDirection = FRotationMatrix(TargetRotation).GetUnitAxis(EAxis::X);
	const bool bPlayerWantsToTurn = FVector::DotProduct(CornerTurnDirection, WallRunControlInputVector) > 0.0;

	if (bPlayerWantsToTurn)
	{
		bIsTurningAroundCorner = true;

		OnCornerTurnBegin.ExecuteIfBound(CornerTurnDirection, CornerType);

		const FVector TargetLocation = WallRunHitResult.ImpactPoint + WallRunHitResult.ImpactNormal * CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();

		const FLatentActionInfo LatentActionInfo{ 0, INDEX_NONE, TEXT("OnTurnedAroundCorner"), this };
		UKismetSystemLibrary::MoveComponentTo(CharacterOwner->GetCapsuleComponent(), TargetLocation, TargetRotation, true, true, WallRunCornerTurnDuration, true, EMoveComponentAction::Move, LatentActionInfo);
	}
	else
	{
		SetMovementMode(EMovementMode::MOVE_Falling);
	}

	
}

void UCustomCharacterMovementComponent::OnTurnedAroundCorner()
{
	bIsTurningAroundCorner = false;
	OnCornerTurnEnd.ExecuteIfBound();
}
