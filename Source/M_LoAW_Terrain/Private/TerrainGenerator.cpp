// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainGenerator.h"
#include "TerrainNoise.h"
#include "ProceduralMeshComponent.h"
#include "M_LoAW_GridData/Public/FlowControlUtility.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "M_LoAW_GridData/Public/Quad.h"
#include "AStarUtility.h"
#include "M_LoAW_GridData/Public/GridDataGameInstance.h"

DEFINE_LOG_CATEGORY(TerrainGenerator);

// Sets default values
ATerrainGenerator::ATerrainGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	TerrainMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMesh"));
	this->SetRootComponent(TerrainMesh);
	TerrainMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	WaterMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("WaterMesh"));
	WaterMesh->SetupAttachment(TerrainMesh);
	WaterMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WaterMesh->SetWorldLocation(FVector(0.0, 0.0, -1.0));

	BindDelegate();
}

// Called when the game starts or when spawned
void ATerrainGenerator::BeginPlay()
{
	Super::BeginPlay();
	
	WorkflowState = Enum_TerrainGeneratorState::InitWorkflow;
	DoWorkFlow();
}

void ATerrainGenerator::BindDelegate()
{
	WorkflowDelegate.BindUFunction(Cast<UObject>(this), TEXT("DoWorkFlow"));
}

void ATerrainGenerator::DoWorkFlow()
{
	switch (WorkflowState)
	{
	case Enum_TerrainGeneratorState::InitWorkflow:
		InitWorkflow();
		break;
	case Enum_TerrainGeneratorState::CreateVertices:
		CreateVertices();
		break;
	case Enum_TerrainGeneratorState::ReMappingZ:
		ReMappingZ();
		break;
	case Enum_TerrainGeneratorState::SetBlockLevel:
		SetBlockLevel();
		break;
	case Enum_TerrainGeneratorState::SetBlockLevelEx:
		SetBlockLevelEx();
		break;
	case Enum_TerrainGeneratorState::CreateRiver:
	case Enum_TerrainGeneratorState::AddRiverEndPoints:
	case Enum_TerrainGeneratorState::DivideUpperRiver:
	case Enum_TerrainGeneratorState::DivideLowerRiver:
	case Enum_TerrainGeneratorState::ChunkToOnePoint:
	case Enum_TerrainGeneratorState::CreateRiverLine:
	case Enum_TerrainGeneratorState::FindRiverLines:
	case Enum_TerrainGeneratorState::DigRiverLine:
	case Enum_TerrainGeneratorState::DigRiverPool:
	case Enum_TerrainGeneratorState::CombinePoolToTerrain:
		CreateRiver();
		break;
	case Enum_TerrainGeneratorState::CreateVertexColorsForAMTB:
		CreateVertexColorsForAMTB();
		break;
	case Enum_TerrainGeneratorState::CreateTriangles:
		CreateTriangles();
		break;
	case Enum_TerrainGeneratorState::CalNormalsInit:
	case Enum_TerrainGeneratorState::CalNormalsAcc:
	case Enum_TerrainGeneratorState::NormalizeNormals:
		CreateNormals();
		break;
	case Enum_TerrainGeneratorState::DrawLandMesh:
		CreateTerrainMesh();
		SetTerrainMaterial();
		WorkflowState = Enum_TerrainGeneratorState::CreateWater;
	case Enum_TerrainGeneratorState::CreateWater:
		CreateWater();
		WorkflowState = Enum_TerrainGeneratorState::CreateWaterfall;
	case Enum_TerrainGeneratorState::CreateWaterfall:
		CreateWaterfall();
		break;
	case Enum_TerrainGeneratorState::Done:
		DoWorkflowDone();
		UE_LOG(TerrainGenerator, Log, TEXT("Create terrain done."));
		break;
	case Enum_TerrainGeneratorState::Error:
		UE_LOG(TerrainGenerator, Warning, TEXT("DoWorkFlow Error!"));
		break;
	default:
		break;
	}
}

void ATerrainGenerator::InitWorkflow()
{
	FTimerHandle TimerHandle;
	if (!GetGameInstance() || !InitNoise() || !CheckMaterialSetting()) {
		WorkflowState = Enum_TerrainGeneratorState::Error;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
		return;
	}

	InitTileParameter();
	InitLoopData();
	InitReceiveDecal();
	InitLandBlendParam();
	InitWater();
	InitProgress();

	WorkflowState = Enum_TerrainGeneratorState::CreateVertices;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
}

bool ATerrainGenerator::GetGameInstance()
{
	UWorld* world = GetWorld();
	if (world) {
		pGI = Cast<UGridDataGameInstance>(world->GetGameInstance());
		if (pGI && pGI->hasTerrainGridLoaded) {
			return true;
		}
	}
	UE_LOG(TerrainGenerator, Warning, TEXT("GetGameInstance Error!"));
	return false;
}

bool ATerrainGenerator::InitNoise()
{
	if (Noise == nullptr) {
		Noise = CreateDefaultSubobject<ATerrainNoise>(TEXT("TerrainNoise"));
		UE_LOG(TerrainGenerator, Log, TEXT("Create default Noise for terrain!"));
	}
	return Noise->Create();
}

bool ATerrainGenerator::CheckMaterialSetting()
{
	if (TerrainMPC == nullptr ||
		TerrainMaterialIns == nullptr || 
		WaterMaterialIns == nullptr ||
		CausticsMaterialIns == nullptr ||
		WaterfallMaterialIns == nullptr) {
		UE_LOG(TerrainGenerator, Warning, TEXT("CheckMaterialSetting Error!"));
		return false;
	}
	return true;
}

void ATerrainGenerator::InitTileParameter()
{
	TileSizeMultiplier = pGI->TerrainGridParam.TileSize;
	TileAltitudeMultiplier = TileAltitudeMax;

	TerrainSize = TileSizeMultiplier * (float)GridRange * FMath::Sqrt(2.0);
	
}

void ATerrainGenerator::InitLoopData()
{
	FlowControlUtility::InitLoopData(CreateVerticesLoopData);
	FlowControlUtility::InitLoopData(ReMappingZLoopData);

	FlowControlUtility::InitLoopData(SetBlockLevelLoopData);
	FlowControlUtility::InitLoopData(SetBlockLevelExLoopData);
	InitBlockLevelExLoopDatas();

	FlowControlUtility::InitLoopData(AddRiverEndPointsLoopData);
	FlowControlUtility::InitLoopData(UpperRiverDivideLoopData);
	FlowControlUtility::InitLoopData(LowerRiverDivideLoopData);
	FlowControlUtility::InitLoopData(FindRiverLinesLoopData);
	FlowControlUtility::InitLoopData(DigRiverLineLoopData);
	FlowControlUtility::InitLoopData(DigRiverPoolLoopData);
	
	FlowControlUtility::InitLoopData(CreateVertexColorsForAMTBLoopData);

	FlowControlUtility::InitLoopData(CreateTrianglesLoopData);
	FlowControlUtility::InitLoopData(CalNormalsInitLoopData);
	FlowControlUtility::InitLoopData(CalNormalsAccLoopData);
	FlowControlUtility::InitLoopData(NormalizeNormalsLoopData);
}

void ATerrainGenerator::InitBlockLevelExLoopDatas()
{
	for (int32 i = 0; i < BlockExTimes; i++)
	{
		FStructLoopData LoopData(SetBlockLevelExLoopData);
		BlockLevelExLoopDatas.Add(LoopData);
	}
}

void ATerrainGenerator::InitReceiveDecal()
{
	TerrainMesh->SetReceivesDecals(true);
	WaterMesh->SetReceivesDecals(false);
}

void ATerrainGenerator::InitLandBlendParam()
{
	UKismetMaterialLibrary::SetScalarParameterValue(this, TerrainMPC, TEXT("MoistureBlendThresholdLow"),
		MoistureBlendThresholdLow);
	UKismetMaterialLibrary::SetScalarParameterValue(this, TerrainMPC, TEXT("MoistureBlendThresholdHigh"),
		MoistureBlendThresholdHigh);
	UKismetMaterialLibrary::SetScalarParameterValue(this, TerrainMPC, TEXT("TempratureBlendThresholdLow"),
		TempratureBlendThresholdLow);
	UKismetMaterialLibrary::SetScalarParameterValue(this, TerrainMPC, TEXT("TempratureBlendThresholdHigh"),
		TempratureBlendThresholdHigh);
}

void ATerrainGenerator::InitWater()
{
	SetWaterZ();
	WaterBase = WaterBaseRatio * TileAltitudeMultiplier - WaterMesh->GetComponentLocation().Z;
}

void ATerrainGenerator::SetWaterZ()
{
	if (HasWater) {
		UKismetMaterialLibrary::SetScalarParameterValue(this, TerrainMPC, TEXT("WaterBaseRatio"),
			WaterBaseRatio);
	}
	else {
		UKismetMaterialLibrary::SetScalarParameterValue(this, TerrainMPC, TEXT("WaterBaseRatio"),
			-2.0);
	}
}

void ATerrainGenerator::InitProgress()
{
	ProgressPassed = 0.f;
	Progress = 0.f;
	StepTotalCount = MAX_int32;
}

void ATerrainGenerator::CreateVertices()
{
	int32 Count = 0;
	TArray<int32> Indices = { 0 };
	bool SaveLoopFlag = false;

	float RatioStd;
	float Ratio;

	if (!CreateVerticesLoopData.HasInitialized) {
		CreateVerticesLoopData.HasInitialized = true;
		if (GridRange > pGI->TerrainGridParam.GridRange) {
			GridRange = pGI->TerrainGridParam.GridRange;
		}
		StepTotalCount = 1 + (QUAD_SIDE_NUM + GridRange * QUAD_SIDE_NUM) * GridRange / 2;
	}

	int32 i = CreateVerticesLoopData.IndexSaved[0];
	int32 X = 0;
	int32 Y = 0;
	for (; i < StepTotalCount; i++) {
		Indices[0] = i;

		FlowControlUtility::SaveLoopData(this, CreateVerticesLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
		if (SaveLoopFlag) {
			return;
		}

		X = pGI->TerrainGridPoints[i].AxialCoord.X;
		Y = pGI->TerrainGridPoints[i].AxialCoord.Y;
		TerrainMeshPointsIndices.Add(FIntPoint(X, Y), i);
		CreateVertex(X, Y, RatioStd, Ratio);
		CreateUV(X, Y);

		Progress = ProgressPassed + (float)CreateVerticesLoopData.Count / (float)StepTotalCount * ProgressWeight_CreateVertices;
		Count++;
	}

	ProgressPassed += ProgressWeight_CreateVertices;

	WorkflowState = Enum_TerrainGeneratorState::ReMappingZ;
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, CreateVerticesLoopData.Rate, false);
	UE_LOG(TerrainGenerator, Log, TEXT("Create vertices done."));
}

bool ATerrainGenerator::CreateVertex(int32 X, int32 Y, float& OutRatioStd, float& OutRatio)
{
	FStructTerrainMeshPointData Data;
	FIntPoint Key(X, Y);
	if (!pGI->TerrainGridPointIndices.Contains(Key)) {
		UE_LOG(TerrainGenerator, Warning, TEXT("X=%d Y=%d not in the TerrainGridPointIndices!"), X, Y);
		return false;
	}
	Data.GridDataIndex = pGI->TerrainGridPointIndices[Key];
	AddVertex(Data, OutRatioStd, OutRatio);
	TerrainMeshPointsData.Add(Data);
	GetZRatioInfo(Data);
	return true;
}

