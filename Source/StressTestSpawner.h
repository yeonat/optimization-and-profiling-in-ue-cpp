// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "StressTestSpawner.generated.h"

UCLASS()
class BOTTLENECK_API AStressTestSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStressTestSpawner();

	UPROPERTY(EditAnywhere, Category = "Optimization Test")
	TSoftObjectPtr<UStaticMesh> MeshToSpawn;

	UPROPERTY(EditAnywhere, Category = "Optimization Test")
	int32 TotalInstances = 3000;

	UFUNCTION(BlueprintCallable, Category = "Optimization Test")
	void SetupOptimizedBatch();

	void UpdateBatchTransforms();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UHierarchicalInstancedStaticMeshComponent* HISMComponent;

	FTimerHandle BatchUpdateTimerHandle;
	float AccumulatedTime = 0.0f;

	// Caché de posiciones base para animar sin reconstruir memoria
	TArray<FTransform> BaseTransforms;
};
