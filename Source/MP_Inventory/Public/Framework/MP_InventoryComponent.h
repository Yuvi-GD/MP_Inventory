// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Database/MP_InventoryStruct.h"
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

	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
    UPROPERTY(BlueprintAssignable, Category = "MP_Inventory|Subsystem")
    FOnInventoryUpdated OnInventoryUpdated;

    // Server-side: Modify local inventory
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
    TArray<FMP_InventoryItem> GetItemsByItemName(FString ItemName) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Component")
    const TArray<FMP_InventoryItem>& GetAllItems() const { return InventoryItems; }

    // Add to protected section
protected:

    UPROPERTY(ReplicatedUsing = OnRep_InventoryItems, BlueprintReadOnly, Category = "MP_Inventory|Component")
    TArray<FMP_InventoryItem> InventoryItems;

    UFUNCTION()
	void OnRep_InventoryItems();

    // Client-side: Sync operations to local Subsystem
    UFUNCTION(Client, Reliable)
    void ClientAddItem(FMP_InventoryItem Item);

    UFUNCTION(Client, Reliable)
    void ClientRemoveItemByIndex(int32 Index, int32 Quantity);

    UFUNCTION(Client, Reliable)
    void ClientReplaceItemByIndex(int32 Index, FMP_InventoryItem NewItem);

    UFUNCTION(Client, Reliable)
    void ClientSwapItems(int32 IndexA, int32 IndexB);

    UFUNCTION(Client, Reliable)
    void ClientUpdateItemTags(FName ItemID, FGameplayTag TagToRemove, FGameplayTag TagToAdd);

    UFUNCTION(Client, Reliable)
    void ClientDropItem(int32 Index, int32 Quantity);

    UFUNCTION(Client, Reliable)
    void ClientSyncFromLocalSubsystem();

    UFUNCTION(Server, Reliable)
    void ServerSyncFromLocalSubsystem(const TArray<FMP_InventoryItem>& Inventory);

    // Multicast RPC to broadcast to all clients
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_BroadcastInventoryUpdate();

private:

		
};
