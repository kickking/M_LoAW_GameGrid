// Fill out your copyright notice in the Description page of Project Settings.


#include "GridDataCreator.h"
#include "FlowControlUtility.h"
#include <fstream>
#include <filesystem>

DEFINE_LOG_CATEGORY(GridDataCreator);

AGridDataCreator::AGridDataCreator()
{
}

void AGridDataCreator::CreateData()
{
	if (IsInWorkingState()) return;

	UE_LOG(GridDataCreator, Log, TEXT("%s: CreateData()."), *CreatorName);
	BindDelegate();
	WorkflowState = Enum_GridDataCreatorState::InitWorkflow;
	DoWorkFlow();
}

void AGridDataCreator::GetProgress(float& Out_Progress)
{
	float Rate;
	if (ProgressTarget == 0) {
		Out_Progress = 0;
	}
	else {
		Rate = float(ProgressCurrent) / float(ProgressTarget);
		Rate = Rate > 1.0 ? 1.0 : Rate;
		Out_Progress = Rate;
	}
}

bool AGridDataCreator::CreateFilePath(const FString& RelPath, FString& FullPath)
{
	FullPath = FPaths::ProjectDir().Append(RelPath);
	FString Path = FPaths::GetPath(FullPath);
	if (!FPaths::DirectoryExists(Path)) {
		if (std::filesystem::create_directories(*Path)) {
			UE_LOG(GridDataCreator, Log, TEXT("%s: Create directory %s success."), *CreatorName, *Path);
			return true;
		}
	}
	return false;
}

void AGridDataCreator::WritePipeDelimiter(std::ofstream& ofs)
{
	ofs << TCHAR_TO_ANSI(*PipeDelim);
}

void AGridDataCreator::WriteColonDelimiter(std::ofstream& ofs)
{
	ofs << TCHAR_TO_ANSI(*ColonDelim);
}

void AGridDataCreator::WriteLineEnd(std::ofstream& ofs)
{
	ofs << std::endl;
}

FString AGridDataCreator::FloatToString(double value, int32 MaximumFractionalDigits)
{
	FNumberFormattingOptions NumberFormatOptions;
	NumberFormatOptions.AlwaysSign = false;
	NumberFormatOptions.UseGrouping = false;
	NumberFormatOptions.RoundingMode = ERoundingMode::HalfFromZero;
	NumberFormatOptions.MinimumIntegralDigits = 1;
	NumberFormatOptions.MaximumIntegralDigits = 324;
	NumberFormatOptions.MinimumFractionalDigits = 0;
	NumberFormatOptions.MaximumFractionalDigits = MaximumFractionalDigits;

	FText::AsNumber(value, &NumberFormatOptions);
	return FText::AsNumber(value, &NumberFormatOptions).ToString();
}

void AGridDataCreator::BindDelegate()
{
	WorkflowDelegate.BindUFunction(Cast<UObject>(this), TEXT("DoWorkFlow"));
}

void AGridDataCreator::DoWorkFlow()
{
	switch (WorkflowState)
	{
	case Enum_GridDataCreatorState::InitWorkflow:
		InitWorkflow();
		break;
	case Enum_GridDataCreatorState::SpiralCreateCenter:
		SpiralCreateCenter();
		break;
	case Enum_GridDataCreatorState::SpiralCreateNeighbors:
		SpiralCreateNeighbors();
		break;
	case Enum_GridDataCreatorState::WritePoints:
		WritePointsToFile();
		break;
	case Enum_GridDataCreatorState::WritePointsNeighbor:
		WriteNeighborsToFile();
		break;
	case Enum_GridDataCreatorState::WritePointIndices:
		WritePointIndicesToFile();
		break;
	case Enum_GridDataCreatorState::WriteParams:
		WriteParamsToFile();
		break;
	case Enum_GridDataCreatorState::Done:
		UE_LOG(GridDataCreator, Log, TEXT("%s: Create Grid data done."), *CreatorName);
		break;
	case Enum_GridDataCreatorState::Error:
		UE_LOG(GridDataCreator, Warning, TEXT("%s: DoWorkFlow Error!"), *CreatorName);
		break;
	default:
		break;
	}
}

