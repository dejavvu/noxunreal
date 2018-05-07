// Fill out your copyright notice in the Description page of Project Settings.

#include "NoxRegionChunk.h"
#include "../../ThirdParty/libnox/Includes/libnox.h"
#include "../../ThirdParty/libnox/Includes/noxconsts.h"

// Sets default values
ANoxRegionChunk::ANoxRegionChunk()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RegionLayers.SetNumUninitialized(nf::CHUNK_SIZE);
}

void ANoxRegionChunk::PostLoad() {
	Super::PostLoad();	
}

void ANoxRegionChunk::ChunkBuilder() {
	// Build the layer actors
	for (int i = 0; i < nf::CHUNK_SIZE; ++i) {
		// Create the layer
		FVector loc = FVector(0, 0, 0);
		FTransform trans = FTransform(loc);
		RegionLayers[i] = GetWorld()->SpawnActorDeferred<ANoxRegionLayer>(ANoxRegionLayer::StaticClass(), trans);
		RegionLayers[i]->chunk_idx = chunk_idx;
		RegionLayers[i]->base_x = base_x;
		RegionLayers[i]->base_y = base_y;
		RegionLayers[i]->base_z = base_z;
		RegionLayers[i]->local_z = i;
		RegionLayers[i]->FinishSpawning(trans);
	}
	StaticModels();
}

void ANoxRegionChunk::Rebuild() {
	for (int i = 0; i < nf::CHUNK_SIZE; ++i) {
		RegionLayers[i]->Rebuild();
	}
}

// Called when the game starts or when spawned
void ANoxRegionChunk::BeginPlay()
{
	Super::BeginPlay();
	ChunkBuilder();
	StaticModels();
}

void ANoxRegionChunk::StaticModels() 
{
	size_t size;
	nf::static_model_t * model_ptr;
	nf::chunk_models(chunk_idx, size, model_ptr);

	if (size > 0) {
		// We need to build some meshes! This needs to be made more efficent, we shouldn't delete everything...
		Models.Empty();
		for (size_t i = 0; i < size; ++i) {
			nf::static_model_t model = model_ptr[i];

			const float mx = model.x + 0.5f;
			const float my = model.y + 0.5f;
			const float mz = model.z;

			FVector loc = FVector(mx * WORLD_SCALE, my * WORLD_SCALE, mz * WORLD_SCALE);
			FTransform trans = FTransform(loc);
			auto newModel = GetWorld()->SpawnActorDeferred<ANoxStaticModel>(ANoxStaticModel::StaticClass(), trans);
			newModel->modelId = model.idx;
			newModel->x = model.x;
			newModel->y = model.y;
			newModel->z = model.z;
			newModel->FinishSpawning(trans);
			Models.Emplace(newModel);
		}
	}
}

// Called every frame
void ANoxRegionChunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

