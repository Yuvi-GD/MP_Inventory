// Copyright 2026 UVSquare. All Rights Reserved.


#include "Framework/MP_InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Containers/Array.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/MP_InventorySave.h"
#include "GameFramework/PlayerController.h"

// =============================================================================
//  LIFECYCLE
// =============================================================================

UMP_InventoryComponent::UMP_InventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
    InventoryItems.SetOwner(this);
}

void UMP_InventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    InventoryItems.SetOwner(this);
}

void UMP_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMP_InventoryComponent, InventoryItems);
    DOREPLIFETIME(UMP_InventoryComponent, OwnerID);
    DOREPLIFETIME(UMP_InventoryComponent, CurrentWeight);
    DOREPLIFETIME(UMP_InventoryComponent, UsedSlots);
    DOREPLIFETIME(UMP_InventoryComponent, bInventoryLocked);
}


// =============================================================================
//  INTERNAL HELPERS
// =============================================================================

UMP_ItemRegistry* UMP_InventoryComponent::GetRegistry() const
{
    const UWorld* World = GetWorld();
    if (!World) return nullptr;
    const UGameInstance* GI = World->GetGameInstance();
    if (!GI) return nullptr;
    return GI->GetSubsystem<UMP_ItemRegistry>();
}

int32 UMP_InventoryComponent::GetArrayIndexFromSlot(int32 SlotIndex) const
{
    if (!bUseStrictSlots) return SlotIndex; // Infinite mode: direct match
    
    // Strict mode: O(n) scan
    for (int32 i = 0; i < InventoryItems.Items.Num(); ++i)
    {
        if (InventoryItems.Items[i].SlotIndex == SlotIndex)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

void UMP_InventoryComponent::FireInventoryUpdate(EInventoryDelta Delta, int32 ArrayIndex)
{
    OnInventoryUpdated.Broadcast(Delta, ArrayIndex);
}

void UMP_InventoryComponent::CompactSlots_Implementation()
{
    if (!GetOwner()->HasAuthority() || !bUseStrictSlots) return;

    int32 NewSlot = 0;
    for (int32 i = 0; i < InventoryItems.Items.Num(); ++i)
    {
        FMP_InventoryItem UpdatedItem = InventoryItems.Items[i];
        UpdatedItem.SlotIndex = NewSlot;
        InventoryItems.UpdateItem(i, UpdatedItem);
        NewSlot++;
    }

    FireInventoryUpdate(EInventoryDelta::Refresh, -1);
}


// =============================================================================
//  PERSISTENCE
// =============================================================================

void UMP_InventoryComponent::SaveInventory(const FString& PlayerID)
{
    UMP_InventorySave* SaveGame = Cast<UMP_InventorySave>(
        UGameplayStatics::CreateSaveGameObject(UMP_InventorySave::StaticClass()));

    if (!SaveGame)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_InventoryComponent::SaveInventory - Failed to create save object."));
        return;
    }

    SaveGame->SavedInventory.Add(FName(*PlayerID), InventoryItems);
    UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, 0);
}

void UMP_InventoryComponent::LoadInventory(const FString& PlayerID)
{
    if (!GetOwner()->HasAuthority()) return;

    if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
    {
        InventoryItems.Items.Empty();
        return;
    }

    UMP_InventorySave* SaveGame = Cast<UMP_InventorySave>(
        UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));

    if (!SaveGame) return;

    const FName PlayerKey(*PlayerID);
    InventoryItems = SaveGame->SavedInventory.Contains(PlayerKey)
        ? SaveGame->SavedInventory[PlayerKey]
        : FMP_InventoryArray();

    InventoryItems.SetOwner(this);
    InventoryItems.MarkArrayDirty();

    UsedSlots = InventoryItems.Items.Num();

    CurrentWeight = 0.0f;
    if (bEnforceWeightLimit)
    {
        if (UMP_ItemRegistry* Registry = GetRegistry())
        {
            for (const FMP_InventoryItem& Item : InventoryItems.Items)
            {
                if (const UMP_ItemDefinition* Def = Registry->GetItemDefinitionByName(Item.ItemID))
                    CurrentWeight += Def->PerItemWeight * Item.Quantity;
            }
        }
    }

    FireInventoryUpdate(EInventoryDelta::Refresh, -1);
}


