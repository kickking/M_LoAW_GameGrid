// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrainInput.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(TerrainInput, Log, All);

UCLASS(MinimalAPI)
class ATerrainInput : public AActor
{
	GENERATED_BODY()
private:
	FTimerDynamicDelegate UpdateMousePosDelegate;
	FTimerHandle UpdateMousePosTimerHandle;

	FVector MousePos;

	class ATerrainGenerator* pTG;

	APlayerController* Controller;

	bool LeftHold = false;
	bool RightHold = false;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Input")
	class UInputMappingContext* InputMapping;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Input")
	class UInputAction* MouseLeftHoldAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Input")
	class UInputAction* MouseRightHoldAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Input")
	float HoldTraceLength = 1000000.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Timer")
	float UpdateMousePosTimerRate = 0.01f;

public:	
	// Sets default values for this actor's properties
	ATerrainInput();

	FORCEINLINE FVector GetMousePosition() {
		return MousePos;
	}

	UFUNCTION(BlueprintCallable)
	M_LOAW_TERRAIN_API FORCEINLINE bool IsLeftHold()
	{
		return LeftHold;
	}

	UFUNCTION(BlueprintCallable)
	M_LOAW_TERRAIN_API FORCEINLINE bool IsRightHold()
	{
		return RightHold;
	}

private:
	void BindDelegate();
	bool GetTerrainGenerator();
	bool EnablePlayer();
	bool AddInputMappingContext();
	bool BindEnchancedInputAction();
	bool IsMouseClick();

	UFUNCTION()
	void OnLeftHoldStarted(const FInputActionValue& Value);
	UFUNCTION()
	void OnLeftHoldCompleted(const FInputActionValue& Value);
	UFUNCTION()
	void OnRightHoldStarted(const FInputActionValue& Value);
	UFUNCTION()
	void OnRightHoldCompleted(const FInputActionValue& Value);

	void StartUpdateMousePos();
	void StopUpdateMousePos();

	UFUNCTION()
	void UpdateMousePosition();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
