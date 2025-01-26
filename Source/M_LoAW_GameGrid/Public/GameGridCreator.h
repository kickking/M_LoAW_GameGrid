// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HexGridCreator.h"
#include "GameGridCreator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(GameGridCreator, Log, All);

/**
 * 
 */
UCLASS()
class M_LOAW_GAMEGRID_API AGameGridCreator : public AHexGridCreator
{
	GENERATED_BODY()
	
public:
	AGameGridCreator();

private:
	TArray<FVector2D> NeighborDirVectors;

	float TileHeight;

	FVector2D TmpPosition2D;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Params", meta = (ClampMin = "0.0"))
	float TileSize = 400.0;

protected:
	virtual void InitByChild() override;

	virtual void InitGridRing(int32 Radius) override;
	virtual int32 AddRingPointAndIndex() override;
	virtual void FindNeighborPointOfRing(int32 DirIndex) override;

	virtual void WriteParamsContentByChild(std::ofstream& ofs) override;

private:
	void InitNeighborDirection();
	void InitTileParams();
};
