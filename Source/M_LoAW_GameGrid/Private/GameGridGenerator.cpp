// Fill out your copyright notice in the Description page of Project Settings.


#include "GameGridGenerator.h"
#include "M_LoAW_Terrain/Public/TerrainGenerator.h"
#include "M_LoAW_GridData/Public/Hex.h"
#include "M_LoAW_GridData/Public/HexGridCreator.h"
#include "M_LoAW_GridData/Public/GridDataGameInstance.h"
#include "M_LoAW_GameGrid/Public/GameGridTerrainTypeTree.h"
#include "M_LoAW_GameGrid/Public/GameGridTreeGenerator.h"

#include <Components/InstancedStaticMeshComponent.h>
#include <Kismet/GameplayStatics.h>
#include <Kismet/KismetMathLibrary.h>
#include <Math/UnrealMathUtility.h>

DEFINE_LOG_CATEGORY(GameGridGenerator);

AGameGridGenerator::AGameGridGenerator() : pGI(nullptr), pTG(nullptr)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	GridInstMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GridInstMesh"));
	this->SetRootComponent(GridInstMesh);
	GridInstMesh->SetMobility(EComponentMobility::Static);
	GridInstMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MouseOverInstMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("MouseOverInstMesh"));
	MouseOverInstMesh->SetupAttachment(RootComponent);
	MouseOverInstMesh->SetMobility(EComponentMobility::Static);
	MouseOverInstMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BindDelegate();
}

// Called when the game starts or when spawned
void AGameGridGenerator::BeginPlay()
{
	Super::BeginPlay();
	if (!bUseGrid) {
		WorkflowState = Enum_GameGridGeneratorState::Done;
	}
	else {
		WorkflowState = Enum_GameGridGeneratorState::InitWorkflow;
	}
	
	DoWorkFlow();
}

void AGameGridGenerator::BindDelegate()
{
	WorkflowDelegate.BindUFunction(Cast<UObject>(this), TEXT("DoWorkFlow"));
}

void AGameGridGenerator::DoWorkFlow()
{
	switch (WorkflowState)
	{
	case Enum_GameGridGeneratorState::InitWorkflow:
		InitWorkflow();
		break;
	case Enum_GameGridGeneratorState::CreateGridPoints:
		CreateGridPoints();
		break;
	case Enum_GameGridGeneratorState::WaitTerrain:
		WaitTerrain();
		break;
	case Enum_GameGridGeneratorState::SetGridPosZ:
		SetGridPosZ();
		break;
	case Enum_GameGridGeneratorState::CalGridNormal:
		CalGridNormal();
		break;
	case Enum_GameGridGeneratorState::SetGridTT:
		SetGridTT();
		break;
	case Enum_GameGridGeneratorState::SetGridTTEdge:
		SetGridTTEdge();
		break;
	case Enum_GameGridGeneratorState::AddTreeInstances:
		AddTreeInstances();
		break;
	case Enum_GameGridGeneratorState::SetGridAreaBlockLevel:
		SetGridAreaBlockLevel();
		break;
	case Enum_GameGridGeneratorState::SetGridAreaBlockLevelEx:
		SetGridAreaBlockLevelEx();
		break;
	case Enum_GameGridGeneratorState::CheckAreaConnection:
	case Enum_GameGridGeneratorState::InitCheckAreaConnection:
	case Enum_GameGridGeneratorState::BreakMABToChunk:
	case Enum_GameGridGeneratorState::CheckChunksAreaConnection:
		CheckAreaConnection();
		break;
	case Enum_GameGridGeneratorState::FindGridIsland:
		FindGridIsland();
		break;
	case Enum_GameGridGeneratorState::SetGridBuildingBlockLevel:
		SetGridBuildingBlockLevel();
		break;
	case Enum_GameGridGeneratorState::SetGridBuildingBlockLevelEx:
		SetGridBuildingBlockLevelEx();
		break;
	case Enum_GameGridGeneratorState::SetGridFlyingBlockLevel:
		SetGridFlyingBlockLevel();
		break;
	case Enum_GameGridGeneratorState::SetGridFlyingBlockLevelEx:
		SetGridFlyingBlockLevelEx();
		break;
	case Enum_GameGridGeneratorState::FindGridFlyingIsland:
		FindGridFlyingIsland();
		break;
	case Enum_GameGridGeneratorState::AddGridInstances:
		AddGridInstances();
		break;
	case Enum_GameGridGeneratorState::Done:
		DoWorkflowDone();
		UE_LOG(GameGridGenerator, Log, TEXT("Create game grid done."));
		break;
	case Enum_GameGridGeneratorState::Error:
		UE_LOG(GameGridGenerator, Warning, TEXT("DoWorkFlow Error!"));
		break;
	default:
		break;
	}
}

void AGameGridGenerator::InitWorkflow()
{
	FTimerHandle TimerHandle;
	if (!GetGameInstance() || !InitTree()) {
		WorkflowState = Enum_GameGridGeneratorState::Error;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
		return;
	}

	InitProgress();
	InitLoopData();

	WorkflowState = Enum_GameGridGeneratorState::CreateGridPoints;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
}

bool AGameGridGenerator::GetGameInstance()
{
	UWorld* world = GetWorld();
	if (world) {
		pGI = Cast<UGridDataGameInstance>(world->GetGameInstance());
		if (pGI && pGI->hasGameGridLoaded) {
			return true;
		}
	}
	UE_LOG(GameGridGenerator, Warning, TEXT("GetGameInstance Error!"));
	return false;
}

bool AGameGridGenerator::InitTree()
{
	if (TreeGenerator) {
		TreeGenerator->CreateTerrainTypeTrees();
		return true;
	}
	UE_LOG(GameGridGenerator, Warning, TEXT("Set AGameGridTreeGenerator first!"));
	return false;
}

void AGameGridGenerator::InitProgress()
{
	ProgressPassed = 0.f;
	Progress = 0.f;
	StepTotalCount = MAX_int32;
}

void AGameGridGenerator::InitLoopData()
{
	FlowControlUtility::InitLoopData(CreateGridPointsLoopData);
	FlowControlUtility::InitLoopData(SetGridPosZLoopData);
	FlowControlUtility::InitLoopData(CalGridNormalLoopData);

	FlowControlUtility::InitLoopData(SetGridTTLoopData);
	FlowControlUtility::InitLoopData(SetGridTTEdgeLoopData);
	FlowControlUtility::InitLoopData(AddTreeInstancesLoopData);

	FlowControlUtility::InitLoopData(SetGridAreaBlockLevelLoopData);
	FlowControlUtility::InitLoopData(SetGridAreaBlockLevelExLoopData);
	InitExLoopDatas(AreaBlockExTimes, SetGridAreaBlockLevelExLoopData, AreaBlockLevelExLoopDatas);

	FlowControlUtility::InitLoopData(BreakMABToChunkLoopData);

	FlowControlUtility::InitLoopData(FindGridIsLandLoopData);

	FlowControlUtility::InitLoopData(SetGridBuildingBlockLevelLoopData);
	FlowControlUtility::InitLoopData(SetGridBuildingBlockLevelExLoopData);
	InitExLoopDatas(BuildingBlockExTimes, SetGridBuildingBlockLevelExLoopData, BuildingBlockLevelExLoopDatas);

	FlowControlUtility::InitLoopData(SetGridFlyingBlockLevelLoopData);
	FlowControlUtility::InitLoopData(SetGridFlyingBlockLevelExLoopData);
	InitExLoopDatas(FlyingBlockExTimes, SetGridFlyingBlockLevelExLoopData, FlyingBlockLevelExLoopDatas);

	FlowControlUtility::InitLoopData(FindGridFlyingIsLandLoopData);

	FlowControlUtility::InitLoopData(AddGridInstancesLoopData);
}