// =============================================================================
//  QUERIES
// =============================================================================

FMP_InventoryItem UMP_InventoryComponent::GetItemBySlotIndex(int32 SlotIndex) const
{
    const int32 ArrayIndex = GetArrayIndexFromSlot(SlotIndex);
    return (ArrayIndex != INDEX_NONE) ? InventoryItems.Items[ArrayIndex] : FMP_InventoryItem();
}

FMP_InventoryItem UMP_InventoryComponent::GetItemByArrayIndex(int32 ArrayIndex) const
{
    return InventoryItems.Items.IsValidIndex(ArrayIndex) ? InventoryItems.Items[ArrayIndex] : FMP_InventoryItem();
}

FMP_InventoryItem UMP_InventoryComponent::GetItemByID(FName ItemID) const
{
    for (const FMP_InventoryItem& Item : InventoryItems.Items)
    {
        if (Item.ItemID == ItemID) return Item;
    }
    return FMP_InventoryItem();
}

TArray<int32> UMP_InventoryComponent::GetAllSlotsForItem(FName ItemID) const
{
    TArray<int32> Slots;
    for (const FMP_InventoryItem& Item : InventoryItems.Items)
    {
        if (Item.ItemID == ItemID) Slots.Add(Item.SlotIndex);
    }
    return Slots;
}

bool UMP_InventoryComponent::IsSlotOccupied(int32 SlotIndex) const
{
    return GetArrayIndexFromSlot(SlotIndex) != INDEX_NONE;
}

bool UMP_InventoryComponent::IsSlotLocked(int32 SlotIndex) const
{
    const int32 ArrayIndex = GetArrayIndexFromSlot(SlotIndex);
    return (ArrayIndex != INDEX_NONE) && InventoryItems.Items[ArrayIndex].bIsLocked;
}

int32 UMP_InventoryComponent::GetRemainingSlots() const
{
    if (!bUseStrictSlots) return TNumericLimits<int32>::Max();
    return FMath::Max(0, MaxInventorySlots - InventoryItems.Items.Num());
}

float UMP_InventoryComponent::GetRemainingWeight() const
{
    if (!bEnforceWeightLimit) return TNumericLimits<float>::Max();
    return FMath::Max(0.0f, MaxWeightCapacity - CurrentWeight);
}

bool UMP_InventoryComponent::IsInventoryFull() const
{
    bool bSlotsFull = bUseStrictSlots ? (InventoryItems.Items.Num() >= MaxInventorySlots) : false;
    bool bWeightFull = bEnforceWeightLimit ? (CurrentWeight >= MaxWeightCapacity) : false;
    return bSlotsFull || bWeightFull;
}


// =============================================================================
//  SERVER COMMANDS
// =============================================================================

