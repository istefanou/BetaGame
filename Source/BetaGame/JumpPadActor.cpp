// Fill out your copyright notice in the Description page of Project Settings.

#include "JumpPadActor.h"
#include "PadActor.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
//#include "Engine/CollisionProfile.h"
//#include "Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"

void AJumpPadActor::InitiatePadAction(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Boop Jump")));
	OtherComp->AddImpulse(FVector(0.0f, 0.0f, jump_impulse));
}

void AJumpPadActor::FinishPadAction(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{

}

AJumpPadActor::AJumpPadActor()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PadMesh(TEXT("/Game/MobileStarterContent/Shapes/Shape_Torus.Shape_Torus"));

	PadInitialize();

	FString JumpPadString = TEXT("JumpPad_" + FString::FromInt(ObjectID));
	FName JumpPadName = FName(*JumpPadString);
	Pad = CreateDefaultSubobject<UStaticMeshComponent>(JumpPadName);
	Pad->SetStaticMesh(PadMesh.Object);

	// Create the root CapsuleComponent to handle the pickup's collision
	FString BaseCollisionComponentString = TEXT("BaseCollisionComponent_JumpPad_" + FString::FromInt(ObjectID));
	FName BaseCollisionComponentName = FName(*BaseCollisionComponentString);
	BaseCollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(BaseCollisionComponentName);

	// Set the SphereComponent as the root component.
	RootComponent = BaseCollisionComponent;

	// Attach the StaticMeshComponent to the RootComponent.
	Pad->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);


	BaseCollisionComponent->bGenerateOverlapEvents = true;
	BaseCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AJumpPadActor::InitiatePadAction);
	BaseCollisionComponent->OnComponentEndOverlap.AddDynamic(this, &AJumpPadActor::FinishPadAction);

	Pad->SetCollisionProfileName(TEXT("OverlapAll"));
	BaseCollisionComponent->SetCollisionProfileName(TEXT("OverlapAll"));

	// Disable Overlap Events on the Mesh
	Pad->bGenerateOverlapEvents = true;

	//Pad->SetWorldScale3D(FVector(2));
	//BaseCollisionComponent->SetWorldScale3D(FVector(4));
	bGenerateOverlapEventsDuringLevelStreaming = true;

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AJumpPadActor::BeginPlay()
{
	Super::BeginPlay();
}