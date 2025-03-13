// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/MP_InventorySave.h"
#include "Kismet/GameplayStatics.h"

bool UMP_InventorySave::SaveToSlot(FString SlotName)
{
    if (SlotName.IsEmpty())
    {
        return false;
    }

    return UGameplayStatics::SaveGameToSlot(this, SlotName, 0);
}

bool UMP_InventorySave::LoadFromSlot(FString SlotName)
{
    if (SlotName.IsEmpty() || !UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        return false;
    }

    UMP_InventorySave* LoadedSave = Cast<UMP_InventorySave>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    if (LoadedSave)
    {
        InventoryItems = LoadedSave->InventoryItems;
        return true;
    }

    return false;
}