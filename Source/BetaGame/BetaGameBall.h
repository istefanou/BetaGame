// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Public/EngineGlobals.h"
#include "Runtime/Engine/Classes/Camera/PlayerCameraManager.h"
#include "Engine.h"
#include "Components/InputComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BetaGameBall.generated.h"

UCLASS(config = Game)
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
	UPROPERTY(EditAnywhere, Category = Ball)
		float JumpImpulse = 11000.0;
	UPROPERTY(EditAnywhere, Category = Ball)
		float DashImpulse = 11000.0;
	/** Torque to apply when trying to roll ball */
	UPROPERTY(EditAnywhere, Category = Ball)
		float RollTorque;

	/** Indicates whether we are in the air though our own action to prevent stamina from regenerating */
	bool isNotInTheAir;

	UPROPERTY(BlueprintReadWrite)
		int current_stamina=3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int max_stamina=3;


	bool phone_debug_messages = true;

	// this are offset in the rotation of the phone so that the ball stops at more comfortable viewing positions
	float zrot_offset = 0.f;
	float xrot_offset = 0.f;
	float yrot_offset = 0.f;

	UPROPERTY(VisibleAnywhere)
		float x_rotation = 0.f;

	UPROPERTY(VisibleAnywhere)
		float y_rotation = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementOptions)
		float max_speed_multiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementOptions)
		float torque_multiplier = 100000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementOptions)
		float x_movement_multiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementOptions)
		float y_movement_multiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementOptions)
		float dead_zone_offset = 0.0001f;


	//these are different when on the phone and when not, just keeping them here so they can be easily fixed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementOptions)
		bool flip_x_movement = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementOptions)
		bool flip_y_movement = false;

	// should be true only when running at a phone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MovementOptions)
		bool phone = false;

	//debug print counter to be used with % to not spam debug messages
	long counter = 0;

	//allows us to controll the player camera
	APlayerCameraManager* camera_manager;


	const float DEG_TO_RAD = PI / 180.f;
	const float RAD_TO_DEG = 180.f / PI;


protected:


	//called per frame
	void Tick(float DeltaTime);

	//changes a vector with yaw zero to the relevant yaw
	FVector toDirectionalVector(const FVector vector);

	// called for the simple ball movement imagining that the floors tilt is that of the phones
	void OnTiltInput(FVector Input);

	//simulated phone gradual tilt for testing instead of the fixed one a keyboard can provide
	void OnSimulatedTiltInputx(float value);
	void OnSimulatedTiltInputy(float value);

	// called to rotate the camera when using a rotation gesture on the phone
	void OnRotationInput(float Input);

	//simulated phone gradual tilt for testing instead of the fixed one a keyboard can provide
	void OnSimulatedRotationInput(float value);

	// omnidirectional boost
	void Boost(FVector direction);

	//simulated boosts for testing
	void BoostRight();
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


public:
	/** Returns Ball subobject **/
	FORCEINLINE class UStaticMeshComponent* GetBall() const { return Ball; }
	/** Returns SpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }


	FVector GetLoc();
};
