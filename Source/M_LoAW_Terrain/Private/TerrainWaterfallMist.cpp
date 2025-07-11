// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainWaterfallMist.h"
#include "NiagaraComponent.h"

void ATerrainWaterfallMist::SetParamWaterfallRadius(FVector2D Radius)
{
	if (NiagaraUnit) {
		WaterfallRadius = Radius;
		NiagaraUnit->SetVariableVec2(WaterfallRadiusName, WaterfallRadius);
	}
}
