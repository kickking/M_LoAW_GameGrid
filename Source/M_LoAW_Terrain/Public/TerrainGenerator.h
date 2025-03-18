// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "M_LoAW_GridData/Public/GridDataStructDefine.h"
#include "TerrainStructDefine.h"
#include "AStarUtility.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrainGenerator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(TerrainGenerator, Log, All);

UENUM(BlueprintType)
enum class Enum_TerrainGeneratorState : uint8
{
	InitWorkflow,
	CreateVertices,

	SetBlockLevel,
	SetBlockLevelEx,

	CreateRiver,
	DivideUpperRiver,
	DivideLowerRiver,
	ChunkToOnePoint,
	CreateRiverLine,
	DigRiverLine,

	CreateVertexColorsForAMTB,

	CreateTriangles,
	CalNormalsInit,
	CalNormalsAcc,
	NormalizeNormals,
	CreateWater,
	DrawLandMesh,
	CreateTree,
	Done,
	Error
};

UCLASS()
class M_LOAW_TERRAIN_API ATerrainGenerator : public AActor
{
	GENERATED_BODY()

private:
	FTimerDynamicDelegate WorkflowDelegate;

	class ULoAWGameInstance* pGI;

	Enum_TerrainGeneratorState WorkflowState = Enum_TerrainGeneratorState::InitWorkflow;

	float TileSizeMultiplier = 0.f;
	float TileAltitudeMultiplier = 0.f;

	float TerrainSize = 0.f;

	float WaterBase = 0.f;

	int32 StepTotalCount = MAX_int32;
	float ProgressPassed = 0.f;
	float Progress = 0.f;

	TArray<FVector> NormalsAcc;

	TArray<FStructTerrainMeshPointData> TerrainMeshPointsData = {};
	TMap<FIntPoint, int32> TerrainMeshPointsIndices = {};

	TMap<FIntPoint, int32> WaterMeshPointsIndices = {};

	int32 BlockLevelMax = 0;
	TArray<FStructLoopData> BlockLevelExLoopDatas;

	TSet<int32> UpperRiverIndices = {};
	TSet<int32> LowerRiverIndices = {};

	TStructBFSData<int32> UpperRiverDivideData;
	TStructBFSData<int32> LowerRiverDivideData;

	TArray<TSet<int32>> UpperRiverChunks = {};
	TArray<TSet<int32>> LowerRiverChunks = {};
	
	TArray<int32> UpperRiverEndPoints = {};
	TArray<int32> LowerRiverEndPoints = {};

	TArray<FStructRiverLinePointData> RiverLinePointDatas = {};

	float RiverNoiseSampleRotSin = 0.0;
	float RiverNoiseSampleRotCos = 0.0;

