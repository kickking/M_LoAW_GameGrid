// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridDataLoader.h"
#include "GameGridLoader.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(GameGridLoader, Log, All);

/**
 * 
 */
UCLASS()
class M_LOAW_GAMEGRID_API AGameGridLoader : public AGridDataLoader
{
	GENERATED_BODY()
private:
	TArray<FVector> VerticesDirVectors;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0"))
	int32 ProgressWeight_CreatePointsVertices = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom|Params")
	float TileSize = 0.0f;

public:
	AGameGridLoader();

protected:
	virtual bool ParseParamsByChild(int32 StartIndex, TArray<FString>& StrArr) override;
	virtual void SetParams() override;

	virtual void InitProgressTotal() override;

	virtual void AddPointIndex(FIntPoint key, int32 value) override;

	virtual void ParsePointLine(const FString& line) override;
	virtual void AddPoint(FStructGridData Data) override;

	virtual void AddNeighbors(int32 Index, FStructGridDataNeighbors Neighbors) override;
	virtual bool PointIndicesContains(FIntPoint Point) override;

	virtual void CreatePointsVertices() override;
	virtual void InitPointVerticesVertors() override;
	virtual void CreatePointVertices(int32 Index) override;
	virtual void AddPosition2D(int32 Index, FVector2D Position2D) override;

	virtual void DoWorkFlowDone() override;
};
