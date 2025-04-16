// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TerrainNiagaraUnit.h"
#include "TerrainWaterfall.generated.h"

/**
 * 
 */
UCLASS()
class M_LOAW_TERRAIN_API ATerrainWaterfall : public ATerrainNiagaraUnit
{
	GENERATED_BODY()

private:
	float LifeTimeScale = 1.0;
	FName LifeTimeScaleName = FName("User.LifeTimeScale");

	FVector VelocityScale = FVector();
	FName VelocityScaleName = FName("User.VelocityScale");

public:

	FORCEINLINE float GetLifeTimeScale() {
		return LifeTimeScale;
	}

	FORCEINLINE FVector GetVelocityScale() {
		return VelocityScale;
	}

	void SetParamLifeTimeScale(float Scale);
	void SetParamVelocityScale(FVector VecScale);
};
