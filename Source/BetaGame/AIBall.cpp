// Fill out your copyright notice in the Description page of Project Settings.

#include "AIBall.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>

// Sets default values for this component's properties
AAIBall::AAIBall()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//PrimaryComponentTick.bCanEverTick = true;

	// ...
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BallMesh(TEXT("/Game/Rolling/Meshes/BallMesh.BallMesh"));
	Ball = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AIBall0"));
	Ball->SetStaticMesh(BallMesh.Object);
	// Create mesh component for the ball
	InitializeBall();
}

void AAIBall::InitializeBall() {
	
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
	isStart = true;

	centerX = Ball->GetComponentLocation().X;
	centerY = Ball->GetComponentLocation().Y;

	PlayerBall = Cast<ABetaGameBall>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	
}


// Called every frame
void AAIBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (isStart==false)return;


	int isPlayerInRange = detectPlayer(DeltaTime);
	if (notMove == false)counter += 1;
	counter %= 360;
	
	
	//else moveTowardsPlayer(DeltaTime);
	//UE_LOG(LogTemp, Warning, TEXT("X: %f, Y: %f"), PlayerBall->GetLoc().X,PlayerBall->GetLoc().Y);

	int range = detectArea(DeltaTime, centerX, centerY);

	if(range==false)autoMovement(DeltaTime);
	else moveTowardsPlayer(DeltaTime);
	
	//Ball->AddRelativeLocation(position);
	//Ball->SetRelativeLocation(position);


	//defense strategy
	//UE_LOG(LogTemp, Warning, TEXT("X: %f, Y: %f, Z: %f"), PlayerBall->GetLoc().X,PlayerBall->GetLoc().Y, PlayerBall->GetLoc().Z);
	
	FVector InputVectorNormal = GetLastMovementInputVector().GetClampedToMaxSize(1.0f);
	FVector Velocity = Ball->GetPhysicsLinearVelocity();
	Velocity += InputVectorNormal * DeltaTime;// *600.0f;

	Ball->SetPhysicsLinearVelocity(Velocity);
	
}

void AAIBall::autoMovement(float delta) {

	float theta = counter;

	FVector position = FVector(radius*cos(theta*(PI / 180)) + centerX - Ball->GetComponentLocation().X, radius*sin(theta*(PI / 180)) + centerY - Ball->GetComponentLocation().Y, 0);

	Ball->SetPhysicsLinearVelocity(position*delta, true);
	

}

bool AAIBall::detectPlayer(float delta) {

	float theta = counter;

	bool isInRange = false;
	float diff = FVector::Dist(Ball->GetComponentLocation(), PlayerBall->GetLoc());

	

	if (diff <= radius)isInRange = true;
	if (isInRange)notMove = true;
	else notMove = false;

	return isInRange;
}

bool AAIBall::detectArea(float delta, float x, float y) {

	float theta = counter;

	bool isInRange = false;

	FVector areaVector = FVector(x,y,0);

	float diff = FVector::Dist(areaVector, PlayerBall->GetLoc());

	//UE_LOG(LogTemp, Warning, TEXT("diff: %f"), diff);

	if (diff <= radius)isInRange = true;

	if (isInRange)follow = true;
	else follow = false;

	return isInRange;
}


void AAIBall::moveTowardsPlayer(float delta) {

	float theta = counter;

	FVector towardsVector = PlayerBall->GetLoc() - Ball->GetComponentLocation();

	FVector movePos = towardsVector.GetSafeNormal()*factor;

	Ball->SetPhysicsLinearVelocity(movePos*delta, true);
	
}

FVector AAIBall::normalizeVec(FVector v){
	float len = sqrt(v.X*v.X+v.Y*v.Y+v.Z*v.Z);
	return v / len;
}

float AAIBall::calcDistance(FVector v1, FVector v2) {
	return sqrt((v1.X-v2.X)*(v1.X - v2.X)+ (v1.Y - v2.Y)*(v1.Y - v2.Y)+ (v1.Z - v2.Z)*(v1.Z - v2.Z));
}

void AAIBall::BoostForward() 
{

	const FVector Impulse = FVector(0, 0.f, 0.f);
	Ball->AddImpulse(Impulse);
}

void AAIBall::BoostBackwards() 
{

	const FVector Impulse = FVector(-0, 0.f, 0.f);
	Ball->AddImpulse(Impulse);
}


