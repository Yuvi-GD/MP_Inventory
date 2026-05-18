// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Database/MP_InventoryStruct.h"
#include "Database/MP_InventoryFastArray.h"
#include "Core/MP_ItemDefinitionStorage.h"
#include "Components/ActorComponent.h"
#include "MP_InventoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MP_INVENTORY_API UMP_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMP_InventoryComponent();

    //DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryUpdated, EInventoryDelta, Delta, int32, Index);

    UPROPERTY(BlueprintAssignable, Category = "MP_Inventory|Component")
    FOnInventoryUpdated OnInventoryUpdated;

    // Called when the game starts
	virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Component")
    void SaveInventory(const FString& PlayerID);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Component")
    void LoadInventory(const FString& PlayerID);

    UFUNCTION(Server, Reliable, Category = "MP_Inventory|Component")
    void ServerSyncFromClient(const TArray<FMP_InventoryItem>& Items);

    /*********************  Server-side: Modify local inventory  ***********************/

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Component")
    void AddItem(FMP_InventoryItem Item);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Component")
    void RemoveItemByIndex(int32 Index, int32 Quantity =1);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Component")
    void RemoveItemByID(FName ItemID, int32 Quantity =1);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Component")
    void ReplaceItemByIndex(int32 Index, FMP_InventoryItem NewItem);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Component")
    void ReplaceItem(FMP_InventoryItem OldItem, FMP_InventoryItem NewItem);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Component")
    void SwapItems(int32 IndexA, int32 IndexB);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Component")
    void UpdateItemTags(FName ItemID, FGameplayTag TagToRemove, FGameplayTag TagToAdd);

    // Server-side: Interact with other players
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Component")
    void DropItem(int32 Index, int32 Quantity);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Component")
    void RequestItemFromPlayer(UMP_InventoryComponent* TargetComponent, FName ItemID, int32 Quantity);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Component")
    void GiveItemToPlayer(UMP_InventoryComponent* TargetComponent, FMP_InventoryItem Item);


    //-------------------------- PURE FUCNTIONS --------------------------------//
    
    // Client-side getters (replicated data)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Component")
    TArray<FMP_InventoryItem> GetItemsByTag(FGameplayTagContainer Tag, bool bRequireAllTags) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Component")
    FMP_InventoryItem GetItemByItemID(FName ItemID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Component")
    FMP_InventoryItem GetItemByIndex(int32 Index) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Component")
    TArray<FMP_InventoryItem> GetItemsByItemName(FString ItemName) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Component")
    const TArray<FMP_InventoryItem>& GetAllItems() const { return InventoryItems.Items; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Component")
    const int32 GetLength() const { return InventoryItems.Items.Num(); }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Component")
    const FMP_InventoryItem GetLastItem() const { return InventoryItems.Items.Last(); }

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated, Category = "MP_Inventory|Component")
    FString UniqueId;

    // Multicast RPC to broadcast to all clients
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_BroadcastInventoryUpdate(const EInventoryDelta& Delta, const int32& Index=-1);


    // Add to protected section
protected:

    //UPROPERTY(ReplicatedUsing = OnRep_InventoryItems, BlueprintReadOnly, Category = "MP_Inventory|Component")
    //TArray<FMP_InventoryItem> InventoryItems;

    UFUNCTION()
	void OnRep_InventoryItems();

    // The FastArray inventory property, replicated with NetDeltaSerialize
    UPROPERTY(Replicated)
    FMP_InventoryArray InventoryItems;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MP_Inventory|Component")
    FString SaveSlotName = "Inventory";

    // Called by FastArray to fire dispatcher/event
    UFUNCTION()
    void FireInventoryUpdate(EInventoryDelta Delta, int32 SlotIndex);
};
