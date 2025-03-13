// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Database/MP_InventoryStruct.h"
#include "Subsystems/GameInstanceSubsystem.h"
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
    TArray<FMP_InventoryStruct> InventoryItems;

    // Flag to control automatic saving after each operation
    UPROPERTY(BlueprintReadWrite, Category = "MP_Inventory|Subsystem")
    bool bAutoSave = true;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
    UPROPERTY(BlueprintAssignable, Category = "MP_Inventory|Subsystem")
    FOnInventoryUpdated OnInventoryUpdated;

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void AddItem(FMP_InventoryStruct Item);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void RemoveItemByIndex(int32 Index, int32 QuantityToRemove = 1);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void RemoveItem(FMP_InventoryStruct Item, int32 QuantityToRemove = 1);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void ReplaceItemByIndex(int32 Index, FMP_InventoryStruct NewItem);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void ReplaceItem(FMP_InventoryStruct OldItem, FMP_InventoryStruct NewItem);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void SwapItems(int32 IndexA, int32 IndexB);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void UpdateItemTags(FName ItemID, FGameplayTag TagToRemove, FGameplayTag TagToAdd);

    //-------------------------- PURE FUCNTIONS --------------------------------//

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Subsystem")
    TArray<FMP_InventoryStruct> GetItemsByTag(FGameplayTagContainer Tag, bool bRequireAllTags) const;

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    const TArray<FMP_InventoryStruct>& GetAllItems() const { return InventoryItems; }

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    UTexture* LoadItemIcon(const FMP_InventoryStruct& Item);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void SaveInventory();

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    void LoadInventory();

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    FMP_InventoryStruct GetItemByItemID(FName ItemID) const;

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Subsystem")
    TArray<FMP_InventoryStruct> GetItemsByItemName(FString ItemName) const;
};
