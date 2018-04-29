// Fill out your copyright notice in the Description page of Project Settings.

#include "AIBall.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"

// Sets default values for this component's properties
AAIBall::AAIBall()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//PrimaryComponentTick.bCanEverTick = true;

	// ...
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BallMesh(TEXT("/Game/Rolling/Meshes/BallMesh.BallMesh"));

	// Create mesh component for the ball
	Ball = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AIBall0"));
	Ball->SetStaticMesh(BallMesh.Object);
	Ball->BodyInstance.SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	Ball->SetSimulatePhysics(true);
	Ball->SetAngularDamping(0.1f);
	Ball->SetLinearDamping(0.1f);
	Ball->BodyInstance.MassScale = 3.5f;
	Ball->BodyInstance.MaxAngularVelocity = 800.0f;
	Ball->SetNotifyRigidBodyCollision(true);
	RootComponent = Ball;
}


// Called when the game starts
void AAIBall::BeginPlay()
{
	Super::BeginPlay();

	// ...
	startX = Ball->GetComponentLocation().X;
	startY = Ball->GetComponentLocation().Y;
	isStart = true;
}


// Called every frame
void AAIBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (isStart==false)return;
	// ...

	counter+=10;
	counter %= 360;
	float i = counter;
	float radius = 500;
	float theta = i;
	FVector position = FVector(radius*cos(theta*(PI/180))+startX, radius*sin(theta*(PI / 180))+startY, Ball->GetComponentLocation().Z);
	//Ball->AddRelativeLocation(position);
	Ball->SetRelativeLocation(position);
	
	
}

void AAIBall::BoostForward() //
{

	const FVector Impulse = FVector(0, 0.f, 0.f);
	Ball->AddImpulse(Impulse);
}

void AAIBall::BoostBackwards() //
{

	const FVector Impulse = FVector(-0, 0.f, 0.f);
	Ball->AddImpulse(Impulse);
}


