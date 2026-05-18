// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "GameplayTagContainer.h"
#include "MP_InventoryStruct.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EInventoryDelta : uint8
{
    Refresh,       // –1 shorthand when you wipe everything
    Added,
    Removed,
    Updated,
    None
};

UENUM(BlueprintType)
enum class ETradeSessionStatus : uint8
{
    Pending,
    Bargain,
    Accepted,
    Completed,
    Cancelled,
    Rejected
};

UENUM(BlueprintType)
enum class ETradeOfferType : uint8
{
    Exchange,
    Buy,
    Sell,
};

USTRUCT(BlueprintType)
struct FMP_InventoryItem : public FFastArraySerializerItem
{
public:
    GENERATED_BODY()

    // Unique identifier for the item (e.g., "Product_001")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    FName ItemID;

    // Quantity of the item in this slot
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    float Value = 0.0f;

    // Tags for filtering (e.g., Item.Digital, Item.Lock)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    FGameplayTagContainer Tags;

    // Display name for UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    FString DisplayName;

    // Soft pointer to icon texture
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Structure")
    TSoftObjectPtr<UTexture> Icon;

    FMP_InventoryItem();
	~FMP_InventoryItem();

    // NetSerialize for efficient network replication for FastArray items.
    bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

    bool operator==(const FMP_InventoryItem& Other) const
    {
        return ItemID == Other.ItemID; // Compare by ItemID only—adjust if needed
    }
};

template<>
struct TStructOpsTypeTraits<FMP_InventoryItem> : public TStructOpsTypeTraitsBase2<FMP_InventoryItem>
{
    enum { WithNetSerializer = true };
};

USTRUCT(BlueprintType)
struct FMP_ItemDefinition : public FTableRowBase
{
    GENERATED_BODY()
public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Definition")
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Definition")
    FString DisplayName;

    // Optional description for tooltips or lore
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Definition", meta = (MultiLine = true))
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Definition")
    FGameplayTagContainer Tags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Definition")
    TSoftObjectPtr<UTexture> Icon;

    // 3D asset reference (mesh, actor, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Definition")
    TSoftObjectPtr<UObject> Model;

    // (Optional) Array for multiple material instances
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Definition")
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials;

    // Initial static volume (e.g., world stock)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Definition")
    int32 InitialVolume = 1;

    // Base (starting) price
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Definition")
    float BasePrice = 1.0f;

    FMP_ItemDefinition();
    ~FMP_ItemDefinition();

};

// Trade offer submitted by a participant in a session.
USTRUCT(BlueprintType)
struct FMP_TradeOffer
{
    GENERATED_BODY()

    // Unique ID of player/NPC making this offer.
    UPROPERTY(BlueprintReadWrite, Category = "MP_Trade|Offer")
    FString OfferId;

    // Unique ID of player/NPC making this offer.
    UPROPERTY(BlueprintReadWrite, Category = "MP_Trade|Offer")
    FString PlayerId;

    // The type of offer (buy, sell, etc).
    UPROPERTY(BlueprintReadWrite, Category = "MP_Trade|Offer")
    ETradeOfferType OfferType;

    // Items offered in this deal.
    UPROPERTY(BlueprintReadWrite, Category = "MP_Trade|Offer")
    TArray<FMP_InventoryItem> ItemsOffered;

    // Value offered, such as currency.
    UPROPERTY(BlueprintReadWrite, Category = "MP_Trade|Offer")
    float ValueOffered = 0.0f;

    // Offer status in the session workflow.
    UPROPERTY(BlueprintReadWrite, Category = "MP_Trade|Offer")
    ETradeSessionStatus Status;

    // When this offer was made.
    UPROPERTY(BlueprintReadWrite, Category = "MP_Trade|Offer")
    FDateTime OfferTime;
};

// Tracks a single trade session and all negotiations.
USTRUCT(BlueprintType)
struct FMP_TradeSession
{
    GENERATED_BODY()

    // Unique ID for this trade session.
    UPROPERTY(BlueprintReadWrite, Category = "MP_Trade|Session")
    FString TradeId;

    // UniqueId of first participant (initiator).
    UPROPERTY(BlueprintReadWrite, Category = "MP_Trade|Session")
    FString PlayerAId;

    // UniqueId of second participant (target).
    UPROPERTY(BlueprintReadWrite, Category = "MP_Trade|Session")
    FString PlayerBId;

    // Current session status (pending, completed, etc).
    UPROPERTY(BlueprintReadWrite, Category = "MP_Trade|Session")
    ETradeSessionStatus CurrentStatus;

    // List of all offers/counter-offers (bargain queue).
    UPROPERTY(BlueprintReadWrite, Category = "MP_Trade|Session")
    TArray<FMP_TradeOffer> Offers;
};

// Single trade event record (used in Analytics/metadata, not by session directly)
USTRUCT(BlueprintType)
struct FMP_ItemTradeRecord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Trade")
    FString TradeId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Trade")
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Trade")
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Trade")
    float TradePrice = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Trade")
    EInventoryDelta TradeType = EInventoryDelta::Added;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Trade")
    FDateTime TradeTime;
};

// Player's trade history is a collection of item trade records
USTRUCT(BlueprintType)
struct FMP_PlayerTradeHistory
{
    GENERATED_BODY()
    UPROPERTY() TArray<FMP_ItemTradeRecord> TradeRecords;
};


// Dynamic and market metadata for an item
USTRUCT(BlueprintType)
struct FMP_ItemMetadata
{
    GENERATED_BODY()

    // --- Top: non-float, non-array properties ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Metadata")
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Metadata")
    int32 TotalExistingItems = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Metadata")
    int32 TotalItemSold = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Metadata")
    int32 TotalItemVolume = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Metadata")
    int32 TradeVolume = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Metadata")
    bool IsPriceDynamic = false;

    // --- Second-last: floats ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Metadata")
    float BasePrice = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Metadata")
    float CurrentPrice = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Metadata")
    float DemandFactor = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Metadata")
    float SupplyFactor = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Metadata")
    float RarityFactor = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Metadata")
    float ControlFactor = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Metadata")
    float DecayRate = .0f;

    // --- Arrays always at the end ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Metadata")
    TArray<float> PriceHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Metadata")
    TSet<FString> AllOwners;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Metadata")
    TArray<FMP_ItemTradeRecord> TradeHistory;
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
struct FItemOwnershipIndexEntry
{
    GENERATED_BODY()

    UPROPERTY()
    FName ItemId;

    UPROPERTY()
    TSet<FString> PlayerIds;
};