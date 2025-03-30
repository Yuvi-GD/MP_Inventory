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
    void EndPlay(const EEndPlayReason::Type EndPlayReason);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
    UPROPERTY(BlueprintAssignable, Category = "MP_Inventory|Subsystem")
    FOnInventoryUpdated OnInventoryUpdated;

    // Server-side: Modify local inventory
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Component")
    void AddItem(FMP_InventoryStruct Item);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Component")
    void RemoveItemByIndex(int32 Index, int32 Quantity);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Component")
    void RemoveItemByID(FName ItemID, int32 Quantity);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Component")
    void DropItem(int32 Index, int32 Quantity);

    // Server-side: Interact with other players
    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Component")
    void RequestItemFromPlayer(UMP_InventoryComponent* TargetComponent, FName ItemID, int32 Quantity);

    UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MP_Inventory|Component")
    void GiveItemToPlayer(UMP_InventoryComponent* TargetComponent, FMP_InventoryStruct Item);

    // Client-side getters (replicated data)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Component")
    TArray<FMP_InventoryStruct> GetItemsByTag(FGameplayTagContainer Tag, bool bRequireAllTags) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Component")
    FMP_InventoryStruct GetItemByItemID(FName ItemID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Component")
    TArray<FMP_InventoryStruct> GetItemsByItemName(FString ItemName) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Component")
    const TArray<FMP_InventoryStruct>& GetAllItems() const { return InventoryItems; }

    // Add to protected section
protected:

    UPROPERTY(ReplicatedUsing = OnRep_InventoryItems, BlueprintReadOnly, Category = "MP_Inventory|Component")
    TArray<FMP_InventoryStruct> InventoryItems;

    UFUNCTION()
	void OnRep_InventoryItems();

    // Client-side: Sync operations to local Subsystem
    UFUNCTION(Client, Reliable)
    void ClientAddItem(FMP_InventoryStruct Item);

    UFUNCTION(Client, Reliable)
    void ClientRemoveItemByIndex(int32 Index, int32 Quantity);

    UFUNCTION(Client, Reliable)
    void ClientRemoveItemByID(FName ItemID, int32 Quantity);

    UFUNCTION(Client, Reliable)
    void ClientDropItem(int32 Index, int32 Quantity);

    UFUNCTION(Client, Reliable)
    void ClientSyncFromLocalSubsystem();

    UFUNCTION(Server, Reliable)
    void ServerSyncFromLocalSubsystem(const TArray<FMP_InventoryStruct>& Inventory);

    // Multicast RPC to broadcast to all clients
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_BroadcastInventoryUpdate();

private:

		
};
