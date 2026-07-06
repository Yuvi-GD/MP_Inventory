// Copyright Epic Games, Inc. All Rights Reserved.


#include "MP_Inventory.h"

DEFINE_LOG_CATEGORY(LogMPInventory);

#define LOCTEXT_NAMESPACE "FMP_InventoryModule"

void FMP_InventoryModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FMP_InventoryModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMP_InventoryModule, MP_Inventory)
