// Copyright 2026 UVSquare. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/MP_InventoryStruct.h"
#include "Data/MP_InventoryFastArray.h"
#include "Core/MP_ItemRegistry.h"
#include "Components/ActorComponent.h"
#include "MP_InventoryComponent.generated.h"

class UMP_InventoryManager;

/**
 * The core actor component responsible for managing inventory state and operations.
 * Handles client-server replication, item transactions, stacking logic, and weight/slot validations.
 * Must be attached to a player state or character to function correctly in a networked environment.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MP_INVENTORY_API UMP_InventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UMP_InventoryComponent();

    // =========================================================================
    //  DELEGATES
    // =========================================================================

    /**
     * Fired on both server and client whenever the inventory changes.
     * SlotIndex represents the logical grid slot that was Updated, Added, or Removed.
     */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryUpdated, EInventoryDelta, Delta, int32, SlotIndex);
    UPROPERTY(BlueprintAssignable, Category = "MP_Inventory|Events")
    FOnInventoryUpdated OnInventoryUpdated;

    /** Fired when the inventory max size changes. */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryResized, int32, NewSize);
    UPROPERTY(BlueprintAssignable, Category = "MP_Inventory|Events")
    FOnInventoryResized OnInventoryResized;


    // =========================================================================
    //  LIFECYCLE
    // =========================================================================

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


    // =========================================================================
    //  CONFIGURATION  -  Set in editor or before first use
    // =========================================================================

    /** 
     * Unique GUID for this inventory component. 
     * Registered in the MP_ItemRegistry.
     */
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_InventoryID, Category = "MP_Inventory|Config")
    FName InventoryID;

    UFUNCTION()
    void OnRep_InventoryID();

    /** Determines who has rights to this inventory (ManagerID, "GLOBAL", or "PRIVATE") */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "MP_Inventory|Config")
    FName OwnerID;

    /** If OwnerID is "PRIVATE", only ManagerIDs in this list can access it. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MP_Inventory|Config")
    TSet<FName> AccessList;

    /** Sets the OwnerID of the inventory at runtime. Must be called on the Server. */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Inventory|Config")
    void SetInventoryOwner(FName NewOwnerID);

    /** Grants a specific Manager access to this inventory (only relevant if OwnerID is PRIVATE). */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Inventory|Config")
    void GrantAccess(UMP_InventoryManager* Manager);

    /** Revokes access from a specific Manager. */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Inventory|Config")
    void RevokeAccess(UMP_InventoryManager* Manager);

    /** 
     * Attempts to find the owning PlayerController and automatically assign its ManagerID as the OwnerID.
     * Call this at runtime when a player possesses a Pawn that holds this inventory.
     * Returns true if successfully assigned.
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Inventory|Config")
    bool TryAutoAssignOwner();

    /** Human readable name (e.g., "Backpack") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Config")
    FName ComponentName;

    /**
     * TRUE  = Fixed grid (Minecraft). Slot count is capped at MaxInventorySlots.
     * FALSE = Infinite list (Skyrim). Items append without a slot ceiling.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Config")
    bool bUseStrictSlots = true;

    /** Only meaningful when bUseStrictSlots is true. Must be >= 1. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "MP_Inventory|Config",
        meta = (EditCondition = "bUseStrictSlots", ClampMin = "1"))
    int32 MaxInventorySlots = 20;

    /** When true, AddItem checks cumulative item weight against MaxWeightCapacity. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Config")
    bool bEnforceWeightLimit = false;

    /** Only meaningful when bEnforceWeightLimit is true. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Config",
        meta = (EditCondition = "bEnforceWeightLimit", ClampMin = "0.0"))
    float MaxWeightCapacity = 100.0f;

    /** Save slot name for UGameplayStatics persistence. Override before calling Save/Load. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MP_Inventory|Config")
    FString SaveSlotName = "Inventory";

    /** If true, the entire inventory is locked. Useful during active trade sessions to prevent race conditions. */
    UPROPERTY(BlueprintReadWrite, Replicated, Category = "MP_Inventory|State")
    bool bInventoryLocked = false;

    // =========================================================================
    //  AUTHORITY COMMANDS  -  All writes must be executed on the Server
    // =========================================================================

    /**
     * Adds an item to the inventory. Handles stacking and slot allocation automatically.
     * @param bPreferNewSlot - If true, allocates a new slot first before stacking with existing items.
     */
    /** 
     * Removes the item and physically spawns it into the world.
     * @return The spawned Actor, or nullptr if it failed.
     */
    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "MP_Inventory|Server")
    AActor* DropItem(int32 SlotIndex, int32 Quantity, FVector DropLocation);

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Inventory|Commands")
    bool AddItem(FName ItemID, int32 Quantity, bool bPreferNewSlot = false);

    /**
     * Adds an item to a specific slot index.
     * Useful for drag-and-drop UI or exact placement. Fails if the slot is occupied by a different item.
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Inventory|Commands")
    bool AddItemAtSlot(FName ItemID, int32 Quantity, int32 TargetSlotIndex);

    /** Adds multiple items in a single call to optimize performance. */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Inventory|Commands")
    bool AddItems(const TArray<FMP_InventoryAddItems> &Items);

    /**
     * Removes Quantity from the item at SlotIndex.
     * If resulting quantity reaches zero, the slot is freed.
     * Fails silently if the slot is locked or does not contain enough quantity.
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Inventory|Commands")
    bool RemoveItem(int32 SlotIndex, int32 Quantity = 1);

    /**
     * Removes a specific Quantity of ItemID starting from the first found stack.
     * Cascades to subsequent stacks if the first stack doesn't hold enough.
     * Fails if total quantity across all stacks is insufficient.
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Inventory|Commands")
    bool RemoveItemByID(FName ItemID, int32 Quantity = 1);

    /**
     * Fully replaces the item at its designated SlotIndex (NewItem.SlotIndex).
     * Intended for: crafting result injection, admin replacement, loot reroll.
     * Fails if the target slot is locked.
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Inventory|Commands")
    bool UpdateItem(FMP_InventoryItem NewItem);

    /**
     * Adjusts the quantity of the item at SlotIndex.
     * Positive QuantityDelta adds to the stack (capped at MaxStackSize).
     * Negative QuantityDelta removes from the stack (frees the slot if it hits 0).
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Inventory|Commands")
    bool AdjustItemQuantity(int32 SlotIndex, int32 QuantityDelta);

    /**
     * Swaps the contents of two slots.
     * Works correctly when one or both slots are empty (effectively a move).
     * Fails if either slot contains a locked item.
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Inventory|Commands")
    bool SwapItems(int32 SlotIndexA, int32 SlotIndexB);

    /**
     * Splits a quantity from a source stack and places it into a target slot.
     * If TargetSlotIndex is -1, it auto-assigns the next free slot.
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Inventory|Commands")
    bool SplitItem(int32 SourceSlotIndex, int32 TargetSlotIndex, int32 QuantityToSplit);

    /**
     * Locks or unlocks the item at SlotIndex.
     * Locked items reject: RemoveItem, UpdateItemAtSlot, SwapItems, SplitItem, AddQuantity.
     * Lock is not replicated separately - it is part of FMP_InventoryItem and travels
     * with the FastArray delta. Reads are always allowed on locked slots.
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Inventory|Commands")
    bool SetItemLock(int32 SlotIndex, bool bLocked);

    /** 
     * Resizes the inventory grid safely.
     * Returns true if successful, false if not enough physical space to shrink.
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Inventory|Commands")
    bool ResizeInventory(int32 NewMaxSlots);

    /**
     * Reassigns all SlotIndexes to be contiguous starting from 0.
     * Reclaims all gaps. Fires a full Refresh delta when complete.
     */
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Inventory|Commands")
    void CompactSlots();


    // =========================================================================
    //  QUERIES  -  Read-only, callable on server and client
    // =========================================================================

    /** 
     * Checks if a specific quantity of an item can be added to the inventory.
     * Evaluates weight limits, stack limits, available slots, and merge capacity.
     * 
     * @param ItemID The item to check.
     * @param Quantity The amount to add.
     * @param bPreferNewSlot Matches the AddItem logic preference.
     * @param OutQuantityForNewSlots Returns how much of the quantity will be placed into fresh slots.
     * @param OutQuantityForMerge Returns how much of the quantity will be merged into existing stacks.
     * @return True if the entire quantity can fit; False if it cannot.
     */
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Queries")
    bool CanAddItem(FName ItemID, int32 Quantity, bool bPreferNewSlot, int32& OutQuantityForNewSlots, int32& OutQuantityForMerge) const;

    /**
     * Primary slot lookup. This is what Blueprint UI should always use.
     * Translates SlotIndex -> ArrayIndex via linear scan (strict) or direct index (infinite).
     * Returns a default (empty) FMP_InventoryItem if the slot is empty.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Queries")
    FMP_InventoryItem GetItemBySlotIndex(int32 SlotIndex) const;

    /**
     * Direct array index lookup.
     * Fast O(1) query for UI items that already store their physical array index.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Queries")
    FMP_InventoryItem GetItemByArrayIndex(int32 ArrayIndex) const;

    /**
     * Returns the first stack found for a given ItemID via linear scan.
     * Useful when you know the item exists but not which slot it's in.
     * Returns a default (empty) FMP_InventoryItem if not found.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Queries")
    FMP_InventoryItem GetItemByID(FName ItemID) const;

    /**
     * Translates SlotIndex to ArrayIndex.
     * Returns INDEX_NONE if the slot is not occupied.
     * All FastArray calls must go through this before touching Items[].
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Queries")
    int32 GetArrayIndexFromSlot(int32 SlotIndex) const;

    /**
     * Returns all SlotIndexes that currently hold the given ItemID.
     * Uses a linear scan. O(N).
     * Use for: checking total quantity across stacks, removing by ID, crafting checks.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Queries")
    TArray<int32> GetAllSlotsForItem(FName ItemID) const;

    /**
     * Raw array access. Use for iteration only.
     * Do NOT assume any ordering - array index has no semantic meaning.
     * For slot-ordered display, iterate GetAllItems and sort by Item.SlotIndex.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Queries")
    const TArray<FMP_InventoryItem>& GetAllItems() const { return InventoryItems.Items; }

    /** Returns true if the slot is occupied. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Queries")
    bool IsSlotOccupied(int32 SlotIndex) const;

    /** Returns true if the item at this slot exists and has bIsLocked = true. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Queries")
    bool IsSlotLocked(int32 SlotIndex) const;


    // =========================================================================
    //  STATE ACCESSORS  -  Limit monitoring for UI (health bars, counters, etc.)
    // =========================================================================

    /** Number of slots currently holding at least one item. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|State")
    int32 GetUsedSlots() const { return InventoryItems.Items.Num(); }

    /**
     * Remaining available slots.
     * Returns TNumericLimits<int32>::Max() when bUseStrictSlots is false (infinite mode).
     * UI should check bUseStrictSlots before displaying this value.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|State")
    int32 GetRemainingSlots() const;

    /** Total weight of all current items. Updated on every Add/Remove. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|State")
    float GetCurrentWeight() const { return CurrentWeight; }

    /**
     * Remaining weight capacity.
     * Returns TNumericLimits<float>::Max() when bEnforceWeightLimit is false.
     * UI should check bEnforceWeightLimit before displaying this value.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|State")
    float GetRemainingWeight() const;

    /** Total number of item entries in the array (occupied slots only, not slot count). */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|State")
    int32 GetItemCount() const { return InventoryItems.Items.Num(); }

    /** Returns true if no more items can be added under current constraints. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|State")
    bool IsInventoryFull() const;

    // =========================================================================
    //  PERSISTENCE
    // =========================================================================

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Inventory|Persistence")
    void SaveInventory(const FString& PlayerID);

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Inventory|Persistence")
    void LoadInventory(const FString& PlayerID);

protected:

    // =========================================================================
    //  REPLICATED STATE  -  Travels over the network
    // =========================================================================

    /** The actual inventory data. NetDeltaSerialized - only dirty items replicate. */
    UPROPERTY(Replicated)
    FMP_InventoryArray InventoryItems;

    /** Cumulative weight of all items. Updated on Add/Remove only. */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "MP_Inventory|State")
    float CurrentWeight = 0.0f;


private:

    // =========================================================================
    //  INTERNAL HELPERS
    // =========================================================================

    /**
     * Safe access to UMP_ItemRegistry subsystem.
     * Returns nullptr and logs a warning if the subsystem is unavailable.
     */
    UMP_ItemRegistry* GetRegistry() const;
    
    /**
     * Compacts slots and explicitly truncates the FastArray tracker to the new size.
     */
    void ShrinkTracker(int32 NewSize);

    /**
     * Called by FMP_InventoryArray callbacks to fire OnInventoryUpdated.
     * Do not call this directly from game code.
     */
    void FireInventoryUpdate(EInventoryDelta Delta, int32 SlotIndex);
};
