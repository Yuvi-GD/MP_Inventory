// Fill out your copyright notice in the Description page of Project Settings.


#include "Database/MP_InventoryFastArray.h"
#include "Database/MP_InventoryStruct.h"

// Utility to notify owner/component
void FMP_InventoryArray::NotifyOwner(EInventoryDelta Delta, int32 SlotIndex)
{
    if (Owner.IsValid())
    {
        // Your component must implement a function with this signature.
        // Usually: void FireInventoryUpdate(EInventoryDelta Delta, int32 SlotIndex)
        UFunction* NotifyFunc = Owner->FindFunction(FName("FireInventoryUpdate"));
        if (NotifyFunc)
        {
            struct FParams
            {
                EInventoryDelta Delta;
                int32 SlotIndex;
            };
            FParams Params;
            Params.Delta = Delta;
            Params.SlotIndex = SlotIndex;
            Owner->ProcessEvent(NotifyFunc, &Params);
        }
    }
}

// Add item (by struct), combine or add new, mark dirty
void FMP_InventoryArray::AddItem(const FMP_InventoryItem& NewItem)
{
    for (int32 i = 0; i < Items.Num(); ++i)
    {
        if (Items[i].ItemID == NewItem.ItemID)
        {
            Items[i].Quantity += NewItem.Quantity;
            MarkItemDirty(Items[i]);
            MarkArrayDirty();
            NotifyOwner(EInventoryDelta::Updated, i);
            return;
        }
    }
	// If not found, add as new item
    Items.Add(NewItem);
    MarkItemDirty(Items.Last());
    MarkArrayDirty();
    NotifyOwner(EInventoryDelta::Added, Items.Num() - 1);
}

// Remove item by index, decrease quantity/remove if zero, mark dirty
void FMP_InventoryArray::RemoveItem(int32 Index, int32 Quantity)
{
    if (!Items.IsValidIndex(Index))
    {
		UE_LOG(LogTemp, Warning, TEXT("FMP_InventoryArray::RemoveItem - Invalid index %d"), Index);
        return;
    }

    FMP_InventoryItem& Item = Items[Index];
    Item.Quantity -= Quantity;

    if (Quantity <= 0 && Quantity != -1)
    {
		UE_LOG(LogTemp, Warning, TEXT("FMP_InventoryArray::RemoveItem - Invalid quantity %d for item %s at index %d"), Quantity, *Item.ItemID.ToString(), Index);
        return; // If quantity is not -1 or less than or equal to zero, do nothing
    }
    if (Quantity == -1 || Item.Quantity == 0) // If quantity is -1 or equal to zero, remove item
    {
        Items.RemoveAt(Index);
        MarkArrayDirty();
        NotifyOwner(EInventoryDelta::Removed, Index);
        // No need to mark dirty after removal (UE will handle removal)
    }
    else
    {
		UE_LOG(LogTemp, Warning, TEXT("FMP_InventoryArray::RemoveItem - Decreased quantity of %s to %d at index %d"), *Item.ItemID.ToString(), Item.Quantity, Index);
        MarkItemDirty(Item);
        NotifyOwner(EInventoryDelta::Updated, Index);
    }
}

// Update item at index, full replacement, mark dirty
void FMP_InventoryArray::UpdateItem(int32 Index, const FMP_InventoryItem& NewItem)
{
    if (!Items.IsValidIndex(Index))
        return;

    Items[Index] = NewItem;
    MarkItemDirty(Items[Index]);
    MarkArrayDirty();
    NotifyOwner(EInventoryDelta::Updated, Index);
}

// Swap two items by index, mark both dirty
void FMP_InventoryArray::SwapItems(int32 IndexA, int32 IndexB)
{
    if (!Items.IsValidIndex(IndexA) || !Items.IsValidIndex(IndexB) || IndexA == IndexB)
        return;

    Items.Swap(IndexA, IndexB);
    MarkItemDirty(Items[IndexA]);
    MarkItemDirty(Items[IndexB]);
    MarkArrayDirty();
    NotifyOwner(EInventoryDelta::Updated, IndexA);
    NotifyOwner(EInventoryDelta::Updated, IndexB);
}

// FastArray replication macro (UE pattern)
bool FMP_InventoryArray::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
    return FFastArraySerializer::FastArrayDeltaSerialize<FMP_InventoryItem, FMP_InventoryArray>(Items, DeltaParms, *this);
}

void FMP_InventoryArray::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
    for (int32 Index : RemovedIndices)
    {
        NotifyOwner(EInventoryDelta::Removed, Index);
    }
}
void FMP_InventoryArray::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
    for (int32 Index : AddedIndices)
    {
        NotifyOwner(EInventoryDelta::Added, Index);
    }
}
void FMP_InventoryArray::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
    for (int32 Index : ChangedIndices)
    {
        NotifyOwner(EInventoryDelta::Updated, Index);
    }
}

