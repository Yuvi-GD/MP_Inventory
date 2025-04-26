// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/MP_InventoryComponent.h"
#include "Framework/MP_InventorySubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"



// Sets default values for this component's properties
UMP_InventoryComponent::UMP_InventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	// ...
}


// Called when the game starts
void UMP_InventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    if (GetNetMode() == NM_DedicatedServer || GetNetMode() == NM_ListenServer)
    {
        //SyncToLocalSubsystem(); // Server syncs initially
    }
    UE_LOG(LogTemp, Log, TEXT("UMP_InventoryComponent::Inventory not Setup From Subsystem"));
    APlayerController* PlayerControllerOwner = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
    if(PlayerControllerOwner)
    {
        if (PlayerControllerOwner->IsLocalController())
        {
            if (UMP_InventorySubsystem* Subsystem = GetOwner()->GetGameInstance()->GetSubsystem<UMP_InventorySubsystem>())
            {
                InventoryItems = Subsystem->GetAllItems();
                ServerSyncFromLocalSubsystem(Subsystem->GetAllItems());
                UE_LOG(LogTemp, Log, TEXT("UMP_InventoryComponent::Inventory Setup From Subsystem with total item %d"), InventoryItems.Max());
            }
        }
    }

}

void UMP_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UMP_InventoryComponent, InventoryItems);
}


void UMP_InventoryComponent::AddItem_Implementation(FMP_InventoryStruct Item)
{
    if (!GetOwner()->HasAuthority()) return;

    if (!Item.Tags.HasTag(FGameplayTag::RequestGameplayTag("Item.Public")))
    {
        Item.Tags.AddTag(FGameplayTag::RequestGameplayTag("Item.Public"));

        if (Item.Tags.HasTag(FGameplayTag::RequestGameplayTag("Item.Private")))
        {
            Item.Tags.RemoveTag(FGameplayTag::RequestGameplayTag("Item.Private"));
        }
    }

    if (Item.ItemID.IsNone() || Item.Quantity <= 0) return;

    for (FMP_InventoryStruct& ExistingItem : InventoryItems)
    {
        if (ExistingItem.ItemID == Item.ItemID)
        {
            ExistingItem.Quantity += Item.Quantity;
            ClientAddItem(Item);
            Multicast_BroadcastInventoryUpdate();
            return;
        }
    }
    InventoryItems.Add(Item);
    ClientAddItem(Item);
    Multicast_BroadcastInventoryUpdate();
    return;
}

void UMP_InventoryComponent::RemoveItemByIndex_Implementation(int32 Index, int32 Quantity)
{
    if (!GetOwner()->HasAuthority()) return;

    if (InventoryItems.IsValidIndex(Index))
    {
        if (Quantity <= 0 && Quantity != -1) return;
        if (Quantity == -1 || Quantity >= InventoryItems[Index].Quantity)
        {
            InventoryItems.RemoveAt(Index);
        }
        else
        {
            InventoryItems[Index].Quantity -= Quantity;
        }
        ClientRemoveItemByIndex(Index, Quantity);
        Multicast_BroadcastInventoryUpdate();
    }
}

void UMP_InventoryComponent::RemoveItemByID_Implementation(FName ItemID, int32 Quantity)
{
    if (!GetOwner()->HasAuthority()) return;

    int32 Index = -1;
    for (int32 i = 0; i < InventoryItems.Num(); i++)
    {
        if (InventoryItems[i].ItemID == ItemID)
        {
            Index = i;
            break;
        }
    }
    if (Index != -1)
    {
        if (Quantity <= 0 && Quantity != -1) return;
        if (Quantity == -1 || Quantity >= InventoryItems[Index].Quantity)
        {
            InventoryItems.RemoveAt(Index);
        }
        else
        {
            InventoryItems[Index].Quantity -= Quantity;
        }
        ClientRemoveItemByIndex(Index, Quantity);
        Multicast_BroadcastInventoryUpdate();
    }
}

void UMP_InventoryComponent::ReplaceItemByIndex_Implementation(int32 Index, FMP_InventoryStruct NewItem)
{
    if (!GetOwner()->HasAuthority()) return;
    if (NewItem.ItemID.IsNone() || NewItem.Quantity <= 0)
    {
        return;
    }

    if (InventoryItems.IsValidIndex(Index))
    {
        InventoryItems[Index] = NewItem;
        ClientReplaceItemByIndex(Index, NewItem);
        Multicast_BroadcastInventoryUpdate();
    }
}