void AGameGridGenerator::InitExLoopDatas(int32 ExTimes, const FStructLoopData& Data, 
	TArray<FStructLoopData>& Datas)
{
	for (int32 i = 0; i < ExTimes; i++)
	{
		FStructLoopData LoopData(Data);
		Datas.Add(LoopData);
	}
}

void AGameGridGenerator::InitAreaBlockLevelExLoopDatas()
{
	for (int32 i = 0; i < AreaBlockExTimes; i++)
	{
		FStructLoopData LoopData(SetGridAreaBlockLevelExLoopData);
		AreaBlockLevelExLoopDatas.Add(LoopData);
	}
}

void AGameGridGenerator::InitBulidingBlockLevelExLoopDatas()
{
	for (int32 i = 0; i < BuildingBlockExTimes; i++)
	{
		FStructLoopData LoopData(SetGridBuildingBlockLevelExLoopData);
		BuildingBlockLevelExLoopDatas.Add(LoopData);
	}
}

bool AGameGridGenerator::GameGridPointsLoopFunction(TFunction<void()> InitFunc, 
	TFunction<void(int32 LoopIndex)> LoopFunc, 
	FStructLoopData& LoopData, 
	Enum_GameGridGeneratorState State, 
	bool bProgress, float ProgressWeight)
{
	int32 Count = 0;
	TArray<int32> Indices = { 0 };
	bool SaveLoopFlag = false;

	if (InitFunc && !LoopData.HasInitialized) {
		LoopData.HasInitialized = true;
		InitFunc();
		StepTotalCount = GameGridPointsData.Num();
	}

	int32 i = LoopData.IndexSaved[0];
	for (; i < StepTotalCount; i++)
	{
		Indices[0] = i;
		FlowControlUtility::SaveLoopData(this, LoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
		if (SaveLoopFlag) {
			return false;
		}
		LoopFunc(i);
		if (bProgress) {
			Progress = ProgressPassed + (float)LoopData.Count / (float)StepTotalCount * ProgressWeight;
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

void AGameGridGenerator::CreateGridPoints()
{
	int32 Count = 0;
	TArray<int32> Indices = { 0 };
	bool SaveLoopFlag = false;

	if (!CreateGridPointsLoopData.HasInitialized) {
		CreateGridPointsLoopData.HasInitialized = true;
		StepTotalCount = 1 + (HEX_SIDE_NUM + pGI->GameGridParam.GridRange * HEX_SIDE_NUM) * pGI->GameGridParam.GridRange / 2;
	}

	int32 i = CreateGridPointsLoopData.IndexSaved[0];
	for (; i < StepTotalCount; i++) {
		Indices[0] = i;

		FlowControlUtility::SaveLoopData(this, CreateGridPointsLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
		if (SaveLoopFlag) {
			return;
		}

		AddPointData(i);

		Progress = ProgressPassed + (float)CreateGridPointsLoopData.Count / (float)StepTotalCount * ProgressWeight_CreateGridPoints;
		Count++;
	}
	ProgressPassed += ProgressWeight_CreateGridPoints;

	WorkflowState = Enum_GameGridGeneratorState::WaitTerrain;
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, CreateGridPointsLoopData.Rate, false);
	UE_LOG(GameGridGenerator, Log, TEXT("CreateGridPoints done."));
}

void AGameGridGenerator::AddPointData(int32 Index)
{
	FStructGameGridPointData Data;
	Data.GridDataIndex = Index;
	GameGridPointsData.Add(Data);
	FIntPoint Key = pGI->GameGridPoints[Index].AxialCoord;
	GameGridPointsIndices.Add(Key, Index);
}

void AGameGridGenerator::WaitTerrain()
{
	FTimerHandle TimerHandle;
	TArray<AActor*> Out_Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATerrainGenerator::StaticClass(), Out_Actors);
	if (Out_Actors.Num() == 1) {
		pTG = Cast<ATerrainGenerator>(Out_Actors[0]);
		if (pTG && pTG->IsLoadingCompleted()) {
			WorkflowState = Enum_GameGridGeneratorState::SetGridPosZ;
			GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
			UE_LOG(GameGridGenerator, Log, TEXT("Wait terrain done!"));
			return;
		}
	}
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, WaitTerrainTimerRate, false);
	return;
}

void AGameGridGenerator::SetGridPosZ()
{
	if (GameGridPointsLoopFunction(nullptr, [this](int32 i) { SetTilePosZ(i); },
		SetGridPosZLoopData, 
		Enum_GameGridGeneratorState::CalGridNormal,
		true, ProgressWeight_SetGridPosZ)) {
		UE_LOG(GameGridGenerator, Log, TEXT("Set grid pos z done!"));
	}
}

void AGameGridGenerator::SetTilePosZ(int32 Index)
{
	SetTileCenterPosZ(Index);
	SetTileVerticesPosZ(Index);
}

void AGameGridGenerator::SetTileCenterPosZ(int32 Index)
{
	FVector Loc;
	FVector2D Pos2D = GetPointPosition2D(Index);
	if (pTG->GetTerrainPointBy2DPos(Pos2D, Pos2D, Loc)) {
		FStructGameGridPointData& Data = GameGridPointsData[Index];
		Data.PositionZ = Loc.Z;
		Data.InTerrainRange = true;
	}
}

void AGameGridGenerator::SetTileVerticesPosZ(int32 Index)
{
	float Sum = 0.0;
	float WaterBase = pTG->GetWaterBase();
	for (int32 i = 0; i < HEX_SIDE_NUM; i++)
	{
		FVector Loc;
		FVector2D Pos2D = GetTileVertexPosition2D(Index, i);
		FStructGameGridPointData& Data = GameGridPointsData[Index];
		if (pTG->GetTerrainPointBy2DPos(Pos2D, Pos2D, Loc)) {
			
			if (Loc.Z < WaterBase) {
				Loc.Z = WaterBase;
			}
			Data.VerticesPositionZ.Add(Loc.Z);
			Sum += Loc.Z;
		}
		else {
			Data.VerticesPositionZ.Add(Data.PositionZ);
			Sum += Data.PositionZ;
		}
	}
	FStructGameGridPointData& Data = GameGridPointsData[Index];
	Data.AvgPositionZ = Sum / (float)HEX_SIDE_NUM;
}

void AGameGridGenerator::CalGridNormal()
{
	if (GameGridPointsLoopFunction(nullptr, [this](int32 i) { CalTileNormal(i); },
		CalGridNormalLoopData, 
		Enum_GameGridGeneratorState::SetGridTT,
		true, ProgressWeight_CalGridNormal)) {
		UE_LOG(GameGridGenerator, Log, TEXT("CalGridNormal done!"));
	}
}

void AGameGridGenerator::CalTileNormal(int32 Index)
{
	FStructGameGridPointData& Data = GameGridPointsData[Index];
	FVector TileNormal(0.0, 0.0, 0.0);
	for (int32 i = 0; i < 2; i++) {
		FVector v0(GetTileVertexPosition2D(Index, i).X, GetTileVertexPosition2D(Index, i).Y, Data.VerticesPositionZ[i]);
		FVector v1(GetTileVertexPosition2D(Index, i + 2).X, GetTileVertexPosition2D(Index, i + 2).Y, Data.VerticesPositionZ[i + 2]);
		FVector v2(GetTileVertexPosition2D(Index, i + 4).X, GetTileVertexPosition2D(Index, i + 4).Y, Data.VerticesPositionZ[i + 4]);
		TileNormal += FVector::CrossProduct(v2 - v0, v2 - v1);
	}
	TileNormal.Normalize();
	Data.Normal = TileNormal;

	float DotProduct = FVector::DotProduct(GridInstMesh->GetUpVector(), TileNormal);
	Data.AngleToUp = acosf(DotProduct);
}

void AGameGridGenerator::SetGridTT()
{
	if (GameGridPointsLoopFunction(nullptr, [this](int32 i) { SetTileTT(i); },
		SetGridTTLoopData,
		Enum_GameGridGeneratorState::SetGridTTEdge,
		true, ProgressWeight_SetGridTT)) {
		UE_LOG(GameGridGenerator, Log, TEXT("Set grid terrain type done!"));
	}
}

void AGameGridGenerator::SetTileTT(int32 Index)
{
	FVector2D Pos2D = GetPointPosition2D(Index);
	FStructGameGridPointData& Data = GameGridPointsData[Index];
	float Moisture;
	float Temperature;
	Data.TerrainType = pTG->GetTerrainType(Pos2D, Moisture, Temperature);
}

void AGameGridGenerator::SetGridTTEdge()
{
	if (GameGridPointsLoopFunction([this]() { InitSetGridTTEdge(); },
		[this](int32 i) { SetTileTTEdgeByNeighbors(i); },
		SetGridTTEdgeLoopData,
		Enum_GameGridGeneratorState::AddTreeInstances,
		true, ProgressWeight_SetGridTTEdge)) {
		UE_LOG(GameGridGenerator, Log, TEXT("Set grid terrain type edge done!"));
	}
}

void AGameGridGenerator::InitSetGridTTEdge()
{
	TTEdgeLevelMax = pGI->GameGridParam.NeighborRange + 1;
}

void AGameGridGenerator::SetTileTTEdgeByNeighbors(int32 Index)
{
	FStructGameGridPointData& Data = GameGridPointsData[Index];
	for (int32 i = 0; i < GetPointNeighborNum(Index); i++)
	{
		if (SetTileTTEdgeByNeighbor(Index, i)) {
			return;
		}
	}
	Data.TerrainTypeEdgeRatio = 1.0f;
}

bool AGameGridGenerator::SetTileTTEdgeByNeighbor(int32 Index, int32 NeighborRangeIndex)
{
	FStructGameGridPointData& Data = GameGridPointsData[Index];
	FStructGridDataNeighbors& Neighbors = pGI->GameGridPoints[Data.GridDataIndex].Neighbors[NeighborRangeIndex];

	int32 TileIndex;
	for (int32 i = 0; i < Neighbors.Points.Num(); i++)
	{
		FIntPoint key = Neighbors.Points[i];
		if (!GameGridPointsIndices.Contains(key)) {
			continue;
		}
		TileIndex = GameGridPointsIndices[key];
		if (SetTileTTEdgeLevel(Index, TileIndex, Neighbors.Radius))
		{
			return true;
		}
	}
	
	return false;
}

bool AGameGridGenerator::SetTileTTEdgeLevel(int32 Index, int32 CheckIndex, int32 Level)
{
	bool flag = CheckTileTTEdge(Index, CheckIndex);
	if (flag) {
		FStructGameGridPointData& Data = GameGridPointsData[Index];
		Data.TerrainTypeEdgeRatio = (float)Level / (float)TTEdgeLevelMax;
	}
	return flag;
}

bool AGameGridGenerator::CheckTileTTEdge(int32 Index, int32 CheckIndex)
{
	FStructGameGridPointData Data = GameGridPointsData[Index];
	FStructGameGridPointData CheckData = GameGridPointsData[CheckIndex];

	if (!CheckData.InTerrainRange
		|| (Data.TerrainType != CheckData.TerrainType 
			&& CheckData.TerrainType > Enum_TerrainType::WaterLevel 
			&& CheckData.TerrainType < Enum_TerrainType::PlainLevel)) {
		return true;
	}

	return false;
}

void AGameGridGenerator::AddTreeInstances()
{

	if (GameGridPointsLoopFunction([this]() { InitAddTreeInstances(); },
		[this](int32 i) { AddTileTreeInstanceInRange(i); },
		AddTreeInstancesLoopData,
		Enum_GameGridGeneratorState::SetGridAreaBlockLevel,
		true, ProgressWeight_AddTreeInstances)) {
		UE_LOG(GameGridGenerator, Log, TEXT("AddTreeInstances done!"));
	}
}

void AGameGridGenerator::InitAddTreeInstances()
{
}

void AGameGridGenerator::AddTileTreeInstanceInRange(int32 Index)
{
	if (IsInMapRange(Index))
	{
		if (pTG->HasTreeAt(FVector2D(GetPointPosition2D(Index))))
		{
			FStructGameGridPointData& Data = GameGridPointsData[Index];
			int32 TreeNum = GetTreeNum(Data);
			for (int32 i = 0; i < TreeNum; i++)
			{
				FStructTreeRecord Record = {};
				int32 InstanceIndex = AddTreeInstance(Index, Data, Record);
				Data.TreeRecords.Add(Record);
				if (InstanceIndex > 0)
				{
					AddTreeInstanceData(Index, InstanceIndex, Record);
				}
			}
		}
	}
}

int32 AGameGridGenerator::AddTreeInstance(int32 Index, FStructGameGridPointData& Data, FStructTreeRecord& Record)
{
	int32 Ret = -1;
	if (TreeGenerator) {
		FVector2D TreePoint2D = FMath::RandPointInCircle(pGI->GameGridParam.TileSize);
		TreePoint2D += GetPointPosition2D(Index);
		Ret = TreeGenerator->AddTreeByTerraintype(Data.TerrainType, Record, FVector(TreePoint2D, Data.PositionZ), bShowTree);
		Data.TreeRecords.Add(Record);
	}

	return Ret;
}

int32 AGameGridGenerator::GetTreeNum(const FStructGameGridPointData& Data)
{
	float TreeDensity = pTG->GetTreeDensity(Data.TerrainType);
	float Rand = UKismetMathLibrary::RandomFloat();
	float ValueOnTreeNum = 1.0 - (1.0 - Data.TerrainTypeEdgeRatio) * TTEdgeLevelOnTreeNum;
	return UKismetMathLibrary::Round(Rand * TreeDensity * ValueOnTreeNum);
}

void AGameGridGenerator::AddTreeInstanceData(int32 PointIndex, int32 InstanceIndex, const FStructTreeRecord& Record)
{
	FStructGameGridPointData Data = GameGridPointsData[PointIndex];
	if (TreeGenerator) {
		TreeGenerator->AddTreeDataByTerraintype(Data.TerrainType, Data, InstanceIndex, Record);
	}
}

void AGameGridGenerator::SetGridAreaBlockLevel()
{
	if (GameGridPointsLoopFunction([this]() { InitSetGridAreaBlockLevel(); },
		[this](int32 i) { SetTileAreaBlockLevelByNeighbors(i); },
		SetGridAreaBlockLevelLoopData, 
		Enum_GameGridGeneratorState::SetGridAreaBlockLevelEx,
		true, ProgressWeight_SetGridAreaBlockLevel)) {
		UE_LOG(GameGridGenerator, Log, TEXT("SetGridAreaBlockLevel done!"));
	}
}

void AGameGridGenerator::InitSetGridAreaBlockLevel()
{
	AreaBlockLevelMax = pGI->GameGridParam.NeighborRange + 1;
}

bool AGameGridGenerator::CheckTileBlock(int32 CheckIndex, float UpperRatio, float LowerRatio, float SlopeRatio)
{
	FStructGameGridPointData CheckData = GameGridPointsData[CheckIndex];
	if (!CheckData.InTerrainRange
		|| CheckData.PositionZ > UpperRatio * pTG->GetTileAltitudeMultiplier()
		|| CheckData.PositionZ < LowerRatio * pTG->GetTileAltitudeMultiplier()
		|| CheckData.AngleToUp >(PI * SlopeRatio / 2.0)) {
		return true;
	}
	return false;
}

bool AGameGridGenerator::SetTileAreaBlock(int32 Index, int32 CheckIndex, int32 BlockLevel)
{
	bool flag = CheckTileBlock(CheckIndex,
		AreaBlockAltitudeUpperRatio, pTG->GetShallowWaterRatio(), AreaBlockSlopeRatio);
	if (flag) {
		FStructGameGridPointData& Data = GameGridPointsData[Index];
		Data.AreaBlockLevel = BlockLevel;
	}
	return flag;
}

void AGameGridGenerator::SetTileAreaBlockLevelByNeighbors(int32 Index)
{
	FStructGameGridPointData& Data = GameGridPointsData[Index];
	if (SetTileAreaBlock(Index, Index, 0)) {
		return;
	}

	for (int32 i = 0; i < GetPointNeighborNum(Index); i++)
	{
		if (SetTileAreaBlockLevelByNeighbor(Index, i)) {
			return;
		}
	}
	Data.AreaBlockLevel = AreaBlockLevelMax;
	if (Data.AreaBlockLevel == (pGI->GameGridParam.NeighborRange * (AreaBlockExTimes + 1) + 1)) {
		MaxAreaBlockTileIndices.Add(Index);
	}
}

bool AGameGridGenerator::SetTileAreaBlockLevelByNeighbor(int32 Index, int32 NeighborRangeIndex)
{
	FStructGameGridPointData& Data = GameGridPointsData[Index];
	FStructGridDataNeighbors& Neighbors = pGI->GameGridPoints[Data.GridDataIndex].Neighbors[NeighborRangeIndex];

	int32 TileIndex;
	for (int32 i = 0; i < Neighbors.Points.Num(); i++)
	{
		FIntPoint key = Neighbors.Points[i];
		if (!GameGridPointsIndices.Contains(key)) {
			continue;
		}

		TileIndex = GameGridPointsIndices[key];
		if (SetTileAreaBlock(Index, TileIndex, Neighbors.Radius))
		{
			return true;
		}
	}
	return false;
}

void AGameGridGenerator::SetGridAreaBlockLevelEx()
{
	if (AreaBlockExTimes == 0) {
		ProgressPassed += ProgressWeight_SetGridAreaBlockLevelEx;
		FTimerHandle TimerHandle;
		WorkflowState = Enum_GameGridGeneratorState::CheckAreaConnection;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
		UE_LOG(GameGridGenerator, Log, TEXT("Set grid area block level extension done!"));
		return;
	}

	int32 i = SetGridAreaBlockLevelExLoopData.IndexSaved[0];
	Enum_GameGridGeneratorState state = Enum_GameGridGeneratorState::SetGridAreaBlockLevelEx;
	for (; i < AreaBlockExTimes;)
	{
		SetGridAreaBlockLevelExLoopData.IndexSaved[0] = i;
		if (i == (AreaBlockExTimes - 1)) {
			state = Enum_GameGridGeneratorState::CheckAreaConnection;
		}
		if (GameGridPointsLoopFunction([this]() { InitSetGridAreaBlockLevelEx(); },
			[this](int32 i) { SetTileAreaBlockLevelByNeighborsEx(i); },
			AreaBlockLevelExLoopDatas[i], state)) {
			Progress = ProgressPassed + (float)(i + 1) / (float)AreaBlockExTimes * ProgressWeight_SetGridAreaBlockLevelEx;
			UE_LOG(GameGridGenerator, Log, TEXT("Set grid area block level extension %d done!"), i + 1);
			i++;
			SetGridAreaBlockLevelExLoopData.IndexSaved[0] = i;
			if (i == (AreaBlockExTimes - 1)) {
				ProgressPassed += ProgressWeight_SetGridAreaBlockLevelEx;
				break;
			}
		}
		return;
	}
	UE_LOG(GameGridGenerator, Log, TEXT("Set grid area block level extension done!"));
}

void AGameGridGenerator::InitSetGridAreaBlockLevelEx()
{
	AreaBlockLevelMax += pGI->GameGridParam.NeighborRange;
}

void AGameGridGenerator::SetTileAreaBlockLevelByNeighborsEx(int32 Index)
{
	FStructGameGridPointData& Data = GameGridPointsData[Index];
	if (Data.AreaBlockLevel == (AreaBlockLevelMax - pGI->GameGridParam.NeighborRange))
	{
		FStructGridDataNeighbors& OutSideNeighbors = pGI->GameGridPoints[Data.GridDataIndex].Neighbors[GetPointNeighborNum(Index) - 1];
		int32 BlockLvMin = AreaBlockLevelMax;
		int32 CurrentBlockLv = Data.AreaBlockLevel;
		for (int32 i = 0; i < OutSideNeighbors.Count; i++)
		{
			CurrentBlockLv = pGI->GameGridParam.NeighborRange + GameGridPointsData[GameGridPointsIndices[OutSideNeighbors.Points[i]]].AreaBlockLevel;
			if (CurrentBlockLv < BlockLvMin) {
				BlockLvMin = CurrentBlockLv;
			}
		}
		Data.AreaBlockLevel = BlockLvMin;
		if (Data.AreaBlockLevel == (pGI->GameGridParam.NeighborRange * (AreaBlockExTimes + 1) + 1)) {
			MaxAreaBlockTileIndices.Add(Index);
		}
	}
}

void AGameGridGenerator::CheckAreaConnection()
{
	switch (WorkflowState)
	{
	case Enum_GameGridGeneratorState::CheckAreaConnection:
		WorkflowState = Enum_GameGridGeneratorState::InitCheckAreaConnection;
	case Enum_GameGridGeneratorState::InitCheckAreaConnection:
		InitCheckAreaConnection();
		WorkflowState = Enum_GameGridGeneratorState::BreakMABToChunk;
	case Enum_GameGridGeneratorState::BreakMABToChunk:
		BreakMABToChunk();
		break;
	case Enum_GameGridGeneratorState::CheckChunksAreaConnection:
		CheckChunksAreaConnection();
		break;
	default:
		break;
	}
}

void AGameGridGenerator::InitCheckAreaConnection()
{
	MaxAreaBlockTileChunks.Empty();
}

void AGameGridGenerator::BreakMABToChunk()
{
	FTimerHandle TimerHandle;
	auto InitFunc = [&]() {
		BreakMABToChunkData.Seed = MaxAreaBlockTileIndices;
		};
	TArray<int32> Indices = {};
	if (AStarUtility::BFSFCLoopFunction<int32>(this,
		BreakMABToChunkData,
		Indices,
		BreakMABToChunkLoopData,
		WorkflowDelegate,
		InitFunc,
		[this](const int32& Current, int32& Next, int32& Index) { return NextPoint(Current, Next, Index); },
		[this]() { AddMABChunk(); })) {
		WorkflowState = Enum_GameGridGeneratorState::CheckChunksAreaConnection;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, BreakMABToChunkLoopData.Rate, false);
		UE_LOG(GameGridGenerator, Log, TEXT("BreakMABToChunk done."));
	}
}

bool AGameGridGenerator::NextPoint(const int32& Current, int32& Next, int32& Index)
{
	FStructGridDataNeighbors Neighbors = pGI->GameGridPoints[GameGridPointsData[Current].GridDataIndex].Neighbors[0];
	if (Index < Neighbors.Points.Num()) {
		FIntPoint key = Neighbors.Points[Index];
		if (GameGridPointsIndices.Contains(key)) {
			Next = GameGridPointsIndices[key];
		}
		Index++;
		return true;
	}
	return false;
}

void AGameGridGenerator::AddMABChunk()
{
	MaxAreaBlockTileChunks.Add(BreakMABToChunkData.Reached);
}

void AGameGridGenerator::CheckChunksAreaConnection()
{
	FTimerHandle TimerHandle;
	if (MaxAreaBlockTileChunks.IsEmpty()) {
		UE_LOG(GameGridGenerator, Warning, TEXT("MaxAreaBlockTileChunks is empty!"));
		WorkflowState = Enum_GameGridGeneratorState::Error;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
		return;
	}
	
	MaxAreaBlockTileChunks.Sort([](const TSet<int32>& A, const TSet<int32>& B) {
		return A.Num() > B.Num();
		});

	TSet<int32> objChunk = MaxAreaBlockTileChunks[0];

	for (int32 i = 1; i < MaxAreaBlockTileChunks.Num(); i++) {
		if (!FindTwoChunksAreaConnection(i, 0)) {
			CheckAreaConnectionNotPass(i);
			continue;
		}
		objChunk.Append(MaxAreaBlockTileChunks[i]);
	}
	WorkflowState = Enum_GameGridGeneratorState::FindGridIsland;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(GameGridGenerator, Log, TEXT("Check Chunks Area Connection done!"));
	UE_LOG(GameGridGenerator, Log, TEXT("Check Area Connection done!"));
}

bool AGameGridGenerator::FindTwoChunksAreaConnection(int32 StartChunkIndex, int32 ObjChunkIndex)
{
	int32 StartIndex = MaxAreaBlockTileChunks[StartChunkIndex].Array()[0];
	TSet<int32>& ObjChunk = MaxAreaBlockTileChunks[ObjChunkIndex];

	TStructBFSData<int32> BFSData;
	BFSData.Frontier.Enqueue(StartIndex);
	BFSData.Reached.Add(StartIndex);
	TFunction<bool(const int32& Current, bool& Res)> EEFunc = [ObjChunk](const int32& Current, bool& Res) {
		if (ObjChunk.Contains(Current)) {
			Res = true;
			return true;
		}
		return false;
	};

	bool Result = AStarUtility::BFSEEFunction<int32>(BFSData, EEFunc,
		[this](const int32& Current, int32& Next, int32& Index, TSet<int32>& Reached) { return NextPoint3Pass(Current, Next, Index, Reached); });

	return Result;
}

bool AGameGridGenerator::NextPoint3Pass(const int32& Current, int32& Next, int32& Index, TSet<int32>& Reached)
{
	bool flag = NextPoint(Current, Next, Index);
	if (flag && GameGridPointsData[Next].AreaBlockLevel < 3) {
		if (!Reached.Contains(Next)) {
			Reached.Add(Next);
		}
	}
	return flag;
}

void AGameGridGenerator::CheckAreaConnectionNotPass(int32 ChunkIndex)
{
	TSet<int32> Ck = MaxAreaBlockTileChunks[ChunkIndex];
	TArray<int32> CkArray = Ck.Array();
	for (int32 i : CkArray) {
		GameGridPointsData[i].AreaConnection = false;
		GameGridPointsData[i].FlyingConnection = false;
	}
}

void AGameGridGenerator::FindGridIsland()
{
	if (GameGridPointsLoopFunction(nullptr,
		[this](int32 i) { FindTileIsLand(i); },
		FindGridIsLandLoopData, 
		Enum_GameGridGeneratorState::SetGridBuildingBlockLevel,
		true, ProgressWeight_FindGridIsland)) {
		UE_LOG(GameGridGenerator, Log, TEXT("FindGridIsland done!"));
	}
}

void AGameGridGenerator::FindTileIsLand(int32 Index)
{
	FStructGameGridPointData& Data = GameGridPointsData[Index];
	if (Data.AreaBlockLevel == AreaBlockLevelMax) {
		if (!Data.AreaConnection) {
			Data.IsLand = true;
		}
	}
	else if (Data.AreaBlockLevel >= 3) {
		Data.IsLand = !Find_ABLM_By_ABL3(Index);
	}
	else if (Data.AreaBlockLevel >= 1) {
		for (int32 i = Data.AreaBlockLevel; i > 0; i--) {
			FStructGridDataNeighbors Neighbors = pGI->GameGridPoints[Data.GridDataIndex].Neighbors[2 - i];
			for (int32 j = 0; j < Neighbors.Points.Num(); j++) {
				FIntPoint key = Neighbors.Points[j];
				if (!GameGridPointsIndices.Contains(key)) {
					continue;
				}
				int32 NIndex = GameGridPointsIndices[key];
				if (GameGridPointsData[NIndex].AreaBlockLevel == 3) {
					if (Find_ABLM_By_ABL3(NIndex)) {
						Data.IsLand = false;
						if (i < Data.AreaBlockLevel) {
							Data.AreaBlockLevel = i;
						}
						return;
					}
				}
			}
		}
		Data.IsLand = true;
	}
	else {
		Data.IsLand = true;
	}
}

/*Find AreaBlockLevelMax tile by AreaBlockLevel=3 tile*/
bool AGameGridGenerator::Find_ABLM_By_ABL3(int32 Index)
{
	TStructBFSData<int32> BFSData;
	BFSData.Frontier.Enqueue(Index);
	BFSData.Reached.Add(Index);
	TFunction<bool(const int32& Current, bool& Res)> EEFunc = [&](const int32& Current, bool& Res) {
		if (GameGridPointsData[Current].AreaBlockLevel == AreaBlockLevelMax) {
			if (GameGridPointsData[Current].AreaConnection) {
				Res = true;
			}
			else {
				Res = false;
			}
			return true;
		}
		return false;
	};

	bool Result = AStarUtility::BFSEEFunction<int32>(BFSData, EEFunc,
		[this](const int32& Current, int32& Next, int32& Index, TSet<int32>& Reached) { return NextPoint3Pass(Current, Next, Index, Reached); });
	return Result;
}

void AGameGridGenerator::SetGridBuildingBlockLevel()
{
	if (GameGridPointsLoopFunction([this]() { InitSetGridBuildingBlockLevel(); },
		[this](int32 i) { SetTileBuildingBlockLevelByNeighbors(i); },
		SetGridBuildingBlockLevelLoopData, 
		Enum_GameGridGeneratorState::SetGridBuildingBlockLevelEx,
		true, ProgressWeight_SetGridBuildingBlockLevel)) {
		UE_LOG(GameGridGenerator, Log, TEXT("SetGridBuildingBlockLevel done!"));
	}
}

void AGameGridGenerator::InitSetGridBuildingBlockLevel()
{
	BuildingBlockLevelMax = pGI->GameGridParam.NeighborRange + 1;
}

bool AGameGridGenerator::SetTileBuildingBlock(int32 Index, int32 CheckIndex, int32 BlockLevel)
{
	bool flag = CheckTileBlock(CheckIndex,
		BuildingBlockAltitudeUpperRatio, BuildingBlockAltitudeLowerRatio, BuildingBlockSlopeRatio);
	FStructGameGridPointData CheckData = GameGridPointsData[CheckIndex];
	if (CheckData.TreeRecords.Num() > 0) {
		flag = true;
	}
	if (flag) {
		FStructGameGridPointData& Data = GameGridPointsData[Index];
		Data.BuildingBlockLevel = BlockLevel;
	}
	return flag;
}

void AGameGridGenerator::SetTileBuildingBlockLevelByNeighbors(int32 Index)
{
	FStructGameGridPointData& Data = GameGridPointsData[Index];
	if (SetTileBuildingBlock(Index, Index, 0)) {
		return;
	}

	for (int32 i = 0; i < GetPointNeighborNum(Index); i++)
	{
		if (SetTileBuildingBlockLevelByNeighbor(Index, i)) {
			return;
		}
	}
	Data.BuildingBlockLevel = BuildingBlockLevelMax;
}

bool AGameGridGenerator::SetTileBuildingBlockLevelByNeighbor(int32 Index, int32 NeighborRangeIndex)
{
	FStructGameGridPointData& Data = GameGridPointsData[Index];
	FStructGridDataNeighbors& Neighbors = pGI->GameGridPoints[Data.GridDataIndex].Neighbors[NeighborRangeIndex];

	int32 TileIndex;
	for (int32 i = 0; i < Neighbors.Points.Num(); i++)
	{
		FIntPoint key = Neighbors.Points[i];
		if (!GameGridPointsIndices.Contains(key)) {
			continue;
		}

		TileIndex = GameGridPointsIndices[key];
		if (SetTileBuildingBlock(Index, TileIndex, Neighbors.Radius))
		{
			return true;
		}
	}
	return false;
}

void AGameGridGenerator::SetGridBuildingBlockLevelEx()
{
	if (BuildingBlockExTimes == 0) {
		ProgressPassed += ProgressWeight_SetGridBuildingBlockLevelEx;
		FTimerHandle TimerHandle;
		WorkflowState = Enum_GameGridGeneratorState::SetGridFlyingBlockLevel;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
		UE_LOG(GameGridGenerator, Log, TEXT("Set grid building block level extension done!"));
		return;
	}
	int32 i = SetGridBuildingBlockLevelExLoopData.IndexSaved[0];
	Enum_GameGridGeneratorState state = Enum_GameGridGeneratorState::SetGridBuildingBlockLevelEx;
	for (; i < BuildingBlockExTimes; )
	{
		SetGridBuildingBlockLevelExLoopData.IndexSaved[0] = i;
		if (i == (BuildingBlockExTimes - 1)) {
			state = Enum_GameGridGeneratorState::SetGridFlyingBlockLevel;
		}
		if (GameGridPointsLoopFunction([this]() { InitSetGridBuildingBlockLevelEx(); },
			[this](int32 i) { SetTileBuildingBlockLevelByNeighborsEx(i); },
			BuildingBlockLevelExLoopDatas[i], state)) {
			Progress = ProgressPassed + (float)(i + 1) / (float)BuildingBlockExTimes * ProgressWeight_SetGridBuildingBlockLevelEx;
			UE_LOG(GameGridGenerator, Log, TEXT("Set grid building block level extension %d done!"), i + 1);
			i++;
			SetGridBuildingBlockLevelExLoopData.IndexSaved[0] = i;
			if (i == BuildingBlockExTimes) {
				ProgressPassed += ProgressWeight_SetGridBuildingBlockLevelEx;
				break;
			}
		}
		return;
	}
	UE_LOG(GameGridGenerator, Log, TEXT("Set grid building block level extension done!"));
}

void AGameGridGenerator::InitSetGridBuildingBlockLevelEx()
{
	BuildingBlockLevelMax += pGI->GameGridParam.NeighborRange;
}

void AGameGridGenerator::SetTileBuildingBlockLevelByNeighborsEx(int32 Index)
{
	FStructGameGridPointData& Data = GameGridPointsData[Index];
	if (Data.BuildingBlockLevel == (BuildingBlockLevelMax - pGI->GameGridParam.NeighborRange))
	{
		FStructGridDataNeighbors& OutSideNeighbors = pGI->GameGridPoints[Data.GridDataIndex].Neighbors[GetPointNeighborNum(Index) - 1];
		int32 BlockLvMin = BuildingBlockLevelMax;
		int32 CurrentBlockLv = Data.BuildingBlockLevel;
		for (int32 i = 0; i < OutSideNeighbors.Count; i++)
		{
			CurrentBlockLv = pGI->GameGridParam.NeighborRange + GameGridPointsData[GameGridPointsIndices[OutSideNeighbors.Points[i]]].BuildingBlockLevel;
			if (CurrentBlockLv < BlockLvMin) {
				BlockLvMin = CurrentBlockLv;
			}
		}
		Data.BuildingBlockLevel = BlockLvMin;
	}
}

void AGameGridGenerator::SetGridFlyingBlockLevel()
{
	if (GameGridPointsLoopFunction([this]() { InitSetGridFlyingBlockLevel(); },
		[this](int32 i) { SetTileFlyingBlockLevelByNeighbors(i); },
		SetGridFlyingBlockLevelLoopData, Enum_GameGridGeneratorState::SetGridFlyingBlockLevelEx,
		true, ProgressWeight_SetGridFlyingBlockLevel)) {
		UE_LOG(GameGridGenerator, Log, TEXT("SetGridFlyingBlockLevel done!"));
	}
}

void AGameGridGenerator::InitSetGridFlyingBlockLevel()
{
	FlyingBlockLevelMax = pGI->GameGridParam.NeighborRange + 1;
}

bool AGameGridGenerator::SetTileFlyingBlock(int32 Index, int32 CheckIndex, int32 BlockLevel)
{
	FStructGameGridPointData& Data = GameGridPointsData[Index];
	FStructGameGridPointData CheckData = GameGridPointsData[CheckIndex];
	if (!CheckData.InTerrainRange
		|| CheckData.PositionZ > FlyingBlockAltitudeRatio * pTG->GetTileAltitudeMultiplier()) {
		Data.FlyingBlockLevel = BlockLevel;
		return true;
	}
	return false;
}

void AGameGridGenerator::SetTileFlyingBlockLevelByNeighbors(int32 Index)
{
	FStructGameGridPointData& Data = GameGridPointsData[Index];
	if (SetTileFlyingBlock(Index, Index, 0)) {
		return;
	}

	for (int32 i = 0; i < GetPointNeighborNum(Index); i++)
	{
		if (SetTileFlyingBlockLevelByNeighbor(Index, i)) {
			return;
		}
	}
	Data.FlyingBlockLevel = FlyingBlockLevelMax;
}

bool AGameGridGenerator::SetTileFlyingBlockLevelByNeighbor(int32 Index, int32 NeighborRangeIndex)
{
	FStructGameGridPointData& Data = GameGridPointsData[Index];
	FStructGridDataNeighbors& Neighbors = pGI->GameGridPoints[Data.GridDataIndex].Neighbors[NeighborRangeIndex];

	int32 TileIndex;
	for (int32 i = 0; i < Neighbors.Points.Num(); i++)
	{
		FIntPoint key = Neighbors.Points[i];
		if (!GameGridPointsIndices.Contains(key)) {
			continue;
		}

		TileIndex = GameGridPointsIndices[key];
		if (SetTileFlyingBlock(Index, TileIndex, Neighbors.Radius))
		{
			return true;
		}
	}
	return false;
}

void AGameGridGenerator::SetGridFlyingBlockLevelEx()
{
	if (FlyingBlockExTimes == 0) {
		ProgressPassed += ProgressWeight_SetGridFlyingBlockLevelEx;
		FTimerHandle TimerHandle;
		WorkflowState = Enum_GameGridGeneratorState::FindGridFlyingIsland;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
		UE_LOG(GameGridGenerator, Log, TEXT("Set grid flying block level extension done!"));
		return;
	}
	int32 i = SetGridFlyingBlockLevelExLoopData.IndexSaved[0];
	Enum_GameGridGeneratorState state = Enum_GameGridGeneratorState::SetGridFlyingBlockLevelEx;
	for (; i < FlyingBlockExTimes;)
	{
		SetGridFlyingBlockLevelExLoopData.IndexSaved[0] = i;
		if (i == (FlyingBlockExTimes - 1)) {
			state = Enum_GameGridGeneratorState::FindGridFlyingIsland;
		}
		if (GameGridPointsLoopFunction([this]() { InitSetGridFlyingBlockLevelEx(); },
			[this](int32 i) { SetTileFlyingBlockLevelByNeighborsEx(i); },
			FlyingBlockLevelExLoopDatas[i], state)) {
			Progress = ProgressPassed + (float)(i + 1) / (float)FlyingBlockExTimes * ProgressWeight_SetGridFlyingBlockLevelEx;
			UE_LOG(GameGridGenerator, Log, TEXT("Set grid flying block level extension %d done!"), i + 1);
			i++;
			SetGridFlyingBlockLevelExLoopData.IndexSaved[0] = i;
			if (i == (FlyingBlockExTimes - 1)) {
				ProgressPassed += ProgressWeight_SetGridFlyingBlockLevelEx;
				break;
			}
			
		}
		return;
	}
	UE_LOG(GameGridGenerator, Log, TEXT("Set grid flying block level extension done!"));
}

void AGameGridGenerator::InitSetGridFlyingBlockLevelEx()
{
	FlyingBlockLevelMax += pGI->GameGridParam.NeighborRange;
}

void AGameGridGenerator::SetTileFlyingBlockLevelByNeighborsEx(int32 Index)
{
	FStructGameGridPointData& Data = GameGridPointsData[Index];
	if (Data.FlyingBlockLevel == (FlyingBlockLevelMax - pGI->GameGridParam.NeighborRange))
	{
		FStructGridDataNeighbors& OutSideNeighbors = pGI->GameGridPoints[Data.GridDataIndex].Neighbors[GetPointNeighborNum(Index) - 1];
		int32 BlockLvMin = FlyingBlockLevelMax;
		int32 CurrentBlockLv = Data.FlyingBlockLevel;
		for (int32 i = 0; i < OutSideNeighbors.Count; i++)
		{
			CurrentBlockLv = pGI->GameGridParam.NeighborRange + GameGridPointsData[GameGridPointsIndices[OutSideNeighbors.Points[i]]].FlyingBlockLevel;
			if (CurrentBlockLv < BlockLvMin) {
				BlockLvMin = CurrentBlockLv;
			}
		}
		Data.FlyingBlockLevel = BlockLvMin;
	}
}

void AGameGridGenerator::FindGridFlyingIsland()
{
	if (GameGridPointsLoopFunction(nullptr,
		[this](int32 i) { FindTileFlyingIsLand(i); },
		FindGridFlyingIsLandLoopData, 
		Enum_GameGridGeneratorState::AddGridInstances,
		true, ProgressWeight_FindGridFlyingIsland)) {
		UE_LOG(GameGridGenerator, Log, TEXT("FindGridFlyingIsland done!"));
	}
}

void AGameGridGenerator::FindTileFlyingIsLand(int32 Index)
{
	FStructGameGridPointData& Data = GameGridPointsData[Index];
	if (Data.FlyingBlockLevel == FlyingBlockLevelMax) {
		if (!Data.FlyingConnection) {
			Data.FlyingIsLand = true;
		}
	}
	else if (Data.FlyingBlockLevel >= 3) {
		Data.FlyingIsLand = !Find_FBLM_By_FBL3(Index);
	}
	else if (Data.FlyingBlockLevel >= 1) {
		for (int32 i = Data.FlyingBlockLevel; i > 0; i--) {
			FStructGridDataNeighbors Neighbors = pGI->GameGridPoints[Data.GridDataIndex].Neighbors[2 - i];
			for (int32 j = 0; j < Neighbors.Points.Num(); j++) {
				FIntPoint key = Neighbors.Points[j];
				if (!GameGridPointsIndices.Contains(key)) {
					continue;
				}
				int32 NIndex = GameGridPointsIndices[key];
				if (GameGridPointsData[NIndex].FlyingBlockLevel == 3) {
					if (Find_FBLM_By_FBL3(NIndex)) {
						Data.FlyingIsLand = false;
						if (i < Data.FlyingBlockLevel) {
							Data.FlyingBlockLevel = i;
						}
						return;
					}
				}
			}
		}
		Data.FlyingIsLand = true;
	}
	else {
		Data.FlyingIsLand = true;
	}
}

/*Find FlyingBlockLevelMax tile by FlyingBlockLevel=3 tile*/
bool AGameGridGenerator::Find_FBLM_By_FBL3(int32 Index)
{
	TStructBFSData<int32> BFSData;
	BFSData.Frontier.Enqueue(Index);
	BFSData.Reached.Add(Index);
	TFunction<bool(const int32& Current, bool& Res)> EEFunc = [&](const int32& Current, bool& Res) {
		if (GameGridPointsData[Current].FlyingBlockLevel == FlyingBlockLevelMax) {
			if (GameGridPointsData[Current].FlyingConnection) {
				Res = true;
			}
			else {
				Res = false;
			}
			return true;
		}
		return false;
		};

	bool Result = AStarUtility::BFSEEFunction<int32>(BFSData, EEFunc,
		[this](const int32& Current, int32& Next, int32& Index, TSet<int32>& Reached) { return NextPoint3PassFlying(Current, Next, Index, Reached); });
	return Result;
}

bool AGameGridGenerator::NextPoint3PassFlying(const int32& Current, int32& Next, int32& Index, TSet<int32>& Reached)
{
	bool flag = NextPoint(Current, Next, Index);
	if (flag && GameGridPointsData[Next].FlyingBlockLevel < 3) {
		if (!Reached.Contains(Next)) {
			Reached.Add(Next);
		}
	}
	return flag;
}

void AGameGridGenerator::AddGridInstances()
{
	if (!bShowGrid) {
		InitAddGridInstances();
		FTimerHandle TimerHandle;
		WorkflowState = Enum_GameGridGeneratorState::Done;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
		UE_LOG(GameGridGenerator, Log, TEXT("Don't show grid!"));
		return;
	}

	if (GameGridPointsLoopFunction([this]() { InitAddGridInstances(); }, 
		[this](int32 i) { AddTileInstanceInRange(i); },
		AddGridInstancesLoopData, 
		Enum_GameGridGeneratorState::Done,
		true, ProgressWeight_AddGridInstances)) {
		UE_LOG(GameGridGenerator, Log, TEXT("AddGridInstances done!"));
	}

}

void AGameGridGenerator::InitAddGridInstances()
{
	GridTileInstanceScale = pGI->GameGridParam.TileSize / GridTileInstMeshSize;
	GridInstMesh->NumCustomDataFloats = 3;
}

int32 AGameGridGenerator::AddTileInstance(int32 Index)
{
	return AddNormalRotISM(Index, GridInstMesh, GridInstMeshOffsetZ);
}

int32 AGameGridGenerator::AddNormalRotISM(int32 Index, UInstancedStaticMeshComponent* ISM,
	float ZOffset)
{
	if (ISM == nullptr) {
		return -1;
	}

	FStructGameGridPointData Data = GameGridPointsData[Index];
	FVector Loc(GetPointPosition2D(Index).X, GetPointPosition2D(Index).Y, Data.AvgPositionZ + ZOffset);
	FVector Scale(GridTileInstanceScale * InstanceScaleMultiplier);
	
	FVector UpVec(0.f, 0.f, 1.0);

	FVector RotationAxis = FVector::CrossProduct(UpVec, Data.Normal);
	RotationAxis.Normalize();

	FQuat Quat = FQuat(RotationAxis, Data.AngleToUp);
	FQuat NewQuat = Quat * GridTileInstMeshRot.Quaternion();

	FTransform TileTransform(NewQuat.Rotator(), Loc, Scale);

	int32 InstanceIndex = ISM->AddInstance(TileTransform);
	return InstanceIndex;
}

void AGameGridGenerator::AddTileInstanceInRange(int32 Index)
{
	if (IsInMapRange(Index))
	{
		if (!bShowBlock && IsBlock(Index)) {
			return;
		}
		if (!bShowIsland && IsIsland(Index)) {
			return;
		}
		int32 InstanceIndex = AddTileInstance(Index);
		if (InstanceIndex >= 0) {
			AddTileInstanceData(Index, InstanceIndex);
		}
	}
}

bool AGameGridGenerator::IsInMapRange(int32 Index)
{
	return GameGridPointsData[Index].InTerrainRange;
}

bool AGameGridGenerator::IsBlock(int32 Index)
{
	if (GridShowMode == Enum_GridShowMode::AreaBlock) {
		return GameGridPointsData[Index].AreaBlockLevel == 0;
	}
	else if (GridShowMode == Enum_GridShowMode::BuildingBlock) {
		return GameGridPointsData[Index].BuildingBlockLevel == 0;
	}
	else if (GridShowMode == Enum_GridShowMode::FlyingBlock) {
		return GameGridPointsData[Index].FlyingBlockLevel == 0;
	}
	return GameGridPointsData[Index].AreaBlockLevel == 0;
}

bool AGameGridGenerator::IsIsland(int32 Index)
{
	if (GridShowMode == Enum_GridShowMode::FlyingBlock) {
		return GameGridPointsData[Index].FlyingIsLand;
	}
	return GameGridPointsData[Index].IsLand;
}

void AGameGridGenerator::AddTileInstanceData(int32 TileIndex, int32 InstanceIndex)
{
	if (GridShowMode == Enum_GridShowMode::TerrainType) {
		AddTileInstanceDataTT(TileIndex, InstanceIndex);
	}
	else {
		AddTileInstanceDataBlock(TileIndex, InstanceIndex);
	}
}

void AGameGridGenerator::AddTileInstanceDataBlock(int32 TileIndex, int32 InstanceIndex)
{
	float H = 0.0;
	if (GridShowMode == Enum_GridShowMode::AreaBlock) {
		if (!GameGridPointsData[TileIndex].IsLand) {
			H = 120.0f / float(AreaBlockLevelMax) * float(GameGridPointsData[TileIndex].AreaBlockLevel);
		}
		else {
			H = 240.0;
		}
	}
	else if (GridShowMode == Enum_GridShowMode::BuildingBlock) {
		if (!GameGridPointsData[TileIndex].IsLand) {
			H = 120.0f / float(BuildingBlockLevelMax) * float(GameGridPointsData[TileIndex].BuildingBlockLevel);
		}
		else {
			H = 240.0;
		}
	}
	else if (GridShowMode == Enum_GridShowMode::FlyingBlock) {
		if (!GameGridPointsData[TileIndex].FlyingIsLand) {
			H = 120.0f / float(FlyingBlockLevelMax) * float(GameGridPointsData[TileIndex].FlyingBlockLevel);
		}
		else {
			H = 240.0;
		}

	}

	FLinearColor LinearColor = UKismetMathLibrary::HSVToRGB(H, 1.0, 1.0, 1.0);

	TArray<float> CustomData = { LinearColor.R, LinearColor.G, LinearColor.B };
	GridInstMesh->SetCustomData(InstanceIndex, CustomData, true);
}

void AGameGridGenerator::AddTileInstanceDataTT(int32 TileIndex, int32 InstanceIndex)
{
	FLinearColor LinearColor(0.0, 0.0, 0.0);
	switch (GameGridPointsData[TileIndex].TerrainType)
	{
	case Enum_TerrainType::Mountain:
		LinearColor = TT_Mountain_Color;
		break;
	case Enum_TerrainType::ShallowWater:
		LinearColor = TT_ShallowWater_Color;
		break;
	case Enum_TerrainType::DeepWater:
		LinearColor = TT_DeepWater_Color;
		break;
	case Enum_TerrainType::Lava:
		LinearColor = TT_Lava_Color;
		break;
	case Enum_TerrainType::DryGrass:
		LinearColor = TT_DryGrass_Color;
		break;
	case Enum_TerrainType::Swamp:
		LinearColor = TT_Swamp_Color;
		break;
	case Enum_TerrainType::Desert:
		LinearColor = TT_Desert_Color;
		break;
	case Enum_TerrainType::Grass:
		LinearColor = TT_Grass_Color;
		break;
	case Enum_TerrainType::Coast:
		LinearColor = TT_Coast_Color;
		break;
	case Enum_TerrainType::Gobi:
		LinearColor = TT_Gobi_Color;
		break;
	case Enum_TerrainType::Tundra:
		LinearColor = TT_Tundra_Color;
		break;
	case Enum_TerrainType::Snow:
		LinearColor = TT_Snow_Color;
		break;
	default:
		break;
	}
	TArray<float> CustomData = { LinearColor.R, LinearColor.G, LinearColor.B };
	GridInstMesh->SetCustomData(InstanceIndex, CustomData, true);
}



FVector2D AGameGridGenerator::GetPointPosition2D(int32 Index)
{
	return pGI->GameGridPoints[GameGridPointsData[Index].GridDataIndex].Position2D;
}

FVector2D AGameGridGenerator::GetTileVertexPosition2D(int32 PointIndex, int32 VertexIndex)
{
	TArray<FVector2D> VerticesPostion2D = pGI->GameGridPoints[GameGridPointsData[PointIndex].GridDataIndex].VerticesPostion2D;
	int32 Num = VerticesPostion2D.Num();
	if (VertexIndex >= 0 && VertexIndex < Num) {
		return VerticesPostion2D[VertexIndex];
	}
	return FVector2D();
}

int32 AGameGridGenerator::GetPointNeighborNum(int32 Index)
{
	return pGI->GameGridPoints[GameGridPointsData[Index].GridDataIndex].Neighbors.Num();
}

FIntPoint AGameGridGenerator::GetPointAxialCoord(int32 Index)
{
	return pGI->GameGridPoints[GameGridPointsData[Index].GridDataIndex].AxialCoord;
}

void AGameGridGenerator::DoWorkflowDone()
{
	Progress = 1.0;
}

void AGameGridGenerator::FindNeighborTilesByRadius(TArray<FIntPoint>& NeighborTiles, 
	int32 CenterIndex, int32 Radius)
{
	Hex center(GetPointAxialCoord(CenterIndex));
	Hex Current;
	for (int32 i = 1; i <= MouseOverShowRadius; i++)
	{
		Current.SetHex(Hex::Add(Hex::Scale(Hex::Direction(HEX_RING_DIRECTION_START_INDEX), i), center));
		for (int32 j = 0; j <= 5; j++)
		{
			for (int32 k = 0; k <= i - 1; k++)
			{
				NeighborTiles.Add(Current.ToIntPoint());
				Current.SetHex(Hex::Neighbor(Current, j));
			}
		}
	}
}

// Called every frame
void AGameGridGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGameGridGenerator::AddMouseOverGrid(Hex& MouseOverHex)
{
	int32 Index = GameGridPointsIndices[MouseOverHex.ToIntPoint()];

	if (IsBlock(Index) || IsIsland(Index)) {
		return;
	}

	int32 InstanceIndex = AddNormalRotISM(Index, MouseOverInstMesh, MouseOverGridOffsetZ);
	TArray<FIntPoint> NeighborTiles;
	FindNeighborTilesByRadius(NeighborTiles, Index, MouseOverShowRadius);
	for (int32 i = 0; i < NeighborTiles.Num(); i++)
	{
		AddNormalRotISM(GameGridPointsIndices[NeighborTiles[i]], MouseOverInstMesh, MouseOverGridOffsetZ);
	}
}

void AGameGridGenerator::RemoveMouseOverGrid()
{
	if (MouseOverInstMesh) {
		MouseOverInstMesh->ClearInstances();
	}
}