// Copyright 2026 UVSquare. All Rights Reserved.

#include "Core/MP_InventoryManager.h"
#include "Core/MP_InventoryComponent.h"
#include "Core/MP_InventoryPickupInterface.h"
#include "Core/MP_ItemRegistry.h"
#include "MP_Inventory.h"
#include "Engine/GameInstance.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

UMP_InventoryManager::UMP_InventoryManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UMP_InventoryManager::BeginPlay()
{
    Super::BeginPlay();    
    GetManagerID();
}

FName UMP_InventoryManager::GetManagerID()
{
    if (!ManagerID.IsNone()) return ManagerID;

    if (GetOwner()->HasAuthority())
    {
        if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
        {
            if (APlayerState* PS = PC->PlayerState)
            {
                if (!bUseDeterministicPlayerID && PS->GetUniqueId().IsValid())
                {
                    // Use the Online Subsystem's persistent ID
                    ManagerID = FName(*PS->GetUniqueId()->ToString());
                }
                else
                {
                    // Fallback to deterministic Player Index for Editor / PIE / Offline
                    int32 PlayerIndex = 0;
                    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
                    {
                        if (It->Get() == PC)
                        {
                            break;
                        }
                        PlayerIndex++;
                    }
                    ManagerID = FName(*FString::Printf(TEXT("Player_%d"), PlayerIndex));
                }
            }
        }
    }
    
    return ManagerID;
}


void UMP_InventoryManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UMP_InventoryManager, ManagerID);
}

UMP_InventoryComponent* UMP_InventoryManager::GetAndValidateComponent(FName InventoryID)
{
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        if (UMP_ItemRegistry* Registry = GameInstance->GetSubsystem<UMP_ItemRegistry>())
        {
            if (UMP_InventoryComponent* Comp = Registry->GetInventoryByInventoryID(InventoryID)) 
            {
                // 1. Is it owned directly by this manager?
                if (Comp->OwnerID == this->ManagerID)
                {
                    return Comp;
                }
                // 2. Is it globally accessible?
                else if (Comp->OwnerID == FName("GLOBAL"))
                {
                    return Comp;
                }
                // 3. Is it private but this manager is explicitly allowed?
                else if (Comp->OwnerID == FName("PRIVATE") && Comp->AccessList.Contains(this->ManagerID))
                {
                    return Comp;
                }
                else
                {
                    UE_LOG(LogMPInventory, Warning, TEXT("MP_InventoryManager: Permission denied for InventoryID %s"), *InventoryID.ToString());
                    if (GetOwner()->HasAuthority()) Client_OnActionNotify(FName("PermissionDenied"));
                    else OnInventoryActionNotify.Broadcast(FName("PermissionDenied"));
                }
            }
            else
            {
                if (GetOwner()->HasAuthority()) Client_OnActionNotify(FName("ComponentNotFound"));
                else OnInventoryActionNotify.Broadcast(FName("ComponentNotFound"));
            }
        }
    }
    return nullptr;
}

void UMP_InventoryManager::Client_OnActionNotify_Implementation(FName Reason)
{
    OnInventoryActionNotify.Broadcast(Reason);
}

// =========================================================================
//  NEARBY LOOT API (Local UI Tracking)
// =========================================================================

void UMP_InventoryManager::AddNearbyLoot(AActor* LootItem)
{
    if (IsValid(LootItem) && !NearbyLoot.Contains(LootItem))
    {
        int32 Index = NearbyLoot.Add(LootItem);
        OnNearbyLootChanged.Broadcast(EInventoryDelta::Added, Index);
    }
}

void UMP_InventoryManager::RemoveNearbyLoot(AActor* LootItem)
{
    int32 Index = NearbyLoot.Find(LootItem);
    if (Index != INDEX_NONE)
    {
        NearbyLoot.RemoveAt(Index);
        OnNearbyLootChanged.Broadcast(EInventoryDelta::Removed, Index);
    }
}

void UMP_InventoryManager::RemoveNearbyLootByIndex(int32 Index)
{
    if (NearbyLoot.IsValidIndex(Index))
    {
        NearbyLoot.RemoveAt(Index);
        OnNearbyLootChanged.Broadcast(EInventoryDelta::Removed, Index);
    }
}

