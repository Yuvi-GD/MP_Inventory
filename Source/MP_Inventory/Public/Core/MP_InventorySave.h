// Copyright 2026 UVSquare. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/MP_InventoryStruct.h"
#include "Data/MP_InventoryFastArray.h"
#include "GameFramework/SaveGame.h"
#include "MP_InventorySave.generated.h"

/**
 * Dedicated USaveGame object for persisting multiplayer inventory states.
 * Utilizes a map of unique player IDs to fast-array serializers, enabling robust
 * backend storage and serialization of items across sessions.
 */
UCLASS(Blueprintable)
class MP_INVENTORY_API UMP_InventorySave : public USaveGame
{
	GENERATED_BODY()
	
public:

    // Map of InventoryIDs to their full inventory save data (items + configuration)
    UPROPERTY(VisibleAnywhere, Category = "MP_Inventory|Save")
    TMap<FName, FMP_InventorySaveData> SavedInventory;
};