void ATerrainGenerator::AddVertex(FStructTerrainMeshPointData& Data, float& OutRatioStd, float& OutRatio)
{
	FStructGridData GridPoint = pGI->TerrainGridPoints[Data.GridDataIndex];
	float VX = GridPoint.Position2D.X;
	float VY = GridPoint.Position2D.Y;
	float VZ = GetAltitude(GridPoint.AxialCoord.X, GridPoint.AxialCoord.Y, 
		OutRatioStd, OutRatio);
	Data.PositionZ = VZ;
	Data.PositionZRatio = OutRatio;
	Vertices.Add(FVector(VX, VY, VZ));
}

void ATerrainGenerator::GetZRatioInfo(const FStructTerrainMeshPointData& Data)
{
	if (Data.PositionZRatio > 0.0) {
		if (Data.PositionZRatio > ZRatioMax) {
			ZRatioMax = Data.PositionZRatio;
		}
		ZRatioLandSum += Data.PositionZRatio;
		ZRatioLandPointCount++;
	}
	
	if (Data.PositionZRatio < ZRatioMin) {
		ZRatioMin = Data.PositionZRatio;
	}
}

float ATerrainGenerator::GetAltitude(float X, float Y, float& OutRatioStd, float& OutRatio)
{
	float k = 0.5;
	FVector2D slope0;
	OutRatio = GetGradientRatioZ(Noise->NWLandLayer0, X, Y, 
		[this](float X, float Y) { return GetLandLayer0Ratio(X, Y); },
		0.0, 0.2,
		FVector2d(0.0, 0.0), slope0) + GetLandLayer1Ratio(X, Y);

	if (HasWater) {
		float wRatio = GetWaterRatio(X, Y);
		OutRatio = CombineWaterLandRatio(wRatio, OutRatio);
	}
	OutRatio = FMath::Clamp<float>(OutRatio, -1.0, 1.0);
	OutRatioStd = OutRatio * 0.5 + 0.5;
	float z = OutRatio * TileAltitudeMultiplier;
	return z;
}

float ATerrainGenerator::GetGradientRatioZ(UFastNoiseWrapper* NWP, float X, float Y, 
	TFunction<float(float X, float Y)> GetRatioFunc,
	float BaseRatio, float k, 
	const FVector2d& BaseSlope, FVector2d& OutSlope)
{
	float value = GetRatioFunc(X, Y);
	float valueX = GetRatioFunc(X + 1, Y);
	float valueY = GetRatioFunc(X, Y + 1);

	float slopeX = (valueX - value) * TileAltitudeMultiplier / pGI->TerrainGridParam.TileSize;
	float slopeY = (valueY - value) * TileAltitudeMultiplier / pGI->TerrainGridParam.TileSize;
	OutSlope.Set(slopeX + BaseSlope.X, slopeY + BaseSlope.Y);

	float m = OutSlope.Length();
	float Ratio = value / (1.0 + m * k);
	return BaseRatio + Ratio;
}


float ATerrainGenerator::CombineWaterLandRatio(float wRatio, float lRatio)
{
	/*float alpha = lRatio / WaterLandCombineRatio;
	alpha = FMath::Clamp<float>(alpha, 0.0, 1.0);
	float outRatio = FMath::Lerp<float>(wRatio, lRatio, alpha);*/

	float outRatio = wRatio + lRatio;
	outRatio = FMath::Clamp<float>(outRatio, -1.0, 1.0);

	return outRatio;
}

float ATerrainGenerator::GetLandLayer0Ratio(float X, float Y)
{
	FStructHeightMapping mapping;
	MappingByLevel(LandLayer0Level, LandLayer0RangeMapping, mapping);
	return GetMappingHeightRatio(Noise->NWLandLayer0, mapping, X, Y, LandLayer0SampleScale);
}

float ATerrainGenerator::GetLandLayer1Ratio(float X, float Y)
{
	FStructHeightMapping mapping;
	MappingByLevel(LandLayer1Level, LandLayer1RangeMapping, mapping);
	return GetMappingHeightRatio(Noise->NWLandLayer1, mapping, X, Y, LandLayer1SampleScale);
}

float ATerrainGenerator::GetWaterRatio(float X, float Y)
{
	FStructHeightMapping mapping;
	MappingByLevel(WaterLevel, WaterRangeMapping, mapping);
	float ratio = GetMappingHeightRatio(Noise->NWWater, mapping, X, Y, WaterSampleScale);
	ratio = CalWaterBank(ratio);
	return ratio;
}

float ATerrainGenerator::CalWaterBank(float Ratio)
{
	Ratio = Ratio > 0.0 ? 0.0 : Ratio;
	Ratio = FMath::Abs<float>(Ratio);
	float alpha = Ratio * WaterBankSharpness;
	alpha = FMath::Clamp<float>(alpha, 0.0, 1.0);
	float exp = FMath::Lerp<float>(3.0, 1.0, alpha);
	Ratio = -FMath::Pow(Ratio, exp);
	return Ratio;
}

void ATerrainGenerator::MappingByLevel(float level, const FStructHeightMapping& InMapping, FStructHeightMapping& OutMapping)
{
	OutMapping.RangeMin = InMapping.RangeMin + InMapping.RangeMinOffset * level;
	OutMapping.RangeMax = InMapping.RangeMax + InMapping.RangeMaxOffset * level;
	OutMapping.MappingMin = InMapping.MappingMin;
	OutMapping.MappingMax = InMapping.MappingMax;
	OutMapping.RangeMinOffset = InMapping.RangeMinOffset;
	OutMapping.RangeMaxOffset = InMapping.RangeMaxOffset;
}

float ATerrainGenerator::GetMappingHeightRatio(UFastNoiseWrapper* NWP, 
	const FStructHeightMapping& Mapping, float X, float Y, float SampleScale)
{
	float value = 0.0;
	if (NWP != nullptr) {
		value = NWP->GetNoise2D(X * SampleScale, Y * SampleScale);
		return MappingFromRangeToRange(value, Mapping);
	}
	return 0.0f;
}

float ATerrainGenerator::MappingFromRangeToRange(float InputValue, 
	const FStructHeightMapping& Mapping)
{
	return MappingFromRangeToRange(InputValue, Mapping.RangeMax, Mapping.RangeMin, 
		Mapping.MappingMax, Mapping.MappingMin);
}

float ATerrainGenerator::MappingFromRangeToRange(float InputValue, float RangeMax, 
	float RangeMin, float MappingMax, float MappingMin)
{
	float alpha = (RangeMax - FMath::Clamp<float>(InputValue, RangeMin, RangeMax)) / (RangeMax - RangeMin);
	return FMath::Lerp<float>(MappingMax, MappingMin, alpha);
}

void ATerrainGenerator::CreateUV(float X, float Y)
{
	float UVx = X * UVScale;
	float UVy = Y * UVScale;
	UVs.Add(FVector2D(UVx, UVy));
	UV1.Add(FVector2D(1.0, 0.0));
}

float ATerrainGenerator::GetNoise2DStd(UFastNoiseWrapper* NWP, float X, float Y, 
	float SampleScale, float ValueScale)
{
	float value = 0.0;
	if (NWP != nullptr) {
		value = NWP->GetNoise2D(X * SampleScale, Y * SampleScale);
		value = FMath::Clamp<float>(value * ValueScale, -1.0, 1.0);
		value = (value + 1) * 0.5;
	}
	return value;
}

bool ATerrainGenerator::TerrainMeshPointsLoopFunction(TFunction<void()> InitFunc, 
	TFunction<void(int32 LoopIndex)> LoopFunc, 
	FStructLoopData& LoopData, 
	Enum_TerrainGeneratorState State, 
	bool bProgress, float ProgressWeight)
{
	int32 Count = 0;
	TArray<int32> Indices = { 0 };
	bool SaveLoopFlag = false;

	if (InitFunc && !LoopData.HasInitialized) {
		LoopData.HasInitialized = true;
		InitFunc();
	}

	int32 i = LoopData.IndexSaved[0];
	int32 Total = TerrainMeshPointsData.Num();
	for (; i < Total; i++)
	{
		Indices[0] = i;
		FlowControlUtility::SaveLoopData(this, LoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
		if (SaveLoopFlag) {
			return false;
		}
		LoopFunc(i);
		if (bProgress) {
			Progress = ProgressPassed + (float)LoopData.Count / (float)Total * ProgressWeight;
		}
		Count++;
	}

	if (bProgress) {
		ProgressPassed += ProgressWeight;
	}

	FTimerHandle TimerHandle;
	WorkflowState = State;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, LoopData.Rate, false);
	return true;
}

void ATerrainGenerator::ReMappingZ()
{
	if (TerrainMeshPointsLoopFunction([this]() { InitReMappingZ(); },
		[this](int32 i) { ReMappingPointZ(i); },
		ReMappingZLoopData,
		Enum_TerrainGeneratorState::SetBlockLevel,
		true, ProgressWeight_ReMappingZ)) {
		UE_LOG(TerrainGenerator, Log, TEXT("ReMappingZ done."));
	}
}

void ATerrainGenerator::InitReMappingZ()
{
	ZRatioMapping.RangeMin = 0.0;
	ZRatioMapping.RangeMax = ZRatioMax;
	ZRatioMapping.MappingMin = 0.0;
	ZRatioMapping.MappingMax = 1.0;
}

void ATerrainGenerator::ReMappingPointZ(int32 Index)
{
	FStructTerrainMeshPointData& Data = TerrainMeshPointsData[Index];
	if (Data.PositionZRatio > 0) {
		Data.PositionZRatio = MappingFromRangeToRange(Data.PositionZRatio, ZRatioMapping);
		Data.PositionZ = Data.PositionZRatio * TileAltitudeMultiplier;
		Vertices[Index].Z = Data.PositionZ;
	}
}

void ATerrainGenerator::SetBlockLevel()
{
	if (TerrainMeshPointsLoopFunction([this]() { InitSetBlockLevel(); },
		[this](int32 i) { SetBlockLevelByNeighbors(i); },
		SetBlockLevelLoopData,
		Enum_TerrainGeneratorState::SetBlockLevelEx,
		true, ProgressWeight_SetBlockLevel)) {
		UE_LOG(TerrainGenerator, Log, TEXT("SetBlockLevel done."));
	}
}

void ATerrainGenerator::InitSetBlockLevel()
{
	BlockLevelMax = pGI->TerrainGridParam.NeighborRange + 1;
}

bool ATerrainGenerator::SetBlock(FStructTerrainMeshPointData& OutData, 
	const FStructTerrainMeshPointData& InData, int32 BlockLevel)
{
	if (InData.PositionZRatio > AltitudeBlockRatio)
	{
		OutData.BlockLevel = BlockLevel;
		return true;
	}
	return false;
}

void ATerrainGenerator::SetBlockLevelByNeighbors(int32 Index)
{
	FStructTerrainMeshPointData& Data = TerrainMeshPointsData[Index];
	if (SetBlock(Data, Data, 0)) {
		return;
	}
	for (int32 i = 0; i < pGI->TerrainGridPoints[Data.GridDataIndex].Neighbors.Num(); i++)
	{
		if (SetBlockLevelByNeighbor(Data, i)) {
			return;
		}
	}
	Data.BlockLevel = BlockLevelMax;
}