void AGridDataCreator::InitWorkflow()
{
	InitLoopData();
	InitByChild();

	FTimerHandle TimerHandle;
	WorkflowState = Enum_GridDataCreatorState::SpiralCreateCenter;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(GridDataCreator, Log, TEXT("%s: Init workflow done."), *CreatorName);
}

void AGridDataCreator::InitLoopData()
{
	FlowControlUtility::InitLoopData(SpiralCreateCenterLoopData);
	SpiralCreateCenterLoopData.IndexSaved[0] = 1;
	FlowControlUtility::InitLoopData(SpiralCreateNeighborsLoopData);
	SpiralCreateNeighborsLoopData.IndexSaved[1] = 1;
	FlowControlUtility::InitLoopData(WritePointsLoopData);
	FlowControlUtility::InitLoopData(WriteNeighborsLoopData);
	WriteNeighborsLoopData.IndexSaved[0] = 1;
	FlowControlUtility::InitLoopData(WritePointIndicesLoopData);
}

void AGridDataCreator::InitByChild()
{
}

void AGridDataCreator::ResetProgress()
{
	ProgressTarget = 0;
	ProgressCurrent = 0;
}

void AGridDataCreator::SpiralCreateCenter()
{
	bool OnceLoop0 = true;
	bool OnceLoop1 = true;
	int32 Count = 0;
	TArray<int32> Indices = { 0, 0, 0 };
	bool SaveLoopFlag = false;

	if (!SpiralCreateCenterLoopData.HasInitialized) {
		SpiralCreateCenterLoopData.HasInitialized = true;
		RingInitFlag = false;
		InitGridCenter();
		ProgressTarget = NeighborStep * (1 + GridRange) * GridRange / 2;
	}

	int32 i = SpiralCreateCenterLoopData.IndexSaved[0];
	i = i < 1 ? 1 : i;
	int32 j, k;

	for (; i <= GridRange; i++)
	{
		Indices[0] = i;
		if (!RingInitFlag) {
			RingInitFlag = true;
			InitGridRing(i);
		}

		j = OnceLoop0 ? SpiralCreateCenterLoopData.IndexSaved[1] : 0;
		for (; j < NeighborStep; j++) {
			Indices[1] = j;
			k = OnceLoop1 ? SpiralCreateCenterLoopData.IndexSaved[2] : 0;
			for (; k <= i - 1; k++) {
				Indices[2] = k;
				FlowControlUtility::SaveLoopData(this, SpiralCreateCenterLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
				if (SaveLoopFlag) {
					return;
				}
				AddRingPointAndIndex();
				FindNeighborPointOfRing(j);

				ProgressCurrent = SpiralCreateCenterLoopData.Count;
				Count++;
			}
			OnceLoop1 = false;
		}
		RingInitFlag = false;
		OnceLoop0 = false;
	}
	ResetProgress();

	FTimerHandle TimerHandle;
	WorkflowState = Enum_GridDataCreatorState::SpiralCreateNeighbors;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, SpiralCreateCenterLoopData.Rate, false);
	UE_LOG(GridDataCreator, Log, TEXT("%s: Spiral create center done."), *CreatorName);
}

void AGridDataCreator::InitGridCenter()
{
	FStructGridData Data;
	Data.AxialCoord.X = 0;
	Data.AxialCoord.Y = 0;
	Data.Position2D.Set(0.0, 0.0);
	Points.Empty();
	Points.Add(Data);

	PointIndices.Empty();
	PointIndices.Add(FIntPoint(0, 0), 0);
}

void AGridDataCreator::InitGridRing(int32 Radius)
{
}

int32 AGridDataCreator::AddRingPointAndIndex()
{
	return 0;
}

void AGridDataCreator::FindNeighborPointOfRing(int32 DirIndex)
{
}

