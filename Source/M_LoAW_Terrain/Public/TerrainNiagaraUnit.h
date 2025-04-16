// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrainNiagaraUnit.generated.h"

UCLASS()
class M_LOAW_TERRAIN_API ATerrainNiagaraUnit : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	class UNiagaraComponent* NiagaraUnit;

public:	
	// Sets default values for this actor's properties
	ATerrainNiagaraUnit();

	void SetActive(bool flag);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
