// Fill out your copyright notice in the Description page of Project Settings.


#include "Database/MP_InventoryStruct.h"

FMP_InventoryStruct::FMP_InventoryStruct()
{
    ItemID = FName();
    Quantity = 1;
    DisplayName = TEXT("");
    Icon = nullptr;
}

FMP_InventoryStruct::~FMP_InventoryStruct()
{
}