void AGridDataCreator::SpiralCreateNeighbors()
{
	bool OnceLoop0 = true;
	bool OnceLoop1 = true;
	bool OnceLoop2 = true;
	int32 Count = 0;
	TArray<int32> Indices = { 0, 0, 0, 0 };
	bool SaveLoopFlag = false;

	if (!SpiralCreateNeighborsLoopData.HasInitialized) {
		SpiralCreateNeighborsLoopData.HasInitialized = true;
		RingInitFlag = false;
		ProgressTarget = Points.Num() * NeighborStep * (1 + NeighborRange) * NeighborRange / 2;
	}

	int32 PointIndex = SpiralCreateNeighborsLoopData.IndexSaved[0];
	int32 i, j, k;
	FIntPoint center;

	for (; PointIndex < Points.Num(); PointIndex++)
	{
		Indices[0] = PointIndex;
		center = Points[PointIndex].AxialCoord;
		i = OnceLoop0 ? SpiralCreateNeighborsLoopData.IndexSaved[1] : 1;
		i = i < 1 ? 1 : i;
		for (; i <= NeighborRange; i++) {
			Indices[1] = i;
			if (!RingInitFlag) {
				RingInitFlag = true;
				AddPointNeighbor(PointIndex, i);
				InitNeighborRing(i, center);
			}

			j = OnceLoop1 ? SpiralCreateNeighborsLoopData.IndexSaved[2] : 0;
			for (; j < NeighborStep; j++) {
				Indices[2] = j;
				k = OnceLoop2 ? SpiralCreateNeighborsLoopData.IndexSaved[3] : 0;
				for (; k <= i - 1; k++) {
					Indices[3] = k;
					FlowControlUtility::SaveLoopData(this, SpiralCreateNeighborsLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
					if (SaveLoopFlag) {
						return;
					}
					SetPointNeighbor(PointIndex, i, j);
					ProgressCurrent = SpiralCreateNeighborsLoopData.Count;
					Count++;
				}
				OnceLoop2 = false;
			}
			RingInitFlag = false;
			OnceLoop1 = false;
		}
		OnceLoop0 = false;
	}
	ResetProgress();

	FTimerHandle TimerHandle;
	WorkflowState = Enum_GridDataCreatorState::WritePoints;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, SpiralCreateNeighborsLoopData.Rate, false);
	UE_LOG(GridDataCreator, Log, TEXT("%s: Spiral create neighbors done."), *CreatorName);

}

void AGridDataCreator::AddPointNeighbor(int32 PointIndex, int32 Radius)
{
	FStructGridDataNeighbors neighbors;
	neighbors.Radius = Radius;
	Points[PointIndex].Neighbors.Add(neighbors);
}

void AGridDataCreator::InitNeighborRing(int32 Radius, FIntPoint center)
{
}

void AGridDataCreator::SetPointNeighbor(int32 PointIndex, int32 Radius, int32 DirIndex)
{
}

void AGridDataCreator::WriteDataToFile(const FString& FileName,
	FStructLoopData* pLoopData, TFunction<void(std::ofstream&)> WriteDataFunc)
{
	FString FullPath;
	FString DataPath = FString(TEXT(""));
	DataPath.Append(DataFileRelPath).Append(FileName);
	CreateFilePath(DataPath, FullPath);

	FTimerHandle TimerHandle;
	std::ofstream ofs;
	float rate = DefaultTimerRate;

	if (pLoopData) {
		if (!pLoopData->HasInitialized) {
			pLoopData->HasInitialized = true;
			ofs.open(*FullPath, std::ios::out | std::ios::trunc);
			ProgressTarget = Points.Num();
		}
		else {
			ofs.open(*FullPath, std::ios::out | std::ios::app);
		}
		rate = pLoopData->Rate;
	}
	else {
		ofs.open(*FullPath, std::ios::out | std::ios::trunc);
		ProgressTarget = 1;
	}

	if (!ofs || !ofs.is_open()) {
		UE_LOG(GridDataCreator, Warning, TEXT("%s: Open file %s failed!"), *CreatorName, *FullPath);
		WorkflowState = Enum_GridDataCreatorState::Error;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, rate, false);
		return;
	}
	WriteDataFunc(ofs);
}

void AGridDataCreator::WritePointsDataLoopFunc(std::ofstream& ofs, 
	FStructLoopData& LoopData,
	TFunction<void(std::ofstream&, int32)> WriteLineFunc, 
	Enum_GridDataCreatorState state)
{
	int32 Count = 0;
	TArray<int32> Indices = { 0 };
	bool SaveLoopFlag = false;

	int32 i = LoopData.IndexSaved[0];
	for (; i < Points.Num(); i++)
	{
		Indices[0] = i;
		FlowControlUtility::SaveLoopData(this, LoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
		if (SaveLoopFlag) {
			ofs.close();
			return;
		}
		WriteLineFunc(ofs, i);
		ProgressCurrent = WritePointsLoopData.Count;
		Count++;
	}
	ofs.close();
	FTimerHandle TimerHandle;
	WorkflowState = state;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, LoopData.Rate, false);
}