void UMP_InventoryManager::UpdateNearbyLoot(AActor* LootItem)
{
    int32 Index = NearbyLoot.Find(LootItem);
    if (Index != INDEX_NONE)
    {
        OnNearbyLootChanged.Broadcast(EInventoryDelta::Updated, Index);
    }
}

AActor* UMP_InventoryManager::GetNearbyLootAt(int32 Index) const
{
    if (NearbyLoot.IsValidIndex(Index))
    {
        return NearbyLoot[Index];
    }
    return nullptr;
}

const TArray<AActor*>& UMP_InventoryManager::GetAllNearbyLoot() const
{
    return NearbyLoot;
}


// =========================================================================
//  CLIENT-TO-SERVER RPC IMPLEMENTATIONS
// =========================================================================

void UMP_InventoryManager::AddItem_Implementation(FName TargetInventoryID, FName ItemID, int32 Quantity, bool bPreferNewSlot)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetInventoryID))
    {
        Comp->AddItem(ItemID, Quantity, bPreferNewSlot);
    }
}

void UMP_InventoryManager::AddItemAtSlot_Implementation(FName TargetInventoryID, FName ItemID, int32 Quantity, int32 TargetSlotIndex)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetInventoryID))
    {
        Comp->AddItemAtSlot(ItemID, Quantity, TargetSlotIndex);
    }
}

void UMP_InventoryManager::AddItems_Implementation(FName TargetInventoryID, const TArray<FMP_InventoryAddItems>& Items)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetInventoryID))
    {
        Comp->AddItems(Items);
    }
}

void UMP_InventoryManager::RemoveItem_Implementation(FName TargetInventoryID, int32 SlotIndex, int32 Quantity)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetInventoryID))
    {
        Comp->RemoveItem(SlotIndex, Quantity);
    }
}

void UMP_InventoryManager::RemoveItemByID_Implementation(FName TargetInventoryID, FName ItemID, int32 Quantity)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetInventoryID))
    {
        Comp->RemoveItemByID(ItemID, Quantity);
    }
}

void UMP_InventoryManager::UpdateItem_Implementation(FName TargetInventoryID, FMP_InventoryItem NewItem)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetInventoryID))
    {
        Comp->UpdateItem(NewItem);
    }
}

void UMP_InventoryManager::AdjustItemQuantity_Implementation(FName TargetInventoryID, int32 SlotIndex, int32 QuantityDelta)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetInventoryID))
    {
        Comp->AdjustItemQuantity(SlotIndex, QuantityDelta);
    }
}

void UMP_InventoryManager::SwapItems_Implementation(FName TargetInventoryID, int32 SlotIndexA, int32 SlotIndexB)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetInventoryID))
    {
        Comp->SwapItems(SlotIndexA, SlotIndexB);
    }
}

void UMP_InventoryManager::SplitItem_Implementation(FName TargetInventoryID, int32 SourceSlotIndex, int32 TargetSlotIndex, int32 QuantityToSplit)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetInventoryID))
    {
        Comp->SplitItem(SourceSlotIndex, TargetSlotIndex, QuantityToSplit);
    }
}

// =========================================================================
//  MULTI-COMPONENT RPCs
//  Operations that bridge two distinct inventories (or the same inventory)
// =========================================================================

