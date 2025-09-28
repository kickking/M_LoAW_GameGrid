// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameGridStructDefine.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameGridTerrainTypeTree.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(GameGridTerrainTypeTree, Log, All);

USTRUCT(BlueprintType)
struct FStructTreeSample
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* TreeMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PropOfAll = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ZScaleUpper = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ZScaleLower = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float XYScaleUpper = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float XYScaleLower = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ColorMaskMultiplier = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VarColorSaturationMultiplier = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VarColorValueMultiplier = 0.0f;

};


UCLASS()
class M_LOAW_GAMEGRID_API AGameGridTerrainTypeTree : public AActor
{
	GENERATED_BODY()

private:
	TArray<class UHierarchicalInstancedStaticMeshComponent*> HISMCs;
	bool IsInitHISMC = false;
	int32 NumCustomDataFloats = 5;// PosX PosY ValueOnColorMask ValueOnVarColorSaturation ValueOnVarColorValue


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FStructTreeSample> Trees;

public:	
	// Sets default values for this actor's properties
	AGameGridTerrainTypeTree();

	void CreateHISMCForSamples();
	int32 AddTreeInstance(FStructTreeRecord& OutRecord, const FVector& Loc, bool ShowTree);
	int32 AddTreeInstanceByRecord(const FStructTreeRecord& InRecord, bool ShowTree);
	void AddTreeInstanceData(int32 InstanceIndex, const FStructTreeRecord& InRecord, const FStructGameGridPointData& PointData);

private:
	int32 GetRandomSampleIndex();
	float CalValueOnTreeMaterialParam(float Multiplier, float PointDataValue);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
