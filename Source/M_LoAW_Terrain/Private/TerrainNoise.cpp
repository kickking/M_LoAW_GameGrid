// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainNoise.h"

DEFINE_LOG_CATEGORY(TerrainNoise);

// Sets default values
ATerrainNoise::ATerrainNoise()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

}

bool ATerrainNoise::Create()
{
	NWLandLayer0 = NewObject<UFastNoiseWrapper>(this);
	NWLandLayer1 = NewObject<UFastNoiseWrapper>(this);
	NWRiverDirection = NewObject<UFastNoiseWrapper>(this);
	NWRiverDepth = NewObject<UFastNoiseWrapper>(this);
	NWWater = NewObject<UFastNoiseWrapper>(this);
	NWMoisture = NewObject<UFastNoiseWrapper>(this);
	NWTemperature = NewObject<UFastNoiseWrapper>(this);
	NWTree = NewObject<UFastNoiseWrapper>(this);

	if (NWLandLayer0 != nullptr && NWLandLayer1 != nullptr &&
		NWRiverDirection != nullptr && NWRiverDepth != nullptr && 
		NWWater != nullptr && NWMoisture != nullptr && NWTemperature != nullptr && 
		NWTree != nullptr) {

		NWLandLayer0->SetupFastNoise(NW_Land_Layer_0_NoiseType,
			NW_Land_Layer_0_NoiseSeed,
			NW_Land_Layer_0_NoiseFrequency,
			NW_Land_Layer_0_Interp,
			NW_Land_Layer_0_FractalType,
			NW_Land_Layer_0_Octaves,
			NW_Land_Layer_0_Lacunarity,
			NW_Land_Layer_0_Gain,
			NW_Land_Layer_0_CellularJitter,
			NW_Land_Layer_0_CDF,
			NW_Land_Layer_0_CRT);

		NWLandLayer1->SetupFastNoise(NW_Land_Layer_1_NoiseType,
			NW_Land_Layer_1_NoiseSeed,
			NW_Land_Layer_1_NoiseFrequency,
			NW_Land_Layer_1_Interp,
			NW_Land_Layer_1_FractalType,
			NW_Land_Layer_1_Octaves,
			NW_Land_Layer_1_Lacunarity,
			NW_Land_Layer_1_Gain,
			NW_Land_Layer_1_CellularJitter,
			NW_Land_Layer_1_CDF,
			NW_Land_Layer_1_CRT);

		NWRiverDirection->SetupFastNoise(NWRiverDirection_NoiseType,
			NWRiverDirection_NoiseSeed,
			NWRiverDirection_NoiseFrequency,
			NWRiverDirection_Interp,
			NWRiverDirection_FractalType,
			NWRiverDirection_Octaves,
			NWRiverDirection_Lacunarity,
			NWRiverDirection_Gain,
			NWRiverDirection_CellularJitter,
			NWRiverDirection_CDF,
			NWRiverDirection_CRT);

		NWRiverDepth->SetupFastNoise(NWRiverDepth_NoiseType,
			NWRiverDepth_NoiseSeed,
			NWRiverDepth_NoiseFrequency,
			NWRiverDepth_Interp,
			NWRiverDepth_FractalType,
			NWRiverDepth_Octaves,
			NWRiverDepth_Lacunarity,
			NWRiverDepth_Gain,
			NWRiverDepth_CellularJitter,
			NWRiverDepth_CDF,
			NWRiverDepth_CRT);

		NWWater->SetupFastNoise(NWWater_NoiseType,
			NWWater_NoiseSeed,
			NWWater_NoiseFrequency,
			NWWater_Interp,
			NWWater_FractalType,
			NWWater_Octaves,
			NWWater_Lacunarity,
			NWWater_Gain,
			NWWater_CellularJitter,
			NWWater_CDF,
			NWWater_CRT);

		NWMoisture->SetupFastNoise(NWMoisture_NoiseType,
			NWMoisture_NoiseSeed,
			NWMoisture_NoiseFrequency,
			NWMoisture_Interp,
			NWMoisture_FractalType,
			NWMoisture_Octaves,
			NWMoisture_Lacunarity,
			NWMoisture_Gain,
			NWMoisture_CellularJitter,
			NWMoisture_CDF,
			NWMoisture_CRT);

		NWTemperature->SetupFastNoise(NWTemperature_NoiseType,
			NWTemperature_NoiseSeed,
			NWTemperature_NoiseFrequency,
			NWTemperature_Interp,
			NWTemperature_FractalType,
			NWTemperature_Octaves,
			NWTemperature_Lacunarity,
			NWTemperature_Gain,
			NWTemperature_CellularJitter,
			NWTemperature_CDF,
			NWTemperature_CRT);

		NWTree->SetupFastNoise(NWTree_NoiseType,
			NWTree_NoiseSeed,
			NWTree_NoiseFrequency,
			NWTree_Interp,
			NWTree_FractalType,
			NWTree_Octaves,
			NWTree_Lacunarity,
			NWTree_Gain,
			NWTree_CellularJitter,
			NWTree_CDF,
			NWTree_CRT);

		UE_LOG(TerrainNoise, Log, TEXT("Create Noise successfully."));
		return true;
	}
	return false;
}

// Called when the game starts or when spawned
void ATerrainNoise::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATerrainNoise::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

