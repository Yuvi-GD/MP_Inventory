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

/**
 * Defines a single transactional bid or proposal within an active trade session.
 * Encapsulates the initiator's ID, the exact items and currency offered, and the
 * current lifecycle state of this specific offer (e.g., pending, accepted).
 */
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

/**
 * Maintains the complete state and lifecycle of a peer-to-peer trade negotiation.
 * Tracks participants, the global status of the session, and the chronological
 * sequence of all offers and counter-offers made during the exchange.
 */
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

/**
 * An immutable ledger entry representing a completed market transaction.
 * Recorded by the AnalyticsManager to track historical pricing, volume, and
 * the exact flow of items across the server economy.
 */
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

/**
 * A specialized container for aggregating a specific player's complete trade history.
 * Designed to be serialized or queried for player-specific economic analytics.
 */
USTRUCT(BlueprintType)
struct MP_TRADING_API FMP_PlayerTradeHistory
{
    GENERATED_BODY()
    UPROPERTY() TArray<FMP_ItemTradeRecord> TradeRecords;
};


/**
 * Comprehensive market metadata for a specific item definition.
 * Tracks global supply, demand, dynamic pricing adjustments, and full historical
 * trade data to power localized economies or server-wide auction houses.
 */
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