void UMP_InventoryComponent::ReplaceItem_Implementation(FMP_InventoryStruct OldItem, FMP_InventoryStruct NewItem)
{
    if (!GetOwner()->HasAuthority()) return;
    if (NewItem.ItemID.IsNone() || NewItem.Quantity <= 0)
    {
        return;
    }

    for (int32 i = 0; i < InventoryItems.Num(); i++)
    {
        if (InventoryItems[i].ItemID == OldItem.ItemID)
        {
            InventoryItems[i] = NewItem;
            ClientReplaceItemByIndex(i, NewItem);
            Multicast_BroadcastInventoryUpdate();
            break;
        }
    }

}

void UMP_InventoryComponent::SwapItems_Implementation(int32 IndexA, int32 IndexB)
{
    if (!GetOwner()->HasAuthority()) return;
    if (InventoryItems.IsValidIndex(IndexA) && InventoryItems.IsValidIndex(IndexB))
    {
        InventoryItems.Swap(IndexA, IndexB);
        ClientSwapItems(IndexA, IndexB);
        Multicast_BroadcastInventoryUpdate();
    }
}

void UMP_InventoryComponent::UpdateItemTags_Implementation(FName ItemID, FGameplayTag TagToRemove, FGameplayTag TagToAdd)
{
    if (!GetOwner()->HasAuthority()) return;

    //for (int32 i = 0; i < InventoryItems.Num(); i++)
    for (FMP_InventoryStruct& Item : InventoryItems)
    {
        if (Item.ItemID == ItemID)
        {
            // Check if the item has the tag to remove
            if (Item.Tags.HasTag(TagToRemove))
            {
                // Remove the old tag and add the new tag
                Item.Tags.RemoveTag(TagToRemove);
                Item.Tags.AddTag(TagToAdd);
                ClientUpdateItemTags(ItemID,TagToRemove,TagToAdd);
                Multicast_BroadcastInventoryUpdate();
            }
            return;
        }
    }
}

void UMP_InventoryComponent::DropItem_Implementation(int32 Index, int32 Quantity)
{
    if (!GetOwner()->HasAuthority()) return;

    if (InventoryItems.IsValidIndex(Index) && !InventoryItems[Index].Tags.HasTag(FGameplayTag::RequestGameplayTag("Item.Private")))
    {
        if (Quantity <= 0 && Quantity != -1) return;
        if (Quantity == -1 || Quantity >= InventoryItems[Index].Quantity)
        {
            InventoryItems.RemoveAt(Index);
        }
        else
        {
            InventoryItems[Index].Quantity -= Quantity;
        }
        ClientDropItem(Index, Quantity);
        // TODO: Spawn item in world
    }
}

void UMP_InventoryComponent::RequestItemFromPlayer_Implementation(UMP_InventoryComponent* TargetComponent, FName ItemID, int32 Quantity)
{
    if (!GetOwner()->HasAuthority()) return;
    if (!TargetComponent || !TargetComponent->GetOwner()->HasAuthority()) return;

    FMP_InventoryStruct RequestedItem = TargetComponent->GetItemByItemID(ItemID);
    if (RequestedItem.ItemID.IsNone() || RequestedItem.Quantity < Quantity) return;

    TargetComponent->RemoveItemByID(ItemID, Quantity);
    FMP_InventoryStruct ItemToAdd = RequestedItem;
    ItemToAdd.Quantity = Quantity;
    AddItem(ItemToAdd);

    UE_LOG(LogTemp, Log, TEXT("UMP_InventoryComponent::RequestItemFromPlayer - Requested %s x%d from %s"), *ItemID.ToString(), Quantity, *TargetComponent->GetOwner()->GetName());
}

void UMP_InventoryComponent::GiveItemToPlayer_Implementation(UMP_InventoryComponent* TargetComponent, FMP_InventoryStruct Item)
{
    if (!GetOwner()->HasAuthority()) return;
    if (!TargetComponent || !TargetComponent->GetOwner()->HasAuthority()) return;

    if (Item.ItemID.IsNone() || Item.Quantity <= 0) return;

    FMP_InventoryStruct LocalItem = GetItemByItemID(Item.ItemID);
    if (LocalItem.Quantity < Item.Quantity) return;

    RemoveItemByID(Item.ItemID, Item.Quantity);
    TargetComponent->AddItem(Item);

    UE_LOG(LogTemp, Log, TEXT("UMP_InventoryComponent::GiveItemToPlayer - Gave %s x%d to %s"), *Item.ItemID.ToString(), Item.Quantity, *TargetComponent->GetOwner()->GetName());
}

//-------------------------------------------- PURE FUCNTIONS --------------------------------------------------//

