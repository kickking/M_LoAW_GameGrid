// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataCreator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(DataCreator, Log, All);

UCLASS()
class LOAW_GRIDDATACREATOR_API ADataCreator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADataCreator();

	virtual void CreateData()
	{
	}

};
