// Copyright 2026 UVSquare. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MP_InventoryStruct.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "MP_InventoryFastArray.generated.h"


/**
 * FMP_InventoryArray
 *
 * FFastArraySerializer wrapper for the inventory item list.
 * Handles delta replication - only modified items travel over the network.
 *
 * IMPORTANT CONTRACTS:
 * - This struct only operates on ArrayIndex (physical position in Items[]).
 * - SlotIndex translation is entirely the responsibility of UMP_InventoryComponent.
 * - Blueprint never touches this struct directly.
 * - All write functions assume the caller has already validated locks and limits.
 */
USTRUCT()
struct MP_INVENTORY_API FMP_InventoryArray : public FFastArraySerializer
{
    GENERATED_BODY()

    /** The physical item array. Always dense - no empty entries, no holes. */
    UPROPERTY()
    TArray<FMP_InventoryItem> Items;

    /**
     * Cached setting from the Inventory Component. Determines if slots are rigid (true) or shifting (false).
     */
    UPROPERTY(NotReplicated)
    bool bIsStrict = true;

    /**
     * Local Tracker mapping SlotIndex to ArrayIndex. 
     * E.g., IndexTracker[5] = 2 means Slot 5 holds the item at Array Index 2.
     * Not replicated!
     */
    UPROPERTY(NotReplicated)
    TArray<int32> IndexTracker;

    /**
     * Safely expands the tracker array to the target size.
     * Newly added indices will be initialized to INDEX_NONE.
     */
    void ExpandTracker(int32 NewSize);

    // =========================================================================
    //  WRITE OPERATIONS
    //  All parameters are ArrayIndex unless explicitly noted.
    //  Caller (UMP_InventoryComponent) is responsible for:
    //    - SlotIndex ? ArrayIndex translation before calling
    //    - Updating SlotToArrayIndex, ItemIDToSlots after each call
    //    - Lock and limit validation before calling
    // =========================================================================

    /**
     * Appends a new item to the array.
     * NewItem.SlotIndex must already be set by the caller via AllocateSlot().
     * Marks the item and array dirty for replication.
     */
    void AddItem(const FMP_InventoryItem& NewItem);

    /**
     * Increases the quantity of the item at ArrayIndex by Quantity.
     * Does not check stack limits - caller is responsible for overflow logic.
     * Marks the item dirty for replication.
     */
    void AddQuantity(int32 ArrayIndex, int32 Quantity);

    /**
     * Decreases quantity. If it reaches zero, performs a swap-remove (Strict Mode):
     *   - The last element moves into ArrayIndex to keep the array dense.
     *   - Crucially, the moved element keeps its original SlotIndex.
     *   - Network Cost: O(1) (Only 2 elements replicate).
     */
    void RemoveAtSwap(int32 ArrayIndex, int32 Quantity);

    /**
     * Decreases quantity. If it reaches zero, performs a shift-remove (Infinite Mode):
     *   - Removes the element and shifts all subsequent elements left.
     *   - Decrements the SlotIndex of all shifted elements so the UI list stays contiguous.
     *   - Network Cost: O(N) (All shifted elements replicate).
     */
    void RemoveAndShift(int32 ArrayIndex, int32 Quantity);
    /**
     * Full replacement of the item at ArrayIndex.
     * Used for: crafting output, admin replacement.
     * NewItem.SlotIndex should match the existing item's SlotIndex unless intentionally moving.
     * Marks the item dirty for replication.
     */
    void UpdateItem(int32 ArrayIndex, const FMP_InventoryItem& NewItem);

    /**
     * Directly sets the SlotIndex of an existing item and marks it dirty.
     * Used exclusively for structural reorganization (e.g., CompactSlots) on the server.
     */
    void SetItemSlot(int32 ArrayIndex, int32 NewSlotIndex);

    /**
     * Logical Swap (Slot to Slot).
     * Swaps the logical slot assignment of two items. Properly handles moving an item to an empty slot.
     */
    void SwapItemsBySlotIndex(int32 SlotA, int32 SlotB);

    // =========================================================================
    //  REPLICATION CALLBACKS
    //  Called by the engine on the receiving end (clients) after delta application.
    //  These notify the component, which then updates its transient lookup maps.
    // =========================================================================

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

    /** Called BEFORE items are removed on the client. Fires Removed delta per item. */
    void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);

    /** Called AFTER items are added on the client. Fires Added delta per item. */
    void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);

    /** Called AFTER items are modified on the client. Fires Updated delta per item. */
    void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);

    /**
     * Called ONCE after ALL changes in a replication batch are applied.
     * This is where the component triggers RebuildLookups() on the client side.
     * Ensures lookups are always in sync with the final replicated array state.
     */
    void PostReplicatedReceive(const FFastArraySerializer::FPostReplicatedReceiveParameters& Parameters);


    // =========================================================================
    //  OWNER LINK
    // =========================================================================

    void SetOwner(UObject* InOwner) { Owner = InOwner; }
    UObject* GetOwner() const { return Owner.Get(); }

    /**
     * Routes a delta notification to the owning UMP_InventoryComponent.
     * Fires OnInventoryUpdated dispatcher on both server and client.
     */
    void NotifyOwner(EInventoryDelta Delta, int32 SlotIndex);

private:

    /** Weak back-reference to the owning component. Set in UMP_InventoryComponent constructor. */
    TWeakObjectPtr<UObject> Owner;
};


template<>
struct TStructOpsTypeTraits<FMP_InventoryArray> : public TStructOpsTypeTraitsBase2<FMP_InventoryArray>
{
    enum { WithNetDeltaSerializer = true };
};