void UMP_InventoryComponent::AddItem_Implementation(FName ItemID, int32 Quantity, bool bPreferNewSlot)
{
if (!GetOwner()->HasAuthority() || bInventoryLocked || ItemID.IsNone() || Quantity <= 0) return;
    UMP_ItemRegistry* Reg = GetRegistry();
    if (!Reg) return;
    const UMP_ItemDefinition* Def = Reg->GetItemDefinitionByName(ItemID);
    if (!Def) return;
    if (bEnforceWeightLimit && (CurrentWeight + (Def->PerItemWeight * Quantity) > MaxWeightCapacity)) return;
    const int32 StackLimit = Def->MaxStackSize;
    int32 TotalMergeCapacity = 0;
    int32 UsedSlotCount = 0;
    
    TArray<bool> OccupiedSlots;
    if (bUseStrictSlots) OccupiedSlots.Init(false, MaxInventorySlots);
    for (const FMP_InventoryItem& Item : InventoryItems.Items)
    {
        if (bUseStrictSlots && Item.SlotIndex >= 0 && Item.SlotIndex < MaxInventorySlots)
        {
            OccupiedSlots[Item.SlotIndex] = true;
            UsedSlotCount++;
        }
        
        if (Item.ItemID == ItemID && !Item.bIsLocked)
        {
            TotalMergeCapacity += FMath::Max(0, StackLimit - Item.Quantity);
        }
    }
    int32 Remaining = Quantity;
    int32 QuantityForNewSlots = 0;
    int32 QuantityForMerge = 0;
    int32 AvailableSlots = bUseStrictSlots ? (MaxInventorySlots - UsedSlotCount) : TNumericLimits<int32>::Max();
    if (bPreferNewSlot)
    {
        int32 MaxCapacityInNewSlots = bUseStrictSlots ? (AvailableSlots * StackLimit) : TNumericLimits<int32>::Max();
        QuantityForNewSlots = FMath::Min(Remaining, MaxCapacityInNewSlots);
        QuantityForMerge = Remaining - QuantityForNewSlots;
        
        if (QuantityForMerge > TotalMergeCapacity) return; 
    }
    else
    {
        QuantityForMerge = FMath::Min(Remaining, TotalMergeCapacity);
        QuantityForNewSlots = Remaining - QuantityForMerge;
        
        int32 SlotsNeeded = FMath::DivideAndRoundUp(QuantityForNewSlots, StackLimit);
        if (bUseStrictSlots && SlotsNeeded > AvailableSlots) return; 
    }
    auto DoMerge = [&]()
    {
        for (int32 i = 0; i < InventoryItems.Items.Num() && QuantityForMerge > 0; ++i)
        {
            FMP_InventoryItem& Item = InventoryItems.Items[i];
            if (Item.ItemID == ItemID && !Item.bIsLocked)
            {
                int32 Room = StackLimit - Item.Quantity;
                if (Room > 0)
                {
                    int32 ToAdd = FMath::Min(QuantityForMerge, Room);
                    InventoryItems.AddQuantity(i, ToAdd);
                    CurrentWeight += Def->PerItemWeight * ToAdd;
                    QuantityForMerge -= ToAdd;
                }
            }
        }
    };
    int32 LastSearchedSlot = 0;
    auto DoNewSlots = [&]()
    {
        while (QuantityForNewSlots > 0)
        {
            int32 ToAdd = FMath::Min(QuantityForNewSlots, StackLimit);
            int32 SlotIdx = INDEX_NONE;
            
            if (bUseStrictSlots)
            {
                for (; LastSearchedSlot < MaxInventorySlots; ++LastSearchedSlot)
                {
                    if (!OccupiedSlots[LastSearchedSlot])
                    {
                        SlotIdx = LastSearchedSlot;
                        OccupiedSlots[LastSearchedSlot] = true;
                        break;
                    }
                }
            }
            else
            {
                SlotIdx = InventoryItems.Items.Num();
            }
            
            if (SlotIdx == INDEX_NONE) break; // Safety fallback
            FMP_InventoryItem NewItem;
            NewItem.ItemID = ItemID;
            NewItem.Quantity = ToAdd;
            NewItem.SlotIndex = SlotIdx;
            
            InventoryItems.AddItem(NewItem);
            CurrentWeight += Def->PerItemWeight * ToAdd;
            UsedSlots++;
            
            QuantityForNewSlots -= ToAdd;
        }
    };
    if (bPreferNewSlot)
    {
        DoNewSlots();
        DoMerge();
    }
    else
    {
        DoMerge();
        DoNewSlots();
    }
}

void UMP_InventoryComponent::RemoveItem_Implementation(int32 ArrayIndex, int32 Quantity)
{
    if (!GetOwner()->HasAuthority() || bInventoryLocked) return;
    if (ArrayIndex == INDEX_NONE) return;

    FMP_InventoryItem& Item = InventoryItems.Items[ArrayIndex];
    if (Item.bIsLocked || Quantity <= 0 || Quantity > Item.Quantity) return;

    const bool bFullRemoval = (Item.Quantity == Quantity);
    float WeightPerUnit = 0.0f;

    if (bEnforceWeightLimit)
    {
        if (UMP_ItemRegistry* Reg = GetRegistry())
        {
            if (const UMP_ItemDefinition* Def = Reg->GetItemDefinitionByName(Item.ItemID))
                WeightPerUnit = Def->PerItemWeight;
        }
    }

    CurrentWeight = FMath::Max(0.0f, CurrentWeight - WeightPerUnit * Quantity);
    InventoryItems.RemoveItem(ArrayIndex, Quantity);
}

