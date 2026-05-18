// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MP_InventoryStruct.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "MP_InventoryFastArray.generated.h"

// FastArray for inventory replication and modification
USTRUCT()
struct FMP_InventoryArray : public FFastArraySerializer
{
    GENERATED_BODY()

    // Main inventory array (always last)
    UPROPERTY()
    TArray<FMP_InventoryItem> Items;

    // -- Inventory Modification Utility Functions --

    // Add by struct, quantity: combines or adds new, marks dirty
    void AddItem(const FMP_InventoryItem& NewItem);

    // Remove by index, quantity: decreases or removes, marks dirty
    void RemoveItem(int32 Index, int32 Quantity);

    // Update item at index, marks dirty
    void UpdateItem(int32 Index, const FMP_InventoryItem& NewItem);

    // Swap two items by index, marks both dirty
    void SwapItems(int32 IndexA, int32 IndexB);

    // -- FastArray Replication Support --

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

    void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
    void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
    void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);

    // Notify the owner/component for changes, implemented by inventory component
    void NotifyOwner(EInventoryDelta Delta, int32 SlotIndex);

    // Link back to owner (usually set by inventory component)
    void SetOwner(UObject* InOwner) { Owner = InOwner; }
    UObject* GetOwner() const { return Owner.Get(); }

private:
    TWeakObjectPtr<UObject> Owner;
};

template<>
struct TStructOpsTypeTraits<FMP_InventoryArray> : public TStructOpsTypeTraitsBase2<FMP_InventoryArray>
{
    enum
    {
        WithNetDeltaSerializer = true,
    };
};