void AGridDataCreator::WritePointsToFile()
{
	WriteDataToFile(PointsDataFileName, &WritePointsLoopData, 
		[this](std::ofstream& ofs) { WritePoints(ofs); });
}

void AGridDataCreator::WritePoints(std::ofstream& ofs)
{
	WritePointsDataLoopFunc(ofs, WritePointsLoopData,
		[this](std::ofstream& stream, int32 i) { WritePointLine(stream, i); },
		Enum_GridDataCreatorState::WritePointsNeighbor);
	UE_LOG(GridDataCreator, Log, TEXT("%s: Write points done."), *CreatorName);
}

void AGridDataCreator::WritePointLine(std::ofstream& ofs, int32 Index)
{
	FStructGridData Data = Points[Index];
	WriteAxialCoord(ofs, Data);
	WritePipeDelimiter(ofs);
	WritePosition2D(ofs, Data);
	WriteLineEnd(ofs);
}

void AGridDataCreator::WriteAxialCoord(std::ofstream& ofs, const FStructGridData& Data)
{
	FIntPoint AC = Data.AxialCoord;
	FString Str = FString::FromInt(AC.X);
	Str.Append(*CommaDelim);
	Str.Append(FString::FromInt(AC.Y));
	ofs << TCHAR_TO_ANSI(*Str);
}

void AGridDataCreator::WritePosition2D(std::ofstream& ofs, const FStructGridData& Data)
{
	FVector2D Pos2D = Data.Position2D;
	FString Str = FloatToString(Pos2D.X, 2);
	Str.Append(*CommaDelim);
	Str.Append(FloatToString(Pos2D.Y, 2));
	ofs << TCHAR_TO_ANSI(*Str);
}

void AGridDataCreator::WriteNeighborsToFile()
{
	int32 i = WriteNeighborsLoopData.IndexSaved[0];
	FTimerHandle TimerHandle;
	std::ofstream ofs;
	ProgressTarget = Points.Num() * CalNeighborsWeight(NeighborRange);

	for (; i <= NeighborRange; i++)
	{
		FString NeighborPath;
		FString FullPath;
		CreateNeighborPath(NeighborPath, i);
		CreateFilePath(NeighborPath, FullPath);

		if (!WriteNeighborsLoopData.HasInitialized) {
			WriteNeighborsLoopData.HasInitialized = true;
			ofs.open(*FullPath, std::ios::out | std::ios::trunc);
		}
		else {
			ofs.open(*FullPath, std::ios::out | std::ios::app);
		}

		if (!ofs || !ofs.is_open()) {
			UE_LOG(GridDataCreator, Warning, TEXT("%s: Open file %s failed!"), *CreatorName, *FullPath);
			WorkflowState = Enum_GridDataCreatorState::Error;
			GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, WriteNeighborsLoopData.Rate, false);
			return;
		}
		if (!WriteNeighbors(ofs, i)) {
			return;
		}
	}

	WorkflowState = Enum_GridDataCreatorState::WritePointIndices;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, WriteNeighborsLoopData.Rate, false);
	UE_LOG(GridDataCreator, Log, TEXT("%s: Write neighbors done."), *CreatorName);
}

int32 AGridDataCreator::CalNeighborsWeight(int32 Range)
{
	int32 weight = 0;
	for (int32 i = 1; i <= Range; i++)
	{
		weight += i * NeighborStep;
	}
	return weight;
}

void AGridDataCreator::CreateNeighborPath(FString& NeighborPath, int32 Radius)
{
	FString NeighborPathPrefix = FString(TEXT(""));
	NeighborPathPrefix.Append(DataFileRelPath).Append(TEXT("N"));
	NeighborPath.Append(NeighborPathPrefix).Append(FString::FromInt(Radius)).Append(FString(TEXT(".data")));
}

