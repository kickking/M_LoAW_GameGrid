// Fill out your copyright notice in the Description page of Project Settings.


#include "GridDataLoader.h"
#include "FlowControlUtility.h"
#include <string>
#include <kismet/KismetStringLibrary.h>
#include "M_LoAW_GameBase/Public/LoAWGameInstance.h"

DEFINE_LOG_CATEGORY(GridDataLoader);

// Sets default values
AGridDataLoader::AGridDataLoader() : pGI(nullptr)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	BindDelegate();

}

void AGridDataLoader::GetProgress(float& Out_Progress)
{
	float Rate;
	if (ProgressTotal == 0) {
		Out_Progress = 0;
	}
	else {
		Rate = float(ProgressCurrent) / float(ProgressTotal);
		Rate = Rate > 1.0 ? 1.0 : Rate;
		Out_Progress = Rate;
	}
}

// Called when the game starts or when spawned
void AGridDataLoader::BeginPlay()
{
	Super::BeginPlay();
	WorkflowState = Enum_GridDataLoaderState::Init;
	DoWorkFlow();
}

void AGridDataLoader::BindDelegate()
{
	WorkflowDelegate.BindUFunction(Cast<UObject>(this), TEXT("DoWorkFlow"));
}

void AGridDataLoader::DoWorkFlow()
{
	switch (WorkflowState)
	{
	case Enum_GridDataLoaderState::Init:
		InitWorkflow();
		break;
	case Enum_GridDataLoaderState::LoadParams:
		LoadParamsFromFile();
		break;
	case Enum_GridDataLoaderState::InitProgress:
		InitProgress();
		break;
	case Enum_GridDataLoaderState::LoadPointIndices:
		LoadPointIndicesFromFile();
		break;
	case Enum_GridDataLoaderState::LoadPoints:
		LoadPointsFromFile();
		break;
	case Enum_GridDataLoaderState::LoadNeighbors:
		LoadNeighborsFromFile();
		break;
	case Enum_GridDataLoaderState::CreatePointsVertices:
		CreatePointsVertices();
		break;
	case Enum_GridDataLoaderState::Done:
		DoWorkFlowDone();
		break;
	case Enum_GridDataLoaderState::Error:
		UE_LOG(GridDataLoader, Warning, TEXT("%s: DoWorkFlow Error!"), *LoaderName);
		break;
	default:
		break;
	}
}

void AGridDataLoader::InitWorkflow()
{
	FTimerHandle TimerHandle;

	InitLoopData();
	ResetProgress();
	if (!GetGameInstance()) {
		WorkflowState = Enum_GridDataLoaderState::Error;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
		UE_LOG(GridDataLoader, Log, TEXT("%s: GetGameInstance Error!"), *LoaderName);
		return;
	}
	
	WorkflowState = Enum_GridDataLoaderState::LoadParams;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(GridDataLoader, Log, TEXT("%s: Init workflow done!"), *LoaderName);
}

bool AGridDataLoader::GetGameInstance()
{
	UWorld* world = GetWorld();
	if (world) {
		pGI = Cast<ULoAWGameInstance>(world->GetGameInstance());
		if (pGI) {
			return true;
		}
	}
	return false;
}

void AGridDataLoader::InitLoopData()
{
	FlowControlUtility::InitLoopData(LoadPointIndicesLoopData);
	FlowControlUtility::InitLoopData(LoadPointsLoopData);
	FlowControlUtility::InitLoopData(LoadNeighborsLoopData);
	LoadNeighborsLoopData.IndexSaved[0] = 1;
	FlowControlUtility::InitLoopData(CreatePointsVerticesLoopData);
}

void AGridDataLoader::ResetProgress()
{
	ProgressTotal = 0;
	ProgressPassed = 0;
	ProgressCurrent = 0;
}

bool AGridDataLoader::GetValidFilePath(const FString& RelPath, FString& FullPath)
{
	bool flag = false;
	FullPath = FPaths::ProjectDir().Append(RelPath);
	if (FPaths::FileExists(FullPath)) {
		flag = true;
	}
	return flag;
}

