// Copyright 2026 UVSquare. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Database/MP_ItemDefinition.h"
#include "MP_ItemRegistry.generated.h"

/**
 * Global Registry for querying Item Definitions.
 * Persists across level loads. Uses Asset Manager backend.
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

};