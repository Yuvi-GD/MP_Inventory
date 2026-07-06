// Copyright 2026 UVSquare. All Rights Reserved.


#include "Core/MP_InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Containers/Array.h"
#include "Kismet/GameplayStatics.h"
#include "Core/MP_InventorySave.h"
#include "GameFramework/PlayerController.h"
#include "Core/MP_ItemRegistry.h"
#include "Core/MP_InventoryManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"

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

    if (GetOwner()->HasAuthority())
    {
        if (ComponentID.IsNone())
        {
            ComponentID = FName(*FGuid::NewGuid().ToString());
        }

        if (OwnerID.IsNone())
        {
            TryAutoAssignOwner();
        }

        if (UMP_ItemRegistry* Registry = GetRegistry())
        {
            Registry->RegisterInventory(ComponentID, this);
        }
    }
}

void UMP_InventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UMP_ItemRegistry* Registry = GetRegistry())
    {
        Registry->UnregisterInventory(ComponentID);
    }
    Super::EndPlay(EndPlayReason);
}

void UMP_InventoryComponent::OnRep_ComponentID()
{
    if (UMP_ItemRegistry* Registry = GetRegistry())
    {
        Registry->RegisterInventory(ComponentID, this);
    }
}

void UMP_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UMP_InventoryComponent, InventoryItems);
    DOREPLIFETIME(UMP_InventoryComponent, ComponentID);
    DOREPLIFETIME(UMP_InventoryComponent, OwnerID);
    DOREPLIFETIME(UMP_InventoryComponent, CurrentWeight);
    DOREPLIFETIME(UMP_InventoryComponent, MaxInventorySlots);
    DOREPLIFETIME(UMP_InventoryComponent, bInventoryLocked);
}

void UMP_InventoryComponent::SetInventoryOwner(FName NewOwnerID)
{
    if (GetOwner()->HasAuthority())
    {
        OwnerID = NewOwnerID;
    }
}

void UMP_InventoryComponent::GrantAccess(UMP_InventoryManager* Manager)
{
    if (GetOwner()->HasAuthority() && Manager)
    {
        AccessList.Add(Manager->ManagerID);
        Manager->OnInventoryActionNotify.Broadcast(FName(*FString::Printf(TEXT("GrantedAccess_%s"), *ComponentName.ToString())));
    }
}

void UMP_InventoryComponent::RevokeAccess(UMP_InventoryManager* Manager)
{
    if (GetOwner()->HasAuthority() && Manager)
    {
        AccessList.Remove(Manager->ManagerID);
        Manager->OnInventoryActionNotify.Broadcast(FName(*FString::Printf(TEXT("RevokedAccess_%s"), *ComponentName.ToString())));
    }
}

bool UMP_InventoryComponent::TryAutoAssignOwner()
{
    if (!GetOwner()->HasAuthority()) return false;

    APlayerController* PC = nullptr;
    PC = Cast<APlayerController>(GetOwner());

    if (!PC)
    {
        if (APawn* OwningPawn = Cast<APawn>(GetOwner()))
        {
            PC = Cast<APlayerController>(OwningPawn->GetController());
        }
        else if (APlayerState* OwningPS = Cast<APlayerState>(GetOwner()))
        {
            PC = OwningPS->GetPlayerController();
        }
    }
    if (PC)
    {
        if (UActorComponent* Comp = PC->GetComponentByClass(UMP_InventoryManager::StaticClass()))
        {
            if (UMP_InventoryManager* Manager = Cast<UMP_InventoryManager>(Comp))
            {
                OwnerID = Manager->ManagerID;
                return true;
            }
        }
    }
    
    return false;
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
    if (!bUseStrictSlots) return SlotIndex; 
    
    if (InventoryItems.IndexTracker.IsValidIndex(SlotIndex))
    {
        return InventoryItems.IndexTracker[SlotIndex];
    }

    return INDEX_NONE;
}

void UMP_InventoryComponent::FireInventoryUpdate(EInventoryDelta Delta, int32 ArrayIndex)
{
    OnInventoryUpdated.Broadcast(Delta, ArrayIndex);
}

