#pragma once
#include "CustomMovementModes.generated.h"

UENUM(BlueprintType)
enum ECustomMovementMode : int
{
	CMOVE_WallRunning	UMETA(DisplayName = "Wall Running"),

	CMOVE_MAX			UMETA(Hidden),
};