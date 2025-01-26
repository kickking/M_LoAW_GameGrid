// Fill out your copyright notice in the Description page of Project Settings.


#include "DataCreator.h"

#include <filesystem>
#include <fstream>

DEFINE_LOG_CATEGORY(DataCreator);

// Sets default values
ADataCreator::ADataCreator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

}


