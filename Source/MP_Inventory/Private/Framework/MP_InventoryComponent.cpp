// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/MP_InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/MP_InventorySave.h"
#include "GameFramework/PlayerController.h"



// Sets default values for this component's properties
UMP_InventoryComponent::UMP_InventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
    InventoryItems.SetOwner(this);
	// ...
}


// Called when the game starts
void UMP_InventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    InventoryItems.SetOwner(this);
    APlayerController* PlayerControllerOwner = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
    if (PlayerControllerOwner && PlayerControllerOwner->IsLocalController()) // Check for owning client
    {
        LoadInventory(SaveSlotName);
    }
}

void UMP_InventoryComponent::EndPlay(const EEndPlayReason::Type Reason)  
{  
   APlayerController* PlayerControllerOwner = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());  
   if (PlayerControllerOwner && PlayerControllerOwner->IsLocalController()) // Run on owning client only  
   {  
       SaveInventory(SaveSlotName);  
   }  
   Super::EndPlay(Reason);
}

void UMP_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UMP_InventoryComponent, InventoryItems);
	DOREPLIFETIME(UMP_InventoryComponent, UniqueId);
}

void UMP_InventoryComponent::SaveInventory(const FString& PlayerID)
{
    UMP_InventorySave* SaveGameInstance = Cast<UMP_InventorySave>(UGameplayStatics::CreateSaveGameObject(UMP_InventorySave::StaticClass()));
    if (SaveGameInstance)
    {

        FString SlotName = PlayerID.IsEmpty() ? SaveSlotName : PlayerID; // Use PlayerID or fallback to DefaultSlotName

        SaveGameInstance->InventoryItems = InventoryItems.Items;
        SaveGameInstance->SaveToSlot(SlotName);
        UE_LOG(LogTemp, Log, TEXT("UMP_InventorySubsystem::SaveInventory - Saved %d items to slot 'InventorySlot'"), InventoryItems.Items.Num());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_InventorySubsystem::SaveInventory - Failed to create SaveGameInstance"));
    }
}

void UMP_InventoryComponent::LoadInventory(const FString& PlayerID)
{
    UMP_InventorySave* SaveGameInstance = Cast<UMP_InventorySave>(UGameplayStatics::CreateSaveGameObject(UMP_InventorySave::StaticClass()));
    FString SlotName = PlayerID.IsEmpty() ? SaveSlotName : PlayerID; // Use PlayerID or fallback to DefaultSlotName

    if (SaveGameInstance && SaveGameInstance->LoadFromSlot(SlotName))
    {
        TArray<FMP_InventoryItem> Items;
        Items = SaveGameInstance->InventoryItems;

        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this,Items]()
            {
                // First action: Sync inventory
                APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
                if (PC && PC->IsLocalController())
                {
                    ServerSyncFromClient(Items);
                }
            }, 1.0f, false);


        UE_LOG(LogTemp, Log, TEXT("UMP_InventorySubsystem::LoadInventory - Loaded %d items from slot 'InventorySlot'"), InventoryItems.Items.Num());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_InventorySubsystem::LoadInventory - Failed to load from slot 'InventorySlot'"));
        InventoryItems.Items.Empty(); // Ensure the array is empty if loading fails
    }
}

void UMP_InventoryComponent::ServerSyncFromClient_Implementation(const TArray<FMP_InventoryItem>& Items)
{
    if (!GetOwner()->HasAuthority()) return;

    InventoryItems.Items = Items;
    InventoryItems.Items.Empty();  // ✅ Clears previous inventory
    for (const FMP_InventoryItem& Item : Items)
    {
        InventoryItems.Items.Add(Item);
        InventoryItems.MarkItemDirty(InventoryItems.Items.Last()); // ✅ Marks individual items for replication
    }
    // Notify for full refresh
    FireInventoryUpdate(EInventoryDelta::Refresh, -1);
    
    //OnInventoryUpdated.Broadcast(EInventoryDelta::Refresh, -1);        // owner already knows; others get via replication
    //Multicast_BroadcastInventoryUpdate(EInventoryDelta::Refresh, -1);
}

