// Fill out your copyright notice in the Description page of Project Settings.


#include "GameGridCreator.h"
#include <fstream>

DEFINE_LOG_CATEGORY(GameGridCreator);

AGameGridCreator::AGameGridCreator()
{
	CreatorName = FString(TEXT("GameGridCreator"));
	DataFileRelPath = FString(TEXT("Data/GameGrid/"));

	GridRange = 300;
	NeighborRange = 4;
	TileSize = 400;
}

void AGameGridCreator::InitByChild()
{
	InitNeighborDirection();
	InitTileParams();
}

void AGameGridCreator::InitNeighborDirection()
{
	FVector ZAxis(0.0, 0.0, 1.0);
	FVector Vec(1.0, 0.0, 0.0);
	Vec = Vec.RotateAngleAxis(30.0, ZAxis);
	float Angle;
	FVector NDir;
	for (int32 i = 0; i <= 5; i++)
	{
		Angle = i * (-60.0);
		NDir = Vec.RotateAngleAxis(Angle, ZAxis);
		NeighborDirVectors.Add(FVector2D(NDir.X, NDir.Y));
	}
}

void AGameGridCreator::InitTileParams()
{
	TileHeight = TileSize * FMath::Sqrt(3.0);
}

void AGameGridCreator::InitGridRing(int32 Radius)
{
	Super::InitGridRing(Radius);

	FVector2D Vec = Radius * TileHeight * NeighborDirVectors[HEX_RING_DIRECTION_START_INDEX];
	TmpPosition2D.Set(Vec.X, Vec.Y);
}

int32 AGameGridCreator::AddRingPointAndIndex()
{
	int32 Index = Super::AddRingPointAndIndex();
	Points[Index].Position2D.Set(TmpPosition2D.X, TmpPosition2D.Y);
	return Index;
}

void AGameGridCreator::FindNeighborPointOfRing(int32 DirIndex)
{
	Super::FindNeighborPointOfRing(DirIndex);
	FVector2D Pos = NeighborDirVectors[DirIndex] * TileHeight + TmpPosition2D;
	TmpPosition2D.Set(Pos.X, Pos.Y);
}

void AGameGridCreator::WriteParamsContentByChild(std::ofstream& ofs)
{
	WritePipeDelimiter(ofs);
	FString Str = FloatToString(TileSize, 2);
	ofs << TCHAR_TO_ANSI(*Str);
}
