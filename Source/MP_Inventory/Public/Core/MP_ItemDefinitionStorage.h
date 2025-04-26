// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Database/MP_InventoryStruct.h"
#include "MP_ItemDefinitionStorage.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class MP_INVENTORY_API UMP_ItemDefinitionStorage : public UObject
{
	GENERATED_BODY()
	
public:
    UMP_ItemDefinitionStorage();

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory")
    const FMP_ItemDefinition& GetItemDefinition(FName ItemID) const;

    static UMP_ItemDefinitionStorage* Get();

private:
    UPROPERTY()
    TMap<FName, FMP_ItemDefinition> ItemDefinitions;

};
