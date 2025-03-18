// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridDataLoader.h"
#include "TerrainGridLoader.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(TerrainGridLoader, Log, All);

/**
 * 
 */
UCLASS()
class M_LOAW_TERRAIN_API ATerrainGridLoader : public AGridDataLoader
{
	GENERATED_BODY()
	
public:
	ATerrainGridLoader();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom|Params")
	float TileSize = 0.0f;

protected:
	virtual bool ParseParamsByChild(int32 StartIndex, TArray<FString>& StrArr) override;
	virtual void SetParams() override;

	virtual void ParsePointLine(const FString& line) override;
	virtual void AddPointIndex(FIntPoint key, int32 value) override;

	virtual void AddPoint(FStructGridData Data) override;

	virtual void AddNeighbors(int32 Index, FStructGridDataNeighbors Neighbors) override;
	virtual bool PointIndicesContains(FIntPoint Point) override;

	virtual void DoWorkFlowDone() override;

};
