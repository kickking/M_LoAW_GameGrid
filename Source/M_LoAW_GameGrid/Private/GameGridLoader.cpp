// Fill out your copyright notice in the Description page of Project Settings.


#include "GameGridLoader.h"
#include <kismet/KismetStringLibrary.h>
#include "M_LoAW_GameBase/Public/LoAWGameInstance.h"

DEFINE_LOG_CATEGORY(GameGridLoader);

AGameGridLoader::AGameGridLoader()
{
	LoaderName = FString(TEXT("GameGridLoader"));
	DataFileRelPath = FString(TEXT("Data/GameGrid/"));
	ParamNum = 4;
}

bool AGameGridLoader::ParseParamsByChild(int32 StartIndex, TArray<FString>& StrArr)
{
	LexFromString(TileSize, StrArr[StartIndex]);
	if (StartIndex != ParamNum - 1) {
		return false;
	}
	return true;
}

void AGameGridLoader::InitProgressTotal()
{
	Super::InitProgressTotal();
	ProgressTotal += ProgressWeight_CreatePointsVertices * PointsNum;
}

void AGameGridLoader::AddPointIndex(FIntPoint key, int32 value)
{
	pGI->GameGridPointIndices.Add(key, value);
}

void AGameGridLoader::ParsePointLine(const FString& line)
{
	TArray<FString> StrArr;
	line.ParseIntoArray(StrArr, *PipeDelim, true);
	FStructGridData Data;
	ParseAxialCoord(StrArr[0], Data);
	ParsePosition2D(StrArr[1], Data);
	AddPoint(Data);
}

void AGameGridLoader::AddPoint(FStructGridData Data)
{
	pGI->GameGridPoints.Add(Data);
}

void AGameGridLoader::AddNeighbors(int32 Index, FStructGridDataNeighbors Neighbors)
{
	pGI->GameGridPoints[Index].Neighbors.Add(Neighbors);
}

bool AGameGridLoader::PointIndicesContains(FIntPoint Point)
{
	return pGI->GameGridPointIndices.Contains(Point);
}

void AGameGridLoader::CreatePointsVertices()
{
	if (PointsLoopFunction([this]() { InitPointVerticesVertors(); }, 
		[this](int32 i) { CreatePointVertices(i); },
		CreatePointsVerticesLoopData, Enum_GridDataLoaderState::Done,
		true, ProgressWeight_CreatePointsVertices)) 
	{
		UE_LOG(GameGridLoader, Log, TEXT("Create pointns vertices done!"));
	}
}

void AGameGridLoader::InitPointVerticesVertors()
{
	FVector Vec(1.0, 0.0, 0.0);
	FVector ZAxis(0.0, 0.0, 1.0);
	for (int32 i = 0; i <= 5; i++)
	{
		FVector TileVector = Vec.RotateAngleAxis(i * 60, ZAxis) * TileSize;
		VerticesDirVectors.Add(TileVector);
	}
}

void AGameGridLoader::CreatePointVertices(int32 Index)
{
	FVector Center(pGI->GameGridPoints[Index].Position2D.X, pGI->GameGridPoints[Index].Position2D.Y, 0);
	for (int32 i = 0; i <= 5; i++) {
		FVector Vertex = Center + VerticesDirVectors[i];
		AddPosition2D(Index, FVector2D(Vertex.X, Vertex.Y));
	}
}

void AGameGridLoader::AddPosition2D(int32 Index, FVector2D Position2D)
{
	pGI->GameGridPoints[Index].VerticesPostion2D.Add(Position2D);
}

