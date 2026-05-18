// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Database/MP_InventoryStruct.h"
#include "UObject/NoExportTypes.h"
#include "MP_AnalyticsManager.generated.h"

/**
 * Analytics Manager for Inventory & Trade
 * Stores all trade and item metadata. Singleton per game instance. BP and C++ friendly.
 */
UCLASS()
class MP_INVENTORY_API UMP_AnalyticsManager : public UObject
{
	GENERATED_BODY()
	
public:
   
};