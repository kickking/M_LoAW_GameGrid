// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainNiagaraUnit.h"
#include "NiagaraComponent.h"
#include "ProceduralMeshComponent.h"

// Sets default values
ATerrainNiagaraUnit::ATerrainNiagaraUnit()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	NiagaraUnit = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraUnit"));
	this->SetRootComponent(NiagaraUnit);
	NiagaraUnit->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	NiagaraUnit->SetAutoActivate(false);
	SetActive(false);

}

void ATerrainNiagaraUnit::SetActive(bool flag)
{
	if (NiagaraUnit) {
		NiagaraUnit->SetActive(flag);
	}
}

// Called when the game starts or when spawned
void ATerrainNiagaraUnit::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ATerrainNiagaraUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

