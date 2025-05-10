// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "M_LoAW_GridData/Public/FlowControlUtility.h"
#include "PriorityQueue.h"
#include "CoreMinimal.h"
#include "Algo/Reverse.h"

template <class T>
struct TStructBFSData
{
	TSet<T> Seed = {};
	TQueue<T> Frontier = {};
	TSet<T> Reached = {};
};

template <class T>
struct TStructAStarData
{
	PriorityQueue<T> Frontier = {};
	TMap<T, T> CameFrom = {};
	TMap<T, float> CostSoFar = {};
	T Goal;
};

/**
 * 
 */
class M_LOAW_TERRAIN_API AStarUtility
{
public:
	AStarUtility();
	~AStarUtility();

public:
	//Breadth First Search
	template <class T>
	FORCEINLINE static void BFSFunction(TStructBFSData<T>& BFSData,
		TFunction<bool(const T& Current, T& Next, int32& Index, TSet<T>& Reached)> NextFunc) 
	{
		while (!BFSData.Frontier.IsEmpty()) {
			T Current;
			BFSData.Frontier.Dequeue(Current);
			T Next;
			int32 Index = 0;

			while (NextFunc(Current, Next, Index, BFSData.Reached)) {
				if (!BFSData.Reached.Contains(Next)) {
					BFSData.Frontier.Enqueue(Next);
					BFSData.Reached.Add(Next);
				}
			}
		}
	}

	template <class T>
	FORCEINLINE static bool BFSLoopFunction(AActor* Owner,
		TStructBFSData<T>& BFSData,
		TArray<int32>& IndexSaved,
		FStructLoopData& LoopData,
		const FTimerDynamicDelegate& Delegate,
		TFunction<void()> InitFunc,
		TFunction<bool(const T& Current, T& Next, int32& Index, TSet<T>& Reached)> NextFunc)
	{
		int32 Count = 0;
		bool SaveLoopFlag = false;
		
		if (InitFunc && !LoopData.HasInitialized) {
			LoopData.HasInitialized = true;
			InitFunc();
		}

		while (!BFSData.Frontier.IsEmpty()) {
			FlowControlUtility::SaveLoopData(Owner, LoopData, Count, IndexSaved, Delegate, SaveLoopFlag);
			if (SaveLoopFlag) {
				return false;
			}

			T Current;
			BFSData.Frontier.Dequeue(Current);
			T Next;
			int32 Index = 0;

			while (NextFunc(Current, Next, Index, BFSData.Reached)) {
				if (!BFSData.Reached.Contains(Next)) {
					BFSData.Frontier.Enqueue(Next);
					BFSData.Reached.Add(Next);
				}
			}
		}
		return true;
	}

	//Breadth First Search for Chunk
	template <class T>
	FORCEINLINE static bool BFSFCLoopFunction(AActor* Owner,
		TStructBFSData<T>& BFSData,
		TArray<int32>& IndexSaved,
		FStructLoopData& LoopData,
		const FTimerDynamicDelegate& Delegate,
		TFunction<void()> InitFunc,
		TFunction<bool(const T& Current, T& Next, int32& Index)> NextFunc,
		TFunction<void()> DoAfterFunc)
	{
		int32 Count = 0;
		bool SaveLoopFlag = false;

		if (InitFunc && !LoopData.HasInitialized) {
			LoopData.HasInitialized = true;
			InitFunc();
		}

		while (!BFSData.Seed.IsEmpty()) {
			if (BFSData.Frontier.IsEmpty()) {
				TArray<T> arr = BFSData.Seed.Array();
				BFSData.Frontier.Enqueue(arr[0]);
				BFSData.Seed.Remove(arr[0]);
				BFSData.Reached.Empty();
				BFSData.Reached.Add(arr[0]);
			}
			while (!BFSData.Frontier.IsEmpty()) {
				FlowControlUtility::SaveLoopData(Owner, LoopData, Count, IndexSaved, Delegate, SaveLoopFlag);
				if (SaveLoopFlag) {
					return false;
				}

				T Current;
				BFSData.Frontier.Dequeue(Current);
				T Next;
				int32 Index = 0;

				while (NextFunc(Current, Next, Index)) {
					if (!BFSData.Reached.Contains(Next)
						&& BFSData.Seed.Contains(Next)) {
						BFSData.Frontier.Enqueue(Next);
						BFSData.Reached.Add(Next);
						BFSData.Seed.Remove(Next);
					}
				}
				Count++;
			}
			DoAfterFunc();
		}
		return true;
	}

