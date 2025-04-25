// Copyright Epic Games, Inc. All Rights Reserved.

#include "WallRunningTutorialGameMode.h"
#include "WallRunningTutorialCharacter.h"
#include "UObject/ConstructorHelpers.h"

AWallRunningTutorialGameMode::AWallRunningTutorialGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