void AGridDataLoader::LoadDataFromFile(const FString& FileName, 
	FStructLoopData* pLoopData, TFunction<void()> LoadDataFunc)
{
	FTimerHandle TimerHandle;
	float rate = DefaultTimerRate;

	FString FullPath;
	FString DataPath = FString(TEXT(""));
	DataPath.Append(DataFileRelPath).Append(FileName);
	if (!GetValidFilePath(DataPath, FullPath)) {
		UE_LOG(GridDataLoader, Warning, TEXT("%s: data file %s not exist!"), *LoaderName, *FullPath);
		WorkflowState = Enum_GridDataLoaderState::Error;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, rate, false);
		return;
	}

	if (pLoopData) {
		if (!pLoopData->HasInitialized) {
			pLoopData->HasInitialized = true;
			DataLoadStream.open(*FullPath, std::ios::in);
		}
		rate = pLoopData->Rate;
	}
	else {
		DataLoadStream.open(*FullPath, std::ios::in);
	}

	if (!DataLoadStream || !DataLoadStream.is_open()) {
		UE_LOG(GridDataLoader, Warning, TEXT("%s: Open file %s failed!"), *LoaderName, *FullPath);
		WorkflowState = Enum_GridDataLoaderState::Error;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, rate, false);
		return;
	}
	LoadDataFunc();
}

bool AGridDataLoader::LoadPointsDataLoopFunc(FStructLoopData& LoopData, 
	TFunction<void(FString&)> ParseLineFunc, Enum_GridDataLoaderState StateNext,
	int32 ProgressWeight)
{
	int32 Count = 0;
	TArray<int32> Indices = { 0 };
	bool SaveLoopFlag = false;

	std::string line;
	while (std::getline(DataLoadStream, line))
	{
		FString fline(line.c_str());
		ParseLineFunc(fline);
		Count++;

		FlowControlUtility::SaveLoopData(this, LoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
		if (SaveLoopFlag) {
			LoopData.Count++;
			return false;
		}
		ProgressCurrent = ProgressPassed + LoopData.Count * ProgressWeight;
	}

	ProgressPassed = ProgressCurrent;

	DataLoadStream.close();
	FTimerHandle TimerHandle;
	WorkflowState = StateNext;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, LoopData.Rate, false);
	return true;
}

bool AGridDataLoader::PointsLoopFunction(TFunction<void()> InitFunc, 
	TFunction<void(int32 LoopIndex)> LoopFunc, FStructLoopData& LoopData, 
	Enum_GridDataLoaderState StateNext, bool bProgress, int32 ProgressWeight)
{
	int32 Count = 0;
	TArray<int32> Indices = { 0 };
	bool SaveLoopFlag = false;

	if (InitFunc && !LoopData.HasInitialized) {
		LoopData.HasInitialized = true;
		InitFunc();
	}

	int32 i = LoopData.IndexSaved[0];
	for (; i < PointsNum; i++)
	{
		Indices[0] = i;
		FlowControlUtility::SaveLoopData(this, LoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
		if (SaveLoopFlag) {
			return false;
		}
		LoopFunc(i);
		Count++;
		if (bProgress) {
			ProgressCurrent = ProgressPassed + LoopData.Count * ProgressWeight;
		}
	}

	if (bProgress) {
		ProgressPassed = ProgressCurrent;
	}

	FTimerHandle TimerHandle;
	WorkflowState = StateNext;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, LoopData.Rate, false);
	return true;
}

void AGridDataLoader::ParseIntPoint(const FString& Str, FIntPoint& Point)
{
	TArray<FString> StrArr;
	Str.ParseIntoArray(StrArr, *CommaDelim, true);
	Point.X = UKismetStringLibrary::Conv_StringToInt(StrArr[0]);
	Point.Y = UKismetStringLibrary::Conv_StringToInt(StrArr[1]);
}

void AGridDataLoader::ParseVector2D(const FString& Str, FVector2D& Vec2D)
{
	TArray<FString> StrArr;
	Str.ParseIntoArray(StrArr, *CommaDelim, true);
	LexFromString(Vec2D.X, StrArr[0]);
	LexFromString(Vec2D.Y, StrArr[1]);
}

void AGridDataLoader::ParseVector(const FString& Str, FVector& Vec)
{
	TArray<FString> StrArr;
	Str.ParseIntoArray(StrArr, *CommaDelim, true);
	LexFromString(Vec.X, StrArr[0]);
	LexFromString(Vec.Y, StrArr[1]);
	LexFromString(Vec.Z, StrArr[2]);
}

void AGridDataLoader::ParseInt(const FString& Str, int32& value)
{
	value = UKismetStringLibrary::Conv_StringToInt(Str);
}

void AGridDataLoader::LoadParamsFromFile()
{
	LoadDataFromFile(ParamsDataFileName, nullptr, [this]() { LoadParams(); });
}

void AGridDataLoader::LoadParams()
{
	FTimerHandle TimerHandle;
	std::string line;
	std::getline(DataLoadStream, line);
	FString fline(line.c_str());
	if (!ParseParams(fline)) {
		DataLoadStream.close();
		UE_LOG(GridDataLoader, Warning, TEXT("%s: Parse Parameters error!"), *LoaderName);
		WorkflowState = Enum_GridDataLoaderState::Error;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
		return;
	}

	DataLoadStream.close();
	WorkflowState = Enum_GridDataLoaderState::InitProgress;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(GridDataLoader, Log, TEXT("%s: Load params done!"), *LoaderName);
}

