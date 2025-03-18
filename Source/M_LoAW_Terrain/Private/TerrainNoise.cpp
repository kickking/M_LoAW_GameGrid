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
	NWHighMountain = NewObject<UFastNoiseWrapper>(this);
	NWLowMountain = NewObject<UFastNoiseWrapper>(this);
	NWRiverDirection = NewObject<UFastNoiseWrapper>(this);
	NWRiverDepth = NewObject<UFastNoiseWrapper>(this);
	NWWater = NewObject<UFastNoiseWrapper>(this);
	NWMoisture = NewObject<UFastNoiseWrapper>(this);
	NWTemperature = NewObject<UFastNoiseWrapper>(this);
	NWBiomes = NewObject<UFastNoiseWrapper>(this);
	NWTree = NewObject<UFastNoiseWrapper>(this);

	if (NWHighMountain != nullptr && NWLowMountain != nullptr && 
		NWRiverDirection != nullptr && NWRiverDepth != nullptr && 
		NWWater != nullptr && NWMoisture != nullptr && NWTemperature != nullptr && 
		NWBiomes != nullptr && NWTree != nullptr) {
		NWHighMountain->SetupFastNoise(NWHighMountain_NoiseType,
			NWHighMountain_NoiseSeed,
			NWHighMountain_NoiseFrequency,
			NWHighMountain_Interp,
			NWHighMountain_FractalType,
			NWHighMountain_Octaves,
			NWHighMountain_Lacunarity,
			NWHighMountain_Gain,
			NWHighMountain_CellularJitter,
			NWHighMountain_CDF,
			NWHighMountain_CRT);

		NWLowMountain->SetupFastNoise(NWLowMountain_NoiseType,
			NWLowMountain_NoiseSeed,
			NWLowMountain_NoiseFrequency,
			NWLowMountain_Interp,
			NWLowMountain_FractalType,
			NWLowMountain_Octaves,
			NWLowMountain_Lacunarity,
			NWLowMountain_Gain,
			NWLowMountain_CellularJitter,
			NWLowMountain_CDF,
			NWLowMountain_CRT);

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

		NWBiomes->SetupFastNoise(NWBiomes_NoiseType,
			NWBiomes_NoiseSeed,
			NWBiomes_NoiseFrequency,
			NWBiomes_Interp,
			NWBiomes_FractalType,
			NWBiomes_Octaves,
			NWBiomes_Lacunarity,
			NWBiomes_Gain,
			NWBiomes_CellularJitter,
			NWBiomes_CDF,
			NWBiomes_CRT);

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

