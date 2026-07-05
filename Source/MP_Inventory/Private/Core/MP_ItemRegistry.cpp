// Copyright 2026 UVSquare. All Rights Reserved.


#include "Core/MP_ItemRegistry.h"
#include "Engine/AssetManager.h"

UMP_ItemDefinition* UMP_ItemRegistry::GetItemDefinition(FPrimaryAssetId ItemId) const
{
    if (!ItemId.IsValid()) return nullptr;

    UAssetManager& AssetManager = UAssetManager::Get();

    // This is a lightning-fast lookup in the engine's native hash map
    // It returns the asset IF it's loaded, or just the default object/pointer if it's not.
    UObject* LoadedAsset = AssetManager.GetPrimaryAssetObject(ItemId);

    // If it's not loaded into memory yet, synchronously load it right now!
    if (!LoadedAsset)
    {
        FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(ItemId);
        if (AssetPath.IsValid())
        {
            LoadedAsset = AssetPath.TryLoad(); // Forces it into memory immediately
        }
    }

    return Cast<UMP_ItemDefinition>(LoadedAsset);
}

UMP_ItemDefinition* UMP_ItemRegistry::GetItemDefinitionByName(FName ItemID) const
{
    // 1. Safety check
    if (ItemID.IsNone()) return nullptr;

    // 2. Convert your simple ItemID into the engine's Primary Asset ID
    // The first parameter MUST match the FName you set in GetPrimaryAssetId() inside the Data Asset class.
    FPrimaryAssetId ConstructedId = FPrimaryAssetId(FName("Item"), ItemID);

    // 3. Call our original high-performance function
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

void UMP_ItemRegistry::RegisterInventory(FName OwnerID, UMP_InventoryComponent* Inventory)
{
    if (OwnerID.IsNone() || !Inventory) return;
    ActiveInventories.Add(OwnerID, Inventory);
}

void UMP_ItemRegistry::UnregisterInventory(FName OwnerID)
{
    if (OwnerID.IsNone()) return;
    ActiveInventories.Remove(OwnerID);
}

UMP_InventoryComponent* UMP_ItemRegistry::GetInventoryByOwnerID(FName OwnerID) const
{
    if (OwnerID.IsNone()) return nullptr;
    if (UMP_InventoryComponent* const* Found = ActiveInventories.Find(OwnerID))
    {
        return *Found;
    }
    return nullptr;
}
