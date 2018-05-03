// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraDirector.h"
#include "../../ThirdParty/libnox/Includes/libnox.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Components/InputComponent.h"

// Sets default values
ACameraDirector::ACameraDirector()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("CursorMesh"));
	RootComponent = mesh;
	// New in UE 4.17, multi-threaded PhysX cooking.
	mesh->bUseAsyncCooking = true;

	LeftClicked = false;
	RightClicked = false;
}

// Called when the game starts or when spawned
void ACameraDirector::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACameraDirector::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &ACameraDirector::ZoomIn);
	PlayerInputComponent->BindAction("ZoomOut", IE_Pressed, this, &ACameraDirector::ZoomOut);
	PlayerInputComponent->BindAction("ZoomIn", IE_Released, this, &ACameraDirector::ZoomOff);
	PlayerInputComponent->BindAction("ZoomOut", IE_Released, this, &ACameraDirector::ZoomOff);

	PlayerInputComponent->BindAction("CameraLeft", IE_Pressed, this, &ACameraDirector::CameraLeft);
	PlayerInputComponent->BindAction("CameraRight", IE_Pressed, this, &ACameraDirector::CameraRight);
	PlayerInputComponent->BindAction("CameraNorth", IE_Pressed, this, &ACameraDirector::CameraNorth);
	PlayerInputComponent->BindAction("CameraSouth", IE_Pressed, this, &ACameraDirector::CameraSouth);
	PlayerInputComponent->BindAction("CameraUp", IE_Pressed, this, &ACameraDirector::CameraUp);
	PlayerInputComponent->BindAction("CameraDown", IE_Pressed, this, &ACameraDirector::CameraDown);

	PlayerInputComponent->BindAction("CameraLeft", IE_Released, this, &ACameraDirector::CameraOff);
	PlayerInputComponent->BindAction("CameraRight", IE_Released, this, &ACameraDirector::CameraOff);
	PlayerInputComponent->BindAction("CameraNorth", IE_Released, this, &ACameraDirector::CameraOff);
	PlayerInputComponent->BindAction("CameraSouth", IE_Released, this, &ACameraDirector::CameraOff);

	PlayerInputComponent->BindAction("CameraMode", IE_Pressed, this, &ACameraDirector::CameraMode);
	PlayerInputComponent->BindAction("CameraPerspective", IE_Pressed, this, &ACameraDirector::CameraPerspective);

	PlayerInputComponent->BindAction("PauseToggler", IE_Pressed, this, &ACameraDirector::PauseToggler);
	PlayerInputComponent->BindAction("PauseOneStep", IE_Pressed, this, &ACameraDirector::PauseOneStep);

	PlayerInputComponent->BindAction("MouseLeftClick", IE_Pressed, this, &ACameraDirector::LeftClickOn);
	PlayerInputComponent->BindAction("MouseLeftClick", IE_Released, this, &ACameraDirector::LeftClickOff);
	PlayerInputComponent->BindAction("MouseRightClick", IE_Pressed, this, &ACameraDirector::RightClickOn);
	PlayerInputComponent->BindAction("MouseRightClick", IE_Released, this, &ACameraDirector::RightClickOff);
}

// Called every frame
void ACameraDirector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Input Support

	if (zooming == 1) {
		nf::camera_zoom_in();
	}
	else if (zooming == 2) {
		nf::camera_zoom_out();
	}

	if (xMove != 0 || yMove != 0 || zMove != 0) {
		nf::camera_move(xMove, yMove, zMove);
		zMove = 0;
	}

	// Render
	float x, y, z, zoom;
	bool perspective;
	int mode;
	nf::get_camera_position(x, y, z, zoom, perspective, mode);

	FVector camera_position;
	FVector camera_target = FVector(x * WORLD_SCALE, y * WORLD_SCALE, z * WORLD_SCALE);

	switch (mode) {
	case 0: camera_position = FVector(x * WORLD_SCALE, (y + (zoom / 3.0f)) * WORLD_SCALE, (z + zoom) * WORLD_SCALE); break;
	case 1: camera_position = FVector(x * WORLD_SCALE, (y + zoom) * WORLD_SCALE, z * WORLD_SCALE); break;
	case 2: camera_position = FVector((x + zoom) * WORLD_SCALE, (y + zoom) * WORLD_SCALE, (z+zoom) * WORLD_SCALE); break;
	case 3: camera_position = FVector((x - zoom) * WORLD_SCALE, (y + zoom) * WORLD_SCALE, (z+zoom) * WORLD_SCALE); break;
	case 4: camera_position = FVector((x + zoom) * WORLD_SCALE, (y + zoom) * WORLD_SCALE, (z-zoom) * WORLD_SCALE); break;
	case 5: camera_position = FVector((x - zoom) * WORLD_SCALE, (y + zoom) * WORLD_SCALE, (z - zoom) * WORLD_SCALE); break;
	}

	FRotator camrot = UKismetMathLibrary::FindLookAtRotation(camera_position, camera_target);

	CameraOne->SetActorLocation(camera_position);
	CameraOne->SetActorRotation(camrot);

	Cursors();
}

