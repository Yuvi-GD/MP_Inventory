#pragma once

#include "CoreMinimal.h"
#include "Data/MP_InventoryStruct.h"
#include "MP_TradingStructs.generated.h"

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

// Trade offer submitted by a participant in a session.
USTRUCT(BlueprintType)
struct MP_TRADING_API FMP_TradeOffer
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
struct MP_TRADING_API FMP_TradeSession
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
struct MP_TRADING_API FMP_ItemTradeRecord
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
struct MP_TRADING_API FMP_PlayerTradeHistory
{
    GENERATED_BODY()
    UPROPERTY() TArray<FMP_ItemTradeRecord> TradeRecords;
};


// Dynamic and market metadata for an item
USTRUCT(BlueprintType)
struct MP_TRADING_API FMP_ItemMetadata
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
