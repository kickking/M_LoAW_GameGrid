// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainGridLoader.h"
#include "M_LoAW_GameBase/Public/LoAWGameInstance.h"

DEFINE_LOG_CATEGORY(TerrainGridLoader);

ATerrainGridLoader::ATerrainGridLoader()
{
	LoaderName = FString(TEXT("TerrainGridLoader"));
	DataFileRelPath = FString(TEXT("Data/TerrainGrid/"));
	ParamNum = 4;
}

bool ATerrainGridLoader::ParseParamsByChild(int32 StartIndex, TArray<FString>& StrArr)
{
	LexFromString(TileSize, StrArr[StartIndex]);
	if (StartIndex != ParamNum - 1) {
		return false;
	}
	return true;
}

void ATerrainGridLoader::ParsePointLine(const FString& line)
{
	TArray<FString> StrArr;
	line.ParseIntoArray(StrArr, *PipeDelim, true);
	FStructGridData Data;
	ParseAxialCoord(StrArr[0], Data);
	ParsePosition2D(StrArr[1], Data);
	AddPoint(Data);
}

void ATerrainGridLoader::AddPointIndex(FIntPoint key, int32 value)
{
	pGI->TerrainGridPointIndices.Add(key, value);
}

void ATerrainGridLoader::AddPoint(FStructGridData Data)
{
	pGI->TerrainGridPoints.Add(Data);
}

void ATerrainGridLoader::AddNeighbors(int32 Index, FStructGridDataNeighbors Neighbors)
{
	pGI->TerrainGridPoints[Index].Neighbors.Add(Neighbors);
}

bool ATerrainGridLoader::PointIndicesContains(FIntPoint Point)
{
	return pGI->TerrainGridPointIndices.Contains(Point);
}