void UMP_InventoryComponent::AddItem_Implementation(FMP_InventoryItem Item)
{
    if (!GetOwner()->HasAuthority()) return;

    if (!Item.Tags.HasTag(FGameplayTag::RequestGameplayTag("Item.Public")))
    {
        Item.Tags.AddTag(FGameplayTag::RequestGameplayTag("Item.Public"));
    }

    if (Item.Tags.HasTag(FGameplayTag::RequestGameplayTag("Item.Private")))
    {
        Item.Tags.RemoveTag(FGameplayTag::RequestGameplayTag("Item.Private"));
    }

    if (Item.ItemID.IsNone() || Item.Quantity <= 0) return;

    InventoryItems.AddItem(Item);
}

void UMP_InventoryComponent::RemoveItemByIndex_Implementation(int32 Index, int32 Quantity)
{
    if (!GetOwner()->HasAuthority()) return;

    InventoryItems.RemoveItem(Index, Quantity);
}

void UMP_InventoryComponent::RemoveItemByID_Implementation(FName ItemID, int32 Quantity)
{
    if (!GetOwner()->HasAuthority())
    {
		UE_LOG(LogTemp, Warning, TEXT("UMP_InventoryComponent::RemoveItemByID - Called on non-authoritative client!"));
        return;
    }

    for (int32 i = 0; i < InventoryItems.Items.Num(); i++)
    {
        if (InventoryItems.Items[i].ItemID == ItemID)
        {
            InventoryItems.RemoveItem(i, Quantity);
			UE_LOG(LogTemp, Log, TEXT("UMP_InventoryComponent::RemoveItemByID - Removed %s x%d from inventory."), *ItemID.ToString(), Quantity);
            return;
        }
    }
	UE_LOG(LogTemp, Warning, TEXT("UMP_InventoryComponent::RemoveItemByID - Item with ID %s not found in inventory."), *ItemID.ToString());
    return;
}

void UMP_InventoryComponent::ReplaceItemByIndex_Implementation(int32 Index, FMP_InventoryItem NewItem)
{
    if (!GetOwner()->HasAuthority()) return;
    if (NewItem.ItemID.IsNone() || NewItem.Quantity <= 0)
    {
        return;
    }
    InventoryItems.UpdateItem(Index, NewItem);
}

void UMP_InventoryComponent::ReplaceItem_Implementation(FMP_InventoryItem OldItem, FMP_InventoryItem NewItem)
{
    if (!GetOwner()->HasAuthority()) return;
    if (NewItem.ItemID.IsNone() || NewItem.Quantity <= 0)
    {
        return;
    }

    for (int32 i = 0; i < InventoryItems.Items.Num(); i++)
    {
        if (InventoryItems.Items[i].ItemID == OldItem.ItemID)
        {
            InventoryItems.UpdateItem(i, NewItem);
            return;
        }
    }

}

void UMP_InventoryComponent::SwapItems_Implementation(int32 IndexA, int32 IndexB)
{
    if (!GetOwner()->HasAuthority()) return;
    InventoryItems.SwapItems(IndexA, IndexB);
}

void UMP_InventoryComponent::UpdateItemTags_Implementation(FName ItemID, FGameplayTag TagToRemove, FGameplayTag TagToAdd)
{
    if (!GetOwner()->HasAuthority()) return;
    for (int32 i = 0; i < InventoryItems.Items.Num(); ++i)
    {
        FMP_InventoryItem& Item = InventoryItems.Items[i];
        if (Item.ItemID == ItemID)
        {
            if (Item.Tags.HasTag(TagToRemove))
            {
                Item.Tags.RemoveTag(TagToRemove);
                Item.Tags.AddTag(TagToAdd);
                InventoryItems.MarkItemDirty(Item);
                FireInventoryUpdate(EInventoryDelta::Updated, i);
                return;
            }
        }
    }
}

void UMP_InventoryComponent::DropItem_Implementation(int32 Index, int32 Quantity)
{
    if (!GetOwner()->HasAuthority()) return;

    if (InventoryItems.Items.IsValidIndex(Index) && !InventoryItems.Items[Index].Tags.HasTag(FGameplayTag::RequestGameplayTag("Item.Private")))
    {
        InventoryItems.RemoveItem(Index, Quantity);
        // TODO: Spawn item in world
    }
}

