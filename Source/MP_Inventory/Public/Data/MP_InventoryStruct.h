// Copyright 2026 UVSquare. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MP_InventoryStruct.generated.h"

/**
 * Enum to represent the type of change in inventory, used for tracking and analytics.
 */
UENUM(BlueprintType)
enum class EInventoryDelta : uint8
{
    Refresh,       // -1 shorthand when you wipe everything
    Added,
    Removed,
    Updated,
    None
};

USTRUCT(BlueprintType)
struct FMP_InventoryAddItems
{
	GENERATED_BODY()

	// Unique identifier for the item (e.g., "Product_001")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
	FName ItemID;

	// Quantity of the item to add
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
	int32 Quantity = 1;

    // Prefer New Slot.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    bool bPreferNewSlot = false;
};

USTRUCT(BlueprintType)
struct MP_INVENTORY_API FMP_InventoryItem : public FFastArraySerializerItem
{
public:
    GENERATED_BODY()

    // Unique identifier for the item (e.g., "Product_001")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    FName ItemID;

    // Quantity of the item in this slot
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    int32 Quantity = 1;

	// Tells the TileView exactly which grid slot this item occupies
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    int32 SlotIndex = -1;

    UPROPERTY()
    bool bIsLocked = false;

    FMP_InventoryItem();
	~FMP_InventoryItem();

    // NetSerialize for efficient network replication for FastArray items.
    bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

    bool operator==(const FMP_InventoryItem& Other) const
    {
        return ItemID == Other.ItemID; // Compare by ItemID only adjust if needed
    }
};

template<>
struct TStructOpsTypeTraits<FMP_InventoryItem> : public TStructOpsTypeTraitsBase2<FMP_InventoryItem>
{
    enum { WithNetSerializer = true };
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

USTRUCT(BlueprintType)
struct FMP_InventorySaveData
{
    GENERATED_BODY()
    UPROPERTY()
    TArray<FMP_InventoryItem> InventoryData;
};

USTRUCT(BlueprintType)
struct FItemOwnershipIndexEntry
{
    GENERATED_BODY()

    UPROPERTY()
    FName ItemId;

    UPROPERTY()
    TSet<FString> PlayerIds;
};