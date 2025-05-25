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

FMP_ItemDefinition::FMP_ItemDefinition()
{
    ItemID = FName();
    DisplayName = TEXT("");
    Icon = nullptr;
}

FMP_ItemDefinition::~FMP_ItemDefinition()
{
}
