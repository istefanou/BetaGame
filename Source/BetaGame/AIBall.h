// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BetaGameBall.h"
#include "AIBall.generated.h"


UCLASS(config = Game)
class AAIBall : public APawn
{
	GENERATED_BODY()

	/** StaticMesh used for the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ball, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Ball;

	UPROPERTY(EditDefaultsOnly, Category = Ball)
	ABetaGameBall* PlayerBall;

public:	
	// Sets default values for this component's properties
	AAIBall();

	float startX;
	float startY;
	bool isStart = false;
	int counter = 0;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Returns Ball subobject **/
	FORCEINLINE class UStaticMeshComponent* GetBall() const { return Ball; }

	void BoostForward();
	void BoostBackwards();
	
};
