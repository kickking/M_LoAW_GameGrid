// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "M_LoAW_GridData/Public/GridDataStructDefine.h"

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "LoAWGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class M_LOAW_GAMEBASE_API ULoAWGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	TMap<FIntPoint, int32> GameGridPointIndices;
	TArray<FStructGridData> GameGridPoints;

	TMap<FIntPoint, int32> TerrainGridPointIndices;
	TArray<FStructGridData> TerrainGridPoints;
	
};
