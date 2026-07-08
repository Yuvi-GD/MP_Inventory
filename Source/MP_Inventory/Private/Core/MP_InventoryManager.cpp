// Copyright 2026 UVSquare. All Rights Reserved.

#include "Core/MP_InventoryManager.h"
#include "Core/MP_InventoryComponent.h"
#include "Core/MP_ItemRegistry.h"
#include "MP_Inventory.h"
#include "Engine/GameInstance.h"
#include "Net/UnrealNetwork.h"

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
        Comp->DropItem(SlotIndex, Quantity, DropLocation);
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

void UMP_InventoryManager::TransferItemBySlot_Implementation(FName SourceInventoryID, int32 SourceSlotIndex, FName TargetInventoryID, int32 TargetSlotIndex, int32 Quantity)
{
    UMP_InventoryComponent* SourceComp = GetAndValidateComponent(SourceInventoryID);
    UMP_InventoryComponent* TargetComp = GetAndValidateComponent(TargetInventoryID);

    if (!SourceComp || !TargetComp || Quantity <= 0) return;

    FMP_InventoryItem SourceItem = SourceComp->GetItemBySlotIndex(SourceSlotIndex);
    if (SourceItem.ItemID.IsNone() || SourceItem.Quantity < Quantity) return;

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
