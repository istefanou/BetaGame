// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PadActor.h"
#include "TeleporterPadActor.generated.h"




UCLASS()
class BETAGAME_API ATeleporterPadActor : public APadActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = TeleporterPad)
	ATeleporterPadActor* DestinationPad;

	UPROPERTY()
	TArray<UPrimitiveComponent*> ComponentsInTransit;

	UFUNCTION()
		void InitiatePadAction(UPrimitiveComponent *OverlappedComponent,
			AActor *OtherActor,
			UPrimitiveComponent *OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult &SweepResult) override;

	UFUNCTION()
		void FinishPadAction(UPrimitiveComponent *OverlappedComponent, AActor * OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;
	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

public:
	// Sets default values for this actor's properties
	ATeleporterPadActor();
	void BeginPlay();
};