	TSet<int32> RiverLinePointBlockTestSet = {};
	float CurrentLineDepthRatio = 0.0;
	int32 CurrentLinePointIndex = 0;
	float UnitLineRisingStep = 0.0;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	class UProceduralMeshComponent* TerrainMesh;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	class UProceduralMeshComponent* WaterMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Timer")
	float DefaultTimerRate = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise")
	class ATerrainNoise* Noise;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Tile", meta = (ClampMin = "0"))
	int32 GridRange = 249;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Tile", meta = (ClampMin = "0.0"))
	float TileAltitudeMax = 10000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Tile", meta = (ClampMin = "0.0"))
	float UVScale = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData CreateVerticesLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData SetBlockLevelLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData SetBlockLevelExLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData UpperRiverDivideLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData LowerRiverDivideLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData CreateVertexColorsForAMTBLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData CreateTrianglesLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData CalNormalsInitLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData CalNormalsAccLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData NormalizeNormalsLoopData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|HighMountain", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HighMountainLevel = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|HighMountain", meta = (ClampMin = "0.0"))
	float HighMountainSampleScale = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|HighMountain")
	FStructHeightMapping HighRangeMapping = { 0.4, 0.6, 0.0, 0.7, -0.2, -0.2 };
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|LowMountain", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowMountainLevel = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|LowMountain", meta = (ClampMin = "0.0"))
	float LowMountainSampleScale = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|LowMountain")
	FStructHeightMapping LowRangeMapping = { 0.5, 1.0, 0.0, 0.3, -0.4, 0.0 };
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float LavaBaseRatio = 0.02;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float DesertBaseRatio = 0.03;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float SwampBaseRatio = 0.03;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain", meta = (ClampMin = "0.0"))
	float MoistureSampleScale = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain")
	float MoistureValueScale = 3.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain", meta = (ClampMin = "0.0"))
	float TemperatureSampleScale = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain")
	float TemperatureValueScale = 3.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain", meta = (ClampMin = "0.0"))
	float BiomesSampleScale = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain")
	float BiomesValueScale = 3.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AltitudeBlockRatio = 0.005;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain", meta = (ClampMin = "0"))
	int32 BlockExTimes = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Water")
	bool HasWater = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Water")
	bool HasCaustics = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Water", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WaterLevel = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Water", meta = (ClampMin = "0.0"))
	float WaterSampleScale = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Water")
	FStructHeightMapping WaterRangeMapping = { -0.6, -0.4, -0.4, 0.0, 0.2, 0.2 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Water", meta = (ClampMin = "-1.0", ClampMax = "0.0"))
	float WaterBaseRatio = -0.025;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Water", meta = (ClampMin = "1"))
	int32 WaterNumRows = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Water", meta = (ClampMin = "1"))
	int32 WaterNumColumns = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Water", meta = (ClampMin = "10", ClampMax = "50"))
	int32 WaterRange = 35;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Water", meta = (ClampMin = "1.0"))
	float WaterBankSharpness = 50.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River")
	bool HasRiver = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River", meta = (ClampMin = "0"))
	int32 MaxRiverNum = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UpperRiverLimitZRatio = 0.3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River", meta = (ClampMin = "-1.0", ClampMax = "0.0"))
	float LowerRiverLimitZRatio = -0.1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River", meta = (ClampMin = "0"))
	int32 MinRiverLength = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River", meta = (ClampMin = "0.0"))
	float RiverDirectionSampleScale = 1.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River")
	float RiverDirectionNoiseCostScale = 50.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River")
	float RiverDirectionAltitudeCostScale = 10.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River")
	FStructHeightMapping RiverDirectionMapping = { 0.05, 1.0, 0.0, 1.0, 0.0, 0.0 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RiverDirectionAltitudeBlockRatio = 0.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RiverDirectionHeuristicRatio = 1.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River", meta = (ClampMin = "-0.1", ClampMax = "0.0"))
	float RiverDepthRatioStart = -0.005;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River", meta = (ClampMin = "-1.0", ClampMax = "0.0"))
	float RiverDepthRatioMax = -0.07;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River", meta = (ClampMin = "-1.0", ClampMax = "0.0"))
	float RiverDepthRatioMin = -0.06;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RiverDepthChangeStep = 0.005;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RiverDepthRisingStep = 0.003;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River", meta = (ClampMin = "0.0"))
	float RiverDepthSampleScale = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Material")
	UMaterialParameterCollection* TerrainMPC;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Material")
	UMaterialInstance* TerrainMaterialIns;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Material")
	UMaterialInstance* WaterMaterialIns;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Material")
	UMaterialInstance* CausticsMaterialIns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_CreateVertices = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_SetBlockLevel = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_SetBlockLevelEx = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_CreateVertexColorsForAMTB = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_CreateTriangles = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_CalNormalsInit = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_CalNormalsAcc = 0.35f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_NormalizeNormals = 0.1f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Custom|Render|Land")
	TArray<FVector> Vertices;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Custom|Render|Land")
	TArray<FVector2D> UVs;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Custom|Render|Land")
	TArray<int32> Triangles;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Custom|Render|Land")
	TArray<FVector> Normals;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Custom|Render|Land")
	TArray<FLinearColor> VertexColors;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Custom|Render|Land")
	TArray<FVector2D> UV1;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Custom|Render|Land")
	TArray<FVector2D> UV2;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Custom|Render|Land")
	TArray<FVector2D> UV3;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Custom|Render|Water")
	TArray<FVector> WaterVertices;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Custom|Render|Water")
	TArray<FVector2D> WaterUVs;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Custom|Render|Water")
	TArray<int32> WaterTriangles;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Custom|Render|Water")
	TArray<FVector> WaterNormals;

public:	
	// Sets default values for this actor's properties
	ATerrainGenerator();

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void GetProgress(float& Out_Progress)
	{
		Out_Progress = Progress;
	}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsLoadingCompleted()
	{
		return WorkflowState == Enum_TerrainGeneratorState::Done;
	}

	FORCEINLINE bool IsWorkFlowStepDone(Enum_TerrainGeneratorState state)
	{
		return WorkflowState > state;
	}

	FORCEINLINE UProceduralMeshComponent* GetTerrainMesh()
	{
		return TerrainMesh;
	}

	FORCEINLINE float GetSize() {
		return TerrainSize;
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
	bool InitNoise();
	bool CheckMaterialSetting();
	void InitTileParameter();
	void InitLoopData();
	void InitBlockLevelExLoopDatas();
	void InitReceiveDecal();
	void InitBaseRatio();
	void InitWater();
	void SetWaterZ();
	void InitProgress();

	//Create vertices
	void CreateVertices();
	bool CreateVertex(int32 X, int32 Y, float& OutRatioStd, float& OutRatio);
	void AddVertex(FStructTerrainMeshPointData& Data, float& OutRatioStd, float& OutRatio);
	void AddRiverEndPoint(const FStructTerrainMeshPointData& Data);

	float GetAltitude(float X, float Y, float& OutRatioStd, float& OutRatio);
	float CombineWaterLandRatio(float wRatio, float lRatio);
	float GetHighMountainRatio(float X, float Y);
	float GetLowMountianRatio(float X, float Y);
	float GetWaterRatio(float X, float Y);
	float CalWaterBank(float Ratio);

	void MappingByLevel(float level, const FStructHeightMapping& InMapping, 
		FStructHeightMapping& OutMapping);
	float GetMappingHeightRatio(class UFastNoiseWrapper* NWP, 
		const FStructHeightMapping& Mapping, float X, float Y, float SampleScale);
	float MappingFromRangeToRange(float InputValue, 
		const FStructHeightMapping& Mapping);

	void CreateUV(float X, float Y);

	float GetNoise2DStd(UFastNoiseWrapper* NWP, float X, float Y, 
		float SampleScale = 1.f, float ValueScale = 1.f);

	//Set block level
	bool TerrainMeshPointsLoopFunction(TFunction<void()> InitFunc, 
		TFunction<void(int32 LoopIndex)> LoopFunc,
		FStructLoopData& LoopData, 
		Enum_TerrainGeneratorState State,
		bool bProgress = false, float ProgressWeight = 0.f);

	void SetBlockLevel();
	void InitSetBlockLevel();
	bool SetBlock(FStructTerrainMeshPointData& OutData, 
		const FStructTerrainMeshPointData& InData,
		int32 BlockLevel);
	void SetBlockLevelByNeighbors(int32 Index);
	bool SetBlockLevelByNeighbor(FStructTerrainMeshPointData& Data, int32 Index);

	void SetBlockLevelEx();
	void InitSetBlockLevelEx();
	void SetBlockLevelExByNeighbors(int32 Index);

	//Create River
	void CreateRiver();

	void DivideRiverEndPointsIntoChunks();
	void DivideUpperRiverEndPointsIntoChunks();
	void DivideLowerRiverEndPointsIntoChunks();
	bool NextPoint(const int32& Current, int32& Next, int32& Index);
	void AddUpperRiverEndPointsChunk();
	void AddLowerRiverEndPointsChunk();

	void RiverChunkToOnePoint();
	void ChunkToOnePoint(TArray<TSet<int32>>& Chunk, TArray<int32>& EndPoints);

	void CreateRiverLine();
	void CreateRiverLinePointDatas();
	void FindRiverLines();
	void CalRiverNoiseSampleRotValue(int32 Total, int32 Index);
	FVector2D GetRiverRotatedAxialCoord(FIntPoint AxialCoord);
	float RiverDirectionCost(const int32& Current, const int32& Next);
	float RiverDirectionNoiseCost(float X, float Y);
	float RiverDirectionAltitudeCost(int32 Index);
	float RiverDirectionHeuristic(const int32& Goal, const int32& Next);
	void DigRiverLine();
	float FindRiverBlockZByNeighbor(int32 Index);
	void UpdateRiverPointZ(int32 Index, float ZRatio);
	bool DigRiverNextPoint(const int32& Current, int32& Next, int32& Index, TSet<int32>& Reached);

	FIntPoint GetPointAxialCoord(int32 Index);
	FVector2D GetPointPosition(int32 Index);
	int32 GetPointsDistance(int32 Index1, int32 Index2);

	void CreateVertexColorsForAMTB();
	void AddAMTBToVertexColor(int32 Index);

	//Create Triangles
	void CreateTriangles();
	void FindTopRightSquareVertices(int32 Index, TArray<int32>& SqVArr, 
		const TMap<FIntPoint, int32>& Indices, int32 RangeLimit);
	void CreatePairTriangles(TArray<int32>& SqVArr, TArray<int32>& TrianglesArr);

	//Create Normals
	void CreateNormals();
	void CalNormalsWorkflow();
	void CalNormalsInit();
	void CalNormalsAcc();
	void CalTriangleNormalForVertex(int32 TriangleIndex);
	void NormalizeNormals();
	void AddNormal(int32 Index);

	//Create water face
	void CreateWater();
	void CreateWaterPlane();
	void CreateWaterVerticesAndUVs();
	void CreateWaterTriangles();
	void CreateWaterNormals();
	void CreateWaterMesh();
	void SetWaterMaterial();
	void CreateCaustics();

	//Create material
	void CreateTerrainMesh();
	void SetTerrainMaterial();

	void DoWorkflowDone();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintCallable)
	void GetDebugRiverLineEndPoints(TArray<FVector>& Points);

	UFUNCTION(BlueprintCallable)
	void GetDebugRiverLinePointsAt(TArray<FVector>& LinePoints, int32 Index);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetDebugRiverNum()
	{
		return RiverLinePointDatas.Num();
	}

	UFUNCTION(BlueprintCallable)
	void GetDebugPriorityQueue(TArray<int32>& Values);
};

