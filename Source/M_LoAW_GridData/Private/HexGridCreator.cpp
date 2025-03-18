// Fill out your copyright notice in the Description page of Project Settings.


#include "HexGridCreator.h"
#include <fstream>

DEFINE_LOG_CATEGORY(HexGridCreator);

AHexGridCreator::AHexGridCreator()
{
	NeighborStep = 6;
}

void AHexGridCreator::InitGridRing(int32 Radius)
{
	Hex hex = Hex::Direction(HEX_RING_DIRECTION_START_INDEX);
	hex = Hex::Scale(hex, Radius);
	TmpHex = Hex::Add(hex, Hex(Points[0].AxialCoord));
}

int32 AHexGridCreator::AddRingPointAndIndex(int32 Range)
{
	int32 Index = Super::AddRingPointAndIndex(Range);
	Points[Index].AxialCoord.X = TmpHex.GetCoord().Q;
	Points[Index].AxialCoord.Y = TmpHex.GetCoord().R;

	PointIndices.Add(FIntPoint(Points[Index].AxialCoord.X, Points[Index].AxialCoord.Y), Index);
	return Index;
}

void AHexGridCreator::FindNeighborPointOfRing(int32 DirIndex)
{
	TmpHex = Hex::Neighbor(TmpHex, DirIndex);
}

void AHexGridCreator::InitNeighborRing(int32 Radius, FIntPoint center)
{
	Hex hex = Hex::Direction(HEX_RING_DIRECTION_START_INDEX);
	hex = Hex::Scale(hex, Radius);
	TmpHex = Hex::Add(hex, Hex(center));
}

void AHexGridCreator::SetPointNeighbor(int32 PointIndex, int32 Radius, int32 DirIndex)
{
	Points[PointIndex].Neighbors[Radius - 1].Points.Add(FIntPoint(TmpHex.GetCoord().Q, TmpHex.GetCoord().R));
	TmpHex = Hex::Neighbor(TmpHex, DirIndex);
}
