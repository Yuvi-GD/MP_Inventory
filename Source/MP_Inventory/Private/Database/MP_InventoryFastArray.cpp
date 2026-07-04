// Copyright 2026 UVSquare. All Rights Reserved.


#include "Database/MP_InventoryFastArray.h"
#include "Database/MP_InventoryStruct.h"
#include "Framework/MP_InventoryComponent.h"

// =============================================================================
//  TRACKER MANAGEMENT
// =============================================================================

void FMP_InventoryArray::ResizeTracker(int32 NewSize)
{
    int32 OldNum = IndexTracker.Num();
    IndexTracker.SetNum(NewSize);
    for (int32 i = OldNum; i < NewSize; ++i)
    {
        IndexTracker[i] = INDEX_NONE;
    }
}

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
    int32 Index = Items.Num() - 1;

    int32 NewSlot = NewItem.SlotIndex;
    if (NewSlot >= IndexTracker.Num())
    {
        ResizeTracker(NewSlot + 1);
    }
    IndexTracker[NewSlot] = Index;

    MarkItemDirty(Items.Last());
    MarkArrayDirty();
    NotifyOwner(EInventoryDelta::Added, Index);
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
    const int32 MovedSlot = Items[LastIndex].SlotIndex; // The slot of the item that will be moved to fill the gap

    // Clear tracker for the slot that is being freed
    if (IndexTracker.IsValidIndex(FreedSlot) && IndexTracker[FreedSlot] == ArrayIndex)
    {
        IndexTracker[FreedSlot] = INDEX_NONE;
    }

    // Fire Removed for the slot we are actually deleting BEFORE it's gone
    NotifyOwner(EInventoryDelta::Removed, FreedSlot);

    Items.RemoveAtSwap(ArrayIndex);

    // If RemoveAtSwap moved another item in, update its tracker to point to the new array index
    if (bSwapWillHappen)
    {
        // We DO NOT change Items[ArrayIndex].SlotIndex here. It physically moved in the array, but logically stays in its slot!
        if (IndexTracker.IsValidIndex(MovedSlot))
        {
            IndexTracker[MovedSlot] = ArrayIndex;
        }

        MarkItemDirty(Items[ArrayIndex]);
        NotifyOwner(EInventoryDelta::Updated, ArrayIndex);
    }

    MarkArrayDirty();
    return FreedSlot;
}

void FMP_InventoryArray::UpdateItem(int32 ArrayIndex, const FMP_InventoryItem& NewItem)
{
    if (!Items.IsValidIndex(ArrayIndex)) return;

    // Enforce that UpdateItem CANNOT change the slot index.
    FMP_InventoryItem UpdatedItem = NewItem;
    UpdatedItem.SlotIndex = Items[ArrayIndex].SlotIndex;

    Items[ArrayIndex] = UpdatedItem;

    MarkItemDirty(Items[ArrayIndex]);
    MarkArrayDirty();
    NotifyOwner(EInventoryDelta::Updated, ArrayIndex);
}

