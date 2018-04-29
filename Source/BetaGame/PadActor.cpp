// Fill out your copyright notice in the Description page of Project Settings.

#include "PadActor.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"


TArray<uint32> APadActor::IdList = TArray<uint32>({}, 0);

// Sets default values
APadActor::APadActor()
{

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PadMesh(TEXT("/Game/MobileStarterContent/Shapes/Shape_Torus.Shape_Torus"));

	PadInitialize();

	FString PadString = TEXT("Pad_" + FString::FromInt(ObjectID));
	FName PadName = FName(*PadString);
	Pad = CreateDefaultSubobject<UStaticMeshComponent>(PadName);
	Pad->SetStaticMesh(PadMesh.Object);
	//Pad->BodyInstance.SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	//Pad->SetSimulatePhysics(true);
	//Pad->SetAngularDamping(0.1f);
	//Pad->SetLinearDamping(0.1f);
	//Pad->BodyInstance.MassScale = 3.5f;
	//Pad->BodyInstance.MaxAngularVelocity = 800.0f;
	//Pad->SetNotifyRigidBodyCollision(true);
	//RootComponent = Pad;

	// Create the root CapsuleComponent to handle the pickup's collision
	FString BaseCollisionComponentString = TEXT("BaseCollisionComponent_Pad_" + FString::FromInt(ObjectID));
	FName BaseCollisionComponentName = FName(*BaseCollisionComponentString);
	BaseCollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(BaseCollisionComponentName);
	//BaseCollisionComponent->
	//SetCollisionChannel (ECollisionResponse::ECR_Overlap); //SetCollisionResponseToAllChannels 

	// Set the SphereComponent as the root component.
	RootComponent = BaseCollisionComponent;

	// Attach the StaticMeshComponent to the RootComponent.
	Pad->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

	BaseCollisionComponent->bGenerateOverlapEvents = true;
	BaseCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &APadActor::InitiatePadAction);
	BaseCollisionComponent->OnComponentEndOverlap.AddDynamic(this, &APadActor::FinishPadAction);

	//Pad->SetSimulatePhysics(false);
	//Pad->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	Pad->SetCollisionProfileName(TEXT("OverlapAll"));
	//Pad->SetCollisionProfileName(TEXT("NoCollision"));
	BaseCollisionComponent->SetCollisionProfileName(TEXT("OverlapAll"));

	// Disable Overlap Events on the Mesh
	Pad->bGenerateOverlapEvents = true;

	//Pad->SetWorldScale3D(FVector(2));
	bGenerateOverlapEventsDuringLevelStreaming = true;
	//Pad->bGenerateOverlapEvents = true;


	//Pad->OnComponentBeginOverlap.AddDynamic(this, &APadActor::InitiatePadAction);
	//Pad->OnComponentBeginOverlap.AddDynamic(this, &APadActor::EndPadAction);
	//OnActorHit.AddDynamic(this, &APadActor::BeginPadAction);

	//OnActorBeginOverlap.AddDynamic(this, &APadActor::InitiatePadAction);
	//OnActorEndOverlap.AddDynamic(this, &APadActor::EndPadAction);

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APadActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APadActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APadActor::InitiatePadAction(UPrimitiveComponent *OverlappedComponent,
	AActor *OtherActor,
	UPrimitiveComponent *OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult &SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Boop comp begin")));
}

void APadActor::FinishPadAction(UPrimitiveComponent *OverlappedComponent, AActor * OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Boop comp end")));
}

void APadActor::PadAction1(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Boop 1")));
}

void APadActor::PadInitialize()
{
	
	CreateId();
}

void APadActor::CreateId()
{
	uint32 tempId = 0;
	
	while (IdList.Contains(tempId))
	{
		tempId++;
	}

	ObjectID = tempId;
	IdList.Add(ObjectID);
}