// Copyright 2026 UVSquare. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MP_InventoryPickupInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UMP_InventoryPickupInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for any physical world Actor that represents a droppable/pickupable inventory item.
 */
class MP_INVENTORY_API IMP_InventoryPickupInterface
{
	GENERATED_BODY()

public:
	/**
	 * Called by the inventory system immediately after dropping/spawning the item into the world.
	 * Blueprint actors should implement this to receive dynamic quantity and item data from the C++ backend.
	 * 
	 * @param ItemID The ID of the item being dropped.
	 * @param Quantity The amount of the item being dropped.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Inventory|Pickup")
	void InitializePickup(FName ItemID, int32 Quantity);
};
