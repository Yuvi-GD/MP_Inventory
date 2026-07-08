// Copyright 2026 UVSquare. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/MP_InventoryStruct.h"
#include "MP_InventoryManager.generated.h"

class UMP_InventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryActionNotify, FName, Reason);

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
    //  CLIENT-TO-SERVER RPCs
    //  These perfectly mirror UMP_InventoryComponent APIs for secure routing
    // =========================================================================

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void AddItem(FName TargetInventoryID, FName ItemID, int32 Quantity, bool bPreferNewSlot = false);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void AddItemAtSlot(FName TargetInventoryID, FName ItemID, int32 Quantity, int32 TargetSlotIndex);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void AddItems(FName TargetInventoryID, const TArray<FMP_InventoryAddItems>& Items);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void RemoveItem(FName TargetInventoryID, int32 SlotIndex, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void RemoveItemByID(FName TargetInventoryID, FName ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void UpdateItem(FName TargetInventoryID, FMP_InventoryItem NewItem);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void AdjustItemQuantity(FName TargetInventoryID, int32 SlotIndex, int32 QuantityDelta);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void SwapItems(FName TargetInventoryID, int32 SlotIndexA, int32 SlotIndexB);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void SplitItem(FName TargetInventoryID, int32 SourceSlotIndex, int32 TargetSlotIndex, int32 QuantityToSplit);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void TransferItemBySlot(FName SourceInventoryID, int32 SourceSlotIndex, FName TargetInventoryID, int32 TargetSlotIndex, int32 Quantity);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void TransferItemByID(FName SourceInventoryID, FName TargetInventoryID, FName ItemID, int32 Quantity);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void DropItem(FName TargetInventoryID, int32 SlotIndex, int32 Quantity, FVector DropLocation);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void SetItemLock(FName TargetInventoryID, int32 SlotIndex, bool bLocked);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void ResizeInventory(FName TargetInventoryID, int32 NewMaxSlots);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Commands")
    void CompactSlots(FName TargetInventoryID);

};