bool ATerrainGenerator::SetBlockLevelByNeighbor(FStructTerrainMeshPointData& Data, int32 Index)
{
	FStructGridDataNeighbors Neighbors = pGI->TerrainGridPoints[Data.GridDataIndex].Neighbors[Index];
	int32 NIndex = 0;
	for (int32 i = 0; i < Neighbors.Points.Num(); i++)
	{
		FIntPoint key = Neighbors.Points[i];
		if (!TerrainMeshPointsIndices.Contains(key)) {
			continue;
		}
		NIndex = TerrainMeshPointsIndices[key];
		if (SetBlock(Data, TerrainMeshPointsData[NIndex], Neighbors.Radius))
		{
			return true;
		}
	}
	return false;
}

void ATerrainGenerator::SetBlockLevelEx()
{
	if (BlockExTimes == 0) {
		FTimerHandle TimerHandle;
		WorkflowState = Enum_TerrainGeneratorState::CreateRiver;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
		UE_LOG(TerrainGenerator, Log, TEXT("Set block level extension done!"));
		return;
	}

	int32 i = SetBlockLevelExLoopData.IndexSaved[0];
	Enum_TerrainGeneratorState state = Enum_TerrainGeneratorState::SetBlockLevelEx;
	for (; i < BlockExTimes; )
	{
		SetBlockLevelExLoopData.IndexSaved[0] = i;
		if (i == (BlockExTimes - 1)) {
			state = Enum_TerrainGeneratorState::CreateRiver;
		}

		if (TerrainMeshPointsLoopFunction([this]() { InitSetBlockLevelEx(); },
			[this](int32 Index) { SetBlockLevelExByNeighbors(Index); },
			BlockLevelExLoopDatas[i],
			state)) {
			Progress = ProgressPassed + (float)(i + 1) / (float)BlockExTimes * ProgressWeight_SetBlockLevelEx;
			UE_LOG(TerrainGenerator, Log, TEXT("Set block level extension %d done!"), i + 1);
			i++;
			SetBlockLevelExLoopData.IndexSaved[0] = i;
			if (i == BlockExTimes) {
				ProgressPassed += ProgressWeight_SetBlockLevelEx;
				break;
			}
		}
		return;
	}
	UE_LOG(TerrainGenerator, Log, TEXT("Set block level extension done!"));
}

void ATerrainGenerator::InitSetBlockLevelEx()
{
	BlockLevelMax += pGI->TerrainGridParam.NeighborRange;
}

void ATerrainGenerator::SetBlockLevelExByNeighbors(int32 Index)
{
	FStructTerrainMeshPointData& Data = TerrainMeshPointsData[Index];
	int32 NeighborRange = pGI->TerrainGridParam.NeighborRange;
	if (Data.BlockLevel == (BlockLevelMax - NeighborRange))
	{
		FStructGridData GridData = pGI->TerrainGridPoints[Data.GridDataIndex];
		FStructGridDataNeighbors OutSideNeighbors = GridData.Neighbors[GridData.Neighbors.Num() - 1];
		int32 BlockLvMin = BlockLevelMax;
		int32 CurrentBlockLv = Data.BlockLevel;
		int32 NIndex = 0;
		for (int32 i = 0; i < OutSideNeighbors.Count; i++)
		{
			FIntPoint key = OutSideNeighbors.Points[i];
			if (!TerrainMeshPointsIndices.Contains(key)) {
				continue;
			}
			NIndex = TerrainMeshPointsIndices[key];
			CurrentBlockLv = NeighborRange + TerrainMeshPointsData[NIndex].BlockLevel;
			if (CurrentBlockLv < BlockLvMin) {
				BlockLvMin = CurrentBlockLv;
			}
		}
		Data.BlockLevel = BlockLvMin;
	}
}

void ATerrainGenerator::CreateRiver()
{
	if (HasRiver) {
		switch (WorkflowState)
		{
		case Enum_TerrainGeneratorState::CreateRiver:
			WorkflowState = Enum_TerrainGeneratorState::AddRiverEndPoints;
		case Enum_TerrainGeneratorState::AddRiverEndPoints:
			AddRiverEndPoints();
			break;
		case Enum_TerrainGeneratorState::DivideUpperRiver:
		case Enum_TerrainGeneratorState::DivideLowerRiver:
			DivideRiverEndPointsIntoChunks();
			break;
		case Enum_TerrainGeneratorState::ChunkToOnePoint:
			RiverChunkToOnePoint();
			break;
		case Enum_TerrainGeneratorState::CreateRiverLine:
			CreateRiverLine();
			break;
		case Enum_TerrainGeneratorState::FindRiverLines:
			FindRiverLines();
			break;
		case Enum_TerrainGeneratorState::DigRiverLine:
			DigRiverLine();
			break;
		case Enum_TerrainGeneratorState::DigRiverPool:
			DigRiverPool();
			break;
		case Enum_TerrainGeneratorState::CombinePoolToTerrain:
			CombinePoolToTerrain();
			break;
		default:
			break;
		}
	}
	else {
		WorkflowState = Enum_TerrainGeneratorState::CreateTriangles;
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
		UE_LOG(TerrainGenerator, Log, TEXT("No river was created."));
	}
}

void ATerrainGenerator::AddRiverEndPoints()
{
	if (TerrainMeshPointsLoopFunction(nullptr,
		[this](int32 i) { AddRiverEndPoint(i); },
		AddRiverEndPointsLoopData,
		Enum_TerrainGeneratorState::DivideUpperRiver,
		true, ProgressWeight_AddRiverEndPoints)) {
		UE_LOG(TerrainGenerator, Log, TEXT("AddRiverEndPoints done."));
	}
}

void ATerrainGenerator::AddRiverEndPoint(int32 Index)
{
	FStructTerrainMeshPointData Data = TerrainMeshPointsData[Index];

	if (Data.PositionZRatio >= UpperRiverLimitZRatio) {
		UpperRiverIndices.Add(TerrainMeshPointsIndices[pGI->TerrainGridPoints[Data.GridDataIndex].AxialCoord]);
	}
	if (Data.PositionZRatio <= LowerRiverLimitZRatio) {
		LowerRiverIndices.Add(TerrainMeshPointsIndices[pGI->TerrainGridPoints[Data.GridDataIndex].AxialCoord]);
	}
}

void ATerrainGenerator::DivideRiverEndPointsIntoChunks()
{
	switch (WorkflowState)
	{
	case Enum_TerrainGeneratorState::DivideUpperRiver:
		DivideUpperRiverEndPointsIntoChunks();
		break;
	case Enum_TerrainGeneratorState::DivideLowerRiver:
		DivideLowerRiverEndPointsIntoChunks();
		break;
	default:
		break;
	}
}

void ATerrainGenerator::DivideUpperRiverEndPointsIntoChunks()
{
	FTimerHandle TimerHandle;
	auto InitFunc = [&]() {
		UpperRiverDivideData.Seed = UpperRiverIndices;
		};
	TArray<int32> Indices = {};
	if (AStarUtility::BFSFCLoopFunction<int32>(this,
		UpperRiverDivideData,
		Indices,
		UpperRiverDivideLoopData,
		WorkflowDelegate,
		InitFunc,
		[this](const int32& Current, int32& Next, int32& Index) { return NextPoint(Current, Next, Index); },
		[this]() { AddUpperRiverEndPointsChunk(); })) {
		WorkflowState = Enum_TerrainGeneratorState::DivideLowerRiver;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, UpperRiverDivideLoopData.Rate, false);
		UE_LOG(TerrainGenerator, Log, TEXT("Divide UpperRiver EndPoints Into Chunks done."));
	}
}

void ATerrainGenerator::DivideLowerRiverEndPointsIntoChunks()
{
	FTimerHandle TimerHandle;
	auto InitFunc = [&]() {
		LowerRiverDivideData.Seed = LowerRiverIndices;
		};
	TArray<int32> Indices = {};
	if (AStarUtility::BFSFCLoopFunction<int32>(this,
		LowerRiverDivideData,
		Indices,
		LowerRiverDivideLoopData,
		WorkflowDelegate,
		InitFunc,
		[this](const int32& Current, int32& Next, int32& Index) { return NextPoint(Current, Next, Index); },
		[this]() { AddLowerRiverEndPointsChunk(); })) {
		WorkflowState = Enum_TerrainGeneratorState::ChunkToOnePoint;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, LowerRiverDivideLoopData.Rate, false);
		UE_LOG(TerrainGenerator, Log, TEXT("Divide LowerRiver EndPoints Into Chunks done."));
	}
}

bool ATerrainGenerator::NextPoint(const int32& Current, int32& Next, int32& Index)
{
	FStructGridDataNeighbors Neighbors = pGI->TerrainGridPoints[TerrainMeshPointsData[Current].GridDataIndex].Neighbors[0];
	if (Index < Neighbors.Points.Num()) {
		FIntPoint key = Neighbors.Points[Index];
		if (TerrainMeshPointsIndices.Contains(key)) {
			Next = TerrainMeshPointsIndices[key];
		}
		Index++;
		return true;
	}
	return false;
}

void ATerrainGenerator::AddUpperRiverEndPointsChunk()
{
	UpperRiverChunks.Add(UpperRiverDivideData.Reached);
}

void ATerrainGenerator::AddLowerRiverEndPointsChunk()
{
	LowerRiverChunks.Add(LowerRiverDivideData.Reached);
}

void ATerrainGenerator::RiverChunkToOnePoint()
{
	ChunkToOnePoint(UpperRiverChunks, UpperRiverEndPoints);
	ChunkToOnePoint(LowerRiverChunks, LowerRiverEndPoints);

	FTimerHandle TimerHandle;
	WorkflowState = Enum_TerrainGeneratorState::CreateRiverLine;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(TerrainGenerator, Log, TEXT("RiverChunkToOnePoint done."));
}

void ATerrainGenerator::ChunkToOnePoint(TArray<TSet<int32>>& Chunk, TArray<int32>& EndPoints)
{
	Chunk.Sort([](const TSet<int32>& A, const TSet<int32>& B) {
		return A.Num() > B.Num();
		});
	for (int32 i = 0; i < Chunk.Num(); i++) {
		TArray<int32> arr = Chunk[i].Array();
		int32 MaxZIndex = 0;
		float MaxZ = 0.0;
		float CurrentZ = 0.0;
		for (int32 j = 0; j < arr.Num(); j++) {
			CurrentZ = FMath::Abs<float>(TerrainMeshPointsData[arr[j]].PositionZ);
			if (CurrentZ > MaxZ) {
				MaxZIndex = arr[j];
				MaxZ = CurrentZ;
			}
		}
		EndPoints.Add(MaxZIndex);
	}
}

void ATerrainGenerator::CreateRiverLine()
{
	CreateRiverLinePointDatas();

	FTimerHandle TimerHandle;
	WorkflowState = Enum_TerrainGeneratorState::FindRiverLines;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(TerrainGenerator, Log, TEXT("CreateRiverLine done."));
}

void ATerrainGenerator::CreateRiverLinePointDatas()
{
	for (int32 i = 0; i < MaxRiverNum; i++) {
		bool find = false;
		int32 j = 0;
		int32 k = 0;
		for (j = 0; j < UpperRiverEndPoints.Num(); j++) {
			for (k = 0; k < LowerRiverEndPoints.Num(); k++) {
				int32 distance = GetPointsDistance(UpperRiverEndPoints[j], LowerRiverEndPoints[k]);
				if (distance >= MinRiverLength) {
					FStructRiverLinePointData data;
					data.UpperPointIndex = UpperRiverEndPoints[j];
					data.LowerPointIndex = LowerRiverEndPoints[k];
					RiverLinePointDatas.Add(data);
					find = true;
					break;
				}
			}
			if (find) {
				break;
			}
		}
		if (find) {
			UpperRiverEndPoints.RemoveAt(j);
			LowerRiverEndPoints.RemoveAt(k);
		}
		else {
			break;
		}
	}
	UE_LOG(TerrainGenerator, Log, TEXT("Add %d river line data."), RiverLinePointDatas.Num());
}

