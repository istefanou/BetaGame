// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PadActor.h"
#include "JumpPadActor.generated.h"



UCLASS()
class BETAGAME_API AJumpPadActor : public APadActor
{
	GENERATED_BODY()


	UFUNCTION()
		void InitiatePadAction(UPrimitiveComponent *OverlappedComponent,
			AActor *OtherActor,
			UPrimitiveComponent *OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult &SweepResult) override;

	UFUNCTION()
		void FinishPadAction(UPrimitiveComponent *OverlappedComponent, AActor * OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

public:
	// Sets default values for this actor's properties
	AJumpPadActor();
	void BeginPlay();
};
