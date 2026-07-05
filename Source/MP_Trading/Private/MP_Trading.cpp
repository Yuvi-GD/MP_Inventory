#include "MP_Trading.h"

#define LOCTEXT_NAMESPACE "FMP_TradingModule"

void FMP_TradingModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FMP_TradingModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMP_TradingModule, MP_Trading)
