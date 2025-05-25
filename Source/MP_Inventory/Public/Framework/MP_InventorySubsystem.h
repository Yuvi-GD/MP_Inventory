// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Database/MP_InventoryStruct.h"
#include "Core/MP_ItemDefinitionStorage.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HAL/CriticalSection.h"
#include "MP_InventorySubsystem.generated.h"


/**
 * 
 */
UCLASS()
class MP_INVENTORY_API UMP_InventorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
    UPROPERTY(BlueprintReadOnly, Category = "MP_Inventory|Subsystem")
    TArray<FMP_InventoryItem> InventoryItems;

    // Flag to control automatic saving after each operation
    UPROPERTY(BlueprintReadWrite, Category = "MP_Inventory|Subsystem")
    bool bAutoSave = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MP_Inventory|Subsystem")
    FString DefaultSlotName = "Inventory";

    // Thread Safety Lock
    mutable FCriticalSection InventoryCriticalSection;

public:

    UPROPERTY(BlueprintReadOnly, Category = "MP_Inventory|Subsystem")
    UMP_ItemDefinitionStorage* ItemStorage; // Reference to the storage object

    UPROPERTY(BlueprintReadWrite, Category = "MP_Inventory|Subsystem")
    bool bIsProcessingServerChange = false;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
    UPROPERTY(BlueprintAssignable, Category = "MP_Inventory|Subsystem")
    FOnInventoryUpdated OnInventoryUpdated;


    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void AddItem(FMP_InventoryItem Item);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void RemoveItemByIndex(int32 Index, int32 QuantityToRemove = 1);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void RemoveItem(FMP_InventoryItem Item, int32 QuantityToRemove = 1);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void ReplaceItemByIndex(int32 Index, FMP_InventoryItem NewItem);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void ReplaceItem(FMP_InventoryItem OldItem, FMP_InventoryItem NewItem);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void SwapItems(int32 IndexA, int32 IndexB);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void UpdateItemTags(FName ItemID, FGameplayTag TagToRemove, FGameplayTag TagToAdd);

    //-------------------------- PURE FUCNTIONS --------------------------------//

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Subsystem")
    TArray<FMP_InventoryItem> GetItemsByTag(FGameplayTagContainer Tag, bool bRequireAllTags) const;

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    FMP_InventoryItem GetItemByItemID(FName ItemID) const;

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    TArray<FMP_InventoryItem> GetItemsByItemName(FString ItemName) const;

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    const TArray<FMP_InventoryItem>& GetAllItems() const { return InventoryItems; }

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    UTexture* LoadItemIcon(const FMP_InventoryItem& Item);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    bool HasSpaceForItem(FMP_InventoryItem Item) const;

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void DropItem(int32 Index, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void HandleDeathLoss();

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void SaveInventory(const FString& PlayerID);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void LoadInventory(const FString& PlayerID);

};
