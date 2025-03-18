// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "FastNoiseWrapper.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrainNoise.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(TerrainNoise, Log, All);

UCLASS()
class M_LOAW_TERRAIN_API ATerrainNoise : public AActor
{
	GENERATED_BODY()
	
private:
	//noise param for high mountain
	EFastNoise_NoiseType NWHighMountain_NoiseType = EFastNoise_NoiseType::PerlinFractal;
	EFastNoise_Interp NWHighMountain_Interp = EFastNoise_Interp::Quintic;
	EFastNoise_FractalType NWHighMountain_FractalType = EFastNoise_FractalType::RigidMulti;
	int32 NWHighMountain_Octaves = 6;
	float NWHighMountain_Lacunarity = 2.0;
	float NWHighMountain_Gain = 0.5;
	float NWHighMountain_CellularJitter = 0.45;
	EFastNoise_CellularDistanceFunction NWHighMountain_CDF = EFastNoise_CellularDistanceFunction::Euclidean;
	EFastNoise_CellularReturnType NWHighMountain_CRT = EFastNoise_CellularReturnType::CellValue;

	//noise param for low mountain
	EFastNoise_NoiseType NWLowMountain_NoiseType = EFastNoise_NoiseType::PerlinFractal;
	EFastNoise_Interp NWLowMountain_Interp = EFastNoise_Interp::Quintic;
	EFastNoise_FractalType NWLowMountain_FractalType = EFastNoise_FractalType::RigidMulti;
	int32 NWLowMountain_Octaves = 6;
	float NWLowMountain_Lacunarity = 2.0;
	float NWLowMountain_Gain = 0.5;
	float NWLowMountain_CellularJitter = 0.45;
	EFastNoise_CellularDistanceFunction NWLowMountain_CDF = EFastNoise_CellularDistanceFunction::Euclidean;
	EFastNoise_CellularReturnType NWLowMountain_CRT = EFastNoise_CellularReturnType::CellValue;

	//noise param for river direction
	EFastNoise_NoiseType NWRiverDirection_NoiseType = EFastNoise_NoiseType::PerlinFractal;
	EFastNoise_Interp NWRiverDirection_Interp = EFastNoise_Interp::Quintic;
	EFastNoise_FractalType NWRiverDirection_FractalType = EFastNoise_FractalType::RigidMulti;
	int32 NWRiverDirection_Octaves = 6;
	float NWRiverDirection_Lacunarity = 2.0;
	float NWRiverDirection_Gain = 0.5;
	float NWRiverDirection_CellularJitter = 0.45;
	EFastNoise_CellularDistanceFunction NWRiverDirection_CDF = EFastNoise_CellularDistanceFunction::Euclidean;
	EFastNoise_CellularReturnType NWRiverDirection_CRT = EFastNoise_CellularReturnType::CellValue;

	//noise param for river depth
	EFastNoise_NoiseType NWRiverDepth_NoiseType = EFastNoise_NoiseType::PerlinFractal;
	EFastNoise_Interp NWRiverDepth_Interp = EFastNoise_Interp::Quintic;
	EFastNoise_FractalType NWRiverDepth_FractalType = EFastNoise_FractalType::FBM;
	int32 NWRiverDepth_Octaves = 2;
	float NWRiverDepth_Lacunarity = 2.0;
	float NWRiverDepth_Gain = 0.5;
	float NWRiverDepth_CellularJitter = 0.45;
	EFastNoise_CellularDistanceFunction NWRiverDepth_CDF = EFastNoise_CellularDistanceFunction::Euclidean;
	EFastNoise_CellularReturnType NWRiverDepth_CRT = EFastNoise_CellularReturnType::CellValue;

	//noise param for water
	EFastNoise_NoiseType NWWater_NoiseType = EFastNoise_NoiseType::PerlinFractal;
	EFastNoise_Interp NWWater_Interp = EFastNoise_Interp::Quintic;
	EFastNoise_FractalType NWWater_FractalType = EFastNoise_FractalType::FBM;
	int32 NWWater_Octaves = 2;
	float NWWater_Lacunarity = 2.0;
	float NWWater_Gain = 0.5;
	float NWWater_CellularJitter = 0.45;
	EFastNoise_CellularDistanceFunction NWWater_CDF = EFastNoise_CellularDistanceFunction::Euclidean;
	EFastNoise_CellularReturnType NWWater_CRT = EFastNoise_CellularReturnType::CellValue;

	//noise param for moisture
	EFastNoise_NoiseType NWMoisture_NoiseType = EFastNoise_NoiseType::PerlinFractal;
	EFastNoise_Interp NWMoisture_Interp = EFastNoise_Interp::Quintic;
	EFastNoise_FractalType NWMoisture_FractalType = EFastNoise_FractalType::FBM;
	int32 NWMoisture_Octaves = 3;
	float NWMoisture_Lacunarity = 2.0;
	float NWMoisture_Gain = 0.5;
	float NWMoisture_CellularJitter = 0.45;
	EFastNoise_CellularDistanceFunction NWMoisture_CDF = EFastNoise_CellularDistanceFunction::Euclidean;
	EFastNoise_CellularReturnType NWMoisture_CRT = EFastNoise_CellularReturnType::CellValue;

