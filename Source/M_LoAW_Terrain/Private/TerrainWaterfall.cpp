// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainWaterfall.h"
#include "NiagaraComponent.h"

void ATerrainWaterfall::SetParamLifeTimeScale(float Scale)
{
	if (NiagaraUnit) {
		LifeTimeScale = Scale;
		NiagaraUnit->SetVariableFloat(LifeTimeScaleName, LifeTimeScale);
	}
}

void ATerrainWaterfall::SetParamVelocityScale(FVector VecScale)
{
	if (NiagaraUnit) {
		VelocityScale = VecScale;
		NiagaraUnit->SetVariableVec3(VelocityScaleName, VelocityScale);
	}
}
