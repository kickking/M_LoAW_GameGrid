// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameGridInput.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(GameGridInput, Log, All);

UCLASS()
class M_LOAW_GAMEGRID_API AGameGridInput : public AActor
{
	GENERATED_BODY()

private:
	//Delegate
	FTimerDynamicDelegate CheckMouseOverDelegate;

	class UGridDataGameInstance* pGI;
	class AGameGridGenerator* pGG;
	class ATerrainInput* pTI;

	APlayerController* Controller;

	FTimerHandle CheckTimerHandle;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Input")
	class UInputMappingContext* InputMapping;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Input")
	class UInputAction* IncMouseOverRadiusAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Input")
	class UInputAction* DecMouseOverRadiusAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Input")
	float TraceLength = 1000000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Timer")
	float CheckTimerRate = 0.02f;

public:	
	// Sets default values for this actor's properties
	AGameGridInput();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void BindDelegate();
	bool GetGameInstance();
	bool GetGameGridGenerator();
	bool GetTerrainInput();
	bool EnablePlayer();
	bool AddInputMappingContext();
	bool BindEnchancedInputAction();

	void OnIncMouseOverRadius();
	void OnDecMouseOverRadius();

	UFUNCTION()
	void CheckMouseOver();

	void MouseOverGrid(const FVector2D& MousePos);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void StartCheckMouseOver();

	UFUNCTION(BlueprintCallable)
	void StopCheckMouseOver();

	UFUNCTION(BlueprintCallable)
	bool IsUseGrid();
	
};
