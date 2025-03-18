#pragma once

#include "TerrainStructDefine.generated.h"

USTRUCT(BlueprintType)
struct FStructHeightMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float RangeMin = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float RangeMax = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float MappingMin = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float MappingMax = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float RangeMinOffset = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float RangeMaxOffset = 0.0;

};

USTRUCT(BlueprintType)
struct FStructTerrainMeshPointData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 GridDataIndex = 0;

	UPROPERTY(BlueprintReadOnly)
	float PositionZ = 0.0;

	UPROPERTY(BlueprintReadOnly)
	float PositionZRatio = 0.0;

	UPROPERTY(BlueprintReadOnly)
	float RiverBlockZRatio = 0.0;

	UPROPERTY(BlueprintReadOnly)
	int32 BlockLevel = 0;

	UPROPERTY(BlueprintReadOnly)
	FVector Normal = FVector();

};

USTRUCT(BlueprintType)
struct FStructRiverLinePointData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 UpperPointIndex = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 LowerPointIndex = 0;

	UPROPERTY(BlueprintReadOnly)
	TArray<int32> LinePointIndices = {};

};
