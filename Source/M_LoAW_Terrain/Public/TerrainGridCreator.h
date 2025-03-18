// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuadGridCreator.h"
#include "TerrainGridCreator.generated.h"

/**
 * 
 */
UCLASS()
class M_LOAW_TERRAIN_API ATerrainGridCreator : public AQuadGridCreator
{
	GENERATED_BODY()
private:
	FVector2D TmpPosition2D;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Params", meta = (ClampMin = "0.0"))
	float TileSize = 500.0;

public:
	ATerrainGridCreator();

protected:
	virtual void InitGridRing(int32 Radius) override;
	virtual int32 AddRingPointAndIndex(int32 Range) override;
	virtual void FindNeighborPointOfRing(int32 DirIndex) override;

	virtual void WriteParamsContentByChild(std::ofstream& ofs) override;
};