void ACameraDirector::ZoomIn() {
	zooming = 1;
}

void ACameraDirector::ZoomOut() {
	zooming = 2;
}

void ACameraDirector::ZoomOff() {
	zooming = 0;
}

void ACameraDirector::CameraLeft() {
	xMove = -1;
}

void ACameraDirector::CameraRight() {
	xMove = 1;
}

void ACameraDirector::CameraNorth() {
	yMove = -1;
}

void ACameraDirector::CameraSouth() {
	yMove = 1;
}

void ACameraDirector::CameraUp() {
	zMove = 1;
}

void ACameraDirector::CameraDown() {
	zMove = -1;
}

void ACameraDirector::CameraOff() {
	xMove = 0;
	yMove = 0;
	zMove = 0;
}

void ACameraDirector::CameraMode() {
	nf::toggle_camera_mode();
}

void ACameraDirector::CameraPerspective() {
	nf::toggle_camera_perspective();
}

void ACameraDirector::PauseToggler() {
	int mode = nf::get_pause_mode();
	if (mode == 0) {
		nf::set_pause_mode(1);
	}
	else {
		nf::set_pause_mode(0);
	}
}

void ACameraDirector::PauseOneStep() {
	nf::set_pause_mode(2);
}

void ACameraDirector::LeftClickOn() {
	LeftClicked = true;
}

void ACameraDirector::LeftClickOff() {
	LeftClicked = false;
}

void ACameraDirector::RightClickOn() {
	RightClicked = true;
}

void ACameraDirector::RightClickOff() {
	RightClicked = false;
}

void ACameraDirector::Cursors() {
	mesh->ClearAllMeshSections();
	geometry_by_material.Empty();

	size_t sz;
	nf::cube_t * cube_ptr;
	nf::cursor_list(sz, cube_ptr);

	if (sz > 0) {
		for (size_t i = 0; i < sz; ++i) {
			nf::cube_t cube = cube_ptr[i];

			geometry_chunk * g = nullptr;
			if (!geometry_by_material.Contains(cube.tex)) {
				geometry_by_material.Add(cube.tex, geometry_chunk());
			}
			g = geometry_by_material.Find(cube.tex);

			//geometry.CreateCube(cube.x * WORLD_SCALE, cube.y * WORLD_SCALE, cube.z * WORLD_SCALE, cube.w * WORLD_SCALE, cube.h * WORLD_SCALE, cube.d * WORLD_SCALE);
			g->CreateCube(cube.x * WORLD_SCALE, cube.y * WORLD_SCALE, cube.z * WORLD_SCALE, cube.w * WORLD_SCALE, cube.h * WORLD_SCALE, cube.d * WORLD_SCALE);
		}

		int sectionCount = 0;
		for (auto &gm : geometry_by_material) {
			FString MaterialAddress;

			switch (gm.Key) {
			case 1: MaterialAddress = "Material'/Game/Cursors/base_cursor_mat.base_cursor_mat'"; break; // Normal
			case 2: MaterialAddress = "Material'/Game/Cursors/tree_cursor_mat.tree_cursor_mat'"; break; // Tree chopping
			case 3: MaterialAddress = "Material'/Game/Cursors/guard_cursor_mat.guard_cursor_mat'"; break; // Guarding
			default: MaterialAddress = "Material'/Game/Cursors/base_cursor_mat.base_cursor_mat'"; break;
			}

			UMaterial* material;
			material = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, *MaterialAddress, nullptr, LOAD_None, nullptr));
			mesh->SetMaterial(sectionCount, material);

			mesh->CreateMeshSection_LinearColor(sectionCount, gm.Value.vertices, gm.Value.Triangles, gm.Value.normals, gm.Value.UV0, gm.Value.vertexColors, gm.Value.tangents, true);

			++sectionCount;
		}
	}
}

