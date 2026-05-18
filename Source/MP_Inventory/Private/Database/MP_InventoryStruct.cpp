// Fill out your copyright notice in the Description page of Project Settings.


#include "Database/MP_InventoryStruct.h"

FMP_InventoryItem::FMP_InventoryItem()
{
    ItemID = FName();
    Quantity = 1;
    DisplayName = TEXT("");
    Icon = nullptr;
}

FMP_InventoryItem::~FMP_InventoryItem()
{
}

bool FMP_InventoryItem::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
    bOutSuccess = true;

    Ar << ItemID;
    Ar << Quantity;
    Ar << Value;
    Ar << DisplayName;

    // --- Serialize FGameplayTagContainer Tags ---
    Tags.NetSerialize(Ar, Map, bOutSuccess);
    // --------------------------------------------

    // Soft object pointer
    UObject* IconPtr = Icon.Get();
    Map->SerializeObject(Ar, UTexture::StaticClass(), IconPtr);
    if (Ar.IsLoading())
    {
        Icon = Cast<UTexture>(IconPtr);
    }

    if (Ar.IsError())
    {
        bOutSuccess = false;
    }
    return true;
}



FMP_ItemDefinition::FMP_ItemDefinition()
{
    ItemID = FName();
    DisplayName = TEXT("");
    Icon = nullptr;
    InitialVolume = 0;
    BasePrice = 1.0f;
    Materials.Add(nullptr);
}

FMP_ItemDefinition::~FMP_ItemDefinition()
{
}

// ---------- FMP_ItemTradeRecord ----------

/* No constructor or destructor needed, uses defaults */

// ---------- FMP_ItemMetadata ----------

/* No constructor or destructor needed, uses defaults */