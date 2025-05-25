// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/MP_InventoryManager.h"

// Sets default values for this component's properties
UMP_InventoryManager::UMP_InventoryManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMP_InventoryManager::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UMP_InventoryManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