bool AGridDataLoader::ParseParams(const FString& line)
{
	TArray<FString> StrArr;
	line.ParseIntoArray(StrArr, *PipeDelim, true);

	if (StrArr.Num() != ParamNum) {
		return false;
	}

	GridRange = UKismetStringLibrary::Conv_StringToInt(StrArr[0]);
	NeighborRange = UKismetStringLibrary::Conv_StringToInt(StrArr[1]);
	PointsNum = UKismetStringLibrary::Conv_StringToInt(StrArr[2]);
	if (!ParseParamsByChild(3, StrArr)) {
		return false;
	}
	SetParams();
	return true;
}

bool AGridDataLoader::ParseParamsByChild(int32 StartIndex, TArray<FString>& StrArr)
{
	return true;
}

void AGridDataLoader::SetParams()
{
}

void AGridDataLoader::InitProgress()
{
	InitProgressTotal();
	FTimerHandle TimerHandle;
	WorkflowState = Enum_GridDataLoaderState::LoadPointIndices;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(GridDataLoader, Log, TEXT("%s: Init progress done!"), *LoaderName);
}

void AGridDataLoader::InitProgressTotal()
{
	ProgressTotal += ProgressWeight_LoadPointIndices * PointsNum;
	ProgressTotal += ProgressWeight_LoadPoints * PointsNum;
	ProgressTotal += ProgressWeight_LoadNeighbors * PointsNum * NeighborRange;
}

void AGridDataLoader::LoadPointIndicesFromFile()
{
	LoadDataFromFile(PointIndicesDataFileName, &LoadPointIndicesLoopData, 
		[this]() { LoadPointIndices(); });
}

void AGridDataLoader::LoadPointIndices()
{
	if (LoadPointsDataLoopFunc(LoadPointIndicesLoopData,
		[this](FString& line) { ParsePointIndexLine(line); },
		Enum_GridDataLoaderState::LoadPoints, ProgressWeight_LoadPointIndices))
	{
		UE_LOG(GridDataLoader, Log, TEXT("%s: Load point indices done!"), *LoaderName);
	}
}

void AGridDataLoader::ParsePointIndexLine(const FString& line)
{
	TArray<FString> StrArr;
	FIntPoint key;
	int32 value;
	line.ParseIntoArray(StrArr, *PipeDelim, true);
	ParseIntPoint(StrArr[0], key);
	ParseInt(StrArr[1], value);
	AddPointIndex(key, value);
}

void AGridDataLoader::AddPointIndex(FIntPoint key, int32 value)
{
}

void AGridDataLoader::LoadPointsFromFile()
{
	LoadDataFromFile(PointsDataFileName, &LoadPointsLoopData,
		[this]() { LoadPoints(); });
}

void AGridDataLoader::LoadPoints()
{
	if (LoadPointsDataLoopFunc(LoadPointsLoopData,
		[this](FString& line) { ParsePointLine(line); },
		Enum_GridDataLoaderState::LoadNeighbors, ProgressWeight_LoadPoints))
	{
		UE_LOG(GridDataLoader, Log, TEXT("%s: Load points done!"), *LoaderName);
	}
}

void AGridDataLoader::ParsePointLine(const FString& line)
{
	TArray<FString> StrArr;
	line.ParseIntoArray(StrArr, *PipeDelim, true);
	FStructGridData Data;
	ParseAxialCoord(StrArr[0], Data);
	AddPoint(Data);
}

void AGridDataLoader::ParseAxialCoord(const FString& Str, FStructGridData& Data)
{
	ParseIntPoint(Str, Data.AxialCoord);
}

void AGridDataLoader::ParsePosition2D(const FString& Str, FStructGridData& Data)
{
	ParseVector2D(Str, Data.Position2D);
}

void AGridDataLoader::ParseRange(const FString& Str, FStructGridData& Data)
{
	ParseInt(Str, Data.RangeFromCenter);
}

void AGridDataLoader::AddPoint(FStructGridData Data)
{
}

