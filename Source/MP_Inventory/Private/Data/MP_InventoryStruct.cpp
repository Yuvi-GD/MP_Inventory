// Copyright 2026 UVSquare. All Rights Reserved.


#include "Data/MP_InventoryStruct.h"

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