	//noise param for temperature
	EFastNoise_NoiseType NWTemperature_NoiseType = EFastNoise_NoiseType::PerlinFractal;
	EFastNoise_Interp NWTemperature_Interp = EFastNoise_Interp::Quintic;
	EFastNoise_FractalType NWTemperature_FractalType = EFastNoise_FractalType::FBM;
	int32 NWTemperature_Octaves = 3;
	float NWTemperature_Lacunarity = 2.0;
	float NWTemperature_Gain = 0.5;
	float NWTemperature_CellularJitter = 0.45;
	EFastNoise_CellularDistanceFunction NWTemperature_CDF = EFastNoise_CellularDistanceFunction::Euclidean;
	EFastNoise_CellularReturnType NWTemperature_CRT = EFastNoise_CellularReturnType::CellValue;

	//noise param for biomes
	EFastNoise_NoiseType NWBiomes_NoiseType = EFastNoise_NoiseType::PerlinFractal;
	EFastNoise_Interp NWBiomes_Interp = EFastNoise_Interp::Quintic;
	EFastNoise_FractalType NWBiomes_FractalType = EFastNoise_FractalType::FBM;
	int32 NWBiomes_Octaves = 3;
	float NWBiomes_Lacunarity = 2.0;
	float NWBiomes_Gain = 0.5;
	float NWBiomes_CellularJitter = 0.45;
	EFastNoise_CellularDistanceFunction NWBiomes_CDF = EFastNoise_CellularDistanceFunction::Euclidean;
	EFastNoise_CellularReturnType NWBiomes_CRT = EFastNoise_CellularReturnType::CellValue;

	//noise param for tree
	EFastNoise_NoiseType NWTree_NoiseType = EFastNoise_NoiseType::PerlinFractal;
	EFastNoise_Interp NWTree_Interp = EFastNoise_Interp::Quintic;
	EFastNoise_FractalType NWTree_FractalType = EFastNoise_FractalType::FBM;
	int32 NWTree_Octaves = 6;
	float NWTree_Lacunarity = 2.0;
	float NWTree_Gain = 0.5;
	float NWTree_CellularJitter = 0.45;
	EFastNoise_CellularDistanceFunction NWTree_CDF = EFastNoise_CellularDistanceFunction::Euclidean;
	EFastNoise_CellularReturnType NWTree_CRT = EFastNoise_CellularReturnType::CellValue;

protected:
	//Noise variables BP for high mountain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise|HighMountain")
	int32 NWHighMountain_NoiseSeed = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise|HighMountain", meta = (ClampMin = "0.0"))
	float NWHighMountain_NoiseFrequency = 0.005;
	//Noise variables BP for low mountain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise|LowMountain")
	int32 NWLowMountain_NoiseSeed = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise|LowMountain", meta = (ClampMin = "0.0"))
	float NWLowMountain_NoiseFrequency = 0.01;
	//Noise variables BP for river direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise|RiverDirection")
	int32 NWRiverDirection_NoiseSeed = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise|RiverDirection", meta = (ClampMin = "0.0"))
	float NWRiverDirection_NoiseFrequency = 0.02;
	//Noise variables BP for river flow
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise|RiverDepth")
	int32 NWRiverDepth_NoiseSeed = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise|RiverDepth", meta = (ClampMin = "0.0"))
	float NWRiverDepth_NoiseFrequency = 0.02;
	//Noise variables BP for water
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise|Water")
	int32 NWWater_NoiseSeed = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise|Water", meta = (ClampMin = "0.0"))
	float NWWater_NoiseFrequency = 0.01;
	//Noise variables BP for moisture
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise|Moisture")
	int32 NWMoisture_NoiseSeed = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise|Moisture", meta = (ClampMin = "0.0"))
	float NWMoisture_NoiseFrequency = 0.007;
	//Noise variables BP for temperature
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise|Temperature")
	int32 NWTemperature_NoiseSeed = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise|Temperature", meta = (ClampMin = "0.0"))
	float NWTemperature_NoiseFrequency = 0.007;
	//Noise variables BP for biomes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise|Biomes")
	int32 NWBiomes_NoiseSeed = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise|Biomes", meta = (ClampMin = "0.0"))
	float NWBiomes_NoiseFrequency = 0.007;
	//Noise variables BP for tree
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise|Tree")
	int32 NWTree_NoiseSeed = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Noise|Tree", meta = (ClampMin = "0.0"))
	float NWTree_NoiseFrequency = 0.01;

public:
	UFastNoiseWrapper* NWHighMountain = nullptr;
	UFastNoiseWrapper* NWLowMountain = nullptr;
	UFastNoiseWrapper* NWRiverDirection = nullptr;
	UFastNoiseWrapper* NWRiverDepth = nullptr;
	UFastNoiseWrapper* NWWater = nullptr;
	UFastNoiseWrapper* NWMoisture = nullptr;
	UFastNoiseWrapper* NWTemperature = nullptr;
	UFastNoiseWrapper* NWBiomes = nullptr;
	UFastNoiseWrapper* NWTree = nullptr;

public:	
	// Sets default values for this actor's properties
	ATerrainNoise();

	bool Create();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
