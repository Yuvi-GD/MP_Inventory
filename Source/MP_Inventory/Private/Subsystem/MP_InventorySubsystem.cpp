// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/MP_InventorySubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/MP_InventorySave.h"

void UMP_InventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadInventory(DefaultSlotName);
    UE_LOG(LogTemp, Log, TEXT("UMP_InventorySubsystem::Initialize - InventoryItems.Num(): %d"), InventoryItems.Num());
}

void UMP_InventorySubsystem::Deinitialize()
{
    SaveInventory(DefaultSlotName);
    Super::Deinitialize();
}

void UMP_InventorySubsystem::AddItem(FMP_InventoryStruct Item)
{
    FScopeLock Lock(&InventoryCriticalSection); // Thread Safety

    UE_LOG(LogTemp, Log, TEXT("UMP_InventorySubsystem::AddItem - ItemID: %s, Quantity: %d"), *Item.ItemID.ToString(), Item.Quantity);

    if (Item.ItemID.IsNone() || Item.Quantity <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_InventorySubsystem::AddItem - Invalid item (ItemID: %s, Quantity: %d)"), *Item.ItemID.ToString(), Item.Quantity);
        return;
    }

    // Add Item.Public tag by default if not already present
    if (!Item.Tags.HasTag(FGameplayTag::RequestGameplayTag("Item.Public")))
    {
        Item.Tags.AddTag(FGameplayTag::RequestGameplayTag("Item.Public"));
    }


    for (FMP_InventoryStruct& ExistingItem : InventoryItems)
    {
        if (ExistingItem.ItemID == Item.ItemID)
        {
            ExistingItem.Quantity += Item.Quantity;
            bIsProcessingServerChange = false;
            OnInventoryUpdated.Broadcast();
            UE_LOG(LogTemp, Log, TEXT("UMP_InventorySubsystem::AddItem - Stacked item. New Quantity: %d"), ExistingItem.Quantity);
            if (bAutoSave)
            {
                SaveInventory(DefaultSlotName);
            }
            return;
        }
    }

    InventoryItems.Add(Item);
    bIsProcessingServerChange = false;
    OnInventoryUpdated.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("UMP_InventorySubsystem::AddItem - Added new item. InventoryItems.Num(): %d"), InventoryItems.Num());
    if (bAutoSave)
    {
        SaveInventory(DefaultSlotName);
    }
}

void UMP_InventorySubsystem::RemoveItemByIndex(int32 Index, int32 QuantityToRemove)
{
    FScopeLock Lock(&InventoryCriticalSection); // Thread Safety

    if (InventoryItems.IsValidIndex(Index))
    {
        if (QuantityToRemove == -1)
        {
            InventoryItems.RemoveAt(Index);
        }
        else
        {
            // Otherwise, reduce by the specified amount
            if (QuantityToRemove <= 0)
            {
                return; // Invalid quantity to remove (except -1)
            }
            InventoryItems[Index].Quantity -= QuantityToRemove;
            if (InventoryItems[Index].Quantity <= 0)
            {
                InventoryItems.RemoveAt(Index);
            }
        }

        OnInventoryUpdated.Broadcast();
        if (bAutoSave)
        {
            SaveInventory(DefaultSlotName);
        }
    }
}

void UMP_InventorySubsystem::RemoveItem(FMP_InventoryStruct Item, int32 QuantityToRemove)
{
    FScopeLock Lock(&InventoryCriticalSection); // Thread Safety

    for (int32 i = 0; i < InventoryItems.Num(); i++)
    {
        if (InventoryItems[i].ItemID == Item.ItemID)
        {
            if (QuantityToRemove == -1)
            {
                InventoryItems.RemoveAt(i);
            }
            else
            {
                // Otherwise, reduce by the specified amount
                if (QuantityToRemove <= 0)
                {
                    return; // Invalid quantity to remove (except -1)
                }
                InventoryItems[i].Quantity -= QuantityToRemove;
                if (InventoryItems[i].Quantity <= 0)
                {
                    InventoryItems.RemoveAt(i);
                }
            }

            OnInventoryUpdated.Broadcast();
            if (bAutoSave)
            {
                SaveInventory(DefaultSlotName);
            }
            return;
        }
    }
}

void UMP_InventorySubsystem::ReplaceItemByIndex(int32 Index, FMP_InventoryStruct NewItem)
{
    FScopeLock Lock(&InventoryCriticalSection); // Thread Safety

    if (NewItem.ItemID.IsNone() || NewItem.Quantity <= 0)
    {
        return;
    }

    if (InventoryItems.IsValidIndex(Index))
    {
        InventoryItems[Index] = NewItem;
        OnInventoryUpdated.Broadcast();
        if (bAutoSave)
        {
            SaveInventory(DefaultSlotName);
        }
    }
}

void UMP_InventorySubsystem::ReplaceItem(FMP_InventoryStruct OldItem, FMP_InventoryStruct NewItem)
{
    FScopeLock Lock(&InventoryCriticalSection); // Thread Safety

    if (NewItem.ItemID.IsNone() || NewItem.Quantity <= 0)
    {
        return;
    }

    for (int32 i = 0; i < InventoryItems.Num(); i++)
    {
        if (InventoryItems[i].ItemID == OldItem.ItemID)
        {
            InventoryItems[i] = NewItem;
            OnInventoryUpdated.Broadcast();
            if (bAutoSave)
            {
                SaveInventory(DefaultSlotName);
            }
            return;
        }
    }
}

