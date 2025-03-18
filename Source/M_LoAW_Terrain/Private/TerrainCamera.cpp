// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainCamera.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "TerrainInput.h"
#include "TerrainGenerator.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"

DEFINE_LOG_CATEGORY(TerrainCamera);

// Sets default values
ATerrainCamera::ATerrainCamera()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bDoCollisionTest = 0;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom);

	AutoPossessPlayer = EAutoReceiveInput::Player0;
	BindDelegate();
}

void ATerrainCamera::BindDelegate()
{
	TerrainInputDelegate.BindUFunction(Cast<UObject>(this), TEXT("OnGetTerrainInput"));
	TerrainInfoDelegate.BindUFunction(Cast<UObject>(this), TEXT("OnGetTerrainInfo"));
	ScrollScreenDelegate.BindUFunction(Cast<UObject>(this), TEXT("OnScrollScreen"));
}

bool ATerrainCamera::InitCamera()
{
	APlayerController* pc = Cast<APlayerController>(GetController());
	if (pc) {
		CameraBoom->bUsePawnControlRotation = true;
		CameraBoom->TargetArmLength = InitArmLength;
		AddControllerPitchInput(InitPitch);

		pc->PlayerCameraManager->ViewPitchMax = ViewPitchMax;
		pc->PlayerCameraManager->ViewPitchMin = ViewPitchMin;

		return true;
	}
	UE_LOG(TerrainCamera, Warning, TEXT("InitCamera error!"));
	return false;
}

bool ATerrainCamera::BindEnchancedInputAction()
{
	if (InputComponent) {
		if (CameraMoveAction != nullptr && CameraRotateAction != nullptr &&
			CameraZoomInAction != nullptr && CameraZoomOutAction != nullptr) {
			UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(InputComponent);
			EnhancedInputComp->BindAction(CameraMoveAction, ETriggerEvent::Triggered, this,
				&ATerrainCamera::OnCameraMove);
			EnhancedInputComp->BindAction(CameraRotateAction, ETriggerEvent::Triggered, this,
				&ATerrainCamera::OnCameraRotate);
			EnhancedInputComp->BindAction(CameraZoomInAction, ETriggerEvent::Triggered, this,
				&ATerrainCamera::OnCameraZoomIn);
			EnhancedInputComp->BindAction(CameraZoomOutAction, ETriggerEvent::Triggered, this,
				&ATerrainCamera::OnCameraZoomOut);
			return true;
		}
		else {
			UE_LOG(TerrainCamera, Warning, TEXT("No setting Input Action."));
		}
	}
	UE_LOG(TerrainCamera, Warning, TEXT("BindEnchancedInputAction error!"));
	return false;
}

void ATerrainCamera::OnCameraMove(const FInputActionValue& Value)
{
	if (pTI != nullptr) {
		if (pTI->IsLeftHold()) {
			Move(Value.Get<FVector2D>());
		}
	}
}

void ATerrainCamera::OnCameraRotate(const FInputActionValue& Value)
{
	if (pTI != nullptr) {
		if (pTI->IsRightHold()) {
			FVector2D Axis2D = Value.Get<FVector2D>();
			AddControllerYawInput(Axis2D.X * RotateSpeed);
			AddControllerPitchInput(-Axis2D.Y * RotateSpeed);
		}
	}
}

void ATerrainCamera::OnCameraZoomIn(const FInputActionValue& Value)
{
	CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength - ZoomStep,
		ZoomMin, ZoomMax);
}

void ATerrainCamera::OnCameraZoomOut(const FInputActionValue& Value)
{
	CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength + ZoomStep,
		ZoomMin, ZoomMax);
}

void ATerrainCamera::OnGetTerrainInput()
{
	TArray<AActor*> FoundActors;
	FindActors(ATerrainInput::StaticClass(), FoundActors);
	if (FoundActors.Num() == 1) {
		pTI = Cast<ATerrainInput>(FoundActors[0]);
	}
	else {
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, TerrainInputDelegate, TimingForWaitTerrain, false);
	}
}

void ATerrainCamera::OnGetTerrainInfo()
{
	TArray<AActor*> FoundActors;
	FindActors(ATerrainGenerator::StaticClass(), FoundActors);
	if (FoundActors.Num() == 1) {
		pTG = Cast<ATerrainGenerator>(FoundActors[0]);
		if (pTG->IsWorkFlowStepDone(Enum_TerrainGeneratorState::InitWorkflow)) {
			BoundaryMin.Set(-BoundaryScalar * pTG->GetSize(), -BoundaryScalar * pTG->GetSize(), -1);
			BoundaryMax.Set(BoundaryScalar * pTG->GetSize(), BoundaryScalar * pTG->GetSize(), 1);
		}
		else {
			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(TimerHandle, TerrainInfoDelegate, TimingForWaitTerrain, false);
		}
	}
	else {
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, TerrainInfoDelegate, TimingForWaitTerrain, false);
	}
}

void ATerrainCamera::FindActors(TSubclassOf<AActor> ActorClass, TArray<AActor*>& Actors)
{
	UWorld* World = GetWorld();
	if (World) {
		UGameplayStatics::GetAllActorsOfClass(World, ActorClass, Actors);
	}
}

void ATerrainCamera::OnScrollScreen()
{
	Scroll();
}

void ATerrainCamera::Scroll()
{
	ScrollLeft();
	ScrollRight();
	ScrollUp();
	ScrollDown();
}