void UMP_InventoryManager::SwapItemsBetweenInventories_Implementation(FName SourceInventoryID, int32 SourceSlotIndex, FName TargetInventoryID, int32 TargetSlotIndex)
{
    UMP_InventoryComponent* SourceComp = GetAndValidateComponent(SourceInventoryID);
    UMP_InventoryComponent* TargetComp = GetAndValidateComponent(TargetInventoryID);

    if (!SourceComp || !TargetComp) return;

    if (SourceComp == TargetComp)
    {
        SourceComp->SwapItems(SourceSlotIndex, TargetSlotIndex);
        return;
    }

    FMP_InventoryItem SourceItem = SourceComp->GetItemBySlotIndex(SourceSlotIndex);
    FMP_InventoryItem TargetItem = TargetComp->GetItemBySlotIndex(TargetSlotIndex);

    if (SourceItem.ItemID.IsNone() && TargetItem.ItemID.IsNone()) return;

    bool bSourceOccupied = !SourceItem.ItemID.IsNone();
    bool bTargetOccupied = !TargetItem.ItemID.IsNone();

    // PERFECT SWAP: If both slots are occupied, we use UpdateItem to perfectly overwrite the data in-place.
    // This prevents the arrays from shrinking/shifting during a Remove/Add cycle (critical for non-strict lists).
    if (bSourceOccupied && bTargetOccupied)
    {
        FMP_InventoryItem NewSourceItem = TargetItem;
        NewSourceItem.SlotIndex = SourceSlotIndex;

        FMP_InventoryItem NewTargetItem = SourceItem;
        NewTargetItem.SlotIndex = TargetSlotIndex;

        // Try applying Target first
        if (TargetComp->UpdateItem(NewTargetItem))
        {
            if (!SourceComp->UpdateItem(NewSourceItem))
            {
                // Revert
                TargetComp->UpdateItem(TargetItem);
            }
        }
        return;
    }

    // TRANSFER: Moving to an empty slot. We just remove from one, and Add to the other.
    if (bSourceOccupied && !bTargetOccupied)
    {
        if (SourceComp->RemoveItem(SourceSlotIndex, SourceItem.Quantity))
        {
            if (!TargetComp->AddItemAtSlot(SourceItem.ItemID, SourceItem.Quantity, TargetSlotIndex))
            {
                SourceComp->AddItemAtSlot(SourceItem.ItemID, SourceItem.Quantity, SourceSlotIndex);
            }
        }
    }
    else if (!bSourceOccupied && bTargetOccupied)
    {
        if (TargetComp->RemoveItem(TargetSlotIndex, TargetItem.Quantity))
        {
            if (!SourceComp->AddItemAtSlot(TargetItem.ItemID, TargetItem.Quantity, SourceSlotIndex))
            {
                TargetComp->AddItemAtSlot(TargetItem.ItemID, TargetItem.Quantity, TargetSlotIndex);
            }
        }
    }
}

void UMP_InventoryManager::TransferItemBySlot_Implementation(FName SourceInventoryID, int32 SourceSlotIndex, FName TargetInventoryID, int32 TargetSlotIndex, int32 Quantity)
{
    UMP_InventoryComponent* SourceComp = GetAndValidateComponent(SourceInventoryID);
    UMP_InventoryComponent* TargetComp = GetAndValidateComponent(TargetInventoryID);

    if (!SourceComp || !TargetComp) return;

    FMP_InventoryItem SourceItem = SourceComp->GetItemBySlotIndex(SourceSlotIndex);
    if (SourceItem.ItemID.IsNone()) return;
    if(Quantity == -1)
    {
        Quantity = SourceItem.Quantity;
    }
    else if (SourceItem.Quantity < Quantity)
    {
        return;
    }

    // 1. Remove First
    if (SourceComp->RemoveItem(SourceSlotIndex, Quantity))
    {
        // 2. Try Add to Target
        bool bAddSuccess = false;
        if (TargetSlotIndex == -1)
        {
            bAddSuccess = TargetComp->AddItem(SourceItem.ItemID, Quantity, true);
        }
        else
        {
            bAddSuccess = TargetComp->AddItemAtSlot(SourceItem.ItemID, Quantity, TargetSlotIndex);
        }

        // 3. Revert if Target rejected
        if (!bAddSuccess)
        {
            // The slot we took it from might be completely empty now, so we can just add it back.
            // Using AddItemAtSlot ensures it goes right back where it was.
            SourceComp->AddItemAtSlot(SourceItem.ItemID, Quantity, SourceSlotIndex);
			Client_OnActionNotify(FName(" Failed To Transfer Item"));
        }
    }
}

void UMP_InventoryManager::TransferItemByID_Implementation(FName SourceInventoryID, FName TargetInventoryID, FName ItemID, int32 Quantity)
{
    UMP_InventoryComponent* SourceComp = GetAndValidateComponent(SourceInventoryID);
    UMP_InventoryComponent* TargetComp = GetAndValidateComponent(TargetInventoryID);

    if (!SourceComp || !TargetComp || Quantity <= 0 || ItemID.IsNone()) return;

    // Remove first. RemoveItemByID returns true only if the full quantity was successfully removed across stacks.
    if (SourceComp->RemoveItemByID(ItemID, Quantity))
    {
        // Try to add to target
        bool bAddSuccess = TargetComp->AddItem(ItemID, Quantity, true);
        
        // Revert if target rejected
        if (!bAddSuccess)
        {
            // Add back to source. Since we pulled from potentially multiple stacks, 
            // we just let it auto-stack on the source.
            SourceComp->AddItem(ItemID, Quantity, true);
        }
    }
}

