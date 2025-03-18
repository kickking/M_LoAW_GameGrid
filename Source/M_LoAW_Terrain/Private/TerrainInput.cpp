// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainInput.h"
#include "Kismet/GameplayStatics.h"
#include "TerrainGenerator.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "ProceduralMeshComponent.h"

DEFINE_LOG_CATEGORY(TerrainInput);

// Sets default values
ATerrainInput::ATerrainInput()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	AutoReceiveInput = EAutoReceiveInput::Player0;
	BindDelegate();
}

void ATerrainInput::BindDelegate()
{
	UpdateMousePosDelegate.BindUFunction(Cast<UObject>(this), TEXT("UpdateMousePosition"));
}

bool ATerrainInput::GetTerrainGenerator()
{
	UWorld* World = GetWorld();
	TArray<AActor*> Actors;
	if (World) {
		UGameplayStatics::GetAllActorsOfClass(World, ATerrainGenerator::StaticClass(), Actors);
		if (Actors.Num() == 1) {
			pTG = Cast<ATerrainGenerator>(Actors[0]);
			return true;
		}
	}
	UE_LOG(TerrainInput, Warning, TEXT("GetTerrainGenerator Error!"));
	return false;
}

bool ATerrainInput::EnablePlayer()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController) {
		EnableInput(PlayerController);
		Controller = PlayerController;
		return true;
	}
	UE_LOG(TerrainInput, Warning, TEXT("EnablePlayer Error!"));
	return false;
}

bool ATerrainInput::AddInputMappingContext()
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
					UE_LOG(TerrainInput, Warning, TEXT("No setting Input Mapping Context."));
				}
			}
		}
	}
	UE_LOG(TerrainInput, Warning, TEXT("AddInputMappingContext error."));
	return false;
}

bool ATerrainInput::BindEnchancedInputAction()
{
	if (InputComponent) {
		if (MouseLeftHoldAction != nullptr && MouseRightHoldAction != nullptr) {
			UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(InputComponent);
			EnhancedInputComp->BindAction(MouseLeftHoldAction, ETriggerEvent::Started, this,
				&ATerrainInput::OnLeftHoldStarted);
			EnhancedInputComp->BindAction(MouseLeftHoldAction, ETriggerEvent::Completed, this,
				&ATerrainInput::OnLeftHoldCompleted);
			EnhancedInputComp->BindAction(MouseRightHoldAction, ETriggerEvent::Started, this,
				&ATerrainInput::OnRightHoldStarted);
			EnhancedInputComp->BindAction(MouseRightHoldAction, ETriggerEvent::Completed, this,
				&ATerrainInput::OnRightHoldCompleted);
			return true;
		}
		else {
			UE_LOG(TerrainInput, Warning, TEXT("No setting Input Action."));
		}
	}
	UE_LOG(TerrainInput, Warning, TEXT("BindEnchancedInputAction error."));
	return false;
}

bool ATerrainInput::IsMouseClick()
{
	FVector location, direction;
	Controller->DeprojectMousePositionToWorld(location, direction);

	FHitResult result;
	FCollisionQueryParams params;
	bool isHit = pTG->GetTerrainMesh()->LineTraceComponent(result, location, HoldTraceLength * direction + location,
		params);
	return isHit;
}

void ATerrainInput::OnLeftHoldStarted(const FInputActionValue& Value)
{
	if (IsMouseClick()) {
		LeftHold = true;
	}
}

void ATerrainInput::OnLeftHoldCompleted(const FInputActionValue& Value)
{
	if (IsMouseClick()) {
		LeftHold = false;
	}
}

void ATerrainInput::OnRightHoldStarted(const FInputActionValue& Value)
{
	if (IsMouseClick()) {
		RightHold = true;
	}
}

void ATerrainInput::OnRightHoldCompleted(const FInputActionValue& Value)
{
	if (IsMouseClick()) {
		RightHold = false;
	}
}

void ATerrainInput::StartUpdateMousePos()
{
	GetWorldTimerManager().SetTimer(UpdateMousePosTimerHandle, 
		UpdateMousePosDelegate, UpdateMousePosTimerRate, true);
	UE_LOG(TerrainInput, Log, TEXT("Start updating mouse Position!"));
}

void ATerrainInput::StopUpdateMousePos()
{
	GetWorldTimerManager().ClearTimer(UpdateMousePosTimerHandle);
	UE_LOG(TerrainInput, Log, TEXT("Stop updating mouse Position!"));
}

void ATerrainInput::UpdateMousePosition()
{
	if (pTG->IsLoadingCompleted()) {
		FVector location, direction;
		Controller->DeprojectMousePositionToWorld(location, direction);

		FHitResult result;
		FCollisionQueryParams params;
		bool isHit = pTG->GetTerrainMesh()->LineTraceComponent(result, location, HoldTraceLength * direction + location,
			params);
		if (isHit) {
			MousePos.Set(result.Location.X, result.Location.Y, result.Location.Z);
		}
	}
}

// Called when the game starts or when spawned
void ATerrainInput::BeginPlay()
{
	Super::BeginPlay();
	if (!GetTerrainGenerator() || !EnablePlayer() || !AddInputMappingContext() || 
		!BindEnchancedInputAction()) {
		UE_LOG(TerrainInput, Warning, TEXT("ATerrainInput::BeginPlay() Error!"));
	}

	StartUpdateMousePos();
}

// Called every frame
void ATerrainInput::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