void UMP_InventoryComponent::RequestItemFromPlayer_Implementation(UMP_InventoryComponent* TargetComponent, FName ItemID, int32 Quantity)
{
    if (!GetOwner()->HasAuthority()) return;
    if (!TargetComponent || !TargetComponent->GetOwner()->HasAuthority()) return;

    FMP_InventoryItem RequestedItem = TargetComponent->GetItemByItemID(ItemID);
    if (RequestedItem.ItemID.IsNone() || RequestedItem.Quantity < Quantity) return;

    TargetComponent->RemoveItemByID(ItemID, Quantity);
    FMP_InventoryItem ItemToAdd = RequestedItem;
    ItemToAdd.Quantity = Quantity;
    AddItem(ItemToAdd);

    UE_LOG(LogTemp, Log, TEXT("UMP_InventoryComponent::RequestItemFromPlayer - Requested %s x%d from %s"), *ItemID.ToString(), Quantity, *TargetComponent->GetOwner()->GetName());
}

void UMP_InventoryComponent::GiveItemToPlayer_Implementation(UMP_InventoryComponent* TargetComponent, FMP_InventoryItem Item)
{
    if (!GetOwner()->HasAuthority()) return;
    if (!TargetComponent || !TargetComponent->GetOwner()->HasAuthority()) return;

    if (Item.ItemID.IsNone() || Item.Quantity <= 0) return;

    FMP_InventoryItem LocalItem = GetItemByItemID(Item.ItemID);
    if (LocalItem.Quantity < Item.Quantity) return;

    RemoveItemByID(Item.ItemID, Item.Quantity);
    TargetComponent->AddItem(Item);

    UE_LOG(LogTemp, Log, TEXT("UMP_InventoryComponent::GiveItemToPlayer - Gave %s x%d to %s"), *Item.ItemID.ToString(), Item.Quantity, *TargetComponent->GetOwner()->GetName());
}

//-------------------------------------------- PURE FUCNTIONS --------------------------------------------------//

TArray<FMP_InventoryItem> UMP_InventoryComponent::GetItemsByTag(FGameplayTagContainer Tag, bool bRequireAllTags) const
{
    TArray<FMP_InventoryItem> FilteredItems;
    for (const FMP_InventoryItem& Item : InventoryItems.Items)
    {
        if (bRequireAllTags ? Item.Tags.HasAll(Tag) : Item.Tags.HasAny(Tag))
        {
            FilteredItems.Add(Item);
        }
    }
    return FilteredItems;
}

FMP_InventoryItem UMP_InventoryComponent::GetItemByItemID(FName ItemID) const
{
    for (const FMP_InventoryItem& Item : InventoryItems.Items)
    {
        if (Item.ItemID == ItemID) return Item;
    }
    return FMP_InventoryItem();
}

FMP_InventoryItem UMP_InventoryComponent::GetItemByIndex(int32 Index) const
{
	if (InventoryItems.Items.IsValidIndex(Index))
	{
		return InventoryItems.Items[Index];
	}
    return FMP_InventoryItem();
}

TArray<FMP_InventoryItem> UMP_InventoryComponent::GetItemsByItemName(FString ItemName) const
{
    TArray<FMP_InventoryItem> FoundItems;
    for (const FMP_InventoryItem& Item : InventoryItems.Items)
    {
        if (Item.DisplayName.Contains(ItemName)) FoundItems.Add(Item);
    }
    return FoundItems;
}

void UMP_InventoryComponent::OnRep_InventoryItems()
{
    //OnInventoryUpdated.Broadcast(); // Fires on all clients AND server
}

void UMP_InventoryComponent::Multicast_BroadcastInventoryUpdate_Implementation(const EInventoryDelta& Delta, const int32& Index)
{
    UE_LOG(LogTemp, Log, TEXT("UMP_InventoryComponent::Multicast Distpatcher event - Requested from %s"), *this->GetOwner()->GetName());
    UE_LOG(LogTemp, Warning, TEXT("UMP_InventoryComponent::Multicast running on: %d"), (int32)GetNetMode());
    OnInventoryUpdated.Broadcast(Delta,Index); // Fires on all clients AND server
}

// ------------- FastArray Notifies Component: Call Dispatcher ----------------

void UMP_InventoryComponent::FireInventoryUpdate(EInventoryDelta Delta, int32 SlotIndex)
{
    OnInventoryUpdated.Broadcast(Delta, SlotIndex);
}

