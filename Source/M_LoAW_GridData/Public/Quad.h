// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class M_LOAW_GRIDDATA_API QuadCoord
{
public:
	float x = 0.0f;
	float y = 0.0f;

	int32 X = 0;
	int32 Y = 0;

	QuadCoord();
	~QuadCoord();

	void Set(const QuadCoord& Coord);
};

/**
 *
 */
class M_LOAW_GRIDDATA_API Quad
{
private:
	QuadCoord Coord;
	TArray<FIntPoint> NeighborDirectionVectors = { FIntPoint(0, -1), FIntPoint(1, 0),
		FIntPoint(0, 1), FIntPoint(-1, 0) };
	TArray<FIntPoint> DiagonalDirectionVectors = { FIntPoint(1, 1), FIntPoint(-1, 1),
		FIntPoint(-1, -1), FIntPoint(1, -1) };

private:
	void UpdateCoordFloat(float a, float b);
	void UpdateCoordInt(int32 A, int32 B);

public:
	Quad();
	Quad(FVector2D Axial);
	Quad(FIntPoint AxialInt);

	void SetAxial(const FVector2D Axial);
	void SetAxialInt(const FIntPoint AxialInt);
	void SetQuad(const Quad& quad);

	QuadCoord GetCoord();

	static Quad Add(const Quad& InQuadA, const Quad& InQuadB);
	static Quad Scale(const Quad& InQuad, float Factor);
	static Quad NeighborDirection(int32 Direction);
	static Quad DiagonalDirection(int32 Direction);
	static Quad Neighbor(const Quad& InQuad, int32 direction);

	~Quad();
};