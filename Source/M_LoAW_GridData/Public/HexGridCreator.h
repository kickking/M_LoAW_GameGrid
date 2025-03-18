// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Hex.h"
#include "CoreMinimal.h"
#include "GridDataCreator.h"
#include "HexGridCreator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(HexGridCreator, Log, All);

#define HEX_RING_DIRECTION_START_INDEX	4
/**
 * 
 */
UCLASS()
class M_LOAW_GRIDDATA_API AHexGridCreator : public AGridDataCreator
{
	GENERATED_BODY()

public:
	AHexGridCreator();

private:
	Hex TmpHex;

protected:
	virtual void InitGridRing(int32 Radius) override;
	virtual int32 AddRingPointAndIndex(int32 Range) override;
	virtual void FindNeighborPointOfRing(int32 DirIndex) override;

	virtual void InitNeighborRing(int32 Radius, FIntPoint center) override;
	virtual void SetPointNeighbor(int32 PointIndex, int32 Radius, int32 DirIndex) override;

};
