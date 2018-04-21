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
	
   
}

void ABetaGameBall::SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent)
{
	// set up gameplay key bindings
	PlayerInputComponent->BindVectorAxis("Tilt", this, &ABetaGameBall::OnRotationInput);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABetaGameBall::Jump);
	PlayerInputComponent->BindAction("BoostRight", IE_Pressed, this, &ABetaGameBall::BoostRight);
	PlayerInputComponent->BindAction("BoostForward", IE_Pressed, this, &ABetaGameBall::BoostForward);
	PlayerInputComponent->BindAction("BoostLeft", IE_Pressed, this, &ABetaGameBall::BoostLeft);
	PlayerInputComponent->BindAction("BoostBackwards", IE_Pressed, this, &ABetaGameBall::BoostBackwards);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ABetaGameBall::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ABetaGameBall::TouchStopped);
}
void ABetaGameBall::OnRotationInput(FVector Input)
{

    static float DEG_TO_RAD = PI / (180.f);

    //TODO: get right and down scale factor from player controller to make the game playable for people who don't want to do full body motions, like while sitting

    //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Rotation is y_rot: %f x_rot: %f roll_left_rot: %f"), Input.X, Input.Y, Input.Z));


    float x_rad = 1 * Input.X;
    float y_rad = 1 * Input.Y;

    float actual_x= x_rad;
    if (x_rad < 0.1 && x_rad > -0.1 || !bCanJump) {
        actual_x = 0;
    }

    float actual_y = y_rad;
    if (y_rad < 0.1 && y_rad > -0.1 || !bCanJump) {
        actual_y = 0;
    }

    const FVector Torque = FVector(actual_x*RollTorque*4, actual_y*RollTorque*4, 0.f);
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
	if (bCanJump) delta_sum += DeltaTime;
	if (delta_sum > 1)
	{
		if (stamina < 3)
			stamina++;
		delta_sum -= 1;
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("STAMINA : %d"), stamina));
	}
	//UE_LOG(LogTemp, Warning, TEXT("DELTA TIME: %f"), DeltaTime);
	Super::Tick(DeltaTime); // Call parent class tick function
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
