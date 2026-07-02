// Copyright 2026 UVSquare. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Database/MP_InventoryStruct.h"
#include "Database/MP_InventoryFastArray.h"
#include "Core/MP_ItemRegistry.h"
#include "Components/ActorComponent.h"
#include "MP_InventoryComponent.generated.h"


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
     * Index is SlotIndex for Removed, ArrayIndex for Added/Updated (-1 on full refresh).
     */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryUpdated, EInventoryDelta, Delta, int32, Index);
    UPROPERTY(BlueprintAssignable, Category = "MP_Inventory|Events")
    FOnInventoryUpdated OnInventoryUpdated;

    /** Fired when a trade requires payment processing. */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProcessPayment, FString, TradeId, float, AmountToPay);
    UPROPERTY(BlueprintAssignable, Category = "MP_Inventory|Events")
    FOnProcessPayment OnProcessPayment;


    // =========================================================================
    //  LIFECYCLE
    // =========================================================================

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


    // =========================================================================
    //  CONFIGURATION  -  Set in editor or before first use
    // =========================================================================

    /**
     * Generic owner identifier. Works for players, NPCs, chests, vendors - anything.
     * Populate from PlayerState unique ID, a DataTable row name, or a GUID string cast.
     * Use FName over FString: hash-stored, O(1) comparison, no heap allocation.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category = "MP_Inventory|Config")
    FName OwnerID;

    /**
     * TRUE  = Fixed grid (Minecraft). Slot count is capped at MaxInventorySlots.
     * FALSE = Infinite list (Skyrim). Items append without a slot ceiling.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Config")
    bool bUseStrictSlots = true;

    /** Only meaningful when bUseStrictSlots is true. Must be >= 1. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Config",
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
    //  SERVER COMMANDS  -  All writes are authority-only server RPCs
    // =========================================================================

    /**
     * Primary add entry point. Handles stacking, stack overflow, and slot allocation.
     *
     * bPreferNewSlot = true  ? Allocates a fresh slot first. Any quantity that exceeds
     *                          the stack limit spills into existing partial stacks.
     *                          Use for: loot drops, pickup flow.
     *
     * bPreferNewSlot = false ? Merges into existing stacks of the same ItemID first.
     *                          Remaining overflow allocates a new slot.
     *                          Use for: crafting output, reward grants.
     *
     * Call CanAddItem first if you need to gate on success in Blueprint.
     * AddItem is fire-and-forget by design; partial adds are not possible -
     * the full quantity either fits or the operation is rejected.
     */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void AddItem(FName ItemID, int32 Quantity, bool bPreferNewSlot = false);

    /**
     * Removes Quantity from the item at ArrayIndex.
     * If resulting quantity reaches zero, the slot is freed.
     * Fails silently if the slot is locked or does not contain enough quantity.
     */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void RemoveItem(int32 ArrayIndex, int32 Quantity = 1);

    /**
     * Removes every stack of ItemID across the entire inventory.
     */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void RemoveAllItems(FName ItemID);

    /**
     * Removes a specific Quantity of ItemID starting from the first found stack.
     * Cascades to subsequent stacks if the first stack doesn't hold enough.
     * Fails if total quantity across all stacks is insufficient.
     */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void RemoveItemByID(FName ItemID, int32 Quantity = 1);

    /**
     * Fully replaces the item at SlotIndex with NewItem.
     * Intended for: crafting result injection, admin replacement, loot reroll.
     * Fails if the target slot is locked.
     */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void UpdateItemAtSlot(int32 SlotIndex, FMP_InventoryItem NewItem);

    /**
     * Swaps the contents of two slots.
     * Works correctly when one or both slots are empty (effectively a move).
     * Fails if either slot contains a locked item.
     */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void SwapItems(int32 SlotIndexA, int32 SlotIndexB);

    /**
     * Splits QuantityToSplit from the source stack into a target slot.
     *
     * TargetSlotIndex = -1 ? auto-assigns next free slot via AllocateSlot().
     *
     * Target must be either:
     *   a) Empty, or
     *   b) The same ItemID with room left in its stack.
     *
     * Fails if:
     *   - Source quantity <= QuantityToSplit (can't split a single unit or the full stack)
     *   - Target holds a different ItemID
     *   - Source or target slot is locked
     *   - No free slots available (strict mode only)
     */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void SplitItem(int32 SourceSlotIndex, int32 TargetSlotIndex, int32 QuantityToSplit);

    /**
     * Locks or unlocks the item at SlotIndex.
     * Locked items reject: RemoveItem, UpdateItemAtSlot, SwapItems, SplitItem, AddQuantity.
     * Lock is not replicated separately - it is part of FMP_InventoryItem and travels
     * with the FastArray delta. Reads are always allowed on locked slots.
     */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void SetItemLock(int32 SlotIndex, bool bLocked);

    /**
     * Reassigns all SlotIndexes to be contiguous starting from 0.
     * Reclaims all gaps. Fires a full Refresh delta when complete.
     */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void CompactSlots();


    // =========================================================================
    //  QUERIES  -  Read-only, callable on server and client
    // =========================================================================

    /**
     * Primary slot lookup. This is what Blueprint UI should always use.
     * Translates SlotIndex -> ArrayIndex via linear scan (strict) or direct index (infinite).
     * Returns a default (empty) FMP_InventoryItem if the slot is empty.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Queries")
    FMP_InventoryItem GetItemBySlotIndex(int32 SlotIndex) const;

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
     * Direct array index lookup.
     * Fast O(1) query for UI items that already store their physical array index.
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Queries")
    FMP_InventoryItem GetItemByArrayIndex(int32 ArrayIndex) const;

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
    int32 GetUsedSlots() const { return UsedSlots; }

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


    // =========================================================================
    //  INTERNAL BRIDGE  -  Not for Blueprint use
    // =========================================================================

    /**
     * Called by FMP_InventoryArray callbacks to fire OnInventoryUpdated.
     * Do not call this directly from game code.
     */
    void FireInventoryUpdate(EInventoryDelta Delta, int32 Index);

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

    /** Number of occupied slots. Updated on Add/Remove only. */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "MP_Inventory|State")
    int32 UsedSlots = 0;


private:

    // =========================================================================
    //  INTERNAL HELPERS
    // =========================================================================

    /**
     * Safe access to UMP_ItemRegistry subsystem.
     * Returns nullptr and logs a warning if the subsystem is unavailable.
     */
    UMP_ItemRegistry* GetRegistry() const;
};
