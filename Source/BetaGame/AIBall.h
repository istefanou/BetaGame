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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Ball, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Ball;

	UPROPERTY(EditDefaultsOnly, Category = Ball)
	ABetaGameBall* PlayerBall;

public:	
	// Sets default values for this component's properties
	AAIBall();

	bool isStart = false;
	int counter = 0;
	bool notMove = false;
	bool follow = false;

	UPROPERTY(EditAnywhere, Category = Ball)
	float factor = 800;

	UPROPERTY(EditAnywhere, Category = Ball)
	float centerX = 0;

	UPROPERTY(EditAnywhere, Category = Ball)
	float centerY = 0;

	UPROPERTY(EditAnywhere, Category = Ball)
	float radius = 400;


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

	float calcDistance(FVector v1, FVector v2);
	FVector normalizeVec(FVector v);

	void autoMovement(float delta);

	bool detectPlayer(float delta);

	bool detectArea(float delta, float x, float y);

	void moveTowardsPlayer(float delta);

	UFUNCTION(BlueprintCallable)
	void InitializeBall();
	
};
