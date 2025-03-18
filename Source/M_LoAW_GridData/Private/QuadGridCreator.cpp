// Fill out your copyright notice in the Description page of Project Settings.


#include "QuadGridCreator.h"

DEFINE_LOG_CATEGORY(QuadGridCreator);


AQuadGridCreator::AQuadGridCreator()
{
	NeighborStep = 4;
}

void AQuadGridCreator::InitGridRing(int32 Radius)
{
	Quad quad = Quad::NeighborDirection(QUAD_RING_DIRECTION_START_INDEX);
	quad = Quad::Scale(quad, Radius);
	TmpQuad = Quad::Add(quad, Quad(Points[0].AxialCoord));
}

int32 AQuadGridCreator::AddRingPointAndIndex(int32 Range)
{
	int32 Index = Super::AddRingPointAndIndex(Range);
	Points[Index].AxialCoord.X = TmpQuad.GetCoord().X;
	Points[Index].AxialCoord.Y = TmpQuad.GetCoord().Y;

	PointIndices.Add(FIntPoint(Points[Index].AxialCoord.X, Points[Index].AxialCoord.Y), Index);
	return Index;
}

void AQuadGridCreator::FindNeighborPointOfRing(int32 DirIndex)
{
	TmpQuad = Quad::Neighbor(TmpQuad, DirIndex);
}

void AQuadGridCreator::InitNeighborRing(int32 Radius, FIntPoint center)
{
	Quad quad = Quad::NeighborDirection(QUAD_RING_DIRECTION_START_INDEX);
	quad = Quad::Scale(quad, Radius);
	TmpQuad = Quad::Add(quad, Quad(center));
}

void AQuadGridCreator::SetPointNeighbor(int32 PointIndex, int32 Radius, int32 DirIndex)
{
	Points[PointIndex].Neighbors[Radius - 1].Points.Add(FIntPoint(TmpQuad.GetCoord().X, TmpQuad.GetCoord().Y));
	TmpQuad = Quad::Neighbor(TmpQuad, DirIndex);
}
