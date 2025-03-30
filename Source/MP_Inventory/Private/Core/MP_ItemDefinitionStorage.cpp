// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/MP_ItemDefinitionStorage.h"

UMP_ItemDefinitionStorage::UMP_ItemDefinitionStorage()
{
    // No hardcoded initialization, fully Blueprint-controlled
}

const FMP_ItemDefinition& UMP_ItemDefinitionStorage::GetItemDefinition(FName ItemID) const
{
    static FMP_ItemDefinition DefaultDefinition; // Fallback if not found
    const FMP_ItemDefinition* FoundDefinition = ItemDefinitions.Find(ItemID);
    return FoundDefinition ? *FoundDefinition : DefaultDefinition;
}

UMP_ItemDefinitionStorage* UMP_ItemDefinitionStorage::Get()
{
    static UMP_ItemDefinitionStorage* Instance = nullptr;
    if (!Instance)
    {
        Instance = NewObject<UMP_ItemDefinitionStorage>();
        Instance->AddToRoot(); // Prevent garbage collection
    }
    return Instance;
}
