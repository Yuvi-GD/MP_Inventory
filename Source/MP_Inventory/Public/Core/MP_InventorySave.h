// Copyright 2026 UVSquare. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/MP_InventoryStruct.h"
#include "Data/MP_InventoryFastArray.h"
#include "GameFramework/SaveGame.h"
#include "MP_InventorySave.generated.h"

/**
 * SaveGame class to store inventory data. Uses a TMap to associate player IDs with their inventory arrays.
 */
UCLASS(Blueprintable)
class MP_INVENTORY_API UMP_InventorySave : public USaveGame
{
	GENERATED_BODY()
	
public:

	// Map of player IDs to their inventory arrays
    UPROPERTY(VisibleAnywhere, Category = "MP_Inventory|Save")
    TMap<FName, FMP_InventoryArray> SavedInventory;
};
