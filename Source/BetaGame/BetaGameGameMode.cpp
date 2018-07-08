// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "BetaGameGameMode.h"
#include "BetaGameBall.h"

ABetaGameGameMode::ABetaGameGameMode()
{
	// set default pawn class to our ball
	DefaultPawnClass = ABetaGameBall::StaticClass();

	
}

void ABetaGameGameMode::BeginPlay() {
	Super::BeginPlay();
	myWorld = GetWorld();
	myWorld->GetFirstPlayerController();
	//ACharacter* myCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	ABetaGameBall* myBall = Cast<ABetaGameBall>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	myBall->OnDestroyed.AddDynamic(this, &ABetaGameGameMode::Restart);
}

void ABetaGameGameMode::Restart() {
	RestartPlayer();
}