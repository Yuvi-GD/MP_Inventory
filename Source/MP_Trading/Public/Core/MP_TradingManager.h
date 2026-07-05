// Copyright 2026 UVSquare. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Database/MP_InventoryStruct.h"
#include "MP_TradingManager.generated.h"

/**
 * Plugin-Driven Trading Manager
 * Supports P2P, NPC, Shop, System, Value/Item trading, and negotiation.
 */
//USTRUCT(BlueprintType)
//struct FTradeSession
//{
//    GENERATED_BODY()
//
//    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
//    FUniqueNetIdRepl PlayerA;
//    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
//    FUniqueNetIdRepl PlayerB;
//
//    // Add other properties for trade session here
//};
//
//USTRUCT(BlueprintType)
//struct FTradePlayer
//{
//    GENERATED_BODY()
//
//    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
//    FString PlayerA;
//
//    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
//    FString PlayerB;
//
//    // Add other properties for trade session here
//};
//
//USTRUCT(BlueprintType)
//struct FTradeOffers
//{
//    GENERATED_BODY()
//
//    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
//    FUniqueNetIdRepl OfferingPlayer;
//
//    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
//    TArray<FMP_InventoryItem> OfferedItems;
//};
//
//USTRUCT(BlueprintType)
//struct FItemList
//{
//    GENERATED_BODY()
//
//    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
//    FName SendItemID;
//
//    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
//    FName ReciveItemID;
//};
//
//USTRUCT(BlueprintType)
//struct FExchangeTradeOffer
//{
//    GENERATED_BODY()
//
//    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
//    FUniqueNetIdRepl OfferingPlayer;
//
//    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
//    TArray<FItemList> OfferedItems;
//};
//
//USTRUCT(BlueprintType)
//struct FPlayerTradeOffer
//{
//    GENERATED_BODY()
//
//    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
//    FString OfferingPlayer;
//
//    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
//    TArray<FItemList> OfferedItems;
//};

UCLASS(Blueprintable)
class MP_INVENTORY_API UMP_TradingManager : public UObject
{
	GENERATED_BODY()

public:
    UMP_TradingManager();

public:

    // Singleton pattern
    UFUNCTION(BlueprintCallable,BlueprintPure, Category = "MP_Trade|Manager", meta = (WorldContext = "WorldContextObject"))
    static UMP_TradingManager* GetTradingManager(UObject* WorldContextObject);

    // ---- TRADE REQUEST FLOW ----

    // Start a trade session between two UniqueIds
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Trade|Manager")
    void RequestTrade(const FString& PlayerAId, const FString& PlayerBId);

    // Player responds to trade request (accept/reject)
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Trade|Manager")
    void RespondTrade(const FString& TradeId, const bool bAccepted);

    // ---- OFFER FLOW ----

    // One player submits a trade offer (new offer or counter)
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Trade|Manager")
    void SubmitOffer(const FString& TradeId, const FMP_TradeOffer& Offer);

    // Other player responds to latest offer (accept/reject)
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Trade|Manager")
    void RespondOffer(const FString& TradeId, const FString& OfferId, bool bAccept);

    // End/cancel a trade (timeout or user action)
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "MP_Trade|Manager")
    void EndTrade(const FString& TradeId);

    // ---- HELPERS ----

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Trade|Manager")
    UMP_InventoryComponent* GetInventoryComponent(const FString& PlayerId) const;

	// Get the trade session for a given Trade ID
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Trade|Manager")
    FMP_TradeSession GetTradeSession(const FString& TradeId) const;

	// Get Trade ID for a player within an active trade session
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Trade|Manager")
    FString GetActiveTradeForPlayer(const FString& PlayerId) const;

	// Check if a player is currently in a trade session
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Trading")
    bool IsPlayerInTrade(const FString& PlayerId) const;

    UFUNCTION(BlueprintCallable, Category = "MP_Trade|Debug")
    void PrintAllActiveTradeIds() const;


protected:
    UPROPERTY()
    TMap<FString, FMP_TradeSession> ActiveTrades;

    UPROPERTY()
    TMap<FString, FMP_TradeSession> PendingTrades;

	// Payment processing is abstracted to Blueprint to allow for different implementations (currency, validation, etc)
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintAuthorityOnly, Category = "MP_Trade|Manager")
    bool ProcessPayment(const FString& PlayerId, float Amount, const FString& TradeId) const;

private:
    static TWeakObjectPtr<UMP_TradingManager> SingletonInstance;

    // Notify a participant (via their owner�s trade interface)
    // NotificationType use with switch statement, Session info, result, if need latest offer for particular intrface fetch from session last index
    void NotifyParticipant(const FString& PlayerId, FName NotificationType, const FMP_TradeSession& Session, const FMP_TradeOffer* Offer, const bool Result) const;

    // Find the other participant in a trade
    FString GetOtherParticipant(const FMP_TradeSession& Session, const FString& PlayerId) const;

    // Utility: Internal logic to complete a trade (remove/add items, analytics)
    bool CompleteTrade(const FString& TradeId, const FMP_TradeOffer& Offer);

    bool TransferItems(const FMP_TradeOffer& Offer, const FString& FromId, const FString& ToId) const;

    // Payment/validation helpers, Payment interface comes separate here
    //bool ValidateAndProcessPayment(const FString& PlayerId, float Amount, const FString& TradeId) const;
	//bool ProcessItemTransfer(const FString& TradeId, const FMP_InventoryItem& Item, const FString& PlayerId, const float Amount) const;
	// custom notification interface would be call anywhere no need specific function or can be another separte function for often call

//private:
//
//    UPROPERTY()
//    TMap<int32, FTradeOffers>OfferA;
//
//    UPROPERTY()
//    TMap<int32, FTradeOffers>OfferB;
//
//    UPROPERTY()
//    TMap<int32, FExchangeTradeOffer>PlayerOfferA;
//
//    UPROPERTY()
//    TMap<int32, FExchangeTradeOffer>PlayerOfferB;
//
//    UPROPERTY()
//    TMap<int32, FPlayerTradeOffer>PlayerTradeA;
//
//    UPROPERTY()
//    TMap<int32, FPlayerTradeOffer>PlayerTradeB;
//
//
//    UPROPERTY()
//    TMap<int32,FTradeSession>ActiveTrades;
//
//    UPROPERTY()
//    TMap<int32, FTradePlayer>TradesList;

};