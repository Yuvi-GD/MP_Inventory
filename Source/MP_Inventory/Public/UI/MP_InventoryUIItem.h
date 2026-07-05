// Copyright 2026 UVSquare. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/MP_ItemDefinition.h"
#include "MP_InventoryUIItem.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MP_INVENTORY_API UMP_InventoryUIItem : public UObject
{
	GENERATED_BODY()
public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUIItemUpdated);
	UPROPERTY(BlueprintAssignable, Category = "MP_Inventory|Events")
	FOnInventoryUIItemUpdated OnInventoryUIItemUpdated;

	/// Core data for an inventory item instance. This is what gets stored in the inventory array, and can be easily extended with new properties as needed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory", meta = (ExposeOnSpawn = "true"))
	UMP_ItemDefinition* ItemData;
	
	// Quantity of the item in the stack. For non-stackable items, this should be 1.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory", meta = (ExposeOnSpawn = "true"))
	int32 Quantity = 0;
	
	// Logical grid position in the UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory", meta = (ExposeOnSpawn = "true"))
	int32 SlotIndex = -1;

    // Physical array position in the backend FastArray
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory", meta = (ExposeOnSpawn = "true"))
	int32 ArrayIndex = -1;

	// Initializes the inventory item with the given data, quantity, slot index, and array index.
	UFUNCTION(BlueprintCallable, Category = "MP_Inventory")
	void Initialize(UMP_ItemDefinition* InItemData, int32 InQuantity, int32 InSlotIndex, int32 InArrayIndex = -1)
	{
		ItemData = InItemData;
		Quantity = InQuantity;
		SlotIndex = InSlotIndex;
        ArrayIndex = InArrayIndex;
		OnInventoryUIItemUpdated.Broadcast();
	}
};

/*
FNetReplicationGraphConnection*
FConnectionReplicationActorInfo&
FNewReplicatedActorInfo&
FReplicatorConnectionKey


   virtual void NotifyAddNetworkActor(const FNewReplicatedActorInfo& ActorInfo) override;
   virtual void NotifyConnectionRemoved(FNetReplicationGraphConnection* ConnectionManager) override;
   virtual void NotifyConnectionAdded(FNetReplicationGraphConnection* ConnectionManager) override;
   virtual void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params) override;
   TMap<FNetReplicationGraphConnection*, TSet<FString>> ConnectionToRoomIDs;
   void UnsubscribeConnectionFromRoom(FNetReplicationGraphConnection* ConnectionManager, const FString& RoomID);

*/
