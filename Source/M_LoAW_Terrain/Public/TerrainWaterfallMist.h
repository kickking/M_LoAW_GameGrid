// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TerrainNiagaraUnit.h"
#include "TerrainWaterfallMist.generated.h"

/**
 * 
 */
UCLASS()
class M_LOAW_TERRAIN_API ATerrainWaterfallMist : public ATerrainNiagaraUnit
{
	GENERATED_BODY()

private:
	FVector2D WaterfallRadius = FVector2D();
	FName WaterfallRadiusName = FName("User.WaterfallRadius");

public:
	FORCEINLINE FVector2D GetWaterfallRadius() {
		return WaterfallRadius;
	}

	void SetParamWaterfallRadius(FVector2D Radius);
	
};
