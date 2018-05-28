// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "BetaGameBall.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformMath.h"
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

	bGenerateOverlapEventsDuringLevelStreaming = true;
	Ball->bGenerateOverlapEvents = true;
	//Ball->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

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
	RollTorque = 250000.0f;
	JumpImpulse = 7000.0f;
	DashImpulse = 10000.0f;
	bCanJump = true; // Start being able to jump

	max_stamina = 3;
	current_stamina = 3;


	for (int i = 0; i < 10; i++) {
		this->accel_diffs[i] = FVector(0.f, 0.f, 0.f);
	}

}

void ABetaGameBall::SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent)
{
	// set up gameplay key bindings

	if (phone) {
		PlayerInputComponent->BindVectorAxis("Tilt", this, &ABetaGameBall::OnRotationInput);
	}
	PlayerInputComponent->BindAxis("MoveRight", this, &ABetaGameBall::OnSimulatedRotationInputx);
	PlayerInputComponent->BindAxis("MoveForward", this, &ABetaGameBall::OnSimulatedRotationInputy);


	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABetaGameBall::Jump);
	PlayerInputComponent->BindAction("BoostRight", IE_Pressed, this, &ABetaGameBall::BoostRight);
	PlayerInputComponent->BindAction("BoostForward", IE_Pressed, this, &ABetaGameBall::BoostForward);
	PlayerInputComponent->BindAction("BoostLeft", IE_Pressed, this, &ABetaGameBall::BoostLeft);
	PlayerInputComponent->BindAction("BoostBackwards", IE_Pressed, this, &ABetaGameBall::BoostBackwards);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ABetaGameBall::TouchStarted);
	//PlayerInputComponent->BindTouch(IE_Released, this, &ABetaGameBall::TouchStopped);
}



void ABetaGameBall::BeginPlay() {
	Super::BeginPlay();
	playercontroller = UGameplayStatics::GetPlayerController(this->GetWorld(), 0);
	ABetaGameBall::phone_debug_messages = true;


	if (phone) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Gamemode is phone")));

		torque_multiplier = torque_multiplier / 2.0f;

		this->yrot_offset = -0.85f;
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Gamemode is pc")));

		this->yrot_offset = 0;
	}
}

void ABetaGameBall::OnRotationInputx(float xvalue) {

	if (!bCanJump) {
		return;
	}

	x_rotation = xvalue;
}

void ABetaGameBall::OnRotationInputy(float yvalue) {

	if (!bCanJump) {
		return;
	}


	y_rotation = yvalue;
}

double simulated_rotation_x = 0;

double simulated_x = 0;

void ABetaGameBall::OnSimulatedRotationInputx(float xvalue) {

	if (!bCanJump) {
		return;
	}

	simulated_x += xvalue;
	x_rotation = simulated_x;
}

double simulated_y = 0;

void ABetaGameBall::OnSimulatedRotationInputy(float yvalue) {

	if (!bCanJump) {
		return;
	}

	simulated_y += yvalue;

	y_rotation = simulated_y;
}


void ABetaGameBall::OnRotationInput(FVector Input)
{

	if (!bCanJump) {
		return;
	}

	
	if (ABetaGameBall::phone_debug_messages && (counter) % 100 == 0) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Tilt x: %f y: %f z: %f"), Input.X, Input.Y, Input.Z));
	}

	x_rotation = Input.X;
	y_rotation = Input.Y;


}

void ABetaGameBall::MoveRight(float Val)
{
	if (bCanJump) {
	const FVector Torque = FVector(-1.f * Val * RollTorque, 0.f, 0.f);
	Ball->AddTorqueInRadians(Torque);
}
}

