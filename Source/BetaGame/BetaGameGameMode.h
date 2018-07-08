// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BetaGameGameMode.generated.h"

UCLASS(minimalapi)
class ABetaGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABetaGameGameMode();
	void BeginPlay();
	void Restart();
	UWorld* myWorld;
};