void UMP_InventoryManager::MergeSlotsBetweenInventories_Implementation(FName SourceInventoryID, int32 SourceSlotIndex, FName TargetInventoryID, int32 TargetSlotIndex)
{
    UMP_InventoryComponent* SourceComp = GetAndValidateComponent(SourceInventoryID);
    UMP_InventoryComponent* TargetComp = GetAndValidateComponent(TargetInventoryID);

    if (!SourceComp || !TargetComp) return;

    FMP_InventoryItem SourceItem = SourceComp->GetItemBySlotIndex(SourceSlotIndex);
    FMP_InventoryItem TargetItem = TargetComp->GetItemBySlotIndex(TargetSlotIndex);

    // Can only merge if they are the exact same item
    if (SourceItem.ItemID.IsNone() || SourceItem.ItemID != TargetItem.ItemID) return;
    
    if (SourceItem.bIsLocked || TargetItem.bIsLocked) return;

    UGameInstance* GameInstance = GetWorld()->GetGameInstance();
    UMP_ItemRegistry* Registry = GameInstance ? GameInstance->GetSubsystem<UMP_ItemRegistry>() : nullptr;
    if (!Registry) return;

    const UMP_ItemDefinition* Def = Registry->GetItemDefinitionByName(SourceItem.ItemID);
    if (!Def) return;

    int32 TargetSlack = Def->MaxStackSize - TargetItem.Quantity;
    if (TargetSlack <= 0) return; // Target stack is already full

    // Check Weight Limits on Target (if moving to a different component)
    if (SourceComp != TargetComp && TargetComp->bEnforceWeightLimit)
    {
        float RemainingWeight = TargetComp->MaxWeightCapacity - TargetComp->GetCurrentWeight();
        int32 MaxAffordableByWeight = FMath::FloorToInt(RemainingWeight / Def->PerItemWeight);
        if (MaxAffordableByWeight < TargetSlack)
        {
            TargetSlack = MaxAffordableByWeight;
        }
    }

    int32 AmountToMove = FMath::Min(SourceItem.Quantity, TargetSlack);
    if (AmountToMove <= 0) return;

    // Execute atomic merge. 
    // IMPORTANT: We MUST adjust the target FIRST before removing from the source.
    // If we remove from the source first in a non-strict list, the array shifts, 
    // and TargetSlotIndex becomes invalid, causing the adjustment to fail and rollback!
    if (TargetComp->AdjustItemQuantity(TargetSlotIndex, AmountToMove))
    {
        if (!SourceComp->RemoveItem(SourceSlotIndex, AmountToMove))
        {
            // Failsafe rollback
            TargetComp->AdjustItemQuantity(TargetSlotIndex, -AmountToMove);
            Client_OnActionNotify(FName(" Failed To Merge Item at slot"));
        }
    }
}

void UMP_InventoryManager::LootGroundItemByIndex(int32 Index, FName TargetInventoryID, int32 TargetSlotIndex, int32 Quantity)
{
    LootGroundItem(GetNearbyLootAt(Index),TargetInventoryID, TargetSlotIndex, Quantity);
}

