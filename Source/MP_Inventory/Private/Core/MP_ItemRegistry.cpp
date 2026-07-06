// Copyright 2026 UVSquare. All Rights Reserved.


#include "Core/MP_ItemRegistry.h"
#include "Engine/AssetManager.h"

UMP_ItemDefinition* UMP_ItemRegistry::GetItemDefinition(FPrimaryAssetId ItemId) const
{
    if (!ItemId.IsValid()) return nullptr;

    UAssetManager& AssetManager = UAssetManager::Get();

    UObject* LoadedAsset = AssetManager.GetPrimaryAssetObject(ItemId);
    if (!LoadedAsset)
    {
        FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(ItemId);
        if (AssetPath.IsValid())
        {
            LoadedAsset = AssetPath.TryLoad();
        }
    }

    return Cast<UMP_ItemDefinition>(LoadedAsset);
}

UMP_ItemDefinition* UMP_ItemRegistry::GetItemDefinitionByName(FName ItemID) const
{
    if (ItemID.IsNone()) return nullptr;

    FPrimaryAssetId ConstructedId = FPrimaryAssetId(FName("Item"), ItemID);
    return GetItemDefinition(ConstructedId);
}

TArray<UMP_ItemDefinition*> UMP_ItemRegistry::GetItemsByTag(FGameplayTagContainer Tags) const
{
    TArray<UMP_ItemDefinition*> Result;
    if (Tags.IsEmpty()) return Result;

    UAssetManager& AssetManager = UAssetManager::Get();
    TArray<UObject*> AllItems;

    // Grabs everything registered under the "Item" primary asset type
    AssetManager.GetPrimaryAssetObjectList(FPrimaryAssetType("Item"), AllItems);

    for (UObject* Obj : AllItems)
    {
        if (UMP_ItemDefinition* ItemDef = Cast<UMP_ItemDefinition>(Obj))
        {
            if (ItemDef->Tags.HasAny(Tags))
            {
                Result.Add(ItemDef);
            }
        }
    }
    return Result;
}

void UMP_ItemRegistry::RegisterInventory(FName ComponentID, UMP_InventoryComponent* Inventory)
{
    if (ComponentID.IsNone() || !Inventory) return;
    ActiveInventories.Add(ComponentID, Inventory);
}

void UMP_ItemRegistry::UnregisterInventory(FName ComponentID)
{
    if (ComponentID.IsNone()) return;
    ActiveInventories.Remove(ComponentID);
}

UMP_InventoryComponent* UMP_ItemRegistry::GetInventoryByComponentID(FName ComponentID) const
{
    if (ComponentID.IsNone()) return nullptr;
    if (UMP_InventoryComponent* const* Found = ActiveInventories.Find(ComponentID))
    {
        return *Found;
    }
    return nullptr;
}
