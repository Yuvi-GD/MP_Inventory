// Copyright 2026 UVSquare. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/MP_InventoryStruct.h"
#include "MP_InventoryManager.generated.h"

class UMP_InventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryActionNotify, FName, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNearbyLootChanged, EInventoryDelta, Delta, int32, SlotIndex);

/**
 * UMP_InventoryManager
 * 
 * The authoritative network bridge designed to be attached to the PlayerController.
 * Routes UI requests (Client-to-Server RPCs) to specific UMP_InventoryComponents,
 * ensuring strict ownership validation and completely decoupled data storage.
 */
UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class MP_INVENTORY_API UMP_InventoryManager : public UActorComponent
{
    GENERATED_BODY()

public:    
    UMP_InventoryManager();

    // =========================================================================
    //  NEARBY LOOT API (Local UI Tracking)
    // =========================================================================

    /** Fired when a nearby loot item is added or removed. bAdded is true if added, false if removed. */
    UPROPERTY(BlueprintAssignable, Category = "MP_Inventory|Manager|NearbyLoot")
    FOnNearbyLootChanged OnNearbyLootChanged;

    /** Adds an actor to the local nearby loot array and fires the delegate. */
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Manager|NearbyLoot")
    void AddNearbyLoot(AActor* LootItem);

    /** Removes an actor from the local nearby loot array and fires the delegate. */
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Manager|NearbyLoot")
    void RemoveNearbyLoot(AActor* LootItem);

    /** Removes an actor from the local nearby loot array and fires the delegate. */
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Manager|NearbyLoot")
    void RemoveNearbyLootByIndex(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Manager|NearbyLoot")
	void UpdateNearbyLoot(AActor* LootItem);

    /** Gets a specific nearby loot actor by its array index. */
    UFUNCTION(BlueprintPure, Category = "MP_Inventory|Manager|NearbyLoot")
    AActor* GetNearbyLootAt(int32 Index) const;

    /** Gets all currently tracked nearby loot actors. */
    UFUNCTION(BlueprintPure, Category = "MP_Inventory|Manager|NearbyLoot")
    const TArray<AActor*>& GetAllNearbyLoot() const;

protected:
    virtual void BeginPlay() override;

public:    
    /** Uniquely identifies this manager/player. Used for authorization against inventory components. */
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "MP_Inventory|Manager")
    FName ManagerID;

    /** If true, uses a deterministic PlayerId instead of the Online Subsystem's UniqueNetId. Enable this for Editor testing or offline games so saves persist. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Manager")
    bool bUseDeterministicPlayerID = true;

    /** Guarantees ManagerID is initialized and returns it. */
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Manager")
    FName GetManagerID();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** 
     * Fetches a component from the registry and validates if this manager has permission to interact with it.
     * Works on both Client and Server. Fires failure delegates if validation fails.
     */
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Manager")
    UMP_InventoryComponent* GetAndValidateComponent(FName InventoryID);

    /** Fired on the client when a server action fails (e.g., validation failed). */
    UPROPERTY(BlueprintAssignable, Category = "MP_Inventory|Manager|Events")
    FOnInventoryActionNotify OnInventoryActionNotify;

    /** Internal RPC sent from Server to Client when validation or an action fails. */
    UFUNCTION(Client, Reliable, Category = "MP_Inventory|Manager|Internal")
    void Client_OnActionNotify(FName Reason);

    // =========================================================================
    //  SINGLE-COMPONENT RPCs
    //  These perfectly mirror UMP_InventoryComponent APIs for secure routing
    // =========================================================================

    /** Requests the server to add an item to the target inventory. Evaluates weight and capacity automatically. */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void AddItem(FName TargetInventoryID, FName ItemID, int32 Quantity, bool bPreferNewSlot = false);

    /** Requests the server to add an item directly into a specific slot. Fails if the slot is occupied. */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void AddItemAtSlot(FName TargetInventoryID, FName ItemID, int32 Quantity, int32 TargetSlotIndex);

    /** Requests the server to add an array of items in bulk. Optimized for loot extraction. */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void AddItems(FName TargetInventoryID, const TArray<FMP_InventoryAddItems>& Items);

    /** Requests the server to deduct a specific quantity from a physical slot. */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void RemoveItem(FName TargetInventoryID, int32 SlotIndex, int32 Quantity = 1);

    /** Requests the server to remove an item by ID, cascading across multiple stacks if necessary. */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void RemoveItemByID(FName TargetInventoryID, FName ItemID, int32 Quantity = 1);

    /** Requests the server to completely overwrite a slot with a new item struct. (e.g. for crafting outputs) */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void UpdateItem(FName TargetInventoryID, FMP_InventoryItem NewItem);

    /** Requests the server to modify an existing slot's quantity. Fails if the slot is empty. */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void AdjustItemQuantity(FName TargetInventoryID, int32 SlotIndex, int32 QuantityDelta);

    /** Requests the server to swap two slots locally within the same inventory component. */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void SwapItems(FName TargetInventoryID, int32 SlotIndexA, int32 SlotIndexB);

    // =========================================================================
    //  MULTI-COMPONENT RPCs
    //  Operations that bridge two distinct inventories (or the same inventory)
    // =========================================================================

    /** Requests an atomic swap between two different inventories. Rolls back if weight limits reject the transfer. */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void SwapItemsBetweenInventories(FName SourceInventoryID, int32 SourceSlotIndex, FName TargetInventoryID, int32 TargetSlotIndex);

    /** Requests the server to extract quantity from one stack and place it in another slot (or auto-assign if TargetSlotIndex is -1). */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void SplitItem(FName TargetInventoryID, int32 SourceSlotIndex, int32 TargetSlotIndex, int32 QuantityToSplit);

    /** Requests the server to move a specific slot's quantity from one inventory to another. */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void TransferItemBySlot(FName SourceInventoryID, int32 SourceSlotIndex, FName TargetInventoryID, int32 TargetSlotIndex, int32 Quantity);

    /**
    * Requests the server to pull a quantity of ItemID from Source and push it into Target, cascading stacks if needed.
    * TargetSlotIndex can be -1 to auto-loot into the first available slot.
    * If Quantity = -1, it will transfer the entire stack from the Source slot.
    */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void TransferItemByID(FName SourceInventoryID, FName TargetInventoryID, FName ItemID, int32 Quantity);

    /** Requests the server to merge as much quantity as possible from the Source slot into the Target slot. */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void MergeSlotsBetweenInventories(FName SourceInventoryID, int32 SourceSlotIndex, FName TargetInventoryID, int32 TargetSlotIndex);

    /**
    * Requests the server to pick up a physical item actor from the ground.
    * TargetSlotIndex can be -1 to auto-loot into the first available slot.
    */
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Commands")
    void LootGroundItemByIndex(int32 Index, FName TargetInventoryID, int32 TargetSlotIndex = -1);

    /**
    * Requests the server to pick up a physical item actor from the ground.
    * TargetSlotIndex can be -1 to auto-loot into the first available slot.
    */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void LootGroundItem(AActor* GroundItemActor, FName TargetInventoryID, int32 TargetSlotIndex = -1, int32 Quantity = -1);

    /** Requests the server to remove an item and physically spawn its corresponding Actor mesh in the 3D world. */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void DropItem(FName TargetInventoryID, int32 SlotIndex, int32 Quantity, FVector DropLocation);

    /** Requests the server to lock/unlock a slot, preventing it from being mutated or swapped by other RPCs. */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void SetItemLock(FName TargetInventoryID, int32 SlotIndex, bool bLocked);

    /** Requests the server to expand or shrink the physical grid capacity. Shrink fails if items exist out of bounds. */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void ResizeInventory(FName TargetInventoryID, int32 NewMaxSlots);

    /** Requests the server to shift all items up to fill empty gaps, enforcing a contiguous index sequence. */
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void CompactSlots(FName TargetInventoryID);

protected:
    /** Local array of nearby loot items. Not replicated. */
    UPROPERTY()
    TArray<AActor*> NearbyLoot;
};
