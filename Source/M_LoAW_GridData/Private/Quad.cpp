// Fill out your copyright notice in the Description page of Project Settings.


#include "Quad.h"

QuadCoord::QuadCoord()
{
}

QuadCoord::~QuadCoord()
{
}

void QuadCoord::Set(const QuadCoord& Coord)
{
	x = Coord.x;
	y = Coord.y;
	X = Coord.X;
	Y = Coord.Y;
}

Quad::Quad()
{
}

Quad::Quad(FVector2D Axial)
{
	UpdateCoordFloat(Axial.X, Axial.Y);
}

Quad::Quad(FIntPoint AxialInt)
{
	UpdateCoordInt(AxialInt.X, AxialInt.Y);
}

void Quad::SetAxial(const FVector2D Axial)
{
	UpdateCoordFloat(Axial.X, Axial.Y);
}

void Quad::SetAxialInt(const FIntPoint AxialInt)
{
	UpdateCoordInt(AxialInt.X, AxialInt.Y);
}

void Quad::SetQuad(const Quad& quad)
{
	Coord.Set(quad.Coord);
}

QuadCoord Quad::GetCoord()
{
	return Coord;
}

Quad Quad::Add(const Quad& InQuadA, const Quad& InQuadB)
{
	return Quad(FIntPoint(InQuadA.Coord.X + InQuadB.Coord.X, InQuadA.Coord.Y + InQuadB.Coord.Y));
}

Quad Quad::Scale(const Quad& InQuad, float Factor)
{
	return Quad(FVector2D(InQuad.Coord.x * Factor, InQuad.Coord.y * Factor));
}

Quad Quad::NeighborDirection(int32 Direction)
{
	return Quad(Quad().NeighborDirectionVectors[Direction]);
}

Quad Quad::DiagonalDirection(int32 Direction)
{
	return Quad(Quad().DiagonalDirectionVectors[Direction]);
}

Quad Quad::Neighbor(const Quad& InQuad, int32 direction)
{
	return Add(InQuad, DiagonalDirection(direction));
}

Quad::~Quad()
{
}

void Quad::UpdateCoordFloat(float a, float b)
{
	Coord.x = a;
	Coord.y = b;

	Coord.X = int32(Coord.x);
	Coord.Y = int32(Coord.y);
}

void Quad::UpdateCoordInt(int32 A, int32 B)
{
	Coord.X = A;
	Coord.Y = B;

	Coord.x = float(Coord.X);
	Coord.y = float(Coord.Y);
}
