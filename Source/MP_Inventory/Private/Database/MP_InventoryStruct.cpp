// Copyright 2026 UVSquare. All Rights Reserved.


#include "Database/MP_InventoryStruct.h"


FMP_InventoryItem::FMP_InventoryItem()
{
    ItemID = FName();
    Quantity = 1;
    SlotIndex = -1;
}

FMP_InventoryItem::~FMP_InventoryItem()
{
}

bool FMP_InventoryItem::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
    bOutSuccess = true;

    Ar << ItemID;
    Ar << Quantity;
    Ar << SlotIndex;
	Ar << bIsLocked;

    if (Ar.IsError())
    {
        bOutSuccess = false;
    }
    return true;
}



FMP_ItemDefinitions::FMP_ItemDefinitions()
{
    ItemID = FName();
    DisplayName = TEXT("");
    Icon = nullptr;
    InitialVolume = 0;
    BasePrice = 1.0f;
    Materials.Add(nullptr);
}

FMP_ItemDefinitions::~FMP_ItemDefinitions()
{
}

// ---------- FMP_ItemTradeRecord ----------

/* No constructor or destructor needed, uses defaults */

// ---------- FMP_ItemMetadata ----------

/* No constructor or destructor needed, uses defaults */