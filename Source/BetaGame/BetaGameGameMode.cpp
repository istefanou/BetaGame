// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "BetaGameGameMode.h"
#include "GameFramework/Actor.h"
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
	//UGameplayStatics::GetPlayerPawn(...);
	//(ABetaGameGameMode*)GetWorld()->GetAuthGameMode();
	ABetaGameBall* myBall = Cast<ABetaGameBall>(myWorld->GetFirstPlayerController()->GetPawn());
	//myBall->OnDestroyed.AddDynamic(this, &ABetaGameGameMode::Restart);
	//myBall->OnClusterMarkedAsPendingKill.AddDynamic(this, &ABetaGameGameMode::Restart);
}

void ABetaGameGameMode::Restart(AActor* Act) {

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("DeadDeadDeadDeadDead")));

	myWorld = GetWorld();
	RestartPlayer(myWorld->GetFirstPlayerController());
}
