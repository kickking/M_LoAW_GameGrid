// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GridDataStructDefine.h"

#include "CoreMinimal.h"
#include "DataCreator.h"
#include "GridDataCreator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(GridDataCreator, Log, All);

UENUM(BlueprintType)
enum class Enum_GridDataCreatorState : uint8
{
	InitWorkflow,
	SpiralCreateCenter,
	SpiralCreateNeighbors,
	WritePoints,
	WritePointsNeighbor,
	WritePointIndices,
	WriteParams,
	Done,
	Error
};

UCLASS()
class M_LOAW_GRIDDATA_API AGridDataCreator : public ADataCreator
{
	GENERATED_BODY()

private:
	FTimerDynamicDelegate WorkflowDelegate;

	int32 ProgressTarget = 0;
	int32 ProgressCurrent = 0;

	bool RingInitFlag = false;

	FString PipeDelim = FString(TEXT("|"));
	FString CommaDelim = FString(TEXT(","));
	FString SpaceDelim = FString(TEXT(" "));
	FString ColonDelim = FString(TEXT(":"));

protected:
	int32 NeighborStep = 0;

	TArray<FStructGridData> Points;
	TMap<FIntPoint, int32> PointIndices;

protected:
	UPROPERTY(BlueprintReadOnly)
	FString CreatorName = FString(TEXT(""));

	UPROPERTY(BlueprintReadWrite)
	Enum_GridDataCreatorState WorkflowState = Enum_GridDataCreatorState::Done;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Timer")
	float DefaultTimerRate = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData SpiralCreateCenterLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData SpiralCreateNeighborsLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData WritePointsLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData WriteNeighborsLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	FStructLoopData WritePointIndicesLoopData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Params", meta = (ClampMin = "1"))
	int32 GridRange = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Params", meta = (ClampMin = "1"))
	int32 NeighborRange = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Path")
	FString DataFileRelPath = FString(TEXT(""));
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Path")
	FString PointsDataFileName = FString(TEXT("Points.data"));
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Path")
	FString PointIndicesDataFileName = FString(TEXT("PointIndices.data"));
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom|Path")
	FString ParamsDataFileName = FString(TEXT("Params.data"));

public:
	AGridDataCreator();

	virtual void CreateData() override;

	UFUNCTION(BlueprintCallable)
	void GetProgress(float& Out_Progress);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsInWorkingState()
	{
		return WorkflowState < Enum_GridDataCreatorState::Done;
	}

protected:
	bool CreateFilePath(const FString& RelPath, FString& FullPath);

	void WritePipeDelimiter(std::ofstream& ofs);
	void WriteColonDelimiter(std::ofstream& ofs);
	void WriteLineEnd(std::ofstream& ofs);

	FString FloatToString(double value, int32 MaximumFractionalDigits);

protected:
	virtual void InitWorkflow();
	virtual void InitLoopData();
	virtual void InitByChild();

	virtual void SpiralCreateCenter();
	virtual void InitGridCenter();
	virtual void InitGridRing(int32 Radius);
	virtual int32 AddRingPointAndIndex(int32 Range);
	virtual void FindNeighborPointOfRing(int32 DirIndex);

	virtual void SpiralCreateNeighbors();
	virtual void AddPointNeighbor(int32 PointIndex, int32 Radius);
	virtual void InitNeighborRing(int32 Radius, FIntPoint center);
	virtual void SetPointNeighbor(int32 PointIndex, int32 Radius, int32 DirIndex);

	virtual void WritePointsToFile();
	virtual void WritePoints(std::ofstream& ofs);
	virtual void WritePointLine(std::ofstream& ofs, int32 Index);
	virtual void WriteAxialCoord(std::ofstream& ofs, const FStructGridData& Data);
	virtual void WritePosition2D(std::ofstream& ofs, const FStructGridData& Data);
	virtual void WriteRange(std::ofstream& ofs, const FStructGridData& Data);

	virtual void WriteNeighborsToFile();
	virtual int32 CalNeighborsWeight(int32 Range);
	virtual void CreateNeighborPath(FString& NeighborPath, int32 Radius);
	virtual bool WriteNeighbors(std::ofstream& ofs, int32 Radius);
	virtual void WriteNeighborLine(std::ofstream& ofs, int32 Index, int32 Radius);

	virtual void WritePointIndicesToFile();
	virtual void WritePointIndices(std::ofstream& ofs);
	virtual void WritePointIndicesLine(std::ofstream& ofs, int32 Index);
	virtual void WriteIndicesKey(std::ofstream& ofs, const FIntPoint& key);
	virtual void WriteIndicesValue(std::ofstream& ofs, int32 Index);

	virtual void WriteParamsToFile();
	virtual void WriteParams(std::ofstream& ofs);
	virtual void WriteParamsContent(std::ofstream& ofs);
	virtual void WriteParamsContentByChild(std::ofstream& ofs);

private:
	void BindDelegate();

	UFUNCTION(CallInEditor)
	virtual void DoWorkFlow();

	void WriteDataToFile(const FString& FileName, FStructLoopData* pLoopData, 
		TFunction<void(std::ofstream&)> WriteDataFunc);
	void WritePointsDataLoopFunc(std::ofstream& ofs, FStructLoopData& LoopData,
		TFunction<void(std::ofstream&, int32)> WriteLineFunc, 
		Enum_GridDataCreatorState state);
	void ResetProgress();

};