void UMP_InventoryManager::LootGroundItem_Implementation(AActor* GroundItemActor, FName TargetInventoryID, int32 TargetSlotIndex, int32 Quantity)
{
    if (!IsValid(GroundItemActor) || GroundItemActor->IsPendingKillPending()) return;

    UMP_InventoryComponent* TargetComp = GetAndValidateComponent(TargetInventoryID);
    if (!TargetComp) return;

    if (!GroundItemActor->Implements<UMP_InventoryPickupInterface>()) return;

    // Distance check to ensure the player isn't cheating by picking up items from across the map
    // Try Case A: Owner is already the Pawn
    APawn* Pawn = Cast<APawn>(TargetComp->GetOwner());

    // Try Case B: Owner is a Controller
    if (!Pawn)
    {
        if (APlayerController* PC = Cast<APlayerController>(TargetComp->GetOwner()))
        {
            Pawn = PC->GetPawn();
        }
    }

    // Try Case C: Owner is the PlayerState
    if (!Pawn)
    {
        if (APlayerState* PS = Cast<APlayerState>(TargetComp->GetOwner()))
        {
            Pawn = PS->GetPawn();
        }
    }

    if (Pawn)
    {
        float Distance = FVector::Dist(Pawn->GetActorLocation(), GroundItemActor->GetActorLocation());
        if (Distance > 450.0f) return;
    }
    else
    {
        // Fail-safe: If we couldn't find a pawn, we shouldn't allow the pickup
        UE_LOG(LogMPInventory, Warning, TEXT("Pickup failed: Could not resolve a valid physical Pawn from the inventory component."));
        return;
    }

    FName ItemID;
    int32 TotalQuantity;
    IMP_InventoryPickupInterface::Execute_GetPickupData(GroundItemActor, ItemID, TotalQuantity);

    if (ItemID.IsNone() || TotalQuantity <= 0) return;

    if (Quantity == -1)
    {
        Quantity = TotalQuantity;
		TotalQuantity = 0;
    }
    else if (Quantity > TotalQuantity)
    {
		return; // Can't loot more than the item has
	}
    else if (Quantity <= TotalQuantity)
    {
        TotalQuantity -= Quantity; // Reduce the quantity to loot to the expected amount, leaving the rest on the ground
	}

    bool bSuccess = false;
    if (TargetSlotIndex == -1)
    {
        bSuccess = TargetComp->AddItem(ItemID, Quantity, true);
    }
    else
    {
        bSuccess = TargetComp->AddItemAtSlot(ItemID, Quantity, TargetSlotIndex);
    }

    if (bSuccess)
    {
        if (TotalQuantity == 0)
        {
            GroundItemActor->Destroy();
        }
		else if (TotalQuantity > 0)
        {
            IMP_InventoryPickupInterface::Execute_SetPickupData(GroundItemActor, ItemID, TotalQuantity);
        }
    }
    else
    {
		Client_OnActionNotify(FName("Can't Add Item To Inventory"));
    }
}


void UMP_InventoryManager::DropItem_Implementation(FName TargetInventoryID, int32 SlotIndex, int32 Quantity, FVector DropLocation)
{
    if (Quantity <= 0) return;

    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetInventoryID))
    {
        if (AActor* CompOwner = Comp->GetOwner())
        {
            FVector OwnerLoc = CompOwner->GetActorLocation();

            // If the inventory is on a Controller or PlayerState, we need the physical Pawn's location instead
            if (APlayerController* PC = Cast<APlayerController>(CompOwner))
            {
                if (APawn* Pawn = PC->GetPawn())
                {
                    OwnerLoc = Pawn->GetActorLocation();
                }
            }
            else if (APlayerState* PS = Cast<APlayerState>(CompOwner))
            {
                if (APawn* Pawn = PS->GetPawn())
                {
                    OwnerLoc = Pawn->GetActorLocation();
                }
            }

            float MaxDist = 500.0f; // Prevent spawning items extremely far away
            if (FVector::DistSquared(OwnerLoc, DropLocation) > FMath::Square(MaxDist))
            {
                FVector Dir = (DropLocation - OwnerLoc).GetSafeNormal();
                DropLocation = OwnerLoc + (Dir * MaxDist);
            }
        }
        if (!Comp->DropItem(SlotIndex, Quantity, DropLocation))
        {
            Client_OnActionNotify(FName("Failed To Drop Items"));
        }
    }
}

void UMP_InventoryManager::SetItemLock_Implementation(FName TargetInventoryID, int32 SlotIndex, bool bLocked)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetInventoryID))
    {
        Comp->SetItemLock(SlotIndex, bLocked);
    }
}

void UMP_InventoryManager::ResizeInventory_Implementation(FName TargetInventoryID, int32 NewMaxSlots)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetInventoryID))
    {
        Comp->ResizeInventory(NewMaxSlots);
    }
}

void UMP_InventoryManager::CompactSlots_Implementation(FName TargetInventoryID)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetInventoryID))
    {
        Comp->CompactSlots();
    }
}