// Fill out your copyright notice in the Description page of Project Settings.


#include "GameGridInput.h"
#include "M_LoAW_Terrain/Public/TerrainInput.h"
#include "Kismet/GameplayStatics.h"
#include "GameGridGenerator.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "M_LoAW_GridData/Public/Hex.h"
#include "M_LoAW_GridData/Public/GridDataGameInstance.h"

DEFINE_LOG_CATEGORY(GameGridInput);

// Sets default values
AGameGridInput::AGameGridInput()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	AutoReceiveInput = EAutoReceiveInput::Player0;
	BindDelegate();

}

// Called when the game starts or when spawned
void AGameGridInput::BeginPlay()
{
	if (!GetGameInstance() || !GetGameGridGenerator() || !GetTerrainInput() || 
		!EnablePlayer() || !AddInputMappingContext() ||
		!BindEnchancedInputAction()) {
		UE_LOG(GameGridInput, Warning, TEXT("AGameGridInput::BeginPlay() Error!"));
		return;
	}
	Super::BeginPlay();
}

void AGameGridInput::BindDelegate()
{
	CheckMouseOverDelegate.BindUFunction(Cast<UObject>(this), TEXT("CheckMouseOver"));
}

bool AGameGridInput::GetGameInstance()
{
	UWorld* world = GetWorld();
	if (world) {
		pGI = Cast<UGridDataGameInstance>(world->GetGameInstance());
		if (pGI && pGI->hasGameGridLoaded) {
			return true;
		}
	}
	UE_LOG(GameGridInput, Warning, TEXT("GetGameInstance Error!"));
	return false;
}

bool AGameGridInput::GetGameGridGenerator()
{
	UWorld* World = GetWorld();
	TArray<AActor*> Actors;
	if (World) {
		UGameplayStatics::GetAllActorsOfClass(World, AGameGridGenerator::StaticClass(), Actors);
		if (Actors.Num() == 1) {
			pGG = Cast<AGameGridGenerator>(Actors[0]);
			return true;
		}
	}
	UE_LOG(GameGridInput, Warning, TEXT("GetGameGridGenerator Error!"));
	return false;
}

bool AGameGridInput::GetTerrainInput()
{
	UWorld* World = GetWorld();
	TArray<AActor*> Actors;
	if (World) {
		UGameplayStatics::GetAllActorsOfClass(World, ATerrainInput::StaticClass(), Actors);
		if (Actors.Num() == 1) {
			pTI = Cast<ATerrainInput>(Actors[0]);
			return true;
		}
	}
	UE_LOG(GameGridInput, Warning, TEXT("GetTerrainInput Error!"));
	return false;
}

bool AGameGridInput::EnablePlayer()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController) {
		EnableInput(PlayerController);
		Controller = PlayerController;
		return true;
	}
	UE_LOG(GameGridInput, Warning, TEXT("EnablePlayer Error!"));
	return false;
}

bool AGameGridInput::AddInputMappingContext()
{
	if (Controller) {
		if (ULocalPlayer* LocalPlayer = Controller->GetLocalPlayer()) {
			if (UEnhancedInputLocalPlayerSubsystem* InputSystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()) {
				if (InputMapping != nullptr) {
					InputSystem->AddMappingContext(InputMapping, 0);
					return true;
				}
				else {
					UE_LOG(GameGridInput, Warning, TEXT("No setting Input Mapping Context."));
				}
			}
		}
	}
	UE_LOG(GameGridInput, Warning, TEXT("AddInputMappingContext error."));
	return false;
}

bool AGameGridInput::BindEnchancedInputAction()
{
	if (InputComponent) {
		if (IncMouseOverRadiusAction != nullptr
			&& DecMouseOverRadiusAction != nullptr) {
			UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(InputComponent);
			EnhancedInputComp->BindAction(IncMouseOverRadiusAction, ETriggerEvent::Triggered, this,
				&AGameGridInput::OnIncMouseOverRadius);
			EnhancedInputComp->BindAction(DecMouseOverRadiusAction, ETriggerEvent::Triggered, this,
				&AGameGridInput::OnDecMouseOverRadius);
			return true;
		}
		else {
			UE_LOG(GameGridInput, Warning, TEXT("No setting Input Action."));
		}
	}
	UE_LOG(GameGridInput, Warning, TEXT("BindEnchancedInputAction error."));
	return false;
}

void AGameGridInput::OnIncMouseOverRadius()
{
	if (pGG->IsLoadingCompleted()) {
		int32 max = pGI->GameGridParam.NeighborRange * (pGG->GetBuildingBlockExTimes() + 1);
		int value = pGG->GetMouseOverShowRadius() + 1;
		value = value < max ? value : max;
		pGG->SetMouseOverShowRadius(value);
	}
}

void AGameGridInput::OnDecMouseOverRadius()
{
	if (pGG->IsLoadingCompleted()) {
		int32 min = 0;
		int value = pGG->GetMouseOverShowRadius() - 1;
		value = value > min ? value : min;
		pGG->SetMouseOverShowRadius(value);
	}
}

void AGameGridInput::CheckMouseOver()
{
	if (pGG->IsLoadingCompleted()) {
		FVector MousePos = pTI->GetMousePosition();
		FVector2D Pos2D(MousePos.X, MousePos.Y);
		MouseOverGrid(Pos2D);
	}
}

void AGameGridInput::MouseOverGrid(const FVector2D& MousePos)
{
	Hex hex = Hex::PosToHex(MousePos, pGI->GameGridParam.TileSize);
	pGG->RemoveMouseOverGrid();
	pGG->AddMouseOverGrid(hex);
}


// Called every frame
void AGameGridInput::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGameGridInput::StartCheckMouseOver()
{
	GetWorldTimerManager().SetTimer(CheckTimerHandle, CheckMouseOverDelegate, CheckTimerRate, true);
	UE_LOG(GameGridInput, Log, TEXT("Start checking mouse over grid!"));
}

void AGameGridInput::StopCheckMouseOver()
{
	GetWorldTimerManager().ClearTimer(CheckTimerHandle);
	UE_LOG(GameGridInput, Log, TEXT("Stop checking mouse over grid!"));
}

bool AGameGridInput::IsUseGrid()
{
	if (pGG) {
		return pGG->IsUseGrid();
	}
	return false;
}