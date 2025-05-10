// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "M_LoAW_GridData/Public/GridDataStructDefine.h"
#include "GameGridStructDefine.h"
#include "M_LoAW_Terrain/Public/AStarUtility.h"
#include "M_LoAW_GridData/Public/Hex.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameGridGenerator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(GameGridGenerator, Log, All);

UENUM(BlueprintType)
enum class Enum_GameGridGeneratorState : uint8
{
	InitWorkflow,
	CreateGridPoints,
	WaitTerrain,
	SetGridPosZ,
	CalGridNormal,

	SetGridAreaBlockLevel,
	SetGridAreaBlockLevelEx,

	CheckAreaConnection,
	InitCheckAreaConnection,
	BreakMABToChunk,
	CheckChunksAreaConnection,

	FindGridIsland,

	SetGridBuildingBlockLevel,
	SetGridBuildingBlockLevelEx,

	SetGridFlyingBlockLevel,
	SetGridFlyingBlockLevelEx,

	FindGridFlyingIsland,

	AddInstances,
	
	Done,
	Error
};

UENUM(BlueprintType)
enum class Enum_GridShowMode : uint8
{
	BuildingBlock,
	AreaBlock,
	FlyingBlock,
};

UCLASS()
class M_LOAW_GAMEGRID_API AGameGridGenerator : public AActor
{
	GENERATED_BODY()

private:
	//Delegate
	FTimerDynamicDelegate WorkflowDelegate;

	class ULoAWGameInstance* pGI;

	Enum_GameGridGeneratorState WorkflowState = Enum_GameGridGeneratorState::InitWorkflow;

	class ATerrainGenerator* pTG;

	int32 StepTotalCount = MAX_int32;
	float ProgressPassed = 0.f;
	float Progress = 0.f;

	TArray<FStructGameGridPointData> GameGridPointsData = {};
	TMap<FIntPoint, int32> GameGridPointsIndices = {};

	//Area Block data
	int32 AreaBlockLevelMax = 0;
	TSet<int32> MaxAreaBlockTileIndices;
	TArray<FStructLoopData> AreaBlockLevelExLoopDatas;

	//Check area connection data
	TArray<TSet<int32>> MaxAreaBlockTileChunks;
	TStructBFSData<int32> BreakMABToChunkData = {};// MAB=MaxAreaBlock

	//Building Block data
	int32 BuildingBlockLevelMax = 0;
	TArray<FStructLoopData> BuildingBlockLevelExLoopDatas;

	//Flying Block data
	int32 FlyingBlockLevelMax = 0;
	TArray<FStructLoopData> FlyingBlockLevelExLoopDatas;

	float GridTileInstanceScale = 1.0;

	int32 MouseOverShowRadius = 2;

protected:
	//Mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom|InstMesh")
	class UInstancedStaticMeshComponent* GridInstMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom|InstMesh")
	class UInstancedStaticMeshComponent* MouseOverInstMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Timer")
	float DefaultTimerRate = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData CreateGridPointsLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData SetGridPosZLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData CalGridNormalLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData SetGridAreaBlockLevelLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData SetGridAreaBlockLevelExLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData BreakMABToChunkLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData FindGridIsLandLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData SetGridBuildingBlockLevelLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData SetGridBuildingBlockLevelExLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData SetGridFlyingBlockLevelLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData SetGridFlyingBlockLevelExLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData FindGridFlyingIsLandLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData AddGridInstancesLoopData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Common")
	bool bUseGrid = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Common")
	bool bShowGrid = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Common")
	bool bShowBlock = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Common")
	bool bShowIsland = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Common")
	Enum_GridShowMode GridShowMode = Enum_GridShowMode::AreaBlock;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Params")
	float GridTileInstMeshSize = 100.0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Params")
	FRotator GridTileInstMeshRot = FRotator(0.0, 30.0, 0.0);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Params")
	float GridInstMeshOffsetZ = 50.0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Params")
	float MouseOverGridOffsetZ = 55.0;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Block|Area", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float AreaBlockAltitudeUpperRatio = 0.3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Block|Area", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float AreaBlockAltitudeLowerRatio = -0.05;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Block|Area", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AreaBlockSlopeRatio = 0.3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Block|Area", meta = (ClampMin = "0"))
	int32 AreaBlockExTimes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Block|Building", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float BuildingBlockAltitudeUpperRatio = 0.3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Block|Building", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float BuildingBlockAltitudeLowerRatio = 0.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Block|Building", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BuildingBlockSlopeRatio = 0.1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Block|Building", meta = (ClampMin = "0"))
	int32 BuildingBlockExTimes = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Block|Flying", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float FlyingBlockAltitudeRatio = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Block|Flying", meta = (ClampMin = "0"))
	int32 FlyingBlockExTimes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_CreateGridPoints = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_SetGridPosZ = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_CalGridNormal = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_SetGridAreaBlockLevel = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_SetGridAreaBlockLevelEx = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_FindGridIsland = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_SetGridBuildingBlockLevel = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_SetGridBuildingBlockLevelEx = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_SetGridFlyingBlockLevel = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_SetGridFlyingBlockLevelEx = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_FindGridFlyingIsland = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_AddInstances = 0.1f;