void ATerrainGenerator::FindRiverLines()
{
	TArray<int32> Indices = { 0 };
	int32 i = FindRiverLinesLoopData.IndexSaved[0];
	for (; i < RiverLinePointDatas.Num(); i++) {
		Indices[0] = i;

		if (!FindRiverLinesLoopData.HasInitialized) {
			FindRiverLinesLoopData.HasInitialized = true;
			AStarUtility::ClearAStarData(FindRiverLineData);
			int32 Start = RiverLinePointDatas[i].UpperPointIndex;
			FindRiverLineData.Frontier.Push(Start, 0.0);
			FindRiverLineData.CameFrom.Add(Start, -1);
			FindRiverLineData.CostSoFar.Add(Start, 0.0);
			FindRiverLineData.Goal = RiverLinePointDatas[i].LowerPointIndex;

			CalRiverNoiseSampleRotValue(RiverLinePointDatas.Num(), i);
		}

		if (AStarUtility::AStarSearchLoopFunction<int32>(this,
			FindRiverLineData,
			Indices,
			FindRiverLinesLoopData,
			WorkflowDelegate,
			nullptr,
			[this](const int32& Current, int32& Next, int32& Index) { return NextPoint(Current, Next, Index); },
			[this](const int32& Current, const int32& Next) { return RiverDirectionCost(Current, Next); },
			[this](const int32& Goal, const int32& Next) { return RiverDirectionHeuristic(Goal, Next); })) {
			AStarUtility::ReconstructPath<int32>(FindRiverLineData.Goal,
				RiverLinePointDatas[i].UpperPointIndex, 
				FindRiverLineData.CameFrom,
				RiverLinePointDatas[i].LinePointIndices);
			
			FindRiverLinesLoopData.HasInitialized = false;
			
		}
		else {
			return;
		}
		Progress = ProgressPassed + (float)(i + 1) / (float)RiverLinePointDatas.Num() * ProgressWeight_FindRiverLines;
	}

	ProgressPassed += ProgressWeight_FindRiverLines;

	FTimerHandle TimerHandle;
	WorkflowState = Enum_TerrainGeneratorState::DigRiverLine;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(TerrainGenerator, Log, TEXT("FindRiverLines done."));
}

void ATerrainGenerator::CalRiverNoiseSampleRotValue(int32 Total, int32 Index)
{
	RiverNoiseSampleRotSin = FMath::Sin(2.0 * PI / (float)Total * (float)Index);
	RiverNoiseSampleRotCos = FMath::Cos(2.0 * PI / (float)Total * (float)Index);
}

FVector2D ATerrainGenerator::GetRiverRotatedAxialCoord(FIntPoint AxialCoord)
{
	float X = (float)AxialCoord.X * RiverNoiseSampleRotCos - (float)AxialCoord.Y * RiverNoiseSampleRotSin;
	float Y = (float)AxialCoord.X * RiverNoiseSampleRotSin + (float)AxialCoord.Y * RiverNoiseSampleRotCos;
	return FVector2D(X, Y);
}

float ATerrainGenerator::RiverDirectionCost(const int32& Current, const int32& Next)
{
	FIntPoint AxialCoord = GetPointAxialCoord(Next);
	FVector2D Coord = GetRiverRotatedAxialCoord(AxialCoord);
	float CostN = RiverDirectionNoiseCost(Coord.X, Coord.Y);
	float CostA = RiverDirectionAltitudeCost(Next);
	float Cost = (CostN + CostA) > 0 ? (CostN + CostA) : 0.0;
	return Cost;
}

float ATerrainGenerator::RiverDirectionNoiseCost(float X, float Y)
{
	float Cost = Noise->NWRiverDirection->GetNoise2D(X * RiverDirectionSampleScale,
		Y * RiverDirectionSampleScale);
	Cost = MappingFromRangeToRange(Cost, RiverDirectionMapping);
	Cost *= RiverDirectionNoiseCostScale;
	return Cost;
}

float ATerrainGenerator::RiverDirectionAltitudeCost(int32 Index)
{
	float Cost = 0.0;
	if (TerrainMeshPointsData[Index].PositionZRatio < WaterBaseRatio) {
		Cost = -1.0;
	}
	else {
		Cost = (float)(BlockLevelMax - TerrainMeshPointsData[Index].BlockLevel) / (float)BlockLevelMax;
	}
	Cost = Cost * RiverDirectionAltitudeCostScale;
	return Cost;
}

float ATerrainGenerator::RiverDirectionHeuristic(const int32& Goal, const int32& Next)
{
	return GetPointsDistance(Goal, Next) * RiverDirectionHeuristicRatio;
}

