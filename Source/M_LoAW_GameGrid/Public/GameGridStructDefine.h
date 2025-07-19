#pragma once

#include "M_LoAW_Terrain/Public/TerrainGenerator.h"
#include "GameGridStructDefine.generated.h"

USTRUCT(BlueprintType)
struct FStructGameGridPointData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 GridDataIndex = 0;

	UPROPERTY(BlueprintReadOnly)
	float PositionZ = 0.0;

	UPROPERTY(BlueprintReadOnly)
	float AvgPositionZ = 0.0;

	UPROPERTY(BlueprintReadOnly)
	float PositionZRatio = 0.0;

	UPROPERTY(BlueprintReadOnly)
	TArray<float> VerticesPositionZ;

	UPROPERTY(BlueprintReadOnly)
	FVector Normal = FVector();

	UPROPERTY(BlueprintReadOnly)
	float AngleToUp = 0.0;

	UPROPERTY(BlueprintReadOnly)
	bool InTerrainRange = false;

	UPROPERTY(BlueprintReadOnly)
	int32 AreaBlockLevel = 0;

	UPROPERTY(BlueprintReadOnly)
	bool AreaConnection = true;

	UPROPERTY(BlueprintReadOnly)
	bool FlyingConnection = true;

	UPROPERTY(BlueprintReadOnly)
	bool IsLand = false;

	UPROPERTY(BlueprintReadOnly)
	bool FlyingIsLand = false;

	UPROPERTY(BlueprintReadOnly)
	int32 BuildingBlockLevel = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 FlyingBlockLevel = 0;

	UPROPERTY(BlueprintReadOnly)
	Enum_TerrainType TerrainType = Enum_TerrainType::None;
};
