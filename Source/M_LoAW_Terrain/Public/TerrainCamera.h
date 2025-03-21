// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TerrainCamera.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(TerrainCamera, Log, All);

UCLASS(MinimalAPI)
class ATerrainCamera : public APawn
{
	GENERATED_BODY()

private:
	FTimerDynamicDelegate TerrainInputDelegate;
	FTimerDynamicDelegate TerrainInfoDelegate;
	FTimerDynamicDelegate ScrollScreenDelegate;

	class ATerrainInput* pTI = nullptr;
	class ATerrainGenerator* pTG = nullptr;

	FVector BoundaryMin;
	FVector BoundaryMax;

protected:
	//Component
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Input")
	class UInputMappingContext* InputMapping;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Input")
	class UInputAction* CameraMoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Input")
	class UInputAction* CameraRotateAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Input")
	class UInputAction* CameraZoomInAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Input")
	class UInputAction* CameraZoomOutAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|MouseParam")
	float MoveScalar = 2.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|MouseParam")
	float RotateSpeed = 1.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|MouseParam")
	float ZoomStep = 4000.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|MouseParam")
	float ZoomMin = 10000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|MouseParam")
	float ZoomMax = 100000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Timer")
	float TimingForWaitTerrain = 0.1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Timer")
	float TimingForScrollScreen = 0.03;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|TerrainInfo")
	float BoundaryScalar = 0.45;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|ScrollScreen")
	float ScrollScalar = 1.5;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|ScrollScreen")
	float ScrollSpeedRatio = 0.7;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|ScrollScreen")
	float ScrollLeftLimitation = 0.005;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|ScrollScreen")
	float ScrollRightLimitation = 0.995;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|ScrollScreen")
	float ScrollUpLimitation = 0.005;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|ScrollScreen")
	float ScrollDownLimitation = 0.99;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Camera")
	float InitArmLength = 30000.0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Camera")
	float InitPitch = 16.0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Camera")
	float ViewPitchMax = -10.0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Camera")
	float ViewPitchMin = -70.0;

public:
	// Sets default values for this pawn's properties
	ATerrainCamera();

private:
	void BindDelegate();

	bool InitCamera();

	bool BindEnchancedInputAction();

	UFUNCTION()
	void OnCameraMove(const FInputActionValue& Value);
	UFUNCTION()
	void OnCameraRotate(const FInputActionValue& Value);
	UFUNCTION()
	void OnCameraZoomIn(const FInputActionValue& Value);
	UFUNCTION()
	void OnCameraZoomOut(const FInputActionValue& Value);

	UFUNCTION()
	void OnGetTerrainInput();
	UFUNCTION()
	void OnGetTerrainInfo();
	void FindActors(TSubclassOf<AActor> ActorClass, TArray<AActor*>& Actors);

	UFUNCTION()
	void OnScrollScreen();

	void Scroll();
	void ScrollLeft();
	void ScrollRight();
	void ScrollUp();
	void ScrollDown();

	void Move(FVector2D MouseMov);
	void OffsetByVector(FVector offset, float scalar);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