void ATerrainCamera::ScrollLeft()
{
	if (ULocalPlayer* LocalPlayer = Cast<APlayerController>(GetController())->GetLocalPlayer()) {
		FVector2D MousePos;
		if (LocalPlayer->ViewportClient->GetMousePosition(MousePos)) {
			FVector2D size;
			LocalPlayer->ViewportClient->GetViewportSize(size);
			float value = UKismetMathLibrary::NormalizeToRange(MousePos.X, 0.f, size.X * ScrollLeftLimitation);
			value = UKismetMathLibrary::FClamp(value, 0.f, 1.f);
			value = 1.f - value;
			if (value > 0.f) {
				OffsetByVector(FVector(0.f, size.X / size.Y * -ScrollSpeedRatio, 0.f), ScrollScalar);
			}
		}
	}
}

void ATerrainCamera::ScrollRight()
{
	if (ULocalPlayer* LocalPlayer = Cast<APlayerController>(GetController())->GetLocalPlayer()) {
		FVector2D MousePos;
		if (LocalPlayer->ViewportClient->GetMousePosition(MousePos)) {
			FVector2D size;
			LocalPlayer->ViewportClient->GetViewportSize(size);
			float value = UKismetMathLibrary::NormalizeToRange(MousePos.X, size.X * ScrollRightLimitation, size.X);
			value = UKismetMathLibrary::FClamp(value, 0.f, 1.f);
			if (value > 0.f) {
				OffsetByVector(FVector(0.f, size.X / size.Y * ScrollSpeedRatio, 0.f), ScrollScalar);
			}
		}
	}
}

void ATerrainCamera::ScrollUp()
{
	if (ULocalPlayer* LocalPlayer = Cast<APlayerController>(GetController())->GetLocalPlayer()) {
		FVector2D MousePos;
		if (LocalPlayer->ViewportClient->GetMousePosition(MousePos)) {
			FVector2D size;
			LocalPlayer->ViewportClient->GetViewportSize(size);
			float value = UKismetMathLibrary::NormalizeToRange(MousePos.Y, 0.f, size.Y * ScrollUpLimitation);
			value = UKismetMathLibrary::FClamp(value, 0.f, 1.f);
			value = 1.f - value;
			if (value > 0.f) {
				OffsetByVector(FVector(1.f, 0.f, 0.f), ScrollScalar);
			}
		}
	}
}

void ATerrainCamera::ScrollDown()
{
	if (ULocalPlayer* LocalPlayer = Cast<APlayerController>(GetController())->GetLocalPlayer()) {
		FVector2D MousePos;
		if (LocalPlayer->ViewportClient->GetMousePosition(MousePos)) {
			FVector2D size;
			LocalPlayer->ViewportClient->GetViewportSize(size);
			float value = UKismetMathLibrary::NormalizeToRange(MousePos.Y, size.Y * ScrollDownLimitation, size.Y);
			value = UKismetMathLibrary::FClamp(value, 0.f, 1.f);
			if (value > 0.f) {
				OffsetByVector(FVector(-1.f, 0.f, 0.f), ScrollScalar);
			}
		}
	}
}

void ATerrainCamera::Move(FVector2D MouseMov)
{
	OffsetByVector(FVector(MouseMov.Y, MouseMov.X, 0.0), MoveScalar);
}

void ATerrainCamera::OffsetByVector(FVector offset, float scalar)
{
	FVector RotateVec = (CameraBoom->TargetArmLength / 100.0 * scalar) * offset;
	FRotator Rotator = FRotator(0.0, Camera->GetComponentRotation().Yaw, 0.0);
	FVector DeltaLoc = Rotator.RotateVector(RotateVec);
	AddActorLocalOffset(DeltaLoc);
	SetActorLocation(UKismetMathLibrary::Vector_BoundedToBox(GetActorLocation(), BoundaryMin, BoundaryMax));
}

// Called when the game starts or when spawned
void ATerrainCamera::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* pc = Cast<APlayerController>(GetController());
	if (!pc) {
		UE_LOG(TerrainCamera, Warning, TEXT("AutoReceiveInput disabled."));
		return;
	}
	pc->SetViewTargetWithBlend(this);

	if (!InitCamera() || !BindEnchancedInputAction()) {
		UE_LOG(TerrainCamera, Warning, TEXT("BeginPlay error!"));
		return;
	}

	FTimerHandle TimerHandleTI;
	GetWorldTimerManager().SetTimer(TimerHandleTI, TerrainInputDelegate, TimingForWaitTerrain, false);
	FTimerHandle TimerHandleTG;
	GetWorldTimerManager().SetTimer(TimerHandleTG, TerrainInfoDelegate, TimingForWaitTerrain, false);
	FTimerHandle TimerHandleSS;
	GetWorldTimerManager().SetTimer(TimerHandleSS, ScrollScreenDelegate, TimingForScrollScreen, true);
	
}

// Called every frame
void ATerrainCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ATerrainCamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (ULocalPlayer* LocalPlayer = Cast<APlayerController>(GetController())->GetLocalPlayer()) {
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()) {
			if (InputMapping != nullptr) {
				InputSystem->AddMappingContext(InputMapping, 0);
			}
			else {
				UE_LOG(TerrainCamera, Error, TEXT("No setting Input Mapping Context."));
			}
		}
	}

}