void UMP_InventorySubsystem::SwapItems(int32 IndexA, int32 IndexB)
{
    FScopeLock Lock(&InventoryCriticalSection); // Thread Safety

    if (InventoryItems.IsValidIndex(IndexA) && InventoryItems.IsValidIndex(IndexB))
    {
        InventoryItems.Swap(IndexA, IndexB);
        OnInventoryUpdated.Broadcast();
        if (bAutoSave)
        {
            SaveInventory(DefaultSlotName);
        }
    }
}

void UMP_InventorySubsystem::UpdateItemTags(FName ItemID, FGameplayTag TagToRemove, FGameplayTag TagToAdd)
{
    FScopeLock Lock(&InventoryCriticalSection); // Thread Safety

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
                OnInventoryUpdated.Broadcast();
                if (bAutoSave)
                {
                    SaveInventory(DefaultSlotName);
                }
            }
            return;
        }
    }
}

TArray<FMP_InventoryStruct> UMP_InventorySubsystem::GetItemsByTag(FGameplayTagContainer Tags, bool bRequireAllTags) const
{
    TArray<FMP_InventoryStruct> FilteredItems;
    for (const FMP_InventoryStruct& Item : InventoryItems)
    {
        if (bRequireAllTags)
        {
            if (Item.Tags.HasAll(Tags))
            {
                FilteredItems.Add(Item);
            }
        }
        else
        {
            if (Item.Tags.HasAny(Tags))
            {
                FilteredItems.Add(Item);
            }
        }
    }
    return FilteredItems;
}

UTexture* UMP_InventorySubsystem::LoadItemIcon(const FMP_InventoryStruct& Item)
{
    if (!Item.Icon.IsNull())
    {
        return Item.Icon.LoadSynchronous();
    }
    return nullptr;
}

bool UMP_InventorySubsystem::HasSpaceForItem(FMP_InventoryStruct Item) const
{
    constexpr int32 MaxInventorySize = 100; // Hard limit for now
    return InventoryItems.Num() < MaxInventorySize;
}

void UMP_InventorySubsystem::DropItem(int32 Index, int32 Quantity)
{
    if (InventoryItems.IsValidIndex(Index))
    {
        FMP_InventoryStruct& Item = InventoryItems[Index];
        if (!Item.Tags.HasTag(FGameplayTag::RequestGameplayTag("Item.Private")))
        {
            RemoveItemByIndex(Index, Quantity);
            // TODO: Spawn item in world -- added from plan
        }
    }
}

void UMP_InventorySubsystem::HandleDeathLoss()
{
    // Placeholder: Implement death loss modes (Keep All default) -- added from plan
    OnInventoryUpdated.Broadcast();
    if (bAutoSave) SaveInventory(DefaultSlotName);
}

void UMP_InventorySubsystem::SaveInventory(const FString& PlayerID)
{
    UMP_InventorySave* SaveGameInstance = Cast<UMP_InventorySave>(UGameplayStatics::CreateSaveGameObject(UMP_InventorySave::StaticClass()));
    if (SaveGameInstance)
    {

        FString SlotName = PlayerID.IsEmpty() ? DefaultSlotName : PlayerID; // Use PlayerID or fallback to DefaultSlotName

        SaveGameInstance->InventoryItems = InventoryItems;
        SaveGameInstance->SaveToSlot(SlotName);
        UE_LOG(LogTemp, Log, TEXT("UMP_InventorySubsystem::SaveInventory - Saved %d items to slot 'InventorySlot'"), InventoryItems.Num());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_InventorySubsystem::SaveInventory - Failed to create SaveGameInstance"));
    }
}

void UMP_InventorySubsystem::LoadInventory(const FString& PlayerID)
{
    
    UMP_InventorySave* SaveGameInstance = Cast<UMP_InventorySave>(UGameplayStatics::CreateSaveGameObject(UMP_InventorySave::StaticClass()));
    FString SlotName = PlayerID.IsEmpty() ? DefaultSlotName : PlayerID; // Use PlayerID or fallback to DefaultSlotName
    if (SaveGameInstance && SaveGameInstance->LoadFromSlot(SlotName))
    {
        InventoryItems = SaveGameInstance->InventoryItems;
        UE_LOG(LogTemp, Log, TEXT("UMP_InventorySubsystem::LoadInventory - Loaded %d items from slot 'InventorySlot'"), InventoryItems.Num());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_InventorySubsystem::LoadInventory - Failed to load from slot 'InventorySlot'"));
        InventoryItems.Empty(); // Ensure the array is empty if loading fails
    }
}

FMP_InventoryStruct UMP_InventorySubsystem::GetItemByItemID(FName ItemID) const
{
    for (const FMP_InventoryStruct& Item : InventoryItems)
    {
        if (Item.ItemID == ItemID)
        {
            return Item;
        }
    }
    return FMP_InventoryStruct();
}

TArray<FMP_InventoryStruct> UMP_InventorySubsystem::GetItemsByItemName(FString ItemName) const
{
    TArray<FMP_InventoryStruct> FoundItems;
    for (const FMP_InventoryStruct& Item : InventoryItems)
    {
        if (Item.DisplayName.Contains(ItemName))
        {
            FoundItems.Add(Item);
        }
    }
    return FoundItems;
}