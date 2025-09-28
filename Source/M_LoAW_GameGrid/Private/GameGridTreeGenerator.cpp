// Fill out your copyright notice in the Description page of Project Settings.


#include "GameGridTreeGenerator.h"

DEFINE_LOG_CATEGORY(GameGridTreeGenerator);

// Sets default values
AGameGridTreeGenerator::AGameGridTreeGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

}

// Called when the game starts or when spawned
void AGameGridTreeGenerator::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AGameGridTreeGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGameGridTreeGenerator::CreateTerrainTypeTrees()
{
	for (auto& TTPair : TerrainTypeToTree)
	{
		if (TTPair.Value) {
			TTPair.Value->CreateHISMCForSamples();
		}
	}
}

int32 AGameGridTreeGenerator::AddTreeByTerraintype(Enum_TerrainType TT, FStructTreeRecord& OutRecord, const FVector& Loc, bool ShowTree)
{
	int32 Ret = -1;
	if (TerrainTypeToTree.Contains(TT)) {
		AGameGridTerrainTypeTree* TypeTree = TerrainTypeToTree[TT];
		if (TypeTree) {
			Ret = TypeTree->AddTreeInstance(OutRecord, Loc, ShowTree);
		}
	}
	return Ret;
}

void AGameGridTreeGenerator::AddTreeDataByTerraintype(Enum_TerrainType TT, const FStructGameGridPointData& PointData, 
	int32 InstanceIndex, const FStructTreeRecord& InRecord)
{
	if (TerrainTypeToTree.Contains(TT)) {
		AGameGridTerrainTypeTree* TypeTree = TerrainTypeToTree[TT];
		if (TypeTree) {
			TypeTree->AddTreeInstanceData(InstanceIndex, InRecord, PointData);
		}
	}
}