void UMP_InventoryComponent::RemoveAllItems_Implementation(FName ItemID)
{
    if (!GetOwner()->HasAuthority() || bInventoryLocked || ItemID.IsNone()) return;

    // Must iterate backwards when removing multiple items to avoid index shifting bugs
    for (int32 i = InventoryItems.Items.Num() - 1; i >= 0; --i)
    {
        if (InventoryItems.Items[i].ItemID == ItemID && !InventoryItems.Items[i].bIsLocked)
        {
            RemoveItem_Implementation(i, InventoryItems.Items[i].Quantity);
        }
    }
}

void UMP_InventoryComponent::RemoveItemByID_Implementation(FName ItemID, int32 Quantity)
{
    if (!GetOwner()->HasAuthority() || bInventoryLocked || ItemID.IsNone() || Quantity <= 0) return;

    int32 Remaining = Quantity;
    for (int32 i = InventoryItems.Items.Num() - 1; i >= 0 && Remaining > 0; --i)
    {
        if (InventoryItems.Items[i].ItemID == ItemID && !InventoryItems.Items[i].bIsLocked)
        {
            const int32 ToRemove = FMath::Min(Remaining, InventoryItems.Items[i].Quantity);
            RemoveItem_Implementation(i, ToRemove);
            Remaining -= ToRemove;
        }
    }
}

void UMP_InventoryComponent::UpdateItemAtSlot_Implementation(int32 SlotIndex, FMP_InventoryItem NewItem)
{
    if (!GetOwner()->HasAuthority() || bInventoryLocked || NewItem.ItemID.IsNone() || NewItem.Quantity <= 0) return;

    const int32 ArrayIndex = GetArrayIndexFromSlot(SlotIndex);
    if (ArrayIndex == INDEX_NONE) return;

    FMP_InventoryItem& OldItem = InventoryItems.Items[ArrayIndex];
    if (OldItem.bIsLocked) return;

    if (bEnforceWeightLimit)
    {
        if (UMP_ItemRegistry* Registry = GetRegistry())
        {
            if (const UMP_ItemDefinition* OldDef = Registry->GetItemDefinitionByName(OldItem.ItemID))
                CurrentWeight -= OldDef->PerItemWeight * OldItem.Quantity;
            if (const UMP_ItemDefinition* NewDef = Registry->GetItemDefinitionByName(NewItem.ItemID))
                CurrentWeight += NewDef->PerItemWeight * NewItem.Quantity;

            CurrentWeight = FMath::Max(0.0f, CurrentWeight);
        }
    }

    NewItem.SlotIndex = SlotIndex;
    InventoryItems.UpdateItem(ArrayIndex, NewItem);
}

void UMP_InventoryComponent::SwapItems_Implementation(int32 SlotIndexA, int32 SlotIndexB)
{
    if (!GetOwner()->HasAuthority() || bInventoryLocked || SlotIndexA == SlotIndexB) return;

    const int32 ArrayIndexA = GetArrayIndexFromSlot(SlotIndexA);
    const int32 ArrayIndexB = GetArrayIndexFromSlot(SlotIndexB);

    const bool bAOccupied = (ArrayIndexA != INDEX_NONE);
    const bool bBOccupied = (ArrayIndexB != INDEX_NONE);

    if (!bAOccupied && !bBOccupied) return;

    if (bAOccupied && InventoryItems.Items[ArrayIndexA].bIsLocked) return;
    if (bBOccupied && InventoryItems.Items[ArrayIndexB].bIsLocked) return;

    if (bAOccupied && bBOccupied)
    {
        InventoryItems.SwapItems(ArrayIndexA, ArrayIndexB);
    }
    else
    {
        const int32 OccupiedArrayIdx = bAOccupied ? ArrayIndexA : ArrayIndexB;
        const int32 NewSlotIdx = bAOccupied ? SlotIndexB : SlotIndexA;

        FMP_InventoryItem MovedItem = InventoryItems.Items[OccupiedArrayIdx];
        MovedItem.SlotIndex = NewSlotIdx;
        InventoryItems.UpdateItem(OccupiedArrayIdx, MovedItem);
    }
}

