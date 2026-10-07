// Fill out your copyright notice in the Description page of Project Settings.

#include "StressTestSpawner.h"
#include "Stats/Stats.h"
#include "TimerManager.h"

// Sets default values
AStressTestSpawner::AStressTestSpawner()
{
	// Regla 1: Desactivar Tick para evitar sobrecoste en el Game Thread
	PrimaryActorTick.bCanEverTick = false;

	HISMComponent = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("HISMComponent"));
	RootComponent = HISMComponent;
}

// Called when the game starts or when spawned
void AStressTestSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (MeshToSpawn.IsValid())
	{
		HISMComponent->SetStaticMesh(MeshToSpawn.Get());
	}
	else if (!MeshToSpawn.IsNull())
	{
		HISMComponent->SetStaticMesh(MeshToSpawn.LoadSynchronous());
	}

	SetupOptimizedBatch();
}

void AStressTestSpawner::SetupOptimizedBatch()
{
	// Marcador visible en Unreal Insights (Pestaña CPU)
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SetupOptimizedBatch);

	HISMComponent->ClearInstances();
	BaseTransforms.Empty();

	const float Spacing = 200.0f;
	const int32 GridSide = FMath::CeilToInt(FMath::Sqrt((float)TotalInstances));

	BaseTransforms.Reserve(TotalInstances);

	for (int32 i = 0; i < TotalInstances; ++i)
	{
		const int32 X = i % GridSide;
		const int32 Y = i / GridSide;

		FVector Location = FVector(X * Spacing, Y * Spacing, 0.0f);
		BaseTransforms.Add(FTransform(FRotator::ZeroRotator, Location, FVector(1.0f)));
	}

	// Añadir instancias en un único bloque; evita reconstrucciones repetidas del RHI
	HISMComponent->AddInstances(BaseTransforms, false);

	// Actualización lógica periódica a 15 Hz en lugar de saturar a 60/120 Hz
	GetWorldTimerManager().SetTimer(
		BatchUpdateTimerHandle,
		this,
		&AStressTestSpawner::UpdateBatchTransforms,
		0.066f,
		true
	);
}

void AStressTestSpawner::UpdateBatchTransforms()
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UpdateBatchTransforms);

	AccumulatedTime += 0.066f;
	const float HeightOffset = FMath::Sin(AccumulatedTime) * 25.0f;

	TArray<FTransform> UpdatedTransforms;
	UpdatedTransforms.Reserve(TotalInstances);

	// Calculamos las nuevas posiciones a partir de la caché base
	for (int32 i = 0; i < TotalInstances; ++i)
	{
		FTransform CurrentTrans = BaseTransforms[i];
		FVector Loc = CurrentTrans.GetLocation();
		Loc.Z += FMath::Sin(AccumulatedTime + (i * 0.05f)) * HeightOffset;
		CurrentTrans.SetLocation(Loc);

		UpdatedTransforms.Add(CurrentTrans);
	}

	// Actualización por lote directa
	// bMarkRenderStateDirty = true despacha los nuevos buffers a la GPU en un solo paso
	HISMComponent->BatchUpdateInstancesTransforms(0, UpdatedTransforms, false, true, true);
}

void AStressTestSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Limpieza segura del temporizador al destruir el actor
	GetWorldTimerManager().ClearTimer(BatchUpdateTimerHandle);
	Super::EndPlay(EndPlayReason);
}