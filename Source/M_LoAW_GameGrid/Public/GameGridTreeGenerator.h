// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "M_LoAW_Terrain/Public/TerrainGenerator.h"
#include "GameGridTerrainTypeTree.h"
#include "GameGridStructDefine.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameGridTreeGenerator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(GameGridTreeGenerator, Log, All);

UCLASS()
class M_LOAW_GAMEGRID_API AGameGridTreeGenerator : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom")
	TMap<Enum_TerrainType, AGameGridTerrainTypeTree*> TerrainTypeToTree;

public:	
	// Sets default values for this actor's properties
	AGameGridTreeGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	void CreateTerrainTypeTrees();
	int32 AddTreeByTerraintype(Enum_TerrainType TT, FStructTreeRecord& OutRecord, const FVector& Loc, bool ShowTree);
	void AddTreeDataByTerraintype(Enum_TerrainType TT, const FStructGameGridPointData& PointData, int32 InstanceIndex, const FStructTreeRecord& InRecord);

};
