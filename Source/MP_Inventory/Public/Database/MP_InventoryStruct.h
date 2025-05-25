// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MP_InventoryStruct.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FMP_InventoryItem
{
public:
    GENERATED_BODY()

    // Unique identifier for the item (e.g., "Product_001")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    FName ItemID;

    // Quantity of the item in this slot
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    int32 Quantity = 1;

    // Tags for filtering (e.g., Item.Digital, Item.Lock)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    FGameplayTagContainer Tags;

    // Display name for UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    TSoftObjectPtr<UTexture> Icon;

    FMP_InventoryItem();
	~FMP_InventoryItem();

    bool operator==(const FMP_InventoryItem& Other) const
    {
        return ItemID == Other.ItemID; // Compare by ItemID only—adjust if needed
    }
};


USTRUCT(BlueprintType)
struct FMP_ItemDefinition
{
    GENERATED_BODY()
public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    FGameplayTagContainer Tags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    TSoftObjectPtr<UTexture> Icon;

    FMP_ItemDefinition();
    ~FMP_ItemDefinition();

};

USTRUCT(BlueprintType)
struct FInventorySnapshot
{
    GENERATED_BODY()

    UPROPERTY()
    FString PlayerId;

    UPROPERTY()
    TArray<FMP_InventoryItem> Items;

    UPROPERTY()
    FDateTime SavedAt;
};

USTRUCT()
struct FItemOwnershipIndexEntry
{
    GENERATED_BODY()

    UPROPERTY()
    FName ItemId;

    UPROPERTY()
    TSet<FString> PlayerIds;
};