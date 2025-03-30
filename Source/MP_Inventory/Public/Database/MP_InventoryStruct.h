// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MP_InventoryStruct.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FMP_InventoryStruct
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

	FMP_InventoryStruct();
	~FMP_InventoryStruct();

    bool operator==(const FMP_InventoryStruct& Other) const
    {
        return ItemID == Other.ItemID; // Compare by ItemID only—adjust if needed
    }
};


USTRUCT(BlueprintType)
struct FMP_ItemDefinition
{
    GENERATED_BODY()
public:

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FName ItemID;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString DisplayName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FGameplayTagContainer Tags;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString Icon;

    FMP_ItemDefinition();
    ~FMP_ItemDefinition();

};
