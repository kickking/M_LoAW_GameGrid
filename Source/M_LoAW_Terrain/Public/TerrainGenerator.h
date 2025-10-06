// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "M_LoAW_GridData/Public/GridDataStructDefine.h"
#include "TerrainStructDefine.h"
#include "AStarUtility.h"
#include "TerrainWaterfall.h"
#include "TerrainWaterfallMist.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrainGenerator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(TerrainGenerator, Log, All);

UENUM(BlueprintType)
enum class Enum_TerrainGeneratorState : uint8
{
	InitWorkflow,
	CreateVertices,
	ReMappingZ,

	SetBlockLevel,
	SetBlockLevelEx,

	CreateRiver,
	AddRiverEndPoints,
	DivideUpperRiver,
	DivideLowerRiver,
	ChunkToOnePoint,
	CreateRiverLine,
	FindRiverLines,
	DigRiverLine,
	DigRiverPool,
	CombinePoolToTerrain,

	CreateVertexColorsForAMTB,

	CreateTriangles,
	CalNormalsInit,
	CalNormalsAcc,
	NormalizeNormals,

	DrawLandMesh,

	CreateWater,
	CreateWaterfall,

	CreateTree,

	Done,
	Error
};

UENUM(BlueprintType)
enum class Enum_TerrainType : uint8
{
	
	DeepWater,
	ShallowWater,

	WaterLevel,

	Lava,
	DryGrass,
	Swamp,
	Desert,
	Grass,
	Coast,
	Gobi,
	Tundra,
	Snow,

	PlainLevel,

	Mountain,

	None
};

UCLASS()
class M_LOAW_TERRAIN_API ATerrainGenerator : public AActor
{
	GENERATED_BODY()

private:
	FTimerDynamicDelegate WorkflowDelegate;

	class UGridDataGameInstance* pGI;

	Enum_TerrainGeneratorState WorkflowState = Enum_TerrainGeneratorState::InitWorkflow;

	float TileSizeMultiplier = 0.f;
	float TileAltitudeMultiplier = 0.f;

	float TerrainSize = 0.f;

	float WaterBase = 0.f;

	int32 StepTotalCount = MAX_int32;
	float ProgressPassed = 0.f;
	float Progress = 0.f;

	TArray<FVector> NormalsAcc = {};

	TArray<FStructTerrainMeshPointData> TerrainMeshPointsData = {};
	TMap<FIntPoint, int32> TerrainMeshPointsIndices = {};

	TMap<FIntPoint, int32> WaterMeshPointsIndices = {};

	int32 BlockLevelMax = 0;
	TArray<FStructLoopData> BlockLevelExLoopDatas = {};

	float ZRatioMax = 0.0;
	float ZRatioMin = 0.0;
	float ZRatioLandSum = 0.0;
	int32 ZRatioLandPointCount = 0;
	FStructHeightMapping ZRatioMapping = {};

	TSet<int32> UpperRiverIndices = {};
	TSet<int32> LowerRiverIndices = {};

	TStructBFSData<int32> UpperRiverDivideData = {};
	TStructBFSData<int32> LowerRiverDivideData = {};

	TArray<TSet<int32>> UpperRiverChunks = {};
	TArray<TSet<int32>> LowerRiverChunks = {};
	
	TArray<int32> UpperRiverEndPoints = {};
	TArray<int32> LowerRiverEndPoints = {};

	TArray<FStructRiverLinePointData> RiverLinePointDatas = {};

	TStructAStarData<int32> FindRiverLineData = {};

	float RiverNoiseSampleRotSin = 0.0;
	float RiverNoiseSampleRotCos = 0.0;

	TSet<int32> RiverLinePointBlockTestSet = {};
	float CurrentLineDepthRatio = 0.0;
	int32 CurrentLinePointIndex = 0;
	float UnitLineRisingStep = 0.0;

