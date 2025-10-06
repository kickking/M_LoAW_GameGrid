// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GridDataStructDefine.h"

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GridDataGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class M_LOAW_GRIDDATA_API UGridDataGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	TMap<FIntPoint, int32> GameGridPointIndices;
	TArray<FStructGridData> GameGridPoints;
	FStructGridDataParam GameGridParam;
	bool hasGameGridLoaded = false;

	TMap<FIntPoint, int32> TerrainGridPointIndices;
	TArray<FStructGridData> TerrainGridPoints;
	FStructGridDataParam TerrainGridParam;
	bool hasTerrainGridLoaded = false;
	
};
