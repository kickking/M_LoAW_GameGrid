// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainGenerator.h"
#include "M_LoAW_GameBase/Public/LoAWGameInstance.h"
#include "TerrainNoise.h"
#include "ProceduralMeshComponent.h"
#include "M_LoAW_GridData/Public/FlowControlUtility.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "M_LoAW_GridData/Public/Quad.h"
#include "AStarUtility.h"

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
	WaterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
	case Enum_TerrainGeneratorState::SetBlockLevel:
		SetBlockLevel();
		break;
	case Enum_TerrainGeneratorState::SetBlockLevelEx:
		SetBlockLevelEx();
		break;
	case Enum_TerrainGeneratorState::CreateRiver:
	case Enum_TerrainGeneratorState::DivideUpperRiver:
	case Enum_TerrainGeneratorState::DivideLowerRiver:
	case Enum_TerrainGeneratorState::ChunkToOnePoint:
	case Enum_TerrainGeneratorState::CreateRiverLine:
	case Enum_TerrainGeneratorState::DigRiverLine:
		CreateRiver();
		break;
	case Enum_TerrainGeneratorState::CreateTriangles:
		CreateTriangles();
		break;
	case Enum_TerrainGeneratorState::CalNormalsInit:
	case Enum_TerrainGeneratorState::CalNormalsAcc:
	case Enum_TerrainGeneratorState::NormalizeNormals:
		CreateNormals();
		break;
	case Enum_TerrainGeneratorState::CreateWater:
		CreateWater();
		WorkflowState = Enum_TerrainGeneratorState::DrawLandMesh;
	case Enum_TerrainGeneratorState::DrawLandMesh:
		CreateTerrainMesh();
		SetTerrainMaterial();
		WorkflowState = Enum_TerrainGeneratorState::Done;
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
	InitBaseRatio();
	InitWater();
	InitProgress();

	WorkflowState = Enum_TerrainGeneratorState::CreateVertices;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
}

