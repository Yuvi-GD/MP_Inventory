// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Database/MP_InventoryStruct.h"
#include "MP_TradingManager.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FTradeSession
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
    FUniqueNetIdRepl PlayerA;
    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
    FUniqueNetIdRepl PlayerB;

    // Add other properties for trade session here
};

USTRUCT(BlueprintType)
struct FTradePlayer
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
    FString PlayerA;

    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
    FString PlayerB;

    // Add other properties for trade session here
};

USTRUCT(BlueprintType)
struct FTradeOffer
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
    FUniqueNetIdRepl OfferingPlayer;

    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
    TArray<FMP_InventoryItem> OfferedItems;
};

USTRUCT(BlueprintType)
struct FItemList
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
    FName SendItemID;

    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
    FName ReciveItemID;
};

USTRUCT(BlueprintType)
struct FExchangeTradeOffer
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
    FUniqueNetIdRepl OfferingPlayer;

    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
    TArray<FItemList> OfferedItems;
};

USTRUCT(BlueprintType)
struct FPlayerTradeOffer
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
    FString OfferingPlayer;

    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
    TArray<FItemList> OfferedItems;
};

UCLASS(Blueprintable)
class MP_INVENTORY_API UMP_TradingManager : public UObject
{
	GENERATED_BODY()

public:
    UMP_TradingManager();

public:
    UFUNCTION(BlueprintCallable, Category = "MP_Trading")
    void RequestTrade(APlayerState* PlayerA, APlayerState* PlayerB);

    UFUNCTION(BlueprintCallable, Category = "MP_Trading")
    int32  AcceptTrade(APlayerState* PlayerA, APlayerState* PlayerB);

    UFUNCTION(BlueprintCallable, Category = "MP_Trading")
    void RequestItemOffer(int32 Trade_ID, APlayerState* PlayerID, FName ItemID);
    
    UFUNCTION(BlueprintCallable, Category = "MP_Trading")
    void RequestExchageOffer(int32 Trade_ID, APlayerState* PlayerID, FName ItemIDA , FName ItemIDB);

    UFUNCTION(BlueprintCallable, Category = "MP_Trading")
    void AcceptExchageOffer(int32 Trade_ID, APlayerState* PlayerID, FName ItemIDA, FName ItemIDB);

    UFUNCTION(BlueprintCallable, Category = "MP_Trading")
    void RejectOffer(int32 Trade_ID, FUniqueNetIdRepl PlayerID);

    UFUNCTION(BlueprintCallable, Category = "MP_Trading")
    void EndTrade(int32 Trade_ID);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Trading")
    FPlayerTradeOffer GetOfferFromPlayer(int32 Trade_ID, FString Player) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Trading")
    bool IsPlayerInTrade(FUniqueNetIdRepl PlayerID) const;

private:

    UPROPERTY()
    TMap<int32, FTradeOffer>OfferA;

    UPROPERTY()
    TMap<int32, FTradeOffer>OfferB;

    UPROPERTY()
    TMap<int32, FExchangeTradeOffer>PlayerOfferA;

    UPROPERTY()
    TMap<int32, FExchangeTradeOffer>PlayerOfferB;

    UPROPERTY()
    TMap<int32, FPlayerTradeOffer>PlayerTradeA;

    UPROPERTY()
    TMap<int32, FPlayerTradeOffer>PlayerTradeB;


    UPROPERTY()
    TMap<int32,FTradeSession>ActiveTrades;

    UPROPERTY()
    TMap<int32, FTradePlayer>TradesList;

};
