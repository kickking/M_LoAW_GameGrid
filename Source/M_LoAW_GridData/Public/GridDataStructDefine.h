#pragma once

#include "GridDataStructDefine.generated.h"

USTRUCT(BlueprintType)
struct FStructLoopData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 LoopCountLimit = 3000;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Rate = 0.01f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (ClampMin = "1"))
	int32 LoopDepthLimit = 4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<int32> IndexSaved = {};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool HasInitialized = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (ClampMin = "0"))
	int32 Count = 0;
};

USTRUCT(BlueprintType)
struct FStructGridDataNeighbors
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Radius = 0;

	UPROPERTY()
	int32 Count = 0;

	UPROPERTY()
	TArray<FIntPoint> Points;

};

USTRUCT(BlueprintType)
struct FStructGridData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FIntPoint AxialCoord = FIntPoint();

	UPROPERTY(BlueprintReadOnly)
	FVector2D Position2D = FVector2D();

	UPROPERTY(BlueprintReadOnly)
	TArray<FVector2D> VerticesPostion2D = {};

	UPROPERTY(BlueprintReadOnly)
	float PositionZ = 0.0;

	UPROPERTY(BlueprintReadOnly)
	TArray<float> VerticesPositionZ;

	UPROPERTY(BlueprintReadOnly)
	FVector Normal = FVector();

	UPROPERTY(BlueprintReadOnly)
	TArray<FStructGridDataNeighbors> Neighbors;
};
