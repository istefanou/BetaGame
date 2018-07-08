// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "BetaGameBall.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"
#include "Runtime/Engine/Classes/Engine/EngineTypes.h"
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
	isNotInTheAir = true; // Start being able to jump

	max_stamina = 3;
	current_stamina = 3;
}

void ABetaGameBall::SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent)
{
	// set up gameplay key bindings

	// Axis stuff are for continuous values

	if (GetWorld())
	{
		//if we are running in editor run in desktop mode else run in phone mode

		if (GetWorld()->WorldType == EWorldType::PIE || GetWorld()->WorldType == EWorldType::Editor) {
			phone = false;
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Gamemode is pc")));
		}
		else {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Gamemode is phone")));
			phone = true;
		}
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Fatal error world not found")));
	}

	if (phone) {
		PlayerInputComponent->BindVectorAxis("Tilt", this, &ABetaGameBall::OnTiltInput);
		PlayerInputComponent->BindGesture("RotateCamera", this, &ABetaGameBall::OnRotationInput);
		//PlayerInputComponent->BindGesture("SwipeBoost", this, &ABetaGameBall::Boost);
	}
	PlayerInputComponent->BindAxis("MoveRight", this, &ABetaGameBall::OnSimulatedTiltInputx);
	PlayerInputComponent->BindAxis("MoveForward", this, &ABetaGameBall::OnSimulatedTiltInputy);
	PlayerInputComponent->BindAxis("RotateCameraRight", this, &ABetaGameBall::OnSimulatedRotationInput);
	// ACtion stuff is for single events

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

	if (GetWorld())
	{
		camera_manager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Fatal error camera manager not instansiated")));
	}

	if (phone) {

		torque_multiplier = torque_multiplier / 2.0f;
		this->yrot_offset = -0.85f;
	}
	else {

		this->yrot_offset = 0;
	}
}

double simulated_rotation_x = 0;

double simulated_x = 0;

void ABetaGameBall::OnSimulatedTiltInputx(float xvalue) {

	if (!isNotInTheAir) {
		return;
	}

	simulated_x += xvalue;
	x_rotation = simulated_x;
}

double simulated_y = 0;

void ABetaGameBall::OnSimulatedTiltInputy(float yvalue) {

	if (!isNotInTheAir) {
		return;
	}

	simulated_y += yvalue;

	y_rotation = simulated_y;
}




void ABetaGameBall::OnTiltInput(FVector Input)
{

	if (!isNotInTheAir) {
		return;
	}

	if (ABetaGameBall::phone_debug_messages && (counter) % 100 == 0) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Tilt x: %f y: %f z: %f"), Input.X, Input.Y, Input.Z));
	}

	x_rotation = Input.X;
	y_rotation = Input.Y;


}

void ABetaGameBall::OnRotationInput(float Input)
{
	FRotator armrotation = SpringArm->RelativeRotation;
	SpringArm->RelativeRotation = FRotator(armrotation.Pitch, armrotation.Yaw + Input, armrotation.Roll);
}

void ABetaGameBall::OnSimulatedRotationInput(float value) {
	OnRotationInput(value);
}


void ABetaGameBall::Boost(FVector direction)
{
	if (current_stamina > 0)
	{


		Ball->SetPhysicsLinearVelocity(FVector(0.f, 0.f, 0.f));
		FVector dash_vector = FVector(DashImpulse*direction.X, DashImpulse*direction.Y, DashImpulse*direction.Z);
		Ball->AddImpulse(toDirectionalVector(dash_vector));
		current_stamina--;
		UE_LOG(LogTemp, Warning, TEXT("STAMINA : %d"), current_stamina);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("NO STAMINA"));
}

void ABetaGameBall::BoostRight() //
{
	Boost(FVector(0.f, 1.0f, 0.f));
}

void ABetaGameBall::BoostLeft() //
{

	Boost(FVector(0.f, -1.0f, 0.f));
}

void ABetaGameBall::BoostForward() //
{

	Boost(FVector(1.f, 0.0f, 0.f));
}

void ABetaGameBall::BoostBackwards() //
{

	Boost(FVector(-1.f, 0.0f, 0.f));
}