	//Breadth First Search for early exit
	template <class T>
	FORCEINLINE static bool BFSEEFunction(TStructBFSData<T>& BFSData,
		TFunction<bool(const T& Current, bool& Res)> EEFunc,
		TFunction<bool(const T& Current, T& Next, int32& Index, TSet<T>& Reached)> NextFunc)
	{
		while (!BFSData.Frontier.IsEmpty()) {
			T Current;
			BFSData.Frontier.Dequeue(Current);
			bool Res;
			if (EEFunc(Current, Res)) {
				return Res;
			}
			T Next;
			int32 Index = 0;

			while (NextFunc(Current, Next, Index, BFSData.Reached)) {
				if (!BFSData.Reached.Contains(Next)) {
					BFSData.Frontier.Enqueue(Next);
					BFSData.Reached.Add(Next);
				}
			}
		}
		return false;
	}

	//A* Search
	template <class T>
	FORCEINLINE static bool AStarSearchLoopFunction(AActor* Owner,
		TStructAStarData<T>& AStarData,
		TArray<int32>& IndexSaved,
		FStructLoopData& LoopData,
		const FTimerDynamicDelegate& Delegate,
		TFunction<void()> InitFunc,
		TFunction<bool(const T& Current, T& Next, int32& Index)> NextFunc,
		TFunction<float(const T& Current, const T& Next)> Cost,
		TFunction<float(const T& Goal, const T& Next)> Heuristic
		)
	{
		int32 Count = 0;
		bool SaveLoopFlag = false;

		if (InitFunc && !LoopData.HasInitialized) {
			LoopData.HasInitialized = true;
			InitFunc();
		}
		
		while (!AStarData.Frontier.IsEmpty()) {
			FlowControlUtility::SaveLoopData(Owner, LoopData, Count, IndexSaved, Delegate, SaveLoopFlag);
			if (SaveLoopFlag) {
				return false;
			}

			T Current;
			Current = AStarData.Frontier.Pop();
			if (AStarData.Goal == Current) {
				break;
			}

			T Next;
			int32 Index = 0;
			float NewCost = 0.f;
			float Priority = 0.f;
			while (NextFunc(Current, Next, Index)) {
				NewCost = AStarData.CostSoFar[Current] + Cost(Current, Next);
				bool flag = false;
				if (!AStarData.CostSoFar.Contains(Next)) {
					AStarData.CostSoFar.Add(Next, NewCost);
					flag = true;
				}
				if (NewCost < AStarData.CostSoFar[Next]) {
					AStarData.CostSoFar[Next] = NewCost;
					flag = true;
				}
				if (flag) {
					Priority = NewCost + Heuristic(AStarData.Goal, Next);
					AStarData.Frontier.Push(Next, Priority);
					if (!AStarData.CameFrom.Contains(Next)) {
						AStarData.CameFrom.Add(Next, Current);
					}
					else {
						AStarData.CameFrom[Next] = Current;
					}
				}
			}
			Count++;
		}

		return true;
	}

	template <class T>
	FORCEINLINE static void AStarSearchFunction(TStructAStarData<T>& AStarData,
		TFunction<bool(const T& Current, T& Next, int32& Index)> NextFunc,
		TFunction<float(const T& Current, const T& Next)> Cost,
		TFunction<float(const T& Goal, const T& Next)> Heuristic
	)
	{
		while (!AStarData.Frontier.IsEmpty()) {
			T Current;
			Current = AStarData.Frontier.Pop();
			if (AStarData.Goal == Current) {
				break;
			}

			T Next;
			int32 Index = 0;
			float NewCost = 0.f;
			float Priority = 0.f;
			while (NextFunc(Current, Next, Index)) {
				NewCost = AStarData.CostSoFar[Current] + Cost(Current, Next);
				bool flag = false;
				if (!AStarData.CostSoFar.Contains(Next)) {
					AStarData.CostSoFar.Add(Next, NewCost);
					flag = true;
				}
				if (NewCost < AStarData.CostSoFar[Next]) {
					AStarData.CostSoFar[Next] = NewCost;
					flag = true;
				}
				if (flag) {
					Priority = NewCost + Heuristic(AStarData.Goal, Next);
					AStarData.Frontier.Push(Next, Priority);
					if (!AStarData.CameFrom.Contains(Next)) {
						AStarData.CameFrom.Add(Next, Current);
					}
					else {
						AStarData.CameFrom[Next] = Current;
					}
				}
			}
		}
	}

	template <class T>
	FORCEINLINE static void ReconstructPath(const T& Goal, const T& Start, 
		const TMap<T, T>& CameFrom, 
		TArray<T>& Path)
	{
		T Current = Goal;
		while (Current != Start) {
			Path.Add(Current);
			Current = CameFrom[Current];
		}
		Path.Add(Start);
		Algo::Reverse(Path);
	}

	template <class T>
	FORCEINLINE static void ClearAStarData(TStructAStarData<T>& AStarData)
	{
		AStarData.Frontier.Empty();
		AStarData.CameFrom.Empty();
		AStarData.CostSoFar.Empty();
	}
};