void ATerrainGenerator::DigRiverLine()
{
	TArray<int32> Indices = { 0, 0 };
	int32 Count = 0;
	bool SaveLoopFlag = false;

	int32 i = DigRiverLineLoopData.IndexSaved[0];
	for (; i < RiverLinePointDatas.Num(); i++)
	{
		Indices[0] = i;
		FStructRiverLinePointData LinePointData = RiverLinePointDatas[i];

		int32 j = 0;
		if (!DigRiverLineLoopData.HasInitialized) {
			DigRiverLineLoopData.HasInitialized = true;

			RiverLinePointBlockTestSet.Empty();
			CurrentLineDepthRatio = RiverDepthRatioStart;
		}
		else {
			j = DigRiverLineLoopData.IndexSaved[1];
		}

		
		for (; j < LinePointData.LinePointIndices.Num(); j++)
		{
			Indices[1] = j;

			FlowControlUtility::SaveLoopData(this, DigRiverLineLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
			if (SaveLoopFlag) {
				return;
			}

			CurrentLinePointIndex = LinePointData.LinePointIndices[j];
			float ZRatio = TerrainMeshPointsData[CurrentLinePointIndex].PositionZRatio;
			if (ZRatio > AltitudeBlockRatio) {
				continue;
			}
			
			float BlockZRatioByNeighbor = FindRiverBlockZByNeighbor(CurrentLinePointIndex);
			CurrentLineDepthRatio = CurrentLineDepthRatio > BlockZRatioByNeighbor ? CurrentLineDepthRatio : BlockZRatioByNeighbor;
			UpdateRiverPointZ(CurrentLinePointIndex, CurrentLineDepthRatio);

			UnitLineRisingStep = FMath::Abs(CurrentLineDepthRatio) / RiverDepthRisingStep;

			TStructBFSData<int32> BFSData;
			BFSData.Frontier.Enqueue(CurrentLinePointIndex);
			BFSData.Reached.Add(CurrentLinePointIndex);
			AStarUtility::BFSFunction<int32>(BFSData,
				[this](const int32& Current, int32& Next, int32& Index, TSet<int32>& Reached) { return DigRiverNextPoint(Current, Next, Index, Reached); });

			FIntPoint AxialCoord = GetPointAxialCoord(CurrentLinePointIndex);
			FVector2D Coord = GetRiverRotatedAxialCoord(AxialCoord);
			float DepthNoise = Noise->NWRiverDepth->GetNoise2D(Coord.X * RiverDepthSampleScale,
				Coord.Y * RiverDepthSampleScale);

			if (CurrentLineDepthRatio > RiverDepthRatioMin) {
				CurrentLineDepthRatio -= RiverDepthChangeStep;
			}
			else {
				if (DepthNoise > 0.0) {
					CurrentLineDepthRatio += RiverDepthChangeStep;
					if (CurrentLineDepthRatio > RiverDepthRatioMin) {
						CurrentLineDepthRatio = RiverDepthRatioMin;
					}
				}
				else {
					CurrentLineDepthRatio -= RiverDepthChangeStep;
					if (CurrentLineDepthRatio < RiverDepthRatioMax) {
						CurrentLineDepthRatio = RiverDepthRatioMax;
					}
				}
			}

			Count++;
			Progress = ProgressPassed + (float)DigRiverLineLoopData.Count / (float)GetTotalRiverLinePointNum() * ProgressWeight_DigRiverLine;

		}
		DigRiverLineLoopData.HasInitialized = false;
	}

	ProgressPassed += ProgressWeight_DigRiverLine;

	FTimerHandle TimerHandle;
	WorkflowState = Enum_TerrainGeneratorState::DigRiverPool;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(TerrainGenerator, Log, TEXT("DigRiverLine done."));
}

int32 ATerrainGenerator::GetTotalRiverLinePointNum()
{
	int32 Num = 0;
	for (int32 i = 0; i < RiverLinePointDatas.Num(); i++)
	{
		Num += RiverLinePointDatas[i].LinePointIndices.Num();
	}
	return Num;
}

float ATerrainGenerator::FindRiverBlockZByNeighbor(int32 Index)
{
	float BlockZRatio = -1.0;
	FStructGridDataNeighbors Neighbors = pGI->TerrainGridPoints[TerrainMeshPointsData[Index].GridDataIndex].Neighbors[0];
	for (int32 i = 0; i < Neighbors.Points.Num(); i++) {
		FIntPoint key = Neighbors.Points[i];
		if (TerrainMeshPointsIndices.Contains(key)) {
			int32 NIndex = TerrainMeshPointsIndices[key];
			float zRatio = TerrainMeshPointsData[NIndex].PositionZRatio;
			float NBlockZRatio = TerrainMeshPointsData[NIndex].RiverBlockZRatio;
			if (zRatio > AltitudeBlockRatio) {
				BlockZRatio = 0.0;
				if (!RiverLinePointBlockTestSet.Contains(Index)) {
					RiverLinePointBlockTestSet.Add(Index);
				}
				break;
			}else if (RiverLinePointBlockTestSet.Contains(NIndex)) {
				if (NBlockZRatio > BlockZRatio) {
					if (!RiverLinePointBlockTestSet.Contains(Index)) {
						RiverLinePointBlockTestSet.Add(Index);
					}
					BlockZRatio = NBlockZRatio;
				}
			}
		}
	}
	BlockZRatio -= RiverDepthChangeStep;
	BlockZRatio = BlockZRatio > 0.0 ? 0.0 : BlockZRatio;
	TerrainMeshPointsData[Index].RiverBlockZRatio = BlockZRatio;
	return BlockZRatio;
}

void ATerrainGenerator::UpdateRiverPointZ(int32 Index, float ZRatio)
{
	if (TerrainMeshPointsData[Index].PositionZRatio > ZRatio) {
		TerrainMeshPointsData[Index].PositionZRatio = ZRatio;
		TerrainMeshPointsData[Index].PositionZ = ZRatio * TileAltitudeMultiplier;
		Vertices[Index].Z = TerrainMeshPointsData[Index].PositionZ;
	}
}

bool ATerrainGenerator::DigRiverNextPoint(const int32& Current, int32& Next, 
	int32& Index, TSet<int32>& Reached)
{
	int32 dis = GetPointsDistance(CurrentLinePointIndex, Current);
	float diff = UnitLineRisingStep - (float)dis - 1.0;
	if (diff > 0) {
		float X = diff / UnitLineRisingStep;
		float ZRatio = X < 0.5 ? FMath::Pow(X, 5.0) * 16.0 : 1 - FMath::Pow(-2.0 * X + 2.0, 5.0) / 2.0;
		ZRatio *= CurrentLineDepthRatio;
		FStructGridDataNeighbors Neighbors = pGI->TerrainGridPoints[TerrainMeshPointsData[Current].GridDataIndex].Neighbors[0];
		if (Index < Neighbors.Points.Num()) {
			FIntPoint key = Neighbors.Points[Index];
			if (TerrainMeshPointsIndices.Contains(key)) {
				Next = TerrainMeshPointsIndices[key];
				float Z = TerrainMeshPointsData[Next].PositionZRatio;
				if (Z > AltitudeBlockRatio) {
					if (!Reached.Contains(Next)) {
						Reached.Add(Next);
					}
				}
				else {
					float BlockZRatioByNeighbor = FindRiverBlockZByNeighbor(Next);
					ZRatio = ZRatio > BlockZRatioByNeighbor ? ZRatio : BlockZRatioByNeighbor;
					UpdateRiverPointZ(Next, ZRatio);
				}
			}
			Index++;
			return true;
		}
	}
	return false;
}

void ATerrainGenerator::DigRiverPool()
{
	if (!HasRiverPool) {
		ProgressPassed += ProgressWeight_DigRiverPool;
		FTimerHandle TimerHandle;
		WorkflowState = Enum_TerrainGeneratorState::CreateVertexColorsForAMTB;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
		UE_LOG(TerrainGenerator, Log, TEXT("DigRiverPool done."));
	}
	
	TArray<int32> Indices = { 0, 0 };
	int32 Count = 0;
	bool SaveLoopFlag = false;

	int32 i = DigRiverPoolLoopData.IndexSaved[0];
	for (; i < RiverLinePointDatas.Num(); i++)
	{
		Indices[0] = i;
		FStructRiverLinePointData LinePointData = RiverLinePointDatas[i];

		int32 j = 0;
		if (!DigRiverPoolLoopData.HasInitialized) {
			DigRiverPoolLoopData.HasInitialized = true;
			CurrentLineDepthRatio = RiverDepthRatioStart;
		}
		else {
			j = DigRiverPoolLoopData.IndexSaved[1];
		}
		
		for (; j < LinePointData.LinePointIndices.Num(); j++)
		{
			Indices[1] = j;
			FlowControlUtility::SaveLoopData(this, DigRiverPoolLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
			if (SaveLoopFlag) {
				return;
			}
			
			CurrentLinePointIndex = LinePointData.LinePointIndices[j];
			float ZRatio = TerrainMeshPointsData[CurrentLinePointIndex].PositionZRatio;
			if (ZRatio < RiverPoolCombineLower) {
				break;
			}

			UpdateRiverPoolZ(CurrentLinePointIndex, CurrentLineDepthRatio);
			UnitLineRisingStep = FMath::Abs(CurrentLineDepthRatio) / RiverPoolDepthRisingStep;
			
			TStructBFSData<int32> BFSData;
			BFSData.Frontier.Enqueue(CurrentLinePointIndex);
			BFSData.Reached.Add(CurrentLinePointIndex);
			AStarUtility::BFSFunction<int32>(BFSData,
				[this](const int32& Current, int32& Next, int32& Index, TSet<int32>& Reached) { return DigRiverPoolNextPoint(Current, Next, Index, Reached); });

			FIntPoint AxialCoord = GetPointAxialCoord(CurrentLinePointIndex);
			FVector2D Coord = GetRiverRotatedAxialCoord(AxialCoord);
			float DepthNoise = Noise->NWRiverDepth->GetNoise2D(Coord.X * RiverDepthSampleScale,
				Coord.Y * RiverDepthSampleScale);

			if (CurrentLineDepthRatio > RiverPoolDepthRatioMin) {
				CurrentLineDepthRatio -= RiverDepthChangeStep;
			}
			else {
				if (DepthNoise > 0.0) {
					CurrentLineDepthRatio += RiverDepthChangeStep;
					if (CurrentLineDepthRatio > RiverPoolDepthRatioMin) {
						CurrentLineDepthRatio = RiverPoolDepthRatioMin;
					}
				}
				else {
					CurrentLineDepthRatio -= RiverDepthChangeStep;
					if (CurrentLineDepthRatio < RiverPoolDepthRatioMax) {
						CurrentLineDepthRatio = RiverPoolDepthRatioMax;
					}
				}
			}
			Count++;
		}
		Progress = ProgressPassed + (float)(i + 1) / (float)RiverLinePointDatas.Num() * ProgressWeight_DigRiverPool;
		DigRiverPoolLoopData.HasInitialized = false;
	}

	ProgressPassed += ProgressWeight_DigRiverPool;

	FTimerHandle TimerHandle;
	WorkflowState = Enum_TerrainGeneratorState::CombinePoolToTerrain;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(TerrainGenerator, Log, TEXT("DigRiverPool done."));
}

void ATerrainGenerator::UpdateRiverPoolZ(int32 Index, float ZRatio)
{
	if (TerrainMeshPointsData[Index].RiverPoolZRatio > ZRatio) {
		TerrainMeshPointsData[Index].RiverPoolZRatio = ZRatio;
	}
}

bool ATerrainGenerator::DigRiverPoolNextPoint(const int32& Current, int32& Next, int32& Index, TSet<int32>& Reached)
{
	int32 dis = GetPointsDistance(CurrentLinePointIndex, Current);
	float diff = UnitLineRisingStep - (float)dis - 1.0;
	if (diff > 0) {
		float X = diff / UnitLineRisingStep;
		float ZRatio = X < 0.5 ? FMath::Pow(X, 5.0) * 16.0 : 1 - FMath::Pow(-2.0 * X + 2.0, 5.0) / 2.0;
		ZRatio *= CurrentLineDepthRatio;
		FStructGridDataNeighbors Neighbors = pGI->TerrainGridPoints[TerrainMeshPointsData[Current].GridDataIndex].Neighbors[0];
		if (Index < Neighbors.Points.Num()) {
			FIntPoint key = Neighbors.Points[Index];
			if (TerrainMeshPointsIndices.Contains(key)) {
				Next = TerrainMeshPointsIndices[key];
				UpdateRiverPoolZ(Next, ZRatio);
			}
			Index++;
			return true;
		}
	}
	return false;
}

void ATerrainGenerator::CombinePoolToTerrain()
{
	for (int32 i = 0; i < TerrainMeshPointsData.Num(); i++)
	{
		float PoolZ = TerrainMeshPointsData[i].RiverPoolZRatio;
		float Z = TerrainMeshPointsData[i].PositionZRatio;
		if (PoolZ < 0) {
			float alpha = 0.0;
			if (Z > RiverPoolCombineRatio) {
				alpha = MappingFromRangeToRange(Z,
					RiverPoolCombineUpper, RiverPoolCombineRatio, 1.0, 0.0);
			}
			else {
				alpha = MappingFromRangeToRange(Z,
					RiverPoolCombineRatio, RiverPoolCombineLower, 0.0, 1.0);
			}

			TerrainMeshPointsData[i].PositionZRatio = FMath::Lerp<float>(PoolZ, Z, alpha);
			TerrainMeshPointsData[i].PositionZ = TerrainMeshPointsData[i].PositionZRatio * TileAltitudeMultiplier;
			Vertices[i].Z = TerrainMeshPointsData[i].PositionZ;
		}
	}

	FTimerHandle TimerHandle;
	WorkflowState = Enum_TerrainGeneratorState::CreateVertexColorsForAMTB;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(TerrainGenerator, Log, TEXT("CombinePoolToTerrain done."));
}

FIntPoint ATerrainGenerator::GetPointAxialCoord(int32 Index)
{
	return pGI->TerrainGridPoints[TerrainMeshPointsData[Index].GridDataIndex].AxialCoord;
}

FVector2D ATerrainGenerator::GetPointPosition2D(int32 Index)
{
	return pGI->TerrainGridPoints[TerrainMeshPointsData[Index].GridDataIndex].Position2D;
}

FVector ATerrainGenerator::GetPointPosition(int32 Index)
{
	FVector2D pos2D = GetPointPosition2D(Index);
	float posZ = TerrainMeshPointsData[Index].PositionZ;
	return FVector(pos2D.X, pos2D.Y, posZ);
}

int32 ATerrainGenerator::GetPointsDistance(int32 Index1, int32 Index2)
{
	Quad quad1(GetPointAxialCoord(Index1));
	Quad quad2(GetPointAxialCoord(Index2));
	return Quad::Distance(quad1, quad2);
}

void ATerrainGenerator::CreateVertexColorsForAMTB()
{
	if (TerrainMeshPointsLoopFunction([this]() { InitCreateVertexColorsForAMTB(); },
		[this](int32 i) { AddAMTBToVertexColor(i); },
		CreateVertexColorsForAMTBLoopData,
		Enum_TerrainGeneratorState::CreateTriangles,
		true, ProgressWeight_CreateVertexColorsForAMTB)) {
		UE_LOG(TerrainGenerator, Log, TEXT("CreateVertexColorsForAMTB done."));
	}
}

void ATerrainGenerator::InitCreateVertexColorsForAMTB()
{
}

//Add vertex Color(R:Altidude G:Moisture B:Temperature A:Biomes)
void ATerrainGenerator::AddAMTBToVertexColor(int32 Index)
{
	float ZRatio = TerrainMeshPointsData[Index].PositionZRatio;
	float ZRatioStd = ZRatio * 0.5 + 0.5;
	float X = GetPointAxialCoord(Index).X;
	float Y = GetPointAxialCoord(Index).Y;
	float Moisture = CalMoisture(X, Y);
	float Temperature = CalTemperature(X, Y);
	float Tree = CalTree(X, Y);
	VertexColors.Add(FLinearColor(ZRatioStd, Moisture, Temperature, Tree));
	
}

float ATerrainGenerator::CalMoisture(int32 X, int32 Y)
{
	float Moisture = GetNoise2DStd(Noise->NWMoisture, X, Y, MoistureSampleScale, MoistureValueScale);
	float WaterNoise = Noise->NWWater->GetNoise2D(X * WaterSampleScale, Y * WaterSampleScale);
	WaterNoise *= MoistureValueScale;
	WaterNoise = FMath::Clamp(WaterNoise, -1.0, 1.0);
	FIntPoint key(X, Y);
	int32 Index = TerrainMeshPointsIndices[key];
	float ZRatio = TerrainMeshPointsData[Index].PositionZRatio;
	ZRatio *= MoistureZRatioScale;
	ZRatio = FMath::Clamp(ZRatio, -1.0, 0.0);
	
	float offset = WaterNoise + ZRatio;
	offset = FMath::Clamp(offset, -1.0, 1.0);
	offset = (offset + 1) * 0.5;
	offset = 1.0 - offset;

	Moisture = (Moisture + offset) * 0.5;
	Moisture = FMath::Clamp(Moisture, 0.0, 1.0);

	return Moisture;
}

float ATerrainGenerator::CalTemperature(int32 X, int32 Y)
{
	float Temperature = GetNoise2DStd(Noise->NWTemperature, X, Y, TemperatureSampleScale, TemperatureValueScale);
	float Layer0Noise = Noise->NWLandLayer0->GetNoise2D(X * LandLayer0SampleScale, Y * LandLayer0SampleScale);
	Layer0Noise = FMath::Clamp(Layer0Noise, 0.0, 1.0);
	float scale = 1.0 - Layer0Noise;
	Temperature *= scale;
	Temperature = FMath::Clamp(Temperature, 0.0, 1.0);

	return Temperature;
}

void ATerrainGenerator::CreateTriangles()
{
	int32 Count = 0;
	TArray<int32> Indices = { 0 };
	bool SaveLoopFlag = false;

	if (!CreateTrianglesLoopData.HasInitialized) {
		CreateTrianglesLoopData.HasInitialized = true;
		StepTotalCount = TerrainMeshPointsData.Num();
	}

	int32 i = CreateTrianglesLoopData.IndexSaved[0];
	TArray<int32> SqVArr = {};

	for (; i < StepTotalCount; i++) {
		Indices[0] = i;
		FlowControlUtility::SaveLoopData(this, CreateTrianglesLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
		if (SaveLoopFlag) {
			return;
		}

		FindTopRightSquareVertices(i, SqVArr, TerrainMeshPointsIndices, GridRange);
		CreatePairTriangles(SqVArr, Triangles);
		Progress = ProgressPassed + (float)CreateTrianglesLoopData.Count / (float)StepTotalCount * ProgressWeight_CreateTriangles;
		Count++;
	}
	ProgressPassed += ProgressWeight_CreateTriangles;
	WorkflowState = Enum_TerrainGeneratorState::CalNormalsInit;
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, CreateTrianglesLoopData.Rate, false);
	UE_LOG(TerrainGenerator, Log, TEXT("Create triangles done."));
}

void ATerrainGenerator::FindTopRightSquareVertices(int32 Index, 
	TArray<int32>& SqVArr, const TMap<FIntPoint, int32>& Indices, 
	int32 RangeLimit)
{
	SqVArr.Add(Index);
	int32 Range = pGI->TerrainGridPoints[Index].RangeFromCenter;
	bool flag = Range < RangeLimit - 2;

	FIntPoint point = pGI->TerrainGridPoints[Index].AxialCoord;
	point = FIntPoint(point.X + 1, point.Y);
	if (flag || Indices.Contains(point)) {
		SqVArr.Add(Indices[point]);
	}
	point = FIntPoint(point.X, point.Y + 1);
	if (flag || Indices.Contains(point)) {
		SqVArr.Add(Indices[point]);
	}
	point = FIntPoint(point.X - 1, point.Y);
	if (flag || Indices.Contains(point)) {
		SqVArr.Add(Indices[point]);
	}
}

void ATerrainGenerator::CreatePairTriangles(TArray<int32>& SqVArr, 
	TArray<int32>& TrianglesArr)
{
	if (SqVArr.Num() == 4) {
		TArray<int32> VIndices = { 0, 0, 0, 0, 0, 0 };
		VIndices[0] = SqVArr[0];
		VIndices[1] = SqVArr[2];
		VIndices[2] = SqVArr[1];
		VIndices[3] = SqVArr[0];
		VIndices[4] = SqVArr[3];
		VIndices[5] = SqVArr[2];
		TrianglesArr.Append(VIndices);
	}
	else if (SqVArr.Num() == 3) {
		TArray<int32> VIndices = { 0, 0, 0 };
		VIndices[0] = SqVArr[0];
		VIndices[1] = SqVArr[2];
		VIndices[2] = SqVArr[1];
		TrianglesArr.Append(VIndices);
	}
	SqVArr.Empty();
}

void ATerrainGenerator::CreateNormals()
{
	CalNormalsWorkflow();
}

void ATerrainGenerator::CalNormalsWorkflow()
{
	switch (WorkflowState)
	{
	case Enum_TerrainGeneratorState::CalNormalsInit:
		CalNormalsInit();
		break;
	case Enum_TerrainGeneratorState::CalNormalsAcc:
		CalNormalsAcc();
		break;
	case Enum_TerrainGeneratorState::NormalizeNormals:
		NormalizeNormals();
		break;
	default:
		break;
	}
}

void ATerrainGenerator::CalNormalsInit()
{
	int32 Count = 0;
	TArray<int32> Indices = { 0 };
	bool SaveLoopFlag = false;

	if (!CalNormalsInitLoopData.HasInitialized) {
		CalNormalsInitLoopData.HasInitialized = true;
		StepTotalCount = Vertices.Num();
	}
	int32 i = CalNormalsInitLoopData.IndexSaved[0];
	for (; i < Vertices.Num(); i++) {
		Indices[0] = i;
		FlowControlUtility::SaveLoopData(this, CalNormalsInitLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
		if (SaveLoopFlag) {
			return;
		}
		NormalsAcc.Add(FVector(0, 0, 0));
		Progress = ProgressPassed + (float)CalNormalsInitLoopData.Count / (float)StepTotalCount * ProgressWeight_CalNormalsInit;
		Count++;
	}
	ProgressPassed += ProgressWeight_CalNormalsInit;

	FTimerHandle TimerHandle;
	WorkflowState = Enum_TerrainGeneratorState::CalNormalsAcc;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, CalNormalsInitLoopData.Rate, false);
	UE_LOG(TerrainGenerator, Log, TEXT("Calculate normals initialization done."));
}

void ATerrainGenerator::CalNormalsAcc()
{
	int32 Count = 0;
	TArray<int32> Indices = { 0 };
	bool SaveLoopFlag = false;

	if (!CalNormalsAccLoopData.HasInitialized) {
		CalNormalsAccLoopData.HasInitialized = true;
		StepTotalCount = Triangles.Num() / 3;
	}

	int32 i = CalNormalsAccLoopData.IndexSaved[0];
	int32 last = Triangles.Num() / 3 - 1;
	for (; i <= last; i++) {
		Indices[0] = i;
		FlowControlUtility::SaveLoopData(this, CalNormalsAccLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
		if (SaveLoopFlag) {
			return;
		}
		CalTriangleNormalForVertex(i);
		Progress = ProgressPassed + (float)CalNormalsAccLoopData.Count / (float)StepTotalCount * ProgressWeight_CalNormalsAcc;
		Count++;
	}
	ProgressPassed += ProgressWeight_CalNormalsAcc;

	FTimerHandle TimerHandle;
	WorkflowState = Enum_TerrainGeneratorState::NormalizeNormals;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, CalNormalsAccLoopData.Rate, false);
	UE_LOG(TerrainGenerator, Log, TEXT("Calculate normals accumulation done."));
}

void ATerrainGenerator::CalTriangleNormalForVertex(int32 TriangleIndex)
{
	int32 Index3Times = TriangleIndex * 3;
	int32 Index1 = Triangles[Index3Times];
	int32 Index2 = Triangles[Index3Times + 1];
	int32 Index3 = Triangles[Index3Times + 2];

	FVector Normal = FVector::CrossProduct(Vertices[Index1] - Vertices[Index2], Vertices[Index3] - Vertices[Index2]);

	FVector N1 = NormalsAcc[Index1] + Normal;
	NormalsAcc[Index1] = N1;

	FVector N2 = NormalsAcc[Index2] + Normal;
	NormalsAcc[Index2] = N2;

	FVector N3 = NormalsAcc[Index3] + Normal;
	NormalsAcc[Index3] = N3;
}

void ATerrainGenerator::NormalizeNormals()
{
	int32 Count = 0;
	TArray<int32> Indices = { 0 };
	bool SaveLoopFlag = false;

	if (!NormalizeNormalsLoopData.HasInitialized) {
		NormalizeNormalsLoopData.HasInitialized = true;
		StepTotalCount = NormalsAcc.Num();
	}

	int32 i = NormalizeNormalsLoopData.IndexSaved[0];
	int32 last = NormalsAcc.Num() - 1;
	for (; i <= last; i++) {
		Indices[0] = i;
		FlowControlUtility::SaveLoopData(this, NormalizeNormalsLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
		if (SaveLoopFlag) {
			return;
		}
		AddNormal(i);
		CalAngleToUp(i);
		Progress = ProgressPassed + (float)NormalizeNormalsLoopData.Count / (float)StepTotalCount * ProgressWeight_NormalizeNormals;
		Count++;
	}
	ProgressPassed += ProgressWeight_NormalizeNormals;

	FTimerHandle TimerHandle;
	WorkflowState = Enum_TerrainGeneratorState::DrawLandMesh;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, NormalizeNormalsLoopData.Rate, false);
	UE_LOG(TerrainGenerator, Log, TEXT("Normalize normals done."));
}

void ATerrainGenerator::AddNormal(int32 Index)
{
	NormalsAcc[Index].Normalize();
	TerrainMeshPointsData[Index].Normal = NormalsAcc[Index];
	Normals.Add(TerrainMeshPointsData[Index].Normal);
}

float ATerrainGenerator::AngleBetweenVectors(const FVector& A, const FVector& B)
{
	return acosf(FVector::DotProduct(A, B));
}

void ATerrainGenerator::CalAngleToUp(int32 Index)
{
	FVector UpVector(0.0, 0.0, 1.0);
	TerrainMeshPointsData[Index].AngleToUp = AngleBetweenVectors(UpVector, Normals[Index]);
}

void ATerrainGenerator::CreateWaterfall()
{
	if (HasWaterfall) {
		CreateWaterfallPlanes();
		//CreateWaterfallAnim();
		if (WaterfallMistClass) {
			AddWaterfallMist();
		}
	}

	FTimerHandle TimerHandle;
	WorkflowState = Enum_TerrainGeneratorState::Done;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(TerrainGenerator, Log, TEXT("Create waterfall done."));
}

void ATerrainGenerator::CreateWaterfallAnim()
{
	if (HasWaterfall && WaterfallClass) {
		FindWaterfallPoints();
		AddWaterfall();
	}
}

void ATerrainGenerator::CreateWaterfallPlanes()
{
	for (int32 i = 0; i < RiverLinePointDatas.Num(); i++)
	{
		InitCreateWaterfall();
		CreateNewWaterfallRender(i);
		CreateWaterfallPlane(i);
	}
}

void ATerrainGenerator::InitCreateWaterfall()
{
	CurrentWaterfallRadius = WaterfallRadiusMin;
}

void ATerrainGenerator::CreateNewWaterfallRender(int32 Index)
{
	FStructWaterfallRenderData Data;
	FString NameStr;
	NameStr.Append(TEXT("WaterfallMesh")).Append(FString::FromInt(Index));

	Data.WaterfallMesh = NewObject<UProceduralMeshComponent>(this, FName(*NameStr));
	Data.WaterfallMesh->SetupAttachment(TerrainMesh);
	Data.WaterfallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Data.WaterfallMesh->RegisterComponent();
	WaterfallRenderDatas.Add(Data);
}

void ATerrainGenerator::CreateWaterfallPlane(int32 Index)
{
	CreateWaterfallVerticesAndNormals(Index);
	CreateWaterfallTriangles(Index);
	CreateWaterfallMesh(Index);
	SetWaterfallMaterial(Index);
}

void ATerrainGenerator::CreateWaterfallVerticesAndNormals(int32 Index)
{
	FStructRiverLinePointData& Data = RiverLinePointDatas[Index];
	Data.HasWaterfall = true;
	float UVUnit = UVScale * WaterRange / GridRange;
	float RetFlag = false;

	for (int32 i = 0; i < Data.LinePointIndices.Num(); i++)
	{
		int32 PointIndex = Data.LinePointIndices[i];
		
		FVector OverPoint = GetWaterfallOverPoint(i, Data);
		FVector Normal = TerrainMeshPointsData[PointIndex].Normal;
		if (i + 1 < Data.LinePointIndices.Num()) {
			FVector NextOverPoint = GetWaterfallOverPoint(i + 1, Data);
			FVector ToNext = NextOverPoint - OverPoint;
			float ToNextLen = ToNext.Length();
			ToNext.Normalize();
			if (i > 0) {
				FVector PreOverPoint = GetWaterfallOverPoint(i - 1, Data);
				FVector ToThis = OverPoint - PreOverPoint;
				ToThis.Normalize();
				ToNext = ToNext + ToThis;
				ToNext.Normalize();
				if (OverPoint.Z < WaterBase && PreOverPoint.Z > WaterBase) {
					GetWaterPointByLineTrance(PreOverPoint, OverPoint, Data.WaterfallEnterWaterPoint);
				}
			}
			FVector VertexDir = FVector::CrossProduct(Normal, ToNext);
			VertexDir.Normalize();

			FVector ToV = CurrentWaterfallRadius * VertexDir;
			FVector ToV0 = ToV.RotateAngleAxisRad(0.0, ToNext);
			FVector ToV1 = ToV0.RotateAngleAxisRad(PI / 2.0, ToNext);
			FVector ToV2 = ToV1.RotateAngleAxisRad(PI / 2.0, ToNext);
			FVector ToV3 = ToV2.RotateAngleAxisRad(PI / 2.0, ToNext);

			FVector V0 = ToV0 + OverPoint;
			FVector V1 = ToV1 * 0.2 + OverPoint;
			FVector V2 = ToV2 + OverPoint;
			FVector V3 = ToV3 * 0.2 + OverPoint;

			WaterfallRenderDatas[Index].WaterfallVertices.Add(V0);
			WaterfallRenderDatas[Index].WaterfallVertices.Add(V1);
			WaterfallRenderDatas[Index].WaterfallVertices.Add(V2);
			WaterfallRenderDatas[Index].WaterfallVertices.Add(V3);

			ToV0.Normalize();
			ToV1.Normalize();
			ToV2.Normalize();
			ToV3.Normalize();
			WaterfallRenderDatas[Index].WaterfallNormals.Add(ToV0);
			WaterfallRenderDatas[Index].WaterfallNormals.Add(ToV1);
			WaterfallRenderDatas[Index].WaterfallNormals.Add(ToV2);
			WaterfallRenderDatas[Index].WaterfallNormals.Add(ToV3);

			float perimeter = 4.0 * FMath::Pow(2.0, 0.5) * CurrentWaterfallRadius;
			float AngleRad = AngleBetweenVectors(FVector(0.0, 0.0, -1.0), ToNext);
			float SinValue = FMath::Sin(AngleRad);
			float HValue = ToNextLen * SinValue;

			WaterfallRenderDatas[Index].WaterfallUVs.Add(FVector2D(0.0, (V0.Z + HValue) / perimeter));
			WaterfallRenderDatas[Index].WaterfallUVs.Add(FVector2D(0.25, (V1.Z + HValue) / perimeter));
			WaterfallRenderDatas[Index].WaterfallUVs.Add(FVector2D(0.5, (V2.Z + HValue) / perimeter));
			WaterfallRenderDatas[Index].WaterfallUVs.Add(FVector2D(0.75, (V3.Z + HValue) / perimeter));
		
			if (V0.Z < WaterBase &&
				V1.Z < WaterBase &&
				V2.Z < WaterBase &&
				V3.Z < WaterBase) {
				Data.WaterfallRadius = CurrentWaterfallRadius;
				break;
			}
		}
		
		CurrentWaterfallRadius = FMath::Min(CurrentWaterfallRadius + WaterfallRadiusStep, WaterfallRadiusMax);
	}
}

FVector ATerrainGenerator::GetWaterfallOverPoint(int32 Index, const FStructRiverLinePointData& Data)
{
	int32 PointIndex = Data.LinePointIndices[Index];
	FVector OverPoint = GetPointPosition(PointIndex);
	OverPoint += TerrainMeshPointsData[PointIndex].Normal * WaterfallPlainOverTerrain;
	return OverPoint;
}

void ATerrainGenerator::CreateWaterfallTriangles(int32 Index)
{
	for (int32 i = 0; i < WaterfallRenderDatas[Index].WaterfallVertices.Num() - 4; i += 4)
	{
		for (int32 j = 0; j < 4; j++)
		{
			WaterfallRenderDatas[Index].WaterfallTriangles.Add(i + j);
			WaterfallRenderDatas[Index].WaterfallTriangles.Add(i + j + 4);
			WaterfallRenderDatas[Index].WaterfallTriangles.Add(i + (j + 1) % 4);
			WaterfallRenderDatas[Index].WaterfallTriangles.Add(i + (j + 1) % 4);
			WaterfallRenderDatas[Index].WaterfallTriangles.Add(i + j + 4);
			WaterfallRenderDatas[Index].WaterfallTriangles.Add(i + (j + 1) % 4 + 4);
			
		}
	}
}

void ATerrainGenerator::CreateWaterfallMesh(int32 Index)
{
	WaterfallRenderDatas[Index].WaterfallMesh->CreateMeshSection_LinearColor(
		0, 
		WaterfallRenderDatas[Index].WaterfallVertices, 
		WaterfallRenderDatas[Index].WaterfallTriangles, 
		WaterfallRenderDatas[Index].WaterfallNormals, 
		WaterfallRenderDatas[Index].WaterfallUVs,
		TArray<FLinearColor>(), TArray<FProcMeshTangent>(), true);

}

void ATerrainGenerator::SetWaterfallMaterial(int32 Index)
{
	WaterfallRenderDatas[Index].WaterfallMesh->SetMaterial(0, WaterfallMaterialIns);
}

void ATerrainGenerator::FindWaterfallPoints()
{
	for (int32 i = 0; i < RiverLinePointDatas.Num(); i++)
	{
		FindWaterfallPoint(i);
	}
}
void ATerrainGenerator::FindWaterfallPoint(int32 Index)
{
	FStructRiverLinePointData& Data = RiverLinePointDatas[Index];
	if (FindEnterWaterPoint(Data)) {
		int32 FirstIndex = Data.LinePointIndices[0];
		FVector FirstPoint = GetPointPosition(FirstIndex);
		float Height = FirstPoint.Z - WaterBase;
		Height *= WaterfallHeightRatio;
		Data.WaterfallHeight = Height;
		FVector TraceStart(Data.WaterfallEnterWaterPoint.X, Data.WaterfallEnterWaterPoint.Y, Height + WaterBase);
		FVector TraceLine = FVector(FirstPoint.X, FirstPoint.Y, 0.0) - FVector(TraceStart.X, TraceStart.Y, 0.0);
		FVector TestLoc;
		float TestAngle = PI / 2.0;
		TraceWaterfallPoint(TraceStart, TraceLine, TestLoc, TestAngle);
		Data.WaterfallTraceStart = TraceStart;
		Data.WaterfallTraceEnd = TraceStart + TraceLine;

		FVector Up(0.0, 0.0, 1.0);
		for (int32 i = 1; i <= WaterfallTraceStep; i++)
		{
			FVector Line = TraceLine.RotateAngleAxisRad(WaterfallTraceDeltaAngle * i, Up);
			TraceWaterfallPoint(TraceStart, Line, TestLoc, TestAngle);
			Line = TraceLine.RotateAngleAxisRad(-WaterfallTraceDeltaAngle * i, Up);
			TraceWaterfallPoint(TraceStart, Line, TestLoc, TestAngle);
		}
		if (TestAngle < WaterfallToleranceAngle) {
			Data.HasWaterfall = true;
			Data.WaterfallPoint = TestLoc;
			Data.WaterfallDirection = TraceStart - TestLoc;
			Data.WaterfallLength = Data.WaterfallDirection.Length();
			Data.WaterfallDirection.Normalize();
		}
	}
}

bool ATerrainGenerator::FindEnterWaterPoint(FStructRiverLinePointData& Data)
{
	int32 UnderWaterCount = 0;
	bool Ret = false;
	for (int32 i = 0; i < Data.LinePointIndices.Num(); i++)
	{
		int32 Index = Data.LinePointIndices[i];
		if (TerrainMeshPointsData[Index].PositionZRatio > WaterBaseRatio) {
			UnderWaterCount = 0;
			continue;
		}
		UnderWaterCount++;
		if (UnderWaterCount >= WaterfallPoolUnderWaterCountLimit) {
			FVector2D Pos2D = GetPointPosition2D(Index);
			Data.WaterfallEnterWaterPoint = FVector(Pos2D.X, Pos2D.Y, WaterBase);
			Ret = true;
			break;
		}
	}
	return Ret;
}

void ATerrainGenerator::TraceWaterfallPoint(FVector TraceStart, FVector TraceLine,
	FVector& TestLoc, float& TestAngle)
{
	FVector TraceEnd = TraceStart + TraceLine;
	FVector Loc;
	if (GetTerrainPointByLineTrace(TraceStart, TraceEnd, Loc)) {
		Quad quad = Quad::PosToQuad(FVector2D(Loc.X, Loc.Y), TileSizeMultiplier);
		FIntPoint key(quad.GetCoord().X, quad.GetCoord().Y);
		if (TerrainMeshPointsIndices.Contains(key)) {
			int32 Index = TerrainMeshPointsIndices[key];
			FVector Normal = TerrainMeshPointsData[Index].Normal;
			FVector UpVec(0.0, 0.0, 1.0);
			float AngleNor2Up = AngleBetweenVectors(UpVec, Normal);
			FVector RotationAxis = FVector::CrossProduct(Normal, UpVec);
			RotationAxis.Normalize();
			Normal = Normal.RotateAngleAxisRad(AngleNor2Up - PI / 2.0, RotationAxis);
			Normal.Normalize();

			FVector Dir = -TraceLine;
			Dir.Normalize();
			float Angle = FMath::Abs<float>(AngleBetweenVectors(Normal, Dir));
			if (Angle < TestAngle) {
				TestAngle = Angle;
				TestLoc.Set(Loc.X, Loc.Y, Loc.Z);
			}
		}
	}
}

void ATerrainGenerator::AddWaterfall()
{
	for (int32 i = 0; i < RiverLinePointDatas.Num(); i++)
	{
		FStructRiverLinePointData& Data = RiverLinePointDatas[i];
		if (!Data.HasWaterfall) {
			continue;
		}
		FVector Loc = Data.WaterfallPoint;
		FVector Scale(1.0, 1.0, 1.0);
		FRotator Rot(0.0, 0.0, 0.0);

		float Angle = AngleBetweenVectors(WaterfallOriDirection, Data.WaterfallDirection);
		FVector RotationAxis = FVector::CrossProduct(WaterfallOriDirection, Data.WaterfallDirection);
		RotationAxis.Normalize();
		if (RotationAxis.Equals(FVector(0.0, 0.0, 0.0))) {
			RotationAxis.Set(0.0, 0.0, -1.0);
		}
		FQuat Quat = FQuat(RotationAxis, Angle);
		FTransform Trans(Quat.Rotator(), Loc, Scale);
		Data.Waterfall = Cast<ATerrainWaterfall>(GetWorld()->SpawnActor(WaterfallClass, &Trans));

		float Gravity = 980;
		float FreefallTime = FMath::Sqrt(Data.WaterfallHeight * 2.0 / Gravity);
		float Velocity = Data.WaterfallLength / FreefallTime;
		Data.Waterfall->SetParamLifeTimeScale(FreefallTime + WaterfallFreefallTimeOffset);
		Data.Waterfall->SetParamVelocityScale(FVector(0.0, Velocity, 0.0));
		Data.Waterfall->SetActive(true);

	}
}

void ATerrainGenerator::AddWaterfallMist()
{
	for (int32 i = 0; i < RiverLinePointDatas.Num(); i++)
	{
		FStructRiverLinePointData Data = RiverLinePointDatas[i];
		if (!Data.HasWaterfall) {
			continue;
		}
		FVector Scale(1.0, 1.0, 1.0);
		FRotator Rot(0.0, 0.0, 0.0);
		FVector Loc = Data.WaterfallEnterWaterPoint;
		FTransform Trans(Rot, Loc, Scale);
		Data.WaterfallMist = Cast<ATerrainWaterfallMist>(GetWorld()->SpawnActor(WaterfallMistClass, &Trans));
		Data.WaterfallMist->SetParamWaterfallRadius(FVector2D(Data.WaterfallRadius, Data.WaterfallRadius));
		Data.WaterfallMist->SetActive(true);
	}
}

void ATerrainGenerator::CreateWater()
{
	if (HasWater) {
		CreateWaterPlane();
		if (HasCaustics) {
			CreateCaustics();
		}
	}

	UE_LOG(TerrainGenerator, Log, TEXT("Create water done."));
}

void ATerrainGenerator::CreateWaterPlane()
{
	CreateWaterVerticesAndUVs();
	CreateWaterTriangles();
	CreateWaterNormals();
	CreateWaterMesh();
	SetWaterMaterial();
}

void ATerrainGenerator::CreateWaterVerticesAndUVs()
{
	UKismetMaterialLibrary::SetScalarParameterValue(this, TerrainMPC, TEXT("WaterBase"),
		WaterBase);
	float WaterTileMultiplier = TileSizeMultiplier * (float)GridRange / (float)WaterRange;
	float UVUnit = UVScale / WaterRange;
	
	int32 X = 0;
	int32 Y = 0;
	StepTotalCount = 1 + (QUAD_SIDE_NUM + WaterRange * QUAD_SIDE_NUM) * WaterRange / 2;
	for (int32 i = 0; i < StepTotalCount; i++) {
		X = pGI->TerrainGridPoints[i].AxialCoord.X;
		Y = pGI->TerrainGridPoints[i].AxialCoord.Y;
		WaterMeshPointsIndices.Add(FIntPoint(X, Y), i);

		WaterVertices.Add(FVector(X * WaterTileMultiplier, Y * WaterTileMultiplier, WaterBase));
		WaterUVs.Add(FVector2D(X * UVUnit, Y * UVUnit));
	}
}

void ATerrainGenerator::CreateWaterTriangles()
{
	StepTotalCount = WaterVertices.Num();
	TArray<int32> SqVArr = {};
	for (int32 i = 0; i < StepTotalCount; i++)
	{
		FindTopRightSquareVertices(i, SqVArr, WaterMeshPointsIndices, WaterRange);
		CreatePairTriangles(SqVArr, WaterTriangles);
	}
}

void ATerrainGenerator::CreateWaterNormals()
{
	int32 i;
	for (i = 0; i <= WaterVertices.Num() - 1; i++) {
		WaterNormals.Add(FVector(0, 0, 1.0));
	}
}

void ATerrainGenerator::CreateWaterMesh()
{
	WaterMesh->CreateMeshSection_LinearColor(0, WaterVertices, WaterTriangles, WaterNormals, WaterUVs,
		TArray<FLinearColor>(), TArray<FProcMeshTangent>(), true);
}

void ATerrainGenerator::SetWaterMaterial()
{
	WaterMesh->SetMaterial(0, WaterMaterialIns);
}

void ATerrainGenerator::CreateCaustics()
{
	float base = UKismetMaterialLibrary::GetScalarParameterValue(this, TerrainMPC, TEXT("WaterBase"));
	float sink = (base - WaterRangeMapping.MappingMin * TileAltitudeMultiplier) / 2.0;
	FVector size(TerrainSize / 1.5, TerrainSize / 1.5, sink + 1);
	FVector location(0, 0, base - sink - 1);
	FRotator rotator(0.0, 45.0, 0.0);

	UGameplayStatics::SpawnDecalAtLocation(this, CausticsMaterialIns, size, location, rotator);
}

void ATerrainGenerator::CreateTerrainMesh()
{
	TerrainMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, UV1, UV2, UV3,
		VertexColors, TArray<FProcMeshTangent>(), true);
	TerrainMesh->bUseComplexAsSimpleCollision = true;
	TerrainMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	TerrainMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	TerrainMesh->bUseComplexAsSimpleCollision = false;

	UE_LOG(TerrainGenerator, Log, TEXT("Create terrain mesh done."));
}

void ATerrainGenerator::SetTerrainMaterial()
{
	TerrainMesh->SetMaterial(0, TerrainMaterialIns);
}

void ATerrainGenerator::DoWorkflowDone()
{
	Progress = 1.0;
}

// Called every frame
void ATerrainGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ATerrainGenerator::GetMeshPointByLineTrance(UProceduralMeshComponent* Mesh, FVector Start, FVector End, FVector& Loc)
{
	if (Mesh) {
		FHitResult result;
		FCollisionQueryParams params;
		bool isHit = Mesh->LineTraceComponent(result, Start, End, params);
		if (isHit) {
			Loc.Set(result.Location.X, result.Location.Y, result.Location.Z);
		}
		return isHit;
	}
	return false;
}

bool ATerrainGenerator::GetTerrainPointByLineTrace(FVector Start, FVector End, FVector& Loc)
{
	return GetMeshPointByLineTrance(TerrainMesh, Start, End, Loc);
}

bool ATerrainGenerator::GetTerrainPointBy2DPos(FVector2D Start2D, FVector2D End2D, FVector& Loc)
{
	FVector Start(Start2D.X, Start2D.Y, TileAltitudeMax);
	FVector End(End2D.X, End2D.Y, -TileAltitudeMax);
	return GetTerrainPointByLineTrace(Start, End, Loc);
}

bool ATerrainGenerator::GetWaterPointByLineTrance(FVector Start, FVector End, FVector& Loc)
{
	return GetMeshPointByLineTrance(WaterMesh, Start, End, Loc);
}

Enum_TerrainType ATerrainGenerator::GetTerrainType(FVector2D Point, float& OutMoisture, float& OutTemperature)
{
	Enum_TerrainType TT = Enum_TerrainType::None;
	OutMoisture = 0.0;
	OutTemperature = 0.0;
	Quad quad = Quad::PosToQuad(Point, TileSizeMultiplier);
	float X = quad.GetCoord().X;
	float Y = quad.GetCoord().Y;
	FIntPoint key(X, Y);
	if (TerrainMeshPointsIndices.Contains(key)) {
		int32 Index = TerrainMeshPointsIndices[key];
		float ZRatio = TerrainMeshPointsData[Index].PositionZRatio;
		float Moisture = CalMoisture(X, Y);
		float Temperature = CalTemperature(X, Y);
		OutMoisture = Moisture;
		OutTemperature = Temperature;

		if (ZRatio > 0.001) {
			TT = Enum_TerrainType::Mountain;
		}
		else if (ZRatio < WaterBaseRatio && ZRatio > ShallowWaterRatio) {
			TT = Enum_TerrainType::ShallowWater;
		}
		else if (ZRatio <= ShallowWaterRatio) {
			TT = Enum_TerrainType::DeepWater;
		}
		else {
			TT = GetPlainType(Moisture, Temperature);
		}
	}

	return TT;
}

Enum_TerrainType ATerrainGenerator::GetPlainType(float Moisture, float Temperature)
{
	int32 MoistureStep = GetScalarStep(Moisture, MoistureBlendThresholdLow, MoistureBlendThresholdHigh);
	int32 TemperatureStep = GetScalarStep(Temperature, TempratureBlendThresholdLow, TempratureBlendThresholdHigh);
	
	Enum_TerrainType TT = Enum_TerrainType::Grass;

	switch (TemperatureStep)
	{
	case -1:
		switch (MoistureStep)
		{
		case -1:
			TT = Enum_TerrainType::Gobi;
			break;
		case 0:
			TT = Enum_TerrainType::Tundra;
			break;
		case 1:
			TT = Enum_TerrainType::Snow;
			break;
		}
		break;
	case 0:
		switch (MoistureStep)
		{
		case -1:
			TT = Enum_TerrainType::Desert;
			break;
		case 0:
			TT = Enum_TerrainType::Grass;
			break;
		case 1:
			TT = Enum_TerrainType::Coast;
			break;
		}
		break;
	case 1:
		switch (MoistureStep)
		{
		case -1:
			TT = Enum_TerrainType::Lava;
			break;
		case 0:
			TT = Enum_TerrainType::DryGrass;
			break;
		case 1:
			TT = Enum_TerrainType::Swamp;
			break;
		}
		break;
	}
	
	return TT;
}

int32 ATerrainGenerator::GetScalarStep(float Scalar, float Lower, float Upper)
{
	int32 Ret = 0;
	if (Scalar <= Lower) {
		Ret = -1;
	}
	else if (Scalar > Upper) {
		Ret = 1;
	}
	return Ret;
}

bool ATerrainGenerator::HasTreeAt(const FVector2D& Point)
{
	bool Ret = false;
	Quad quad = Quad::PosToQuad(Point, TileSizeMultiplier);
	float X = quad.GetCoord().X;
	float Y = quad.GetCoord().Y;
	FIntPoint key(X, Y);
	if (TerrainMeshPointsIndices.Contains(key)) {
		float TreeValue = CalTree(X, Y);
		Ret = (1.0 - TreeValue) < TreeRange;
	}
	return Ret;
}

float ATerrainGenerator::GetTreeDensity(Enum_TerrainType TT)
{
	if (TypeToTreeDensity.Contains(TT)) {
		return TypeToTreeDensity[TT];
	}
	return 0.0;
}

float ATerrainGenerator::CalTree(int32 X, int32 Y)
{
	return GetNoise2DStd(Noise->NWTree, X, Y, TreeSampleScale, TreeValueScale);
}

void ATerrainGenerator::GetDebugRiverLineEndPoints(TArray<FVector>& Points)
{
	for (int32 i = 0; i < RiverLinePointDatas.Num(); i++) {
		FVector2D pos1 = GetPointPosition2D(RiverLinePointDatas[i].UpperPointIndex);
		FVector2D pos2 = GetPointPosition2D(RiverLinePointDatas[i].LowerPointIndex);
		Points.Add(FVector(pos1.X, pos1.Y, TerrainMeshPointsData[RiverLinePointDatas[i].UpperPointIndex].PositionZ));
		Points.Add(FVector(pos2.X, pos2.Y, TerrainMeshPointsData[RiverLinePointDatas[i].LowerPointIndex].PositionZ));
	}
}

void ATerrainGenerator::GetDebugRiverLinePointsAt(TArray<FVector>& LinePoints, int32 Index)
{
	if (Index < RiverLinePointDatas.Num()) {
		FStructRiverLinePointData Data = RiverLinePointDatas[Index];
		for (int32 i = 0; i < Data.LinePointIndices.Num(); i++)
		{
			FVector2D pos = GetPointPosition2D(Data.LinePointIndices[i]);
			LinePoints.Add(FVector(pos.X, pos.Y, TerrainMeshPointsData[Data.LinePointIndices[i]].PositionZ));
		}
	}
}

bool ATerrainGenerator::GetDebugWaterfallTracePointAt(FVector& TraceStart, FVector& TraceEnd, int32 Index)
{
	if (Index < RiverLinePointDatas.Num())
	{
		FStructRiverLinePointData Data = RiverLinePointDatas[Index];
		if (!Data.HasWaterfall) {
			return false;
		}
		TraceStart.Set(Data.WaterfallTraceStart.X, Data.WaterfallTraceStart.Y, Data.WaterfallTraceStart.Z);
		TraceEnd.Set(Data.WaterfallTraceEnd.X, Data.WaterfallTraceEnd.Y, Data.WaterfallTraceEnd.Z);
	}
	return true;
}

