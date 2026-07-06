// Copyright 2026 UVSquare. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/MP_ItemDefinition.h"
#include "MP_ItemRegistry.generated.h"

/**
 * Subsystem responsible for providing global access to item definitions via the Asset Manager.
 * Operates across level transitions, serving as the central caching and lookup authority
 * for querying statically defined inventory items by their primary asset identifiers.
 */
UCLASS()
class MP_INVENTORY_API UMP_ItemRegistry : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Fast synchronous lookup (Assumes asset is known to the manager)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Registry")
    UMP_ItemDefinition* GetItemDefinition(FPrimaryAssetId ItemId) const;

    // The exact function you need for your FastArray and Database logic
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Registry")
    UMP_ItemDefinition* GetItemDefinitionByName(FName ItemID) const;

    // Fetch all items matching a tag
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Registry")
    TArray<UMP_ItemDefinition*> GetItemsByTag(FGameplayTagContainer Tags) const;

    // =========================================================================
    //  GLOBAL INVENTORY TRACKING
    // =========================================================================

    /** Registers an inventory component so it can be found globally by its ComponentID. */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Registry")
    void RegisterInventory(FName ComponentID, class UMP_InventoryComponent* Inventory);

    /** Unregisters an inventory component when it is destroyed. */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Registry")
    void UnregisterInventory(FName ComponentID);

    /** Retrieves an active inventory component anywhere in the world by its ComponentID. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Registry")
    class UMP_InventoryComponent* GetInventoryByComponentID(FName ComponentID) const;

private:
    UPROPERTY()
    TMap<FName, class UMP_InventoryComponent*> ActiveInventories;
};
