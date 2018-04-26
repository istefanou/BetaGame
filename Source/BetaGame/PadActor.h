// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Components/CapsuleComponent.h"
#include "PadActor.generated.h"



UCLASS()
class BETAGAME_API APadActor : public AActor
{
	GENERATED_BODY()

	
public:	
	// Sets default values for this actor's properties
	APadActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	/** StaticMesh used for the pad */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pad, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Pad;

	//UPROPERTY()
	static TArray<uint32> IdList;
	
	UPROPERTY()
		uint32 ObjectID = 0;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Pad)
	UCapsuleComponent* BaseCollisionComponent;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//UFUNCTION()
	//void InitiatePadAction(AActor* OverlappedActor, AActor* OtherActor);

	//UFUNCTION()
	//void EndPadAction(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	virtual void InitiatePadAction(UPrimitiveComponent *OverlappedComponent,
		AActor *OtherActor,
		UPrimitiveComponent *OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult &SweepResult);

	UFUNCTION()
	virtual void FinishPadAction(UPrimitiveComponent *OverlappedComponent,  AActor * OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void PadAction1(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);


	UFUNCTION()
	void PadInitialize();

	void CreateId();
	
};