bool AGridDataCreator::WriteNeighbors(std::ofstream& ofs, int32 Radius)
{
	int32 Count = 0;
	TArray<int32> Indices = { Radius, 0 };
	bool SaveLoopFlag = false;

	int32 ProgressPassed = Points.Num() * CalNeighborsWeight(Radius - 1);
	int32 ProgressRatio = Radius * NeighborStep;
	int32 i = WriteNeighborsLoopData.IndexSaved[1];

	for (; i < Points.Num(); i++)
	{
		Indices[1] = i;
		FlowControlUtility::SaveLoopData(this, WriteNeighborsLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
		if (SaveLoopFlag) {
			ofs.close();
			return false;
		}
		WriteNeighborLine(ofs, i, Radius);
		ProgressCurrent = ProgressPassed + WriteNeighborsLoopData.Count * ProgressRatio;
		Count++;
	}

	ofs.close();
	FlowControlUtility::InitLoopData(WriteNeighborsLoopData);
	WriteNeighborsLoopData.IndexSaved[0] = Radius;
	UE_LOG(GridDataCreator, Log, TEXT("%s: Write neighbor N%d done."), *CreatorName, Radius);
	return true;
}

void AGridDataCreator::WriteNeighborLine(std::ofstream& ofs, int32 Index, int32 Radius)
{
	FStructGridDataNeighbors Neighbors = Points[Index].Neighbors[Radius - 1];
	for (int32 i = 0; i < Neighbors.Points.Num(); i++)
	{
		FString Str = FString::FromInt(Neighbors.Points[i].X);
		Str.Append(*CommaDelim);
		Str.Append(FString::FromInt(Neighbors.Points[i].Y));
		if (i != Neighbors.Points.Num() - 1) {
			Str.Append(*SpaceDelim);
		}
		ofs << TCHAR_TO_ANSI(*Str);
	}
	WriteLineEnd(ofs);
}

void AGridDataCreator::WritePointIndicesToFile()
{
	WriteDataToFile(PointIndicesDataFileName, &WritePointIndicesLoopData,
		[this](std::ofstream& ofs) { WritePointIndices(ofs); });
}

void AGridDataCreator::WritePointIndices(std::ofstream& ofs)
{
	WritePointsDataLoopFunc(ofs, WritePointIndicesLoopData,
		[this](std::ofstream& stream, int32 i) { WritePointIndicesLine(stream, i); },
		Enum_GridDataCreatorState::WriteParams);
	UE_LOG(GridDataCreator, Log, TEXT("%s: Write points indices done."), *CreatorName);
}

void AGridDataCreator::WritePointIndicesLine(std::ofstream& ofs, int32 Index)
{
	FIntPoint key = Points[Index].AxialCoord;
	WriteIndicesKey(ofs, key);
	WritePipeDelimiter(ofs);
	WriteIndicesValue(ofs, Index);
	WriteLineEnd(ofs);
}

void AGridDataCreator::WriteIndicesKey(std::ofstream& ofs, const FIntPoint& key)
{
	FString Str = FString::FromInt(key.X);
	Str.Append(*CommaDelim);
	Str.Append(FString::FromInt(key.Y));
	ofs << TCHAR_TO_ANSI(*Str);
}

void AGridDataCreator::WriteIndicesValue(std::ofstream& ofs, int32 Index)
{
	FString Str = FString::FromInt(Index);
	ofs << TCHAR_TO_ANSI(*Str);
}

void AGridDataCreator::WriteParamsToFile()
{
	WriteDataToFile(ParamsDataFileName, nullptr,
		[this](std::ofstream& ofs) { WriteParams(ofs); });
}

void AGridDataCreator::WriteParams(std::ofstream& ofs)
{
	WriteParamsContent(ofs);
	ProgressCurrent = 1;
	ofs.close();
	FTimerHandle TimerHandle;
	WorkflowState = Enum_GridDataCreatorState::Done;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(GridDataCreator, Log, TEXT("%s: Write params done."), *CreatorName);
}

void AGridDataCreator::WriteParamsContent(std::ofstream& ofs)
{
	FString Str = FString::FromInt(GridRange);
	ofs << TCHAR_TO_ANSI(*Str);
	WritePipeDelimiter(ofs);
	Str = FString::FromInt(NeighborRange);
	ofs << TCHAR_TO_ANSI(*Str);
	WritePipeDelimiter(ofs);
	Str = FString::FromInt(Points.Num());
	ofs << TCHAR_TO_ANSI(*Str);

	WriteParamsContentByChild(ofs);

	WriteLineEnd(ofs);
}

void AGridDataCreator::WriteParamsContentByChild(std::ofstream& ofs)
{
}