void AGridDataLoader::LoadNeighborsFromFile()
{
	int32 i = LoadNeighborsLoopData.IndexSaved[0];
	FTimerHandle TimerHandle;

	for (; i <= NeighborRange; i++)
	{
		FString NeighborPath;
		FString FullPath;

		CreateNeighborPath(NeighborPath, i);

		if (!GetValidFilePath(NeighborPath, FullPath)) {
			UE_LOG(GridDataLoader, Warning, TEXT("%s: Neighbor path file %s not exist!"), *LoaderName, *NeighborPath);
			WorkflowState = Enum_GridDataLoaderState::Error;
			GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, LoadNeighborsLoopData.Rate, false);
			return;
		}

		if (!LoadNeighborsLoopData.HasInitialized) {
			LoadNeighborsLoopData.HasInitialized = true;
			DataLoadStream.open(*FullPath, std::ios::in);
		}

		if (!DataLoadStream || !DataLoadStream.is_open()) {
			UE_LOG(GridDataLoader, Warning, TEXT("%s: Open file %s failed!"), *LoaderName, *FullPath);
			WorkflowState = Enum_GridDataLoaderState::Error;
			GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, LoadNeighborsLoopData.Rate, false);
			return;
		}
		if (!LoadNeighbors(i)) {
			return;
		}
	}

	WorkflowState = Enum_GridDataLoaderState::CreatePointsVertices;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, LoadNeighborsLoopData.Rate, false);
	UE_LOG(GridDataLoader, Log, TEXT("%s: Load neighbors done!"), *LoaderName);
}

void AGridDataLoader::CreateNeighborPath(FString& NeighborPath, int32 Radius)
{
	NeighborPath.Append(DataFileRelPath).Append(NeighborsDataFileNamePrefix);
	NeighborPath.Append(FString::FromInt(Radius)).Append(FString(TEXT(".data")));
}

bool AGridDataLoader::LoadNeighbors(int32 Radius)
{
	int32 Count = 0;
	TArray<int32> Indices = { Radius, 0 };
	bool SaveLoopFlag = false;
	int32 CountAdd = int32((Radius - 1) * LoadNeighborsLoopData.LoopCountLimit * NeighborCountAddRatio);
	CountAdd = CountAdd < NeighborCountAddMin ? NeighborCountAddMin : CountAdd;

	int32 i = LoadNeighborsLoopData.IndexSaved[1];
	std::string line;
	while (std::getline(DataLoadStream, line))
	{
		FString fline(line.c_str());
		ParseNeighborsLine(fline, i, Radius);
		i++;
		Count++;
		Indices[1] = i;
		FlowControlUtility::SaveLoopData(this, LoadNeighborsLoopData, Count + CountAdd, Indices, WorkflowDelegate, SaveLoopFlag);
		if (SaveLoopFlag) {
			LoadNeighborsLoopData.Count++;
			return false;
		}
		ProgressCurrent = ProgressPassed + LoadNeighborsLoopData.Count * ProgressWeight_LoadNeighbors;
	}

	ProgressPassed = ProgressCurrent;
	DataLoadStream.close();
	FlowControlUtility::InitLoopData(LoadNeighborsLoopData);
	LoadNeighborsLoopData.IndexSaved[0] = Radius;
	UE_LOG(GridDataLoader, Log, TEXT("%s: Load neighbor N%d done!"), *LoaderName, Radius);
	return true;
}

void AGridDataLoader::ParseNeighborsLine(const FString& Str, int32 Index, int32 Radius)
{
	ParseNeighbors(Str, Index, Radius);
}

void AGridDataLoader::ParseNeighbors(const FString& Str, int32 Index, int32 Radius)
{
	FStructGridDataNeighbors Neighbors;
	Neighbors.Radius = Radius;
	int32 Count = 0;
	TArray<FString> StrArr;
	Str.ParseIntoArray(StrArr, *SpaceDelim, true);
	for (int32 i = 0; i < StrArr.Num(); i++)
	{
		FIntPoint Point;
		ParseIntPoint(StrArr[i], Point);
		if (PointIndicesContains(Point)) {
			Neighbors.Points.Add(Point);
			Count++;
		}
	}
	Neighbors.Count = Count;
	AddNeighbors(Index, Neighbors);
}

void AGridDataLoader::AddNeighbors(int32 Index, FStructGridDataNeighbors Neighbors)
{
}

bool AGridDataLoader::PointIndicesContains(FIntPoint Point)
{
	return false;
}


void AGridDataLoader::CreatePointsVertices()
{
	FTimerHandle TimerHandle;
	WorkflowState = Enum_GridDataLoaderState::Done;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(GridDataLoader, Log, TEXT("%s: Create points vertices done!"), *LoaderName);
}

void AGridDataLoader::InitPointVerticesVertors()
{
}

void AGridDataLoader::CreatePointVertices(int32 Index)
{
}

void AGridDataLoader::AddPosition2D(int32 Index, FVector2D Position2D)
{
}

void AGridDataLoader::DoWorkFlowDone()
{
}
