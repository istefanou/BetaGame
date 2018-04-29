// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "BetaGameBall.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/GameFramework/PawnMovementComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Character.h"


ABetaGameBall::ABetaGameBall()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BallMesh(TEXT("/Game/Rolling/Meshes/BallMesh.BallMesh"));

	// Create mesh component for the ball
	Ball = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ball0"));
	Ball->SetStaticMesh(BallMesh.Object);
	Ball->BodyInstance.SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	Ball->SetSimulatePhysics(true);
	Ball->SetAngularDamping(0.1f);
	Ball->SetLinearDamping(0.1f);
	Ball->BodyInstance.MassScale = 3.5f;
	Ball->BodyInstance.MaxAngularVelocity = 800.0f;
	Ball->SetNotifyRigidBodyCollision(true);
	RootComponent = Ball;

	// Create a camera boom attached to the root (ball)
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bDoCollisionTest = false;
	SpringArm->bAbsoluteRotation = true; // Rotation of the ball should not affect rotation of boom
	SpringArm->RelativeRotation = FRotator(-45.f, 0.f, 0.f);
	SpringArm->TargetArmLength = 1200.f;
	SpringArm->bEnableCameraLag = false;
	SpringArm->CameraLagSpeed = 3.f;

	// Create a camera and attach to boom
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // We don't want the controller rotating the camera

	// Set up forces
	RollTorque = 50000000.0f*(1 / scaleFactor);
	JumpImpulse = 350000.0f;
	DashImpulse = 3500000.0f;
	bCanJump = true; // Start being able to jump
	stamina = 3;

	this->yrot_offset = 1;

	for (int i = 0; i < 10; i++) {
		this->accel_diffs[i] = FVector(0.f, 0.f, 0.f);
	}

}

void ABetaGameBall::SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent)
{
	// set up gameplay key bindings
	PlayerInputComponent->BindVectorAxis("Tilt", this, &ABetaGameBall::OnRotationInput);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABetaGameBall::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &ABetaGameBall::MoveForward);



	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABetaGameBall::Jump);
	PlayerInputComponent->BindAction("BoostRight", IE_Pressed, this, &ABetaGameBall::BoostRight);
	PlayerInputComponent->BindAction("BoostForward", IE_Pressed, this, &ABetaGameBall::BoostForward);
	PlayerInputComponent->BindAction("BoostLeft", IE_Pressed, this, &ABetaGameBall::BoostLeft);
	PlayerInputComponent->BindAction("BoostBackwards", IE_Pressed, this, &ABetaGameBall::BoostBackwards);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ABetaGameBall::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ABetaGameBall::TouchStopped);
}



void ABetaGameBall::BeginPlay() {
	Super::BeginPlay();
	playercontroller = UGameplayStatics::GetPlayerController(this->GetWorld(), 0);
	ABetaGameBall::phone_debug_messages = true;
}

void ABetaGameBall::OnRotationInput(FVector Input)
{

	static float DEG_TO_RAD = PI / (180.f);

	//TODO: get right and down scale factor from player controller to make the game playable for people who don't want to do full body motions, like while sitting

	float x_rad = Input.X;
	float y_rad = Input.Y;

	if (ABetaGameBall::phone_debug_messages && (this->counter) % 10 == 0) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Tilt x: %f y: %f z: %f"), Input.X, Input.Y, Input.Z));
	}

	const float zero_offset = 0.1;
	FVector Torque=FVector(0,0,0);

	//smartphone
	if (x_rad != 0 && y_rad != 0) {

		float actual_x = x_rad;
		if (x_rad < this->xrot_offset + zero_offset && x_rad > this->xrot_offset - zero_offset || !bCanJump) {
			actual_x = xrot_offset;
		}

		float actual_y = y_rad;
		if (y_rad < this->yrot_offset + zero_offset && y_rad > this->yrot_offset - zero_offset || !bCanJump) {
			actual_y = yrot_offset;
		}


		const FVector current_angular_velocity = Ball->GetPhysicsAngularVelocity();

		const FVector target_angular_velocity = FVector(actual_x * this->max_speed_multiplier, actual_y * this->max_speed_multiplier, 0.f);
		const float torque_multiplier = 100000;

		float x_max_accel = 0;
		float y_max_accel = 0;

		if (target_angular_velocity.X - current_angular_velocity.X > 0 && target_angular_velocity.X>0) {
			x_max_accel = 1;
		}
		if (target_angular_velocity.X - current_angular_velocity.X < 0 && target_angular_velocity.X<0) {
			x_max_accel = -1;
		}
		if (target_angular_velocity.Y - current_angular_velocity.Y > 0 && target_angular_velocity.Y>0) {
			y_max_accel = 1;
		}
		if (target_angular_velocity.Y - current_angular_velocity.Y < 0 && target_angular_velocity.Y<0) {
			y_max_accel = -1;
		}
		Torque = FVector(x_max_accel*torque_multiplier, y_max_accel * torque_multiplier, 0.f);\
			
			if (ABetaGameBall::phone_debug_messages && (this->counter) % 10 == 0) {
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Target Velocity x: %f y: %f z: %f"), target_angular_velocity.X, target_angular_velocity.Y, target_angular_velocity.Z));

				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Torque aplied x: %f y: %f z: %f"), Torque.X, Torque.Y, Torque.Z));
			}
		
	}
	//pc
	else {
		if (false) {
			float actual_x = x_rad;
			if (x_rad < zero_offset && x_rad > -zero_offset || !bCanJump) {
				actual_x = xrot_offset;
			}

			float actual_y = y_rad;
			if (y_rad < zero_offset  && y_rad > -zero_offset && y_rad != 0 || !bCanJump) {
				actual_y = yrot_offset;
			}

			Torque = FVector(actual_x, actual_y, 0.f);
		}
	}

	Ball->AddTorqueInRadians(Torque);

}

