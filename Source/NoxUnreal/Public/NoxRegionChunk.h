// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "NoxRegionLayer.h"
#include "NoxStaticModel.h"
#include "NoxStaticFoliage.h"
#include "Runtime/Engine/Classes/Components/HierarchicalInstancedStaticMeshComponent.h"
#include "../../ThirdParty/libnox/Includes/libnox.h"
#include "../../ThirdParty/libnox/Includes/noxconsts.h"
#include "NoxRegionChunk.generated.h"

UCLASS()
class NOXUNREAL_API ANoxRegionChunk : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANoxRegionChunk();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostLoad() override;

public:	
	const int WORLD_SCALE = 200;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	int base_x;
	int base_y;
	int base_z;
	int chunk_idx;
	TArray<ANoxRegionLayer *> RegionLayers;
	void ChunkBuilder();
	void Rebuild();

	TArray<ANoxStaticModel *> Models;
	void StaticModels();

	UHierarchicalInstancedStaticMeshComponent * grass1 = nullptr;
	UHierarchicalInstancedStaticMeshComponent * grass2 = nullptr;
	UHierarchicalInstancedStaticMeshComponent * flower1 = nullptr;
	UHierarchicalInstancedStaticMeshComponent * bush1 = nullptr;
	UHierarchicalInstancedStaticMeshComponent * bush2 = nullptr;
	void InitializeFoliageContainers();
	void FoliageInit(FString &voxAddress, UHierarchicalInstancedStaticMeshComponent *& target);
	void FoliageClear();
	void FoliageSieve(nf::veg_t &model);
	void StaticFoliage();
};
