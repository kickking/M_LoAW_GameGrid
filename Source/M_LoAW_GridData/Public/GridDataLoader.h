// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GridDataStructDefine.h"
#include <fstream>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridDataLoader.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(GridDataLoader, Log, All);

UENUM(BlueprintType)
enum class Enum_GridDataLoaderState : uint8
{
	Ready,
	Init,
	LoadParams,
	InitProgress,
	LoadPointIndices,
	LoadPoints,
	LoadNeighbors,
	CreatePointsVertices,
	Done,
	Error
};

UCLASS()
class M_LOAW_GRIDDATA_API AGridDataLoader : public AActor
{
	GENERATED_BODY()

private:
	FTimerDynamicDelegate WorkflowDelegate;

protected:
	Enum_GridDataLoaderState WorkflowState = Enum_GridDataLoaderState::Ready;

	FString LoaderName = FString(TEXT(""));

	std::ifstream DataLoadStream;

	FString PipeDelim = FString(TEXT("|"));
	FString CommaDelim = FString(TEXT(","));
	FString SpaceDelim = FString(TEXT(" "));

	int32 ProgressTotal = 0;
	int32 ProgressPassed = 0;
	int32 ProgressCurrent = 0;

	class ULoAWGameInstance* pGI;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData LoadPointIndicesLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData LoadPointsLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData LoadNeighborsLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData CreatePointsVerticesLoopData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Timer")
	float DefaultTimerRate = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Path")
	FString DataFileRelPath = FString(TEXT(""));
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Path")
	FString ParamsDataFileName = FString(TEXT("Params.data"));
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Path")
	FString PointIndicesDataFileName = FString(TEXT("PointIndices.data"));
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Path")
	FString PointsDataFileName = FString(TEXT("Points.data"));
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Path")
	FString NeighborsDataFileNamePrefix = FString(TEXT("N"));
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0"))
	int32 ProgressWeight_LoadPointIndices = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0"))
	int32 ProgressWeight_LoadPoints = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0"))
	int32 ProgressWeight_LoadNeighbors = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Params")
	int32 ParamNum = 3;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom|Params")
	int32 GridRange = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom|Params")
	int32 NeighborRange = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom|Params")
	int32 PointsNum = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0.0"))
	float NeighborCountAddRatio = 0.15;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Progress", meta = (ClampMin = "0"))
	int32 NeighborCountAddMin = 1000;

public:	
	// Sets default values for this actor's properties
	AGridDataLoader();

	UFUNCTION(BlueprintCallable)
	void GetProgress(float& Out_Progress);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsLoadingCompleted()
	{
		return WorkflowState == Enum_GridDataLoaderState::Done;
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void LoadParamsFromFile();
	virtual void LoadParams();
	virtual bool ParseParams(const FString& line);
	virtual bool ParseParamsByChild(int32 StartIndex, TArray<FString>& StrArr);
	virtual void SetParams();

	virtual void InitProgress();
	virtual void InitProgressTotal();

	virtual void LoadPointIndicesFromFile();
	virtual void LoadPointIndices();
	virtual void ParsePointIndexLine(const FString& line);
	virtual void AddPointIndex(FIntPoint key, int32 value);

	virtual void LoadPointsFromFile();
	virtual void LoadPoints();
	virtual void ParsePointLine(const FString& line);
	virtual void ParseAxialCoord(const FString& Str, FStructGridData& Data);
	virtual void ParsePosition2D(const FString& Str, FStructGridData& Data);
	virtual void ParseRange(const FString& Str, FStructGridData& Data);
	virtual void AddPoint(FStructGridData Data);

	virtual void LoadNeighborsFromFile();
	virtual void CreateNeighborPath(FString& NeighborPath, int32 Radius);
	virtual bool LoadNeighbors(int32 Radius);
	virtual void ParseNeighborsLine(const FString& Str, int32 Index, int32 Radius);
	virtual void ParseNeighbors(const FString& Str, int32 Index, int32 Radius);
	virtual void AddNeighbors(int32 Index, FStructGridDataNeighbors Neighbors);
	virtual bool PointIndicesContains(FIntPoint Point);

	virtual void CreatePointsVertices();
	virtual void InitPointVerticesVertors();
	virtual void CreatePointVertices(int32 Index);
	virtual void AddPosition2D(int32 Index, FVector2D Position2D);

	virtual void DoWorkFlowDone();

protected:
	void LoadDataFromFile(const FString& FileName, FStructLoopData* pLoopData,
		TFunction<void()> LoadDataFunc);
	bool LoadPointsDataLoopFunc(FStructLoopData& LoopData,
		TFunction<void(FString&)> ParseLineFunc,
		Enum_GridDataLoaderState StateNext, int32 ProgressWeight);
	bool PointsLoopFunction(TFunction<void()> InitFunc, TFunction<void(int32 LoopIndex)> LoopFunc,
		FStructLoopData& LoopData, Enum_GridDataLoaderState StateNext,
		bool bProgress = false, int32 ProgressWeight = 0);

	void ParseIntPoint(const FString& Str, FIntPoint& Point);
	void ParseVector2D(const FString& Str, FVector2D& Vec2D);
	void ParseVector(const FString& Str, FVector& Vec);
	void ParseInt(const FString& Str, int32& value);

private:
	void BindDelegate();

	UFUNCTION()
	void DoWorkFlow();

	void InitWorkflow();
	bool GetGameInstance();
	void InitLoopData();
	void ResetProgress();

	bool GetValidFilePath(const FString& RelPath, FString& FullPath);

};
