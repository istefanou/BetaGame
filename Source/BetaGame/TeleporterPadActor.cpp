// Fill out your copyright notice in the Description page of Project Settings.

#include "TeleporterPadActor.h"
#include "PadActor.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
//#include "Engine/CollisionProfile.h"
//#include "Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"

void ATeleporterPadActor::InitiatePadAction(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Boop Tele")));
	if (DestinationPad != nullptr) {
		OtherComp->SetWorldTransform(DestinationPad->GetTransform());
	}
}

void ATeleporterPadActor::FinishPadAction(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
}

void ATeleporterPadActor::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Boop tele Edit")));
}

ATeleporterPadActor::ATeleporterPadActor()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PadMesh(TEXT("/Game/MobileStarterContent/Shapes/Shape_Torus.Shape_Torus"));

	PadInitialize();

	FString TelePadString = TEXT("TelePad_" + FString::FromInt(ObjectID));
	FName TelePadName = FName(*TelePadString);
	Pad = CreateDefaultSubobject<UStaticMeshComponent>(TelePadName);
	Pad->SetStaticMesh(PadMesh.Object);

	// Create the root CapsuleComponent to handle the pickup's collision
	FString BaseCollisionComponentString = TEXT("BaseCollisionComponent_TelePad_" + FString::FromInt(ObjectID));
	FName BaseCollisionComponentName = FName(*BaseCollisionComponentString);
	BaseCollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(BaseCollisionComponentName);

	// Set the SphereComponent as the root component.
	RootComponent = BaseCollisionComponent;

	// Attach the StaticMeshComponent to the RootComponent.
	Pad->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);


	BaseCollisionComponent->bGenerateOverlapEvents = true;
	BaseCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ATeleporterPadActor::InitiatePadAction);
	BaseCollisionComponent->OnComponentEndOverlap.AddDynamic(this, &ATeleporterPadActor::FinishPadAction);

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

void ATeleporterPadActor::BeginPlay()
{
	Super::BeginPlay();
}