void FMP_InventoryArray::SwapItemsBySlotIndex(int32 SlotA, int32 SlotB)
{
    if (SlotA == SlotB) return;

    int32 ArrayIndexA = IndexTracker.IsValidIndex(SlotA) ? IndexTracker[SlotA] : INDEX_NONE;
    int32 ArrayIndexB = IndexTracker.IsValidIndex(SlotB) ? IndexTracker[SlotB] : INDEX_NONE;

    bool bAOccupied = Items.IsValidIndex(ArrayIndexA);
    bool bBOccupied = Items.IsValidIndex(ArrayIndexB);

    if (!bAOccupied && !bBOccupied) return;

    int32 MaxSlot = FMath::Max(SlotA, SlotB);
    if (MaxSlot >= IndexTracker.Num())
    {
        ResizeTracker(MaxSlot + 1);
    }

    if (bAOccupied && bBOccupied)
    {
        Swap(Items[ArrayIndexA].SlotIndex, Items[ArrayIndexB].SlotIndex);
        IndexTracker[SlotA] = ArrayIndexB;
        IndexTracker[SlotB] = ArrayIndexA;
        MarkItemDirty(Items[ArrayIndexA]);
        MarkItemDirty(Items[ArrayIndexB]);
        NotifyOwner(EInventoryDelta::Updated, ArrayIndexA);
        NotifyOwner(EInventoryDelta::Updated, ArrayIndexB);
    }
    else if (bAOccupied && !bBOccupied)
    {
        Items[ArrayIndexA].SlotIndex = SlotB;
        IndexTracker[SlotB] = ArrayIndexA;
        IndexTracker[SlotA] = INDEX_NONE;
        MarkItemDirty(Items[ArrayIndexA]);
        NotifyOwner(EInventoryDelta::Removed, SlotA);
        NotifyOwner(EInventoryDelta::Updated, ArrayIndexA);
    }
    else if (!bAOccupied && bBOccupied)
    {
        Items[ArrayIndexB].SlotIndex = SlotA;
        IndexTracker[SlotA] = ArrayIndexB;
        IndexTracker[SlotB] = INDEX_NONE;
        MarkItemDirty(Items[ArrayIndexB]);
        NotifyOwner(EInventoryDelta::Removed, SlotB);
        NotifyOwner(EInventoryDelta::Updated, ArrayIndexB);
    }

    MarkArrayDirty();
}

void FMP_InventoryArray::SwapItemsByArrayIndex(int32 ArrayIndexA, int32 ArrayIndexB)
{
    if (!Items.IsValidIndex(ArrayIndexA) || !Items.IsValidIndex(ArrayIndexB) || ArrayIndexA == ArrayIndexB) return;

    int32 SlotA = Items[ArrayIndexA].SlotIndex;
    int32 SlotB = Items[ArrayIndexB].SlotIndex;

    Items.Swap(ArrayIndexA, ArrayIndexB);

    if (IndexTracker.IsValidIndex(SlotA)) IndexTracker[SlotA] = ArrayIndexB;
    if (IndexTracker.IsValidIndex(SlotB)) IndexTracker[SlotB] = ArrayIndexA;

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
            int32 OldSlot = Items[Index].SlotIndex;
            
            // Clear from tracker
            if (IndexTracker.IsValidIndex(OldSlot) && IndexTracker[OldSlot] == Index)
            {
                IndexTracker[OldSlot] = INDEX_NONE;
            }

            // For Removed, we pass the SlotIndex so UI knows which tile to clear
            NotifyOwner(EInventoryDelta::Removed, OldSlot);
        }
    }
}

void FMP_InventoryArray::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
    for (int32 Index : AddedIndices)
    {
        if (Items.IsValidIndex(Index))
        {
            int32 NewSlot = Items[Index].SlotIndex;
            
            // Update tracker safely
            if (NewSlot >= IndexTracker.Num())
            {
                ResizeTracker(NewSlot + 1);
            }
            IndexTracker[NewSlot] = Index;

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
            int32 NewSlot = Items[Index].SlotIndex;
            
            // OPTIMIZATION: If the item is already mapped to this slot, it hasn't moved!
            // This means only its data (like Quantity or ItemID) changed. Skip the old-slot search.
            if (IndexTracker.IsValidIndex(NewSlot) && IndexTracker[NewSlot] == Index)
            {
                NotifyOwner(EInventoryDelta::Updated, Index);
                continue;
            }

            int32 OldSlot = INDEX_NONE;

            // 1. Find the old slot using the tracker since it moved
            for (int32 i = 0; i < IndexTracker.Num(); ++i)
            {
                if (IndexTracker[i] == Index)
                {
                    OldSlot = i;
                    IndexTracker[i] = INDEX_NONE;
                    break;
                }
            }

            // 2. Clear the UI ghost if the slot changed
            if (OldSlot != INDEX_NONE && OldSlot != NewSlot)
            {
                NotifyOwner(EInventoryDelta::Removed, OldSlot);
            }

            // 3. Update the tracker with the new slot
            if (NewSlot >= IndexTracker.Num())
            {
                ResizeTracker(NewSlot + 1);
            }
            IndexTracker[NewSlot] = Index;

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