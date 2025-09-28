// Fill out your copyright notice in the Description page of Project Settings.


#include "GameGridTerrainTypeTree.h"

#include <Components/HierarchicalInstancedStaticMeshComponent.h>
#include <Math/UnrealMathUtility.h>

DEFINE_LOG_CATEGORY(GameGridTerrainTypeTree);

// Sets default values
AGameGridTerrainTypeTree::AGameGridTerrainTypeTree()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AGameGridTerrainTypeTree::CreateHISMCForSamples()
{
	if (!IsInitHISMC)
	{
		HISMCs.Empty();
		UHierarchicalInstancedStaticMeshComponent* HISMC;
		for (int32 i = 0; i < Trees.Num(); i++)
		{
			HISMC = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, TEXT("HISMC"));
			HISMC->SetMobility(EComponentMobility::Static);
			HISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			HISMC->SetStaticMesh(Trees[i].TreeMesh);
			HISMC->RegisterComponent();
			HISMC->SetNumCustomDataFloats(NumCustomDataFloats);
			HISMCs.Add(HISMC);
		}
		if (Trees.Num() > 0) {
			IsInitHISMC = true;
		}
	}
	
}


int32 AGameGridTerrainTypeTree::AddTreeInstance(FStructTreeRecord& OutRecord, const FVector& Loc, bool ShowTree)
{
	if (!IsInitHISMC) {
		//UE_LOG(GameGridTerrainTypeTree, Warning, TEXT("Pls call CreateHISMCForSamples first!"));
		return -1;
	}

	int32 SampleIndex = GetRandomSampleIndex();
	if (SampleIndex < 0) {
		//UE_LOG(GameGridTerrainTypeTree, Warning, TEXT("Pls set the appropriate param for trees first!"));
		return -1;
	}
	OutRecord.SampleIndex = SampleIndex;

	float ZScale = FMath::RandRange(Trees[SampleIndex].ZScaleLower, Trees[SampleIndex].ZScaleUpper);
	float XYScale = FMath::RandRange(Trees[SampleIndex].XYScaleLower, Trees[SampleIndex].XYScaleUpper);

	OutRecord.ZScale = ZScale;
	OutRecord.XYScale = XYScale;
	OutRecord.Loc = Loc;

	float AngleRotZ = FMath::SRand() * PI * 2.0;
	OutRecord.AngleRotZ = AngleRotZ;

	return AddTreeInstanceByRecord(OutRecord, ShowTree);
}

int32 AGameGridTerrainTypeTree::AddTreeInstanceByRecord(const FStructTreeRecord& InRecord, bool ShowTree)
{
	if (!IsInitHISMC || !ShowTree) {
		return -1;
	}

	FVector Scale(InRecord.XYScale, InRecord.XYScale, InRecord.ZScale);
	FVector ZAxis(0.f, 0.f, 1.0);
	FQuat Quat = FQuat(ZAxis, InRecord.AngleRotZ);

	FTransform TreeTransform(Quat.Rotator(), InRecord.Loc, Scale);

	int32 InstanceIndex = HISMCs[InRecord.SampleIndex]->AddInstance(TreeTransform);

	return InstanceIndex;
}

void AGameGridTerrainTypeTree::AddTreeInstanceData(int32 InstanceIndex, const FStructTreeRecord& InRecord, 
	const FStructGameGridPointData& PointData)
{
	TArray<float> CustomData = {};
	CustomData.Add((float)InRecord.Loc.X);
	CustomData.Add((float)InRecord.Loc.Y);

	float ValueOnColorMask = CalValueOnTreeMaterialParam(Trees[InRecord.SampleIndex].ColorMaskMultiplier, PointData.TerrainTypeEdgeRatio);
	CustomData.Add(ValueOnColorMask);

	float ValueOnVarColorSaturation = CalValueOnTreeMaterialParam(Trees[InRecord.SampleIndex].VarColorSaturationMultiplier, PointData.TerrainTypeEdgeRatio);
	CustomData.Add(ValueOnVarColorSaturation);

	float ValueOnVarColorValue = CalValueOnTreeMaterialParam(Trees[InRecord.SampleIndex].VarColorValueMultiplier, PointData.TerrainTypeEdgeRatio);
	CustomData.Add(ValueOnVarColorValue);

	HISMCs[InRecord.SampleIndex]->SetCustomData(InstanceIndex, CustomData, true);
}

int32 AGameGridTerrainTypeTree::GetRandomSampleIndex()
{
	float Prop = FMath::FRand();
	int32 Index = -1;
	for (int32 i = 0; i < Trees.Num(); i++)
	{
		Index = i;
		if (Prop < Trees[i].PropOfAll) {
			break;
		}
		else {
			Prop -= Trees[i].PropOfAll;
		}
	}
	return Index;
}

float AGameGridTerrainTypeTree::CalValueOnTreeMaterialParam(float Multiplier, float PointDataValue)
{
	float Sign = FMath::Sign<float>(Multiplier);
	float Abs = FMath::Abs<float>(Multiplier);
	float ValueOn = (FMath::Clamp(-Sign, 0.0, 1.0) + Sign * PointDataValue) * Abs + (1.0 - FMath::Floor(Abs));
	return ValueOn;
}

// Called when the game starts or when spawned
void AGameGridTerrainTypeTree::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGameGridTerrainTypeTree::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

