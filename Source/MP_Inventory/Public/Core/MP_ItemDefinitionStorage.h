// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Database/MP_InventoryStruct.h"
#include "MP_ItemDefinitionStorage.generated.h"

/**
 * Item Definition Storage Singleton
 * Stores all item static definitions for the runtime. Supports dynamic add/update/remove from BP or C++.
 */
UCLASS(Blueprintable, BlueprintType)
class MP_INVENTORY_API UMP_ItemDefinitionStorage : public UObject
{
	GENERATED_BODY()
	
public:
    UMP_ItemDefinitionStorage();

    // Get or create singleton (one per world) **Alway Fetch from One Context other wise Address may change**
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|DefinitionStorage", meta = (WorldContext = "WorldContextObject"))
    static UMP_ItemDefinitionStorage* Get(UObject* WorldContextObject);

    // Add a single item definition
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|DefinitionStorage")
    void AddItemDefinition(const FMP_ItemDefinition& Definition);

    // Add multiple definitions (batch)
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|DefinitionStorage")
    void AddDefinitionsFromArray(const TArray<FMP_ItemDefinition>& Definitions);

    // Remove by ID
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|DefinitionStorage")
    void RemoveItemDefinition(FName ItemID);

    // Update (replace) by ID
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|DefinitionStorage")
    void UpdateItemDefinition(const FMP_ItemDefinition& Definition);

    // Get definition by ItemID (returns copy)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|DefinitionStorage")
    FMP_ItemDefinition GetItemDefinition(FName ItemID) const;

	// Get definitions by tag (returns copy)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|DefinitionStorage")
    TArray<FMP_ItemDefinition> GetItemDefinitionByTag(FGameplayTagContainer Tags) const;

    // Get all definitions as array (for shop, UI, etc)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|DefinitionStorage")
    TArray<FMP_ItemDefinition> GetAllDefinitions() const;

    // Log all definitions (debug)
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|DefinitionStorage")
    void PrintAllDefinitions() const;

protected:
    // Map for fast runtime lookups
    UPROPERTY()
    TMap<FName, FMP_ItemDefinition> ItemDefinitions;

private:
    // Internal: static pointer per world (for demo; can upgrade to subsystem later)
    static TWeakObjectPtr<UMP_ItemDefinitionStorage> SingletonInstance;
};