void UMP_InventoryComponent::CompactSlots()
{
    if (!GetOwner()->HasAuthority() || !bUseStrictSlots) return;

    TArray<int32> OldTracker = InventoryItems.IndexTracker;
    InventoryItems.IndexTracker.Init(INDEX_NONE, MaxInventorySlots);

    int32 NewSlot = 0;
    
    for (int32 OldSlot = 0; OldSlot < OldTracker.Num(); ++OldSlot)
    {
        int32 ArrayIndex = OldTracker[OldSlot];
        if (ArrayIndex != INDEX_NONE)
        {
            if (NewSlot >= MaxInventorySlots) break; 

            if (OldSlot != NewSlot)
            {
                InventoryItems.SetItemSlot(ArrayIndex, NewSlot);
            }
            
            InventoryItems.IndexTracker[NewSlot] = ArrayIndex;
            NewSlot++;
        }
    }
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
    return (ArrayIndex != INDEX_NONE) ? GetItemByArrayIndex(ArrayIndex) : FMP_InventoryItem();
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

void UMP_InventoryComponent::AddItem(FName ItemID, int32 Quantity, bool bPreferNewSlot)
{
    if (!GetOwner()->HasAuthority() || bInventoryLocked || ItemID.IsNone() || Quantity <= 0) return;

    UMP_ItemRegistry* Reg = GetRegistry();
    if (!Reg) return;
    const UMP_ItemDefinition* Def = Reg->GetItemDefinitionByName(ItemID);
    if (!Def) return;
    if (bEnforceWeightLimit && (CurrentWeight + (Def->PerItemWeight * Quantity) > MaxWeightCapacity)) return;

    const int32 StackLimit = Def->MaxStackSize;
    int32 UsedSlotCount = InventoryItems.Items.Num();
    int32 TotalMergeCapacity = 0;
    
    for (const FMP_InventoryItem& Item : InventoryItems.Items)
    {
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
    auto DoNewSlots = [&]()
    {
        if (bUseStrictSlots)
        {
            for (int32 i = 0; i < MaxInventorySlots && QuantityForNewSlots > 0; ++i)
            {
                if (!InventoryItems.IndexTracker.IsValidIndex(i) || InventoryItems.IndexTracker[i] == INDEX_NONE)
                {
                    int32 ToAdd = FMath::Min(QuantityForNewSlots, StackLimit);
                    
                    FMP_InventoryItem NewItem;
                    NewItem.ItemID = ItemID;
                    NewItem.Quantity = ToAdd;
                    NewItem.SlotIndex = i;
                    
                    InventoryItems.AddItem(NewItem);
                    CurrentWeight += Def->PerItemWeight * ToAdd;
                    
                    QuantityForNewSlots -= ToAdd;
                }
            }
        }
        else
        {
            while (QuantityForNewSlots > 0)
            {
                int32 ToAdd = FMath::Min(QuantityForNewSlots, StackLimit);
                
                FMP_InventoryItem NewItem;
                NewItem.ItemID = ItemID;
                NewItem.Quantity = ToAdd;
                NewItem.SlotIndex = InventoryItems.Items.Num();
                
                InventoryItems.AddItem(NewItem);
                CurrentWeight += Def->PerItemWeight * ToAdd;
                
                QuantityForNewSlots -= ToAdd;
            }
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

void UMP_InventoryComponent::AddItemAtSlot(FName ItemID, int32 Quantity, int32 TargetSlotIndex)
{
    if (!GetOwner()->HasAuthority() || bInventoryLocked || ItemID.IsNone() || Quantity <= 0 || TargetSlotIndex < 0) return;

    UMP_ItemRegistry* Reg = GetRegistry();
    if (!Reg) return;
    const UMP_ItemDefinition* Def = Reg->GetItemDefinitionByName(ItemID);
    if (!Def) return;

    if (bEnforceWeightLimit && (CurrentWeight + (Def->PerItemWeight * Quantity) > MaxWeightCapacity)) return;

    const int32 ArrayIndex = GetArrayIndexFromSlot(TargetSlotIndex);

    if (ArrayIndex != INDEX_NONE)
    {
        FMP_InventoryItem& Item = InventoryItems.Items[ArrayIndex];
        if (Item.bIsLocked || Item.ItemID != ItemID) return;

        int32 SpaceLeft = Def->MaxStackSize - Item.Quantity;
        if (Quantity > SpaceLeft) return; 

        InventoryItems.AddQuantity(ArrayIndex, Quantity);
        CurrentWeight += Def->PerItemWeight * Quantity;
    }
    else
    {
        if (bUseStrictSlots && (TargetSlotIndex >= MaxInventorySlots || InventoryItems.Items.Num() >= MaxInventorySlots)) return;
        if (!bUseStrictSlots && TargetSlotIndex > InventoryItems.Items.Num()) return;
        
        if (Quantity > Def->MaxStackSize) return;

        FMP_InventoryItem NewItem;
        NewItem.ItemID = ItemID;
        NewItem.Quantity = Quantity;
        NewItem.SlotIndex = TargetSlotIndex;
        InventoryItems.AddItem(NewItem);
        CurrentWeight += Def->PerItemWeight * Quantity;
    }
}

void UMP_InventoryComponent::AddItems(const TArray<FMP_InventoryAddItems> &Items)
{
    if (!GetOwner()->HasAuthority() || bInventoryLocked) return;

    for (const FMP_InventoryAddItems& Item : Items)
    {
        AddItem(Item.ItemID, Item.Quantity, Item.bPreferNewSlot);
	}
}

void UMP_InventoryComponent::RemoveItem(int32 SlotIndex, int32 Quantity)
{
    if (!GetOwner()->HasAuthority() || bInventoryLocked) return;
    
    int32 ArrayIndex = GetArrayIndexFromSlot(SlotIndex);
    if (ArrayIndex == INDEX_NONE) return;

    FMP_InventoryItem& Item = InventoryItems.Items[ArrayIndex];
    if (Item.bIsLocked || Quantity <= 0 || Quantity > Item.Quantity) return;

    float WeightPerUnit = 0.0f;

    if (bEnforceWeightLimit)
    {
        if (UMP_ItemRegistry* Reg = GetRegistry())
        {
            if (const UMP_ItemDefinition* Def = Reg->GetItemDefinitionByName(Item.ItemID))
            {
                WeightPerUnit = Def->PerItemWeight;
                CurrentWeight = FMath::Max(0.0f, CurrentWeight - WeightPerUnit * Quantity);
            }
        }
    }
    
    if (bUseStrictSlots)
    {
        InventoryItems.RemoveAtSwap(ArrayIndex, Quantity);
    }
    else
    {
        InventoryItems.RemoveAndShift(ArrayIndex, Quantity);
    }
}

void UMP_InventoryComponent::RemoveItemByID(FName ItemID, int32 Quantity)
{
    if (!GetOwner()->HasAuthority() || bInventoryLocked || ItemID.IsNone() || Quantity <= 0) return;

    int32 Remaining = Quantity;

    for (int32 i = InventoryItems.Items.Num() - 1; i >= 0 && Remaining > 0; --i)
    {
        if (InventoryItems.Items[i].ItemID == ItemID && !InventoryItems.Items[i].bIsLocked)
        {
            int32 SlotIndex = InventoryItems.Items[i].SlotIndex;
            const int32 ToRemove = FMath::Min(Remaining, InventoryItems.Items[i].Quantity);
            RemoveItem(SlotIndex, ToRemove);
            Remaining -= ToRemove;
        }
    }
}

void UMP_InventoryComponent::UpdateItem(FMP_InventoryItem NewItem)
{
    if (!GetOwner()->HasAuthority() || bInventoryLocked || NewItem.ItemID.IsNone() || NewItem.Quantity <= 0) return;

    int32 SlotIndex = NewItem.SlotIndex;
    const int32 ArrayIndex = GetArrayIndexFromSlot(SlotIndex);
    if (ArrayIndex == INDEX_NONE || !InventoryItems.Items.IsValidIndex(ArrayIndex)) return;

    FMP_InventoryItem& OldItem = InventoryItems.Items[ArrayIndex];
    if (OldItem.bIsLocked) return;

    if (UMP_ItemRegistry* Registry = GetRegistry())
    {
        if (const UMP_ItemDefinition* NewDef = Registry->GetItemDefinitionByName(NewItem.ItemID))
        {
            // Security: Prevent malicious UI commands from exceeding the stack limit!
            NewItem.Quantity = FMath::Min(NewItem.Quantity, NewDef->MaxStackSize);
            
            if (bEnforceWeightLimit)
            {
                if (const UMP_ItemDefinition* OldDef = Registry->GetItemDefinitionByName(OldItem.ItemID))
                    CurrentWeight -= OldDef->PerItemWeight * OldItem.Quantity;
                
                CurrentWeight += NewDef->PerItemWeight * NewItem.Quantity;
                CurrentWeight = FMath::Max(0.0f, CurrentWeight);
            }
        }
    }

    InventoryItems.UpdateItem(ArrayIndex, NewItem);
}

void UMP_InventoryComponent::AdjustItemQuantity(int32 SlotIndex, int32 QuantityDelta)
{
    if (!GetOwner()->HasAuthority() || bInventoryLocked || QuantityDelta == 0) return;

    int32 ArrayIndex = GetArrayIndexFromSlot(SlotIndex);
    if (ArrayIndex == INDEX_NONE || !InventoryItems.Items.IsValidIndex(ArrayIndex)) return;

    FMP_InventoryItem& Item = InventoryItems.Items[ArrayIndex];
    if (Item.bIsLocked) return;

    if (QuantityDelta > 0)
    {
        if (UMP_ItemRegistry* Registry = GetRegistry())
        {
            if (const UMP_ItemDefinition* Def = Registry->GetItemDefinitionByName(Item.ItemID))
            {
                int32 SpaceLeft = Def->MaxStackSize - Item.Quantity;
                int32 ToAdd = FMath::Min(QuantityDelta, SpaceLeft);
                if (ToAdd <= 0) return;

                if (bEnforceWeightLimit)
                {
                    CurrentWeight += Def->PerItemWeight * ToAdd;
                }

                InventoryItems.AddQuantity(ArrayIndex, ToAdd);
            }
        }
    }
    else
    {
        int32 ToRemove = FMath::Abs(QuantityDelta);
        RemoveItem(SlotIndex, ToRemove); // Properly routes through removal logic (weight, swap/shift)
    }
}

void UMP_InventoryComponent::SwapItems(int32 SlotIndexA, int32 SlotIndexB)
{
    if (!GetOwner()->HasAuthority() || bInventoryLocked || SlotIndexA == SlotIndexB) return;

    const int32 ArrayIndexA = GetArrayIndexFromSlot(SlotIndexA);
    const int32 ArrayIndexB = GetArrayIndexFromSlot(SlotIndexB);

    const bool bAOccupied = (ArrayIndexA != INDEX_NONE);
    const bool bBOccupied = (ArrayIndexB != INDEX_NONE);

    if (!bAOccupied && !bBOccupied) return; // Both empty, nothing to do
    
    // In infinite list mode, there are no empty slots, so you can only swap two occupied items
    if (!bUseStrictSlots && (!bAOccupied || !bBOccupied)) return; 

    if (bAOccupied && InventoryItems.Items[ArrayIndexA].bIsLocked) return;
    if (bBOccupied && InventoryItems.Items[ArrayIndexB].bIsLocked) return;

    InventoryItems.SwapItemsBySlotIndex(SlotIndexA, SlotIndexB);
}

void UMP_InventoryComponent::SplitItem(int32 SourceSlotIndex, int32 TargetSlotIndex, int32 QuantityToSplit)
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

    int32 FinalSplitQuantity = QuantityToSplit;

    if (!bAutoTarget && bTargetOccupied)
    {
        const FMP_InventoryItem& TargetItem = InventoryItems.Items[TargetArrayIndex];
        if (TargetItem.bIsLocked || TargetItem.ItemID != SplitItemID) return;

        if (const UMP_ItemRegistry* Registry = GetRegistry())
        {
            if (const UMP_ItemDefinition* ItemDef = Registry->GetItemDefinitionByName(SplitItemID))
            {
                int32 RoomLeft = ItemDef->MaxStackSize - TargetItem.Quantity;
                if (RoomLeft <= 0) return; // Target is completely full

                // Clamp the split quantity to whatever fits!
                FinalSplitQuantity = FMath::Min(QuantityToSplit, RoomLeft);
            }
        }
    }
    else if (!bAutoTarget && bUseStrictSlots)
    {
        if (IsSlotOccupied(TargetSlotIndex)) return;
    }

    // Secondary safety check after clamp
    if (SourceItem.Quantity <= FinalSplitQuantity) return;

    // Commit
    InventoryItems.RemoveAtSwap(SourceArrayIndex, FinalSplitQuantity);

    if (!bAutoTarget && bTargetOccupied)
    {
        InventoryItems.AddQuantity(TargetArrayIndex, FinalSplitQuantity);
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
                for (int32 i = 0; i < MaxInventorySlots; ++i)
                {
                    if (!InventoryItems.IndexTracker.IsValidIndex(i) || InventoryItems.IndexTracker[i] == INDEX_NONE)
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
        SplitItem.Quantity = FinalSplitQuantity;
        SplitItem.SlotIndex = NewSlotIndex;

        InventoryItems.AddItem(SplitItem);
    }
}

void UMP_InventoryComponent::SetItemLock(int32 SlotIndex, bool bLocked)
{
    if (!GetOwner()->HasAuthority() || bInventoryLocked) return;

    const int32 ArrayIndex = GetArrayIndexFromSlot(SlotIndex);
    if (ArrayIndex == INDEX_NONE) return;

    FMP_InventoryItem UpdatedItem = InventoryItems.Items[ArrayIndex];
    UpdatedItem.bIsLocked = bLocked;
    InventoryItems.UpdateItem(ArrayIndex, UpdatedItem);
}