void ABetaGameBall::MoveRight(float Val)
{
	const FVector Torque = FVector(-1.f * Val * RollTorque, 0.f, 0.f);
	Ball->AddTorqueInRadians(Torque);
}

void ABetaGameBall::MoveForward(float Val)
{
	const FVector Torque = FVector(0.f, Val * RollTorque, 0.f);
	Ball->AddTorqueInRadians(Torque);
}

void ABetaGameBall::BoostRight() //
{

	const FVector Impulse = FVector(0.f, DashImpulse, 0.f);
	Ball->AddImpulse(Impulse);
}

void ABetaGameBall::BoostLeft() //
{

	const FVector Impulse = FVector(0.f, -DashImpulse, 0.f);
	Ball->AddImpulse(Impulse);
}

void ABetaGameBall::BoostForward() //
{

	const FVector Impulse = FVector(DashImpulse, 0.f, 0.f);
	Ball->AddImpulse(Impulse);
}

void ABetaGameBall::BoostBackwards() //
{

	const FVector Impulse = FVector(-DashImpulse, 0.f, 0.f);
	Ball->AddImpulse(Impulse);
}

float delta_sum = 0;

void ABetaGameBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCanJump) delta_sum += DeltaTime;
	if (delta_sum > 1)
	{
		if (stamina < 3)
			stamina++;
		delta_sum -= 1;
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("STAMINA : %d"), stamina));
	}

	FVector Tilt;
	FVector RotationRate;
	FVector Gravity;
	FVector Acceleration;


	if (playercontroller != nullptr) {
		playercontroller->GetInputMotionState(Tilt, RotationRate, Gravity, Acceleration);
		if(ABetaGameBall::phone_debug_messages && (this->counter)%10==0)
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("RotationRate x: %f y: %f z: %f"), RotationRate.X, RotationRate.Y, RotationRate.Z));
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Gravity x: %f y: %f z: %f"), Gravity.X, Gravity.Y, Gravity.Z));
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Acceleration x: %f y: %f z: %f"), Acceleration.X, Acceleration.Y, Acceleration.Z));
			
			if (addloc % 10 == 0) {
				int actual_loc = addloc % 10;
				int previous_loc;
				if (actual_loc == 0) {
					previous_loc = 9;
				}
				else {
					previous_loc = actual_loc - 1;
				}
			}
			this->counter++;
			this->addloc++;
			if (addloc == 100) {
				addloc = 0;
			}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("playercontroller is null"));
	}

}

void ABetaGameBall::Jump()
{

	if (stamina > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("STAMINA : %d"), stamina);
		const FVector Impulse = FVector(0.f, 0.f, JumpImpulse);
		Ball->AddImpulse(Impulse);
		stamina--;
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("NO STAMINA"));
}

void ABetaGameBall::NotifyHit(class UPrimitiveComponent *MyComp, class AActor *Other, class UPrimitiveComponent *OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult &Hit)
{
	bCanJump = true;
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
}

void ABetaGameBall::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (bCanJump)
	{
		const FVector Impulse = FVector(0.f, 0.f, JumpImpulse);
		Ball->AddImpulse(Impulse);
		bCanJump = false;
	}
}

void ABetaGameBall::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (bCanJump)
	{
		const FVector Impulse = FVector(0.f, 0.f, JumpImpulse);
		Ball->AddImpulse(Impulse);
		bCanJump = false;
	}
}