bool ATerrainGenerator::GetGameInstance()
{
	UWorld* world = GetWorld();
	if (world) {
		pGI = Cast<ULoAWGameInstance>(world->GetGameInstance());
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
		CausticsMaterialIns == nullptr) {
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

	FlowControlUtility::InitLoopData(SetBlockLevelLoopData);
	FlowControlUtility::InitLoopData(SetBlockLevelExLoopData);
	InitBlockLevelExLoopDatas();

	FlowControlUtility::InitLoopData(UpperRiverDivideLoopData);
	FlowControlUtility::InitLoopData(LowerRiverDivideLoopData);

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

void ATerrainGenerator::InitBaseRatio()
{
	if (HasWater) {
		UKismetMaterialLibrary::SetScalarParameterValue(this, TerrainMPC, TEXT("LavaBaseRatio"),
			LavaBaseRatio);
	}
	else {
		UKismetMaterialLibrary::SetScalarParameterValue(this, TerrainMPC, TEXT("LavaBaseRatio"),
			-2.0);
	}
	UKismetMaterialLibrary::SetScalarParameterValue(this, TerrainMPC, TEXT("DesertBaseRatio"),
		DesertBaseRatio);
	UKismetMaterialLibrary::SetScalarParameterValue(this, TerrainMPC, TEXT("SwampBaseRatio"),
		SwampBaseRatio);
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
		CreateVertexColorsForAMTB(RatioStd, X, Y);

		Progress = ProgressPassed + (float)CreateVerticesLoopData.Count / (float)StepTotalCount * ProgressWeight_CreateVertices;
		Count++;
	}

	ProgressPassed += ProgressWeight_CreateVertices;

	WorkflowState = Enum_TerrainGeneratorState::SetBlockLevel;
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, CreateVerticesLoopData.Rate, false);
	UE_LOG(TerrainGenerator, Log, TEXT("Create vertices and UVs done."));
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
	if (HasRiver) {
		AddRiverEndPoint(Data);
	}
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

void ATerrainGenerator::AddRiverEndPoint(const FStructTerrainMeshPointData& Data)
{
	if (Data.PositionZ >= TileAltitudeMultiplier * UpperRiverLimitZRatio) {
		UpperRiverIndices.Add(TerrainMeshPointsIndices[pGI->TerrainGridPoints[Data.GridDataIndex].AxialCoord]);
	}
	if (Data.PositionZ <= TileAltitudeMultiplier * LowerRiverLimitZRatio) {
		LowerRiverIndices.Add(TerrainMeshPointsIndices[pGI->TerrainGridPoints[Data.GridDataIndex].AxialCoord]);
	}
}

float ATerrainGenerator::GetAltitude(float X, float Y, float& OutRatioStd, float& OutRatio)
{
	OutRatio = GetHighMountainRatio(X, Y) + GetLowMountianRatio(X, Y);
	if (HasWater) {
		float wRatio = GetWaterRatio(X, Y);
		OutRatio = CombineWaterLandRatio(wRatio, OutRatio);
	}
	OutRatioStd = OutRatio * 0.5 + 0.5;
	float z = OutRatio * TileAltitudeMultiplier;
	return z;
}

float ATerrainGenerator::CombineWaterLandRatio(float wRatio, float lRatio)
{
	float alpha = 1.0;
	if (WaterBaseRatio < 0) {
		alpha = 1 - wRatio / WaterBaseRatio;
	}
	alpha = FMath::Clamp<float>(alpha, 0.0, 1.0);
	float outRatio = wRatio + FMath::Lerp<float>(wRatio, lRatio, alpha);
	return outRatio;
}

float ATerrainGenerator::GetHighMountainRatio(float X, float Y)
{
	FStructHeightMapping mapping;
	MappingByLevel(HighMountainLevel, HighRangeMapping, mapping);
	return GetMappingHeightRatio(Noise->NWHighMountain, mapping, X, Y, HighMountainSampleScale);
}

float ATerrainGenerator::GetLowMountianRatio(float X, float Y)
{
	FStructHeightMapping mapping;
	MappingByLevel(LowMountainLevel, LowRangeMapping, mapping);
	return GetMappingHeightRatio(Noise->NWLowMountain, mapping, X, Y, LowMountainSampleScale);
}

float ATerrainGenerator::GetWaterRatio(float X, float Y)
{
	FStructHeightMapping mapping;
	MappingByLevel(WaterLevel, WaterRangeMapping, mapping);
	float ratio = GetMappingHeightRatio(Noise->NWWater, mapping, X, Y, WaterSampleScale);

	//calculate water bank
	/*ratio = ratio > 0.0 ? 0.0 : ratio;
	ratio = FMath::Abs<float>(ratio);
	float alpha = ratio * WaterBankSharpness;
	alpha = FMath::Clamp<float>(alpha, 0.0, 1.0);
	float exp = FMath::Lerp<float>(3.0, 1.0, alpha);
	ratio = -FMath::Pow(ratio, exp);*/

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
	float alpha = (Mapping.RangeMax - FMath::Clamp<float>(InputValue, Mapping.RangeMin, Mapping.RangeMax)) / (Mapping.RangeMax - Mapping.RangeMin);
	return FMath::Lerp<float>(Mapping.MappingMax, Mapping.MappingMin, alpha);
}

void ATerrainGenerator::CreateUV(float X, float Y)
{
	float UVx = X * UVScale;
	float UVy = Y * UVScale;
	UVs.Add(FVector2D(UVx, UVy));
	UV1.Add(FVector2D(1.0, 0.0));
}

//Create vertex Color(R:Altidude G:Moisture B:Temperature A:Biomes)
void ATerrainGenerator::CreateVertexColorsForAMTB(float RatioStd, float X, float Y)
{
	float Moisture = GetNoise2DStd(Noise->NWMoisture, X, Y, MoistureSampleScale, MoistureValueScale);
	float Temperature = GetNoise2DStd(Noise->NWTemperature, X, Y, TemperatureSampleScale, TemperatureValueScale);
	float Biomes = GetNoise2DStd(Noise->NWBiomes, X, Y, BiomesSampleScale, BiomesValueScale);
	VertexColors.Add(FLinearColor(RatioStd, Moisture, Temperature, Biomes));
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
	for (; i < BlockExTimes; i++)
	{
		SetBlockLevelExLoopData.IndexSaved[0] = i;
		if (i == (BlockExTimes - 1)) {
			state = Enum_TerrainGeneratorState::CreateRiver;
		}

		if (TerrainMeshPointsLoopFunction([this]() { InitSetBlockLevelEx(); },
			[this](int32 i) { SetBlockLevelExByNeighbors(i); },
			BlockLevelExLoopDatas[i],
			state)) {
			Progress = ProgressPassed + (float)(i + 1) / (float)BlockExTimes * ProgressWeight_SetBlockLevelEx;
			UE_LOG(TerrainGenerator, Log, TEXT("Set block level extension %d done!"), i + 1);
		}
		else {
			return;
		}
	}
	ProgressPassed += ProgressWeight_SetBlockLevelEx;
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
			WorkflowState = Enum_TerrainGeneratorState::DivideUpperRiver;
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
		case Enum_TerrainGeneratorState::DigRiverLine:
			DigRiverLine();
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
	if (AStarUtility::BFSFCLoopFunction<int32>(this,
		UpperRiverDivideData,
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
	if (AStarUtility::BFSFCLoopFunction<int32>(this,
		LowerRiverDivideData,
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
		for (int32 j = 0; j < arr.Num(); j++) {
			float z1 = FMath::Abs<float>(TerrainMeshPointsData[arr[j]].PositionZ);
			float z2 = FMath::Abs<float>(TerrainMeshPointsData[MaxZIndex].PositionZ);
			if (z1 > z2) {
				MaxZIndex = arr[j];
			}
		}
		EndPoints.Add(MaxZIndex);
	}
}

void ATerrainGenerator::CreateRiverLine()
{
	CreateRiverLinePointDatas();
	FindRiverLines();

	FTimerHandle TimerHandle;
	WorkflowState = Enum_TerrainGeneratorState::DigRiverLine;
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
	for (int32 i = 0; i < RiverLinePointDatas.Num(); i++) {
		TStructAStarData<int32> Data;
		int32 Start = RiverLinePointDatas[i].UpperPointIndex;
		Data.Frontier.Push(Start, 0.0);
		Data.CameFrom.Add(Start, -1);
		Data.CostSoFar.Add(Start, 0.0);
		Data.Goal = RiverLinePointDatas[i].LowerPointIndex;

		CalRiverNoiseSampleRotValue(RiverLinePointDatas.Num(), i);

		AStarUtility::AStarSearchFunction<int32>(Data,
			[this](const int32& Current, int32& Next, int32& Index) { return NextPoint(Current, Next, Index); },
			[this](const int32& Current, const int32& Next) { return RiverDirectionCost(Current, Next); },
			[this](const int32& Goal, const int32& Next) { return RiverDirectionHeuristic(Goal, Next); });

		AStarUtility::ReconstructPath<int32>(Data.Goal, Start, Data.CameFrom,
			RiverLinePointDatas[i].LinePointIndices);
	}
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
	for (int32 i = 0; i < RiverLinePointDatas.Num(); i++)
	{
		FStructRiverLinePointData LinePointData = RiverLinePointDatas[i];
		RiverLinePointBlockTestSet.Empty();
		CurrentLineDepthRatio = RiverDepthRatioStart;
		for (int32 j = 0; j < LinePointData.LinePointIndices.Num(); j++)
		{
			CurrentLinePointIndex = LinePointData.LinePointIndices[j];
			float ZRatio = TerrainMeshPointsData[CurrentLinePointIndex].PositionZRatio;
			if (ZRatio > 0.0) {
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
		}
	}

	FTimerHandle TimerHandle;
	WorkflowState = Enum_TerrainGeneratorState::CreateTriangles;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(TerrainGenerator, Log, TEXT("DigRiverLine done."));
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

FIntPoint ATerrainGenerator::GetPointAxialCoord(int32 Index)
{
	return pGI->TerrainGridPoints[TerrainMeshPointsData[Index].GridDataIndex].AxialCoord;
}

FVector2D ATerrainGenerator::GetPointPosition(int32 Index)
{
	return pGI->TerrainGridPoints[TerrainMeshPointsData[Index].GridDataIndex].Position2D;
}

int32 ATerrainGenerator::GetPointsDistance(int32 Index1, int32 Index2)
{
	Quad quad1(GetPointAxialCoord(Index1));
	Quad quad2(GetPointAxialCoord(Index2));
	return Quad::Distance(quad1, quad2);
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
		Progress = ProgressPassed + (float)NormalizeNormalsLoopData.Count / (float)StepTotalCount * ProgressWeight_NormalizeNormals;
		Count++;
	}
	ProgressPassed += ProgressWeight_NormalizeNormals;

	FTimerHandle TimerHandle;
	WorkflowState = Enum_TerrainGeneratorState::CreateWater;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, NormalizeNormalsLoopData.Rate, false);
	UE_LOG(TerrainGenerator, Log, TEXT("Normalize normals done."));
}

void ATerrainGenerator::AddNormal(int32 Index)
{
	NormalsAcc[Index].Normalize();
	TerrainMeshPointsData[Index].Normal = NormalsAcc[Index];
	Normals.Add(TerrainMeshPointsData[Index].Normal);
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

void ATerrainGenerator::GetDebugRiverLineEndPoints(TArray<FVector>& Points)
{
	for (int32 i = 0; i < RiverLinePointDatas.Num(); i++) {
		FVector2D pos1 = GetPointPosition(RiverLinePointDatas[i].UpperPointIndex);
		FVector2D pos2 = GetPointPosition(RiverLinePointDatas[i].LowerPointIndex);
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
			FVector2D pos = GetPointPosition(Data.LinePointIndices[i]);
			LinePoints.Add(FVector(pos.X, pos.Y, TerrainMeshPointsData[Data.LinePointIndices[i]].PositionZ));
		}
	}
}

void ATerrainGenerator::GetDebugPriorityQueue(TArray<int32>& Values)
{
	PriorityQueue<int32> Queue = {};
	Queue.Push(0, 1000.1);
	Queue.Push(1, 10.1);
	Queue.Push(2, 1000.2);
	Queue.Push(3, 10.2);
	Queue.Push(4, 1000.3);
	Queue.Push(5, -1.3);
	Queue.Push(6, -1000.3);

	while (!Queue.IsEmpty()) {
		Values.Add(Queue.Pop());
	}
}


