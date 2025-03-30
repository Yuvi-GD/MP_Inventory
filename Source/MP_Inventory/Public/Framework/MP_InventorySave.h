// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Database/MP_InventoryStruct.h"
#include "GameFramework/SaveGame.h"
#include "MP_InventorySave.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class MP_INVENTORY_API UMP_InventorySave : public USaveGame
{
	GENERATED_BODY()
	
public:


    UPROPERTY(BlueprintReadWrite, Category = "MP_Inventory|Save")
    TArray<FMP_InventoryStruct> InventoryItems;

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Save")
    bool SaveToSlot(FString SlotName);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Save")
    bool LoadFromSlot(FString SlotName);
};