	TArray<FStructWaterfallRenderData> WaterfallRenderDatas = {};
	float CurrentWaterfallRadius = 0.0;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	class UProceduralMeshComponent* TerrainMesh;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	class UProceduralMeshComponent* WaterMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ATerrainWaterfall> WaterfallClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ATerrainWaterfallMist> WaterfallMistClass;

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
	FStructLoopData ReMappingZLoopData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData SetBlockLevelLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData SetBlockLevelExLoopData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData AddRiverEndPointsLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData UpperRiverDivideLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData LowerRiverDivideLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData FindRiverLinesLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData DigRiverLineLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData DigRiverPoolLoopData;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain", meta = (ClampMin = "0.0"))
	float MoistureSampleScale = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain")
	float MoistureValueScale = 3.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain")
	float MoistureZRatioScale = 2.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain", meta = (ClampMin = "0.0"))
	float TemperatureSampleScale = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain")
	float TemperatureValueScale = 3.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AltitudeBlockRatio = 0.005;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain", meta = (ClampMin = "0"))
	int32 BlockExTimes = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain")
	float WaterLandCombineRatio = 0.3;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Land", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LandLayer0Level = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Land", meta = (ClampMin = "0.0"))
	float LandLayer0SampleScale = 1.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Land")
	FStructHeightMapping LandLayer0RangeMapping = { 0.35, 1.0, 0.0, 0.8, -0.2, 0.0 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Land", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LandLayer1Level = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Land", meta = (ClampMin = "0.0"))
	float LandLayer1SampleScale = 1.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Land")
	FStructHeightMapping LandLayer1RangeMapping = { 0.35, 1.0, 0.0, 0.2, -0.2, 0.0 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Land", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MoistureBlendThresholdLow = 0.35;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Land", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MoistureBlendThresholdHigh = 0.65;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Land", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TempratureBlendThresholdLow = 0.35;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Land", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TempratureBlendThresholdHigh = 0.65;


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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|Water", meta = (ClampMin = "-1.0", ClampMax = "0.0"))
	float ShallowWaterRatio = -0.07;
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River", meta = (ClampMin = "-1.0", ClampMax = "-0.1"))
	float RiverDepthRatioMax = -0.07;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River", meta = (ClampMin = "-1.0", ClampMax = "0.0"))
	float RiverDepthRatioMin = -0.06;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RiverDepthChangeStep = 0.005;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RiverDepthRisingStep = 0.003;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River", meta = (ClampMin = "0.0"))
	float RiverDepthSampleScale = 1.0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Pool")
	bool HasRiverPool = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Pool", meta = (ClampMin = "-1.0", ClampMax = "-0.1"))
	float RiverPoolDepthRatioMax = -0.1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Pool", meta = (ClampMin = "-1.0", ClampMax = "0.0"))
	float RiverPoolDepthRatioMin = -0.08;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Pool", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RiverPoolDepthRisingStep = 0.003;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Pool", meta = (ClampMin = "0.0"))
	float RiverPoolCombineRatio = 0.01;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Pool")
	float RiverPoolCombineUpper = 0.04;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Pool")
	float RiverPoolCombineLower = -0.06;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Waterfall")
	bool HasWaterfall = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Waterfall")
	int32 WaterfallPoolUnderWaterCountLimit = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Waterfall", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WaterfallHeightRatio = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Waterfall")
	float WaterfallTraceDeltaAngle = PI / 36.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Waterfall")
	int32 WaterfallTraceStep = 18;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Waterfall")
	float WaterfallToleranceAngle = PI / 9.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Waterfall")
	FVector WaterfallOriDirection = FVector(0.f, 1.0, 0.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Waterfall", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WaterfallFreefallTimeOffset = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Waterfall")
	float WaterfallPlainOverTerrain = 0.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Waterfall")
	float WaterfallRadiusMin = 150.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Waterfall")
	float WaterfallRadiusMax = 900.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Terrain|River|Waterfall")
	float WaterfallRadiusStep = 30.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Tree", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TreeRange = 0.3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Tree", meta = (ClampMin = "0.0"))
	float TreeSampleScale = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Tree", meta = (ClampMin = "0.0"))
	float TreeValueScale = 2.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Tree")
	TMap<Enum_TerrainType, float> TypeToTreeDensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Material")
	UMaterialParameterCollection* TerrainMPC;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Material")
	UMaterialInstance* TerrainMaterialIns;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Material")
	UMaterialInstance* WaterMaterialIns;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Material")
	UMaterialInstance* CausticsMaterialIns;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Material")
	UMaterialInstance* WaterfallMaterialIns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_CreateVertices = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_ReMappingZ = 0.04f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_SetBlockLevel = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_SetBlockLevelEx = 0.05f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_AddRiverEndPoints = 0.03f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_FindRiverLines = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_DigRiverLine = 0.3f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_DigRiverPool = 0.25f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_CreateVertexColorsForAMTB = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_CreateTriangles = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_CalNormalsInit = 0.02f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_CalNormalsAcc = 0.04f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0", ClampMax = "1.0"))
	float ProgressWeight_NormalizeNormals = 0.02f;

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

	FORCEINLINE float GetTileAltitudeMultiplier() {
		return TileAltitudeMultiplier;
	}

	FORCEINLINE float GetWaterBase() {
		return WaterBase;
	}

	FORCEINLINE float GetShallowWaterRatio() {
		return ShallowWaterRatio;
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
	void InitLandBlendParam();
	void InitWater();
	void SetWaterZ();
	void InitProgress();

	//Create vertices
	void CreateVertices();
	bool CreateVertex(int32 X, int32 Y, float& OutRatioStd, float& OutRatio);
	void AddVertex(FStructTerrainMeshPointData& Data, float& OutRatioStd, float& OutRatio);
	void GetZRatioInfo(const FStructTerrainMeshPointData& Data);

	float GetAltitude(float X, float Y, float& OutRatioStd, float& OutRatio);
	float GetGradientRatioZ(class UFastNoiseWrapper* NWP, float X, float Y, 
		TFunction<float(float X, float Y)> GetRatioFunc,
		float BaseRatio, float k, const FVector2d& BaseSlope, FVector2d& OutSlope);

	float CombineWaterLandRatio(float wRatio, float lRatio);
	float GetLandLayer0Ratio(float X, float Y);
	float GetLandLayer1Ratio(float X, float Y);
	float GetWaterRatio(float X, float Y);
	float CalWaterBank(float Ratio);

	void MappingByLevel(float level, const FStructHeightMapping& InMapping, 
		FStructHeightMapping& OutMapping);
	float GetMappingHeightRatio(class UFastNoiseWrapper* NWP, 
		const FStructHeightMapping& Mapping, float X, float Y, float SampleScale);
	float MappingFromRangeToRange(float InputValue, 
		const FStructHeightMapping& Mapping);
	float MappingFromRangeToRange(float InputValue, float RangeMax, float RangeMin, 
		float MappingMax, float MappingMin);

	void CreateUV(float X, float Y);

	float GetNoise2DStd(UFastNoiseWrapper* NWP, float X, float Y, 
		float SampleScale = 1.f, float ValueScale = 1.f);

	//Set block level
	bool TerrainMeshPointsLoopFunction(TFunction<void()> InitFunc, 
		TFunction<void(int32 LoopIndex)> LoopFunc,
		FStructLoopData& LoopData, 
		Enum_TerrainGeneratorState State,
		bool bProgress = false, float ProgressWeight = 0.f);

	void ReMappingZ();
	void InitReMappingZ();
	void ReMappingPointZ(int32 Index);

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

	void AddRiverEndPoints();
	void AddRiverEndPoint(int32 Index);

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
	int32 GetTotalRiverLinePointNum();
	float FindRiverBlockZByNeighbor(int32 Index);
	void UpdateRiverPointZ(int32 Index, float ZRatio);
	bool DigRiverNextPoint(const int32& Current, int32& Next, int32& Index, TSet<int32>& Reached);

	void DigRiverPool();
	void UpdateRiverPoolZ(int32 Index, float ZRatio);
	bool DigRiverPoolNextPoint(const int32& Current, int32& Next, int32& Index, TSet<int32>& Reached);

	void CombinePoolToTerrain();

	FIntPoint GetPointAxialCoord(int32 Index);
	FVector2D GetPointPosition2D(int32 Index);
	FVector GetPointPosition(int32 Index);
	int32 GetPointsDistance(int32 Index1, int32 Index2);

	void CreateVertexColorsForAMTB();
	void InitCreateVertexColorsForAMTB();
	void AddAMTBToVertexColor(int32 Index);
	float CalMoisture(int32 X, int32 Y);
	float CalTemperature(int32 X, int32 Y);

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
	float AngleBetweenVectors(const FVector& A, const FVector& B);
	void CalAngleToUp(int32 Index);

	void CreateWaterfall();
	void CreateWaterfallAnim();
	void CreateWaterfallPlanes();
	void InitCreateWaterfall();
	void CreateNewWaterfallRender(int32 Index);
	void CreateWaterfallPlane(int32 Index);
	void CreateWaterfallVerticesAndNormals(int32 Index);
	FVector GetWaterfallOverPoint(int32 Index, const FStructRiverLinePointData& Data);
	void CreateWaterfallTriangles(int32 Index);
	void CreateWaterfallMesh(int32 Index);
	void SetWaterfallMaterial(int32 Index);

	void FindWaterfallPoints();
	void FindWaterfallPoint(int32 Index);
	bool FindEnterWaterPoint(FStructRiverLinePointData& Data);
	void TraceWaterfallPoint(FVector TraceStart, FVector TraceLine, FVector& TestLoc, float& TestAngle);
	void AddWaterfall();
	void AddWaterfallMist();

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
	bool GetMeshPointByLineTrance(UProceduralMeshComponent* Mesh, FVector Start, FVector End, FVector& Loc);
	bool GetTerrainPointByLineTrace(FVector Start, FVector End, FVector& Loc);
	bool GetTerrainPointBy2DPos(FVector2D Start2D, FVector2D End2D, FVector& Loc);
	bool GetWaterPointByLineTrance(FVector Start, FVector End, FVector& Loc);

public:
	Enum_TerrainType GetTerrainType(FVector2D Point, float& OutMoisture, float& OutTemperature);
private:
	Enum_TerrainType GetPlainType(float Moisture, float Temperature);
	int32 GetScalarStep(float Scalar, float Lower, float Upper);

public:
	bool HasTreeAt(const FVector2D& Point);
	float GetTreeDensity(Enum_TerrainType TT);
private:
	float CalTree(int32 X, int32 Y);

public:
	UFUNCTION(BlueprintCallable)
	void GetDebugRiverLineEndPoints(TArray<FVector>& Points);

	UFUNCTION(BlueprintCallable)
	void GetDebugRiverLinePointsAt(TArray<FVector>& LinePoints, int32 Index);

	UFUNCTION(BlueprintCallable)
	bool GetDebugWaterfallTracePointAt(FVector& TraceStart, FVector& TraceEnd, int32 Index);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetDebugRiverNum()
	{
		return RiverLinePointDatas.Num();
	}

};

