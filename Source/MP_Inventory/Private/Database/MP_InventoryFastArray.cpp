// Copyright 2026 UVSquare. All Rights Reserved.


#include "Database/MP_InventoryFastArray.h"
#include "Database/MP_InventoryStruct.h"
#include "Framework/MP_InventoryComponent.h"

// =============================================================================
//  OWNER NOTIFICATION
// =============================================================================

void FMP_InventoryArray::NotifyOwner(EInventoryDelta Delta, int32 ArrayIndex)
{
    if (!Owner.IsValid()) return;
    if (UMP_InventoryComponent* InvComp = Cast<UMP_InventoryComponent>(Owner.Get()))
    {
        InvComp->FireInventoryUpdate(Delta, ArrayIndex);
    }
}


// =============================================================================
//  WRITE OPERATIONS
// =============================================================================

void FMP_InventoryArray::AddItem(const FMP_InventoryItem& NewItem)
{
    Items.Add(NewItem);
    MarkItemDirty(Items.Last());
    MarkArrayDirty();
    NotifyOwner(EInventoryDelta::Added, Items.Num() - 1);
}

void FMP_InventoryArray::AddQuantity(int32 ArrayIndex, int32 Quantity)
{
    if (!Items.IsValidIndex(ArrayIndex) || Quantity <= 0) return;
    
    FMP_InventoryItem& Item = Items[ArrayIndex];
    Item.Quantity += Quantity;
    MarkItemDirty(Item);
    MarkArrayDirty();
    NotifyOwner(EInventoryDelta::Updated, ArrayIndex);
}

int32 FMP_InventoryArray::RemoveItem(int32 ArrayIndex, int32 Quantity)
{
    if (!Items.IsValidIndex(ArrayIndex)) return INDEX_NONE;

    FMP_InventoryItem& Item = Items[ArrayIndex];

    if (Quantity <= 0 || Quantity > Item.Quantity)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("FMP_InventoryArray::RemoveItem - Invalid quantity %d for slot %d (has %d)."),
            Quantity, Item.SlotIndex, Item.Quantity);
        return INDEX_NONE;
    }

    Item.Quantity -= Quantity;

    if (Item.Quantity > 0)
    {
        MarkItemDirty(Item);
        NotifyOwner(EInventoryDelta::Updated, ArrayIndex);
        MarkArrayDirty();
        return INDEX_NONE;
    }

    const int32 LastIndex = Items.Num() - 1;
    const bool bSwapWillHappen = (ArrayIndex != LastIndex);
    const int32 FreedSlot = Item.SlotIndex;

    // Fire BEFORE removal so Items[ArrayIndex] is still valid.
    // For Removed, we pass the SlotIndex.
    // For Remove at Swap always last element will be removed and delete index will update wit last index.
    NotifyOwner(EInventoryDelta::Removed, Items[LastIndex].SlotIndex);

    Items.RemoveAtSwap(ArrayIndex);

    // If RemoveAtSwap moved another item in, notify UI to update its stored ArrayIndex
    if (bSwapWillHappen)
    {
        Items[ArrayIndex].SlotIndex = FreedSlot;
        MarkItemDirty(Items[ArrayIndex]);
        NotifyOwner(EInventoryDelta::Updated, ArrayIndex);
    }

    MarkArrayDirty();
    return FreedSlot;
}

void FMP_InventoryArray::UpdateItem(int32 ArrayIndex, const FMP_InventoryItem& NewItem)
{
    if (!Items.IsValidIndex(ArrayIndex)) return;

    Items[ArrayIndex] = NewItem;
    MarkItemDirty(Items[ArrayIndex]);
    MarkArrayDirty();
    NotifyOwner(EInventoryDelta::Updated, ArrayIndex);
}

void FMP_InventoryArray::SwapItems(int32 ArrayIndexA, int32 ArrayIndexB)
{
    if (!Items.IsValidIndex(ArrayIndexA) ||
        !Items.IsValidIndex(ArrayIndexB) ||
        ArrayIndexA == ArrayIndexB) return;

    Swap(Items[ArrayIndexA].SlotIndex, Items[ArrayIndexB].SlotIndex);

    MarkItemDirty(Items[ArrayIndexA]);
    MarkItemDirty(Items[ArrayIndexB]);
    MarkArrayDirty();

    NotifyOwner(EInventoryDelta::Updated, ArrayIndexA);
    NotifyOwner(EInventoryDelta::Updated, ArrayIndexB);
}


// =============================================================================
//  REPLICATION CALLBACKS
// =============================================================================

bool FMP_InventoryArray::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
    return FFastArraySerializer::FastArrayDeltaSerialize<FMP_InventoryItem, FMP_InventoryArray>(
        Items, DeltaParms, *this);
}

void FMP_InventoryArray::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
    for (int32 Index : RemovedIndices)
    {
        if (Items.IsValidIndex(Index))
        {
            // For Removed, we pass the SlotIndex so UI knows which tile to clear
            NotifyOwner(EInventoryDelta::Removed, Items[Index].SlotIndex);
        }
    }
}

void FMP_InventoryArray::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
    for (int32 Index : AddedIndices)
    {
        if (Items.IsValidIndex(Index))
        {
            NotifyOwner(EInventoryDelta::Added, Index);
        }
    }
}

void FMP_InventoryArray::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
    for (int32 Index : ChangedIndices)
    {
        if (Items.IsValidIndex(Index))
        {
            NotifyOwner(EInventoryDelta::Updated, Index);
        }
    }
}

void FMP_InventoryArray::PostReplicatedReceive(
    const FFastArraySerializer::FPostReplicatedReceiveParameters& Parameters)
{
    if (UMP_InventoryComponent* InvComp = Cast<UMP_InventoryComponent>(Owner.Get()))
    {
        InvComp->FireInventoryUpdate(EInventoryDelta::Refresh, -1);
    }
}