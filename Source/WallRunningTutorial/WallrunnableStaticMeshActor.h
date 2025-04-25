// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "WallrunnableInterface.h"
#include "WallrunnableStaticMeshActor.generated.h"

/**
 * 
 */
UCLASS()
class WALLRUNNINGTUTORIAL_API AWallrunnableStaticMeshActor : public AStaticMeshActor, public IWallrunnableInterface
{
	GENERATED_BODY()
	
};
