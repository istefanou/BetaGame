// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "BetaGameGameMode.h"
#include "BetaGameBall.h"

ABetaGameGameMode::ABetaGameGameMode()
{
	// set default pawn class to our ball
	DefaultPawnClass = ABetaGameBall::StaticClass();
}