void ABetaGameBall::MoveForward(float Val)
{
	if (bCanJump) {
		const FVector Torque = FVector(0.f, Val * RollTorque, 0.f);
		Ball->AddTorqueInRadians(Torque);
	}
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


	simulated_x -= simulated_x / 10.0;
	simulated_y -= simulated_y / 10.0;

	if (bCanJump) delta_sum += DeltaTime;
	if (delta_sum > 1 && bCanJump)
	{
		if (current_stamina < max_stamina)
			current_stamina++;
		delta_sum -= 1;
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("STAMINA : %d"), stamina));
	}

	FVector Tilt;
	FVector RotationRate;
	FVector Gravity;
	FVector Acceleration;


	FVector Torque = FVector(0, 0, 0);

	static float DEG_TO_RAD = PI / (180.f);

	// fix x axis to zero around a small area
	float actual_x = x_rotation;
	
	if (actual_x < xrot_offset + dead_zone_offset && actual_x > xrot_offset - dead_zone_offset) {
		actual_x = xrot_offset;
	}
	actual_x -= xrot_offset;

	if (flip_x_movement ^ phone) {
		actual_x = -actual_x;
	}

	// fix y axis to zero around a small area and when the viewer tilts the phone a bit
	float actual_y = y_rotation;
	
	if (actual_y < yrot_offset + dead_zone_offset && actual_y > yrot_offset - dead_zone_offset) {
		actual_y = yrot_offset;
	}
	actual_y -= yrot_offset;

	if (flip_y_movement) {
		actual_y = -actual_y;
	}

	int x_sign=0;
	if (actual_x > 0) {
		x_sign = 1;
	}
	else {
		x_sign = -1;
	}

	int y_sign = 0;
	if (actual_y > 0) {
		y_sign = 1;
	}
	else {
		y_sign = -1;
	}

	// get the speed we have and we want to reach
	const FVector current_angular_velocity = Ball->GetPhysicsAngularVelocityInDegrees();
	const FVector target_angular_velocity = FVector(x_sign * pow(x_sign * actual_x, x_movement_multiplier) * max_speed_multiplier, y_sign * pow(y_sign * actual_y, y_movement_multiplier) * max_speed_multiplier, 0.f);


	//the force is the difference of the above two limited to [-10,+10]
	float x_max_accel = target_angular_velocity.X - current_angular_velocity.X;
	x_max_accel = FMath::Min(1000.0f, x_max_accel);
	x_max_accel = FMath::Max(-1000.0f, x_max_accel);

	float y_max_accel = target_angular_velocity.Y - current_angular_velocity.Y;
	y_max_accel = FMath::Min(1000.0f, y_max_accel);
	y_max_accel = FMath::Max(-1000.0f, y_max_accel);

	Torque = FVector(x_max_accel * torque_multiplier * DeltaTime, y_max_accel*torque_multiplier * DeltaTime, 0.f);


	if (ABetaGameBall::phone_debug_messages && (counter) % 10 == 0) {

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("simulated x: %f simulated y: %f "), simulated_x, simulated_y));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("actual x: %f actual y: %f "), actual_x, actual_y));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("current Velocity x: %f y: %f z: %f"), current_angular_velocity.X, current_angular_velocity.Y, current_angular_velocity.Z));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Target Velocity x: %f y: %f z: %f"), target_angular_velocity.X, target_angular_velocity.Y, target_angular_velocity.Z));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("actual xacel: %f actual yacel: %f "), x_max_accel, y_max_accel));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Torque aplied x: %f y: %f z: %f"), Torque.X, Torque.Y, Torque.Z));
	}


	x_rotation = 0;
	y_rotation = 0;

	if (bCanJump) {
		Ball->AddTorqueInRadians(Torque);
	}

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

	if (current_stamina > 0)
	{
		
		FVector current_linear_velocity = Ball->GetPhysicsLinearVelocity();
		current_linear_velocity.Z = 0;
		Ball->SetPhysicsLinearVelocity(current_linear_velocity);

		const FVector Impulse = FVector(0.f, 0.f, JumpImpulse);
		Ball->AddImpulse(Impulse);
		current_stamina--;
		bCanJump = false;
		UE_LOG(LogTemp, Warning, TEXT("STAMINA : %d"), current_stamina);
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
		current_stamina--;
		UE_LOG(LogTemp, Warning, TEXT("STAMINA : %d"), current_stamina);
	}
}

FVector ABetaGameBall::GetLoc()
{
	return Ball->GetComponentLocation();
}