public:	
	// Sets default values for this actor's properties
	AGameGridGenerator();

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void GetProgress(float& Out_Progress)
	{
		Out_Progress = Progress;
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsLoadingCompleted()
	{
		return WorkflowState == Enum_GameGridGeneratorState::Done;
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	//Timer delegate
	void BindDelegate();


	UFUNCTION()
	void DoWorkFlow();

	void InitWorkflow();
	bool GetGameInstance();
	void InitProgress();
	void InitLoopData();
	void InitExLoopDatas(int32 ExTimes, const FStructLoopData& Data, 
		TArray<FStructLoopData>& Datas);
	void InitAreaBlockLevelExLoopDatas();
	void InitBulidingBlockLevelExLoopDatas();

	bool GameGridPointsLoopFunction(TFunction<void()> InitFunc,
		TFunction<void(int32 LoopIndex)> LoopFunc,
		FStructLoopData& LoopData,
		Enum_GameGridGeneratorState State,
		bool bProgress = false, float ProgressWeight = 0.f);

	void CreateGridPoints();
	void AddPointData(int32 Index);

	void WaitTerrain();

	void SetGridPosZ();
	void SetTilePosZ(int32 Index);
	void SetTileCenterPosZ(int32 Index);
	void SetTileVerticesPosZ(int32 Index);

	void CalGridNormal();
	void CalTileNormal(int32 Index);

	void SetGridAreaBlockLevel();
	void InitSetGridAreaBlockLevel();
	bool CheckTileBlock(int32 CheckIndex, float UpperRatio, float LowerRatio, float SlopeRatio);
	bool SetTileAreaBlock(int32 Index, int32 CheckIndex, int32 BlockLevel);
	void SetTileAreaBlockLevelByNeighbors(int32 Index);
	bool SetTileAreaBlockLevelByNeighbor(int32 Index, int32 NeighborRangeIndex);

	//Set Area block level extension
	void SetGridAreaBlockLevelEx();
	void InitSetGridAreaBlockLevelEx();
	void SetTileAreaBlockLevelByNeighborsEx(int32 Index);

	void CheckAreaConnection();
	void InitCheckAreaConnection();
	void BreakMABToChunk();
	bool NextPoint(const int32& Current, int32& Next, int32& Index);
	void AddMABChunk();
	void CheckChunksAreaConnection();
	bool FindTwoChunksAreaConnection(int32 StartChunkIndex, int32 ObjChunkIndex);
	bool NextPoint3Pass(const int32& Current, int32& Next, int32& Index, TSet<int32>& Reached);
	void CheckAreaConnectionNotPass(int32 ChunkIndex);

	void FindGridIsland();
	void FindTileIsLand(int32 Index);
	bool Find_ABLM_By_ABL3(int32 Index);

	void SetGridBuildingBlockLevel();
	void InitSetGridBuildingBlockLevel();
	bool SetTileBuildingBlock(int32 Index, int32 CheckIndex, int32 BlockLevel);
	void SetTileBuildingBlockLevelByNeighbors(int32 Index);
	bool SetTileBuildingBlockLevelByNeighbor(int32 Index, int32 NeighborRangeIndex);

	void SetGridBuildingBlockLevelEx();
	void InitSetGridBuildingBlockLevelEx();
	void SetTileBuildingBlockLevelByNeighborsEx(int32 Index);

	void SetGridFlyingBlockLevel();
	void InitSetGridFlyingBlockLevel();
	bool SetTileFlyingBlock(int32 Index, int32 CheckIndex, int32 BlockLevel);
	void SetTileFlyingBlockLevelByNeighbors(int32 Index);
	bool SetTileFlyingBlockLevelByNeighbor(int32 Index, int32 NeighborRangeIndex);

	void SetGridFlyingBlockLevelEx();
	void InitSetGridFlyingBlockLevelEx();
	void SetTileFlyingBlockLevelByNeighborsEx(int32 Index);

	void FindGridFlyingIsland();
	void FindTileFlyingIsLand(int32 Index);
	bool Find_FBLM_By_FBL3(int32 Index);
	bool NextPoint3PassFlying(const int32& Current, int32& Next, int32& Index, TSet<int32>& Reached);

	void AddGridInstances();
	void InitAddGridInstances();
	int32 AddTileInstance(int32 Index);
	int32 AddNormalRotISM(int32 Index, class UInstancedStaticMeshComponent* ISM, float ZOffset = 0.f);
	void AddTileInstanceInRange(int32 Index);
	bool IsInMapRange(int32 Index);
	bool IsBlock(int32 Index);
	bool IsIsland(int32 Index);
	void AddTileInstanceData(int32 TileIndex, int32 InstanceIndex);

	FVector2D GetPointPosition2D(int32 Index);
	FVector2D GetTileVertexPosition2D(int32 PointIndex, int32 VertexIndex);
	int32 GetPointNeighborNum(int32 Index);
	FIntPoint GetPointAxialCoord(int32 Index);

	void DoWorkflowDone();

	void FindNeighborTilesByRadius(TArray<FIntPoint>& NeighborTiles, 
		int32 CenterIndex, int32 Radius);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE int32 GetBuildingBlockExTimes()
	{
		return BuildingBlockExTimes;
	}

	FORCEINLINE int32 GetMouseOverShowRadius()
	{
		return MouseOverShowRadius;
	}

	FORCEINLINE void SetMouseOverShowRadius(int32 Value)
	{
		MouseOverShowRadius = Value;
	}

	void AddMouseOverGrid(Hex& MouseOverHex);

	void RemoveMouseOverGrid();
};