float delta_sum = 0;

void ABetaGameBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	simulated_x -= simulated_x / 10.0;
	simulated_y -= simulated_y / 10.0;

	if (isNotInTheAir) delta_sum += DeltaTime;
	if (delta_sum > 1 && isNotInTheAir)
	{
		if (current_stamina < max_stamina)
			current_stamina++;
		delta_sum -= 1;
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("STAMINA : %d"), stamina));
	}

	FVector Torque = FVector(0, 0, 0);

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

	// get the speed we have
	const FVector current_angular_velocity = Ball->GetPhysicsAngularVelocity();

	// and the speed we want to reach modified to be logarithmic on the phone
	if (actual_x >= 0) {
		actual_x = FMath::Min(2000.0f*max_speed_multiplier, (float)pow(actual_x * 20, x_movement_multiplier));
	}
	else {
		actual_x = -FMath::Min(2000.0f*max_speed_multiplier, (float)pow(-actual_x * 20, x_movement_multiplier));
	}

	if (actual_y >= 0) {
		actual_y = FMath::Min(2000.0f*max_speed_multiplier, (float)pow(actual_y * 20, y_movement_multiplier));
	}
	else {
		actual_y = -FMath::Min(2000.0f*max_speed_multiplier, (float)pow(-actual_y * 20, y_movement_multiplier));
	}

	const FVector target_angular_velocity = toDirectionalVector(FVector(actual_x , actual_y, 0.f));


	// find the difference between current and target so that we can apply it trying to reach the target speed
	float x_angular_difference = (target_angular_velocity.X - current_angular_velocity.X);
	float y_angular_difference = (target_angular_velocity.Y - current_angular_velocity.Y);

	Torque = FVector(
		x_angular_difference * torque_multiplier * DeltaTime,
		y_angular_difference * torque_multiplier * DeltaTime,
		0.f);

	if (false && ABetaGameBall::phone_debug_messages && (counter) % 10 == 0) {
		float yaw = SpringArm->RelativeRotation.Yaw;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("yaw: %f degrees"), yaw));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("target_angular_velocity y: %f target_angular_velocity y: %f "), target_angular_velocity.X, target_angular_velocity.Y));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("current_angular_velocity x: %f current_angular_velocity y: %f "), current_angular_velocity.X, current_angular_velocity.Y));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("x_angular_difference: %f y_angular_difference: %f "), x_angular_difference, y_angular_difference));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("cos of yaw : %f sin of yaw: %f "), cos(yaw*DEG_TO_RAD), sin(yaw*DEG_TO_RAD)));
	}

	x_rotation = 0;
	y_rotation = 0;

	if (isNotInTheAir) {
		Ball->AddTorque(Torque);
	}

}

//transforms anything calculated for a 0 degree camera angle to whatever the camera angle is
FVector ABetaGameBall::toDirectionalVector(const FVector vector) {
	float yaw = SpringArm->RelativeRotation.Yaw;
	return FVector(
		vector.X * cos(yaw*DEG_TO_RAD) - vector.Y * sin(yaw*DEG_TO_RAD),
		vector.X * sin(yaw*DEG_TO_RAD) + vector.Y * cos(yaw*DEG_TO_RAD),
		vector.Z);
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
		isNotInTheAir = false;
		UE_LOG(LogTemp, Warning, TEXT("STAMINA : %d"), current_stamina);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("NO STAMINA"));
}

void ABetaGameBall::NotifyHit(class UPrimitiveComponent *MyComp, class AActor *Other, class UPrimitiveComponent *OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult &Hit)
{
	isNotInTheAir = true;
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
}

void ABetaGameBall::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (isNotInTheAir)
	{

		const FVector Impulse = FVector(0.f, 0.f, JumpImpulse);
		Ball->AddImpulse(Impulse);
		isNotInTheAir = false;
		current_stamina--;
		UE_LOG(LogTemp, Warning, TEXT("STAMINA : %d"), current_stamina);
	}
}

FVector ABetaGameBall::GetLoc()
{
	return Ball->GetComponentLocation();
}