void UMP_InventoryComponent::SplitItem_Implementation(int32 SourceSlotIndex, int32 TargetSlotIndex, int32 QuantityToSplit)
{
    if (!GetOwner()->HasAuthority() || bInventoryLocked || QuantityToSplit <= 0) return;

    const int32 SourceArrayIndex = GetArrayIndexFromSlot(SourceSlotIndex);
    if (SourceArrayIndex == INDEX_NONE) return;

    const FMP_InventoryItem& SourceItem = InventoryItems.Items[SourceArrayIndex];
    if (SourceItem.bIsLocked || SourceItem.Quantity <= QuantityToSplit) return;

    const FName SplitItemID = SourceItem.ItemID;
    const bool bAutoTarget = (TargetSlotIndex == -1);
    
    // Quick empty-slot validation
    if (bAutoTarget && bUseStrictSlots && InventoryItems.Items.Num() >= MaxInventorySlots) return;

    const int32 TargetArrayIndex = bAutoTarget ? INDEX_NONE : GetArrayIndexFromSlot(TargetSlotIndex);
    const bool bTargetOccupied = (TargetArrayIndex != INDEX_NONE);

    if (!bAutoTarget && bTargetOccupied)
    {
        const FMP_InventoryItem& TargetItem = InventoryItems.Items[TargetArrayIndex];
        if (TargetItem.bIsLocked || TargetItem.ItemID != SplitItemID) return;

        if (const UMP_ItemRegistry* Registry = GetRegistry())
        {
            if (const UMP_ItemDefinition* ItemDef = Registry->GetItemDefinitionByName(SplitItemID))
            {
                if (ItemDef->MaxStackSize > 0 && (ItemDef->MaxStackSize - TargetItem.Quantity) < QuantityToSplit) return;
            }
        }
    }
    else if (!bAutoTarget && bUseStrictSlots)
    {
        if (IsSlotOccupied(TargetSlotIndex)) return; // Specified target is invalid or not free
    }

    // Commit
    InventoryItems.RemoveItem(SourceArrayIndex, QuantityToSplit);

    if (!bAutoTarget && bTargetOccupied)
    {
        InventoryItems.AddQuantity(TargetArrayIndex, QuantityToSplit);
    }
    else
    {
        int32 NewSlotIndex = TargetSlotIndex;
        if (bAutoTarget)
        {
            if (!bUseStrictSlots)
            {
                NewSlotIndex = InventoryItems.Items.Num();
            }
            else
            {
                TArray<bool> Occupied;
                Occupied.Init(false, MaxInventorySlots);
                for (const FMP_InventoryItem& Item : InventoryItems.Items)
                {
                    if (Item.SlotIndex >= 0 && Item.SlotIndex < MaxInventorySlots)
                        Occupied[Item.SlotIndex] = true;
                }
                for (int32 i = 0; i < MaxInventorySlots; ++i)
                {
                    if (!Occupied[i])
                    {
                        NewSlotIndex = i;
                        break;
                    }
                }
            }
        }
        
        if (NewSlotIndex == INDEX_NONE) return;

        FMP_InventoryItem SplitItem;
        SplitItem.ItemID = SplitItemID;
        SplitItem.Quantity = QuantityToSplit;
        SplitItem.SlotIndex = NewSlotIndex;

        InventoryItems.AddItem(SplitItem);
        UsedSlots++;
    }
}

void UMP_InventoryComponent::SetItemLock_Implementation(int32 SlotIndex, bool bLocked)
{
    if (!GetOwner()->HasAuthority() || bInventoryLocked) return;

    const int32 ArrayIndex = GetArrayIndexFromSlot(SlotIndex);
    if (ArrayIndex == INDEX_NONE) return;

    FMP_InventoryItem UpdatedItem = InventoryItems.Items[ArrayIndex];
    UpdatedItem.bIsLocked = bLocked;
    InventoryItems.UpdateItem(ArrayIndex, UpdatedItem);
}
