// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Public/EngineGlobals.h"
#include "Engine.h"
#include "Components/InputComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include <Runtime/Engine/Classes/GameFramework/Actor.h>
#include "BetaGameBall.generated.h"

UCLASS(config=Game)
class ABetaGameBall : public APawn
{
	GENERATED_BODY()

	/** StaticMesh used for the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ball, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Ball;

	/** Spring arm for positioning the camera above the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ball, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArm;

	/** Camera to view the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ball, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

public:
	
	ABetaGameBall();
	APlayerController* playercontroller;


	/** Vertical impulse to apply when pressing jump */
	UPROPERTY(EditAnywhere, Category=Ball)
	float JumpImpulse;
	UPROPERTY(EditAnywhere, Category = Ball)
	float DashImpulse;
	/** Torque to apply when trying to roll ball */
	UPROPERTY(EditAnywhere, Category=Ball)
	float RollTorque;

	/** Indicates whether we can currently jump, use to prevent double jumping */
	bool bCanJump;

	UPROPERTY(BlueprintReadWrite)
	int current_stamina;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int max_stamina; 


	bool phone_debug_messages = true;


	float zrot_offset=0.f;
	float xrot_offset= 0.f;
	float yrot_offset= 0.f;

	UPROPERTY(VisibleAnywhere)
	float x_rotation = 0.f;

	UPROPERTY(VisibleAnywhere)
	float y_rotation = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementOptions)
	float max_speed_multiplier = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementOptions)
	float torque_multiplier = 100000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementOptions)
	float x_movement_multiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementOptions)
	float y_movement_multiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementOptions)
	float dead_zone_offset = 0.0001f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementOptions)
	bool flip_x_movement = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementOptions)
	bool flip_y_movement = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementOptions)
	bool phone = false;

	//to detect phone dashes
	FVector accel_diffs[10];

	//location of next value
	int addloc = 0;

	//debug print counter
	long counter = 0;

protected:

/** Called for side to side input */
    void OnRotationInput(FVector Input );
	void OnRotationInputx(float value);
	void OnRotationInputy(float value);
	void OnSimulatedRotationInputx(float value);
	void OnSimulatedRotationInputy(float value);

	void Tick(float DeltaTime);

	void BoostRight();

	void MoveRight(float Val);

	void MoveForward(float Val);


	void BoostLeft(); 


	void BoostForward(); 


	void BoostBackwards(); 


	/** Handle jump action. */
	void Jump();

	// AActor interface
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	// End of AActor interface

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	void BeginPlay() override;
	// End of APawn interface

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	UPROPERTY()
	float RespawnDelay = 5;

	UPROPERTY()
	FVector SpawnLocation = FVector(0);

public:
	/** Returns Ball subobject **/
	FORCEINLINE class UStaticMeshComponent* GetBall() const { return Ball; }
	/** Returns SpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }

	FVector GetLoc();
};
