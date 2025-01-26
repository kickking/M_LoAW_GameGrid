// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainGridCreator.h"
#include <fstream>

ATerrainGridCreator::ATerrainGridCreator()
{
	CreatorName = FString(TEXT("TerrainGridCreator"));
	DataFileRelPath = FString(TEXT("Data/TerrainGrid/"));

	GridRange = 505;
	NeighborRange = 3;
	TileSize = 500;
}

void ATerrainGridCreator::InitGridRing(int32 Radius)
{
	Super::InitGridRing(Radius);
	FVector2D Vec(TmpQuad.GetCoord().x * TileSize, TmpQuad.GetCoord().y * TileSize);
	TmpPosition2D.Set(Vec.X, Vec.Y);
}

int32 ATerrainGridCreator::AddRingPointAndIndex()
{
	int32 Index = Super::AddRingPointAndIndex();
	Points[Index].Position2D.Set(TmpPosition2D.X, TmpPosition2D.Y);
	return Index;
}

void ATerrainGridCreator::FindNeighborPointOfRing(int32 DirIndex)
{
	Super::FindNeighborPointOfRing(DirIndex);
	FVector2D Pos(TmpQuad.GetCoord().x * TileSize, TmpQuad.GetCoord().y * TileSize);
	TmpPosition2D.Set(Pos.X, Pos.Y);
}

void ATerrainGridCreator::WriteParamsContentByChild(std::ofstream& ofs)
{
	WritePipeDelimiter(ofs);
	FString Str = FloatToString(TileSize, 2);
	ofs << TCHAR_TO_ANSI(*Str);
}
