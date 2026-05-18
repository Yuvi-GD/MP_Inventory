// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/MP_ItemDefinitionStorage.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UMP_ItemDefinitionStorage::UMP_ItemDefinitionStorage()
{
    // No hardcoded initialization, fully Blueprint-controlled
}

TWeakObjectPtr<UMP_ItemDefinitionStorage> UMP_ItemDefinitionStorage::SingletonInstance = nullptr;

UMP_ItemDefinitionStorage* UMP_ItemDefinitionStorage::Get(UObject* WorldContextObject)
{
    if (SingletonInstance.IsValid())
        return SingletonInstance.Get();

    UObject* Outer = WorldContextObject ? WorldContextObject : GetTransientPackage();
    UMP_ItemDefinitionStorage* NewInstance = NewObject<UMP_ItemDefinitionStorage>(Outer);
    NewInstance->AddToRoot(); // Avoid GC
    SingletonInstance = NewInstance;
    return NewInstance;
}

void UMP_ItemDefinitionStorage::AddItemDefinition(const FMP_ItemDefinition& Definition)
{
    if (Definition.ItemID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("AddItemDefinition: ItemID is None!"));
        return;
    }
    ItemDefinitions.Add(Definition.ItemID, Definition);
}

void UMP_ItemDefinitionStorage::AddDefinitionsFromArray(const TArray<FMP_ItemDefinition>& Definitions)
{
    for (const FMP_ItemDefinition& Def : Definitions)
    {
        AddItemDefinition(Def);
    }
}

void UMP_ItemDefinitionStorage::RemoveItemDefinition(FName ItemID)
{
    ItemDefinitions.Remove(ItemID);
}

void UMP_ItemDefinitionStorage::UpdateItemDefinition(const FMP_ItemDefinition& Definition)
{
    if (Definition.ItemID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateItemDefinition: ItemID is None!"));
        return;
    }
    ItemDefinitions.Add(Definition.ItemID, Definition); // Overwrites if exists
}

FMP_ItemDefinition UMP_ItemDefinitionStorage::GetItemDefinition(FName ItemID) const
{
    const FMP_ItemDefinition* Found = ItemDefinitions.Find(ItemID);
    return Found ? *Found : FMP_ItemDefinition();
}

TArray<FMP_ItemDefinition> UMP_ItemDefinitionStorage::GetItemDefinitionByTag(FGameplayTagContainer Tags) const
{
	if (Tags.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetItemDefinitionByTag: Tags are empty!"));
		return TArray<FMP_ItemDefinition>();
	}

	TArray<FMP_ItemDefinition> Result;

	// Find all definitions that match any of the tags
	for (const auto& Pair : ItemDefinitions)
	{
		if (Pair.Value.Tags.HasAll(Tags))
		{
			Result.Add(Pair.Value);
		}
	}
	if (Result.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetItemDefinitionByTag: No definitions found for tags: %s"), *Tags.ToString());
	}
	return Result;
}

TArray<FMP_ItemDefinition> UMP_ItemDefinitionStorage::GetAllDefinitions() const
{
    TArray<FMP_ItemDefinition> All;
    ItemDefinitions.GenerateValueArray(All);
    return All;
}

void UMP_ItemDefinitionStorage::PrintAllDefinitions() const
{
    UE_LOG(LogTemp, Log, TEXT("---- All Item Definitions ----"));
    for (const auto& Pair : ItemDefinitions)
    {
        UE_LOG(LogTemp, Log, TEXT("ItemID: %s, DisplayName: %s, BasePrice: %.2f"),
            *Pair.Key.ToString(),
            *Pair.Value.DisplayName,
            Pair.Value.BasePrice);
    }
}

