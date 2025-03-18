// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

template <class ElementType>
struct TStructPriorityQueueNode
{
	ElementType Element;
	float Priority;

	TStructPriorityQueueNode() {}

	TStructPriorityQueueNode(ElementType InElement, float InPriority) {
		Element = InElement;
		Priority = InPriority;
	}

	bool operator<(const TStructPriorityQueueNode<ElementType> Node) const {
		return Priority < Node.Priority;
	}
};

/**
 * 
 */
template <class ElementType>
class M_LOAW_TERRAIN_API PriorityQueue
{
private:
	TArray<TStructPriorityQueueNode<ElementType>> Queue = {};

public:
	PriorityQueue() { Queue.Heapify(); };
	~PriorityQueue() {};

	FORCEINLINE void Push(ElementType Element, float Priority) {
		Queue.HeapPush(TStructPriorityQueueNode<ElementType>(Element, Priority));
	}

	FORCEINLINE bool IsEmpty() const {
		return Queue.Num() == 0;
	}

	FORCEINLINE ElementType Pop() {
		TStructPriorityQueueNode<ElementType> Node;
		Queue.HeapPop(Node);
		return Node.Element;
	}
};