TArray<FMP_InventoryStruct> UMP_InventoryComponent::GetItemsByTag(FGameplayTagContainer Tag, bool bRequireAllTags) const
{
    TArray<FMP_InventoryStruct> FilteredItems;
    for (const FMP_InventoryStruct& Item : InventoryItems)
    {
        if (bRequireAllTags ? Item.Tags.HasAll(Tag) : Item.Tags.HasAny(Tag))
        {
            FilteredItems.Add(Item);
        }
    }
    return FilteredItems;
}

FMP_InventoryStruct UMP_InventoryComponent::GetItemByItemID(FName ItemID) const
{
    for (const FMP_InventoryStruct& Item : InventoryItems)
    {
        if (Item.ItemID == ItemID) return Item;
    }
    return FMP_InventoryStruct();
}

TArray<FMP_InventoryStruct> UMP_InventoryComponent::GetItemsByItemName(FString ItemName) const
{
    TArray<FMP_InventoryStruct> FoundItems;
    for (const FMP_InventoryStruct& Item : InventoryItems)
    {
        if (Item.DisplayName == ItemName) FoundItems.Add(Item);
    }
    return FoundItems;
}

void UMP_InventoryComponent::OnRep_InventoryItems()
{
    //SyncToLocalSubsystem();
    //if (UMP_InventorySubsystem* Subsystem = GetOwner()->GetGameInstance()->GetSubsystem<UMP_InventorySubsystem>())
    //{
    //    Subsystem->OnInventoryUpdated.Broadcast();
    //    OnInventoryUpdated.Broadcast();
    //}
}

void UMP_InventoryComponent::ClientAddItem_Implementation(FMP_InventoryStruct Item)
{
    if (UMP_InventorySubsystem* Subsystem = GetOwner()->GetGameInstance()->GetSubsystem<UMP_InventorySubsystem>())
    {
        Subsystem->bIsProcessingServerChange = true;
        Subsystem->AddItem(Item);
    }
}

void UMP_InventoryComponent::ClientRemoveItemByIndex_Implementation(int32 Index, int32 Quantity)
{
    if (UMP_InventorySubsystem* Subsystem = GetOwner()->GetGameInstance()->GetSubsystem<UMP_InventorySubsystem>())
    {
        Subsystem->RemoveItemByIndex(Index, Quantity);
    }
}

void UMP_InventoryComponent::ClientReplaceItemByIndex_Implementation(int32 Index, FMP_InventoryStruct NewItem)
{
    if (UMP_InventorySubsystem* Subsystem = GetOwner()->GetGameInstance()->GetSubsystem<UMP_InventorySubsystem>())
    {
        Subsystem->ReplaceItemByIndex(Index,NewItem);
    }
}

void UMP_InventoryComponent::ClientSwapItems_Implementation(int32 IndexA, int32 IndexB)
{
    if (UMP_InventorySubsystem* Subsystem = GetOwner()->GetGameInstance()->GetSubsystem<UMP_InventorySubsystem>())
    {
        Subsystem->SwapItems(IndexA,IndexB);
    }
}

void UMP_InventoryComponent::ClientUpdateItemTags_Implementation(FName ItemID, FGameplayTag TagToRemove, FGameplayTag TagToAdd)
{
    if (UMP_InventorySubsystem* Subsystem = GetOwner()->GetGameInstance()->GetSubsystem<UMP_InventorySubsystem>())
    {
        Subsystem->UpdateItemTags(ItemID,TagToRemove,TagToAdd);
    }
}

void UMP_InventoryComponent::ClientDropItem_Implementation(int32 Index, int32 Quantity)
{
    if (UMP_InventorySubsystem* Subsystem = GetOwner()->GetGameInstance()->GetSubsystem<UMP_InventorySubsystem>())
    {
        Subsystem->DropItem(Index, Quantity);
    }
}

void UMP_InventoryComponent::ClientSyncFromLocalSubsystem_Implementation()
{
    if (UMP_InventorySubsystem* Subsystem = GetOwner()->GetGameInstance()->GetSubsystem<UMP_InventorySubsystem>())
    {
        InventoryItems = Subsystem->GetAllItems();
        ServerSyncFromLocalSubsystem(Subsystem->GetAllItems());
        Subsystem->OnInventoryUpdated.Broadcast();
    }
}

void UMP_InventoryComponent::ServerSyncFromLocalSubsystem_Implementation(const TArray<FMP_InventoryStruct>& Inventory)
{
    if (GetOwner()->HasAuthority())
    {
        InventoryItems = Inventory;
        Multicast_BroadcastInventoryUpdate();
    }
}

void UMP_InventoryComponent::Multicast_BroadcastInventoryUpdate_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("UMP_InventoryComponent::Multicast Distpatcher event - Requested from %s"), *this->GetOwner()->GetName());
    OnInventoryUpdated.Broadcast(); // Fires on all clients AND server
}
