// Copyright 2026 UVSquare. All Rights Reserved.

#include "Core/MP_InventoryManager.h"
#include "Core/MP_InventoryComponent.h"
#include "Core/MP_ItemRegistry.h"
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

    // If ManagerID is empty, generate a unique GUID.
    if (GetOwner()->HasAuthority() && ManagerID.IsNone())
    {
        ManagerID = FName(*FGuid::NewGuid().ToString());
    }
}

void UMP_InventoryManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UMP_InventoryManager, ManagerID);
}

UMP_InventoryComponent* UMP_InventoryManager::GetAndValidateComponent(FName ComponentID)
{
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        if (UMP_ItemRegistry* Registry = GameInstance->GetSubsystem<UMP_ItemRegistry>())
        {
            if (UMP_InventoryComponent* Comp = Registry->GetInventoryByComponentID(ComponentID)) 
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
                    UE_LOG(LogTemp, Warning, TEXT("MP_InventoryManager: Permission denied for ComponentID %s"), *ComponentID.ToString());
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

void UMP_InventoryManager::AddItem_Implementation(FName TargetComponentID, FName ItemID, int32 Quantity, bool bPreferNewSlot)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetComponentID))
    {
        Comp->AddItem(ItemID, Quantity, bPreferNewSlot);
    }
}

void UMP_InventoryManager::AddItemAtSlot_Implementation(FName TargetComponentID, FName ItemID, int32 Quantity, int32 TargetSlotIndex)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetComponentID))
    {
        Comp->AddItemAtSlot(ItemID, Quantity, TargetSlotIndex);
    }
}

void UMP_InventoryManager::AddItems_Implementation(FName TargetComponentID, const TArray<FMP_InventoryAddItems>& Items)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetComponentID))
    {
        Comp->AddItems(Items);
    }
}

void UMP_InventoryManager::RemoveItem_Implementation(FName TargetComponentID, int32 SlotIndex, int32 Quantity)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetComponentID))
    {
        Comp->RemoveItem(SlotIndex, Quantity);
    }
}

void UMP_InventoryManager::RemoveItemByID_Implementation(FName TargetComponentID, FName ItemID, int32 Quantity)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetComponentID))
    {
        Comp->RemoveItemByID(ItemID, Quantity);
    }
}

void UMP_InventoryManager::UpdateItem_Implementation(FName TargetComponentID, FMP_InventoryItem NewItem)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetComponentID))
    {
        Comp->UpdateItem(NewItem);
    }
}

void UMP_InventoryManager::AdjustItemQuantity_Implementation(FName TargetComponentID, int32 SlotIndex, int32 QuantityDelta)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetComponentID))
    {
        Comp->AdjustItemQuantity(SlotIndex, QuantityDelta);
    }
}

void UMP_InventoryManager::SwapItems_Implementation(FName TargetComponentID, int32 SlotIndexA, int32 SlotIndexB)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetComponentID))
    {
        Comp->SwapItems(SlotIndexA, SlotIndexB);
    }
}

void UMP_InventoryManager::SplitItem_Implementation(FName TargetComponentID, int32 SourceSlotIndex, int32 TargetSlotIndex, int32 QuantityToSplit)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetComponentID))
    {
        Comp->SplitItem(SourceSlotIndex, TargetSlotIndex, QuantityToSplit);
    }
}

void UMP_InventoryManager::SetItemLock_Implementation(FName TargetComponentID, int32 SlotIndex, bool bLocked)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetComponentID))
    {
        Comp->SetItemLock(SlotIndex, bLocked);
    }
}

void UMP_InventoryManager::CompactSlots_Implementation(FName TargetComponentID)
{
    if (UMP_InventoryComponent* Comp = GetAndValidateComponent(TargetComponentID))
    {
        Comp->CompactSlots();
    }
}
