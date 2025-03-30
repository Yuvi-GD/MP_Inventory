// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Database/MP_InventoryStruct.h"
#include "MP_TradingManager.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ETradeState : uint8
{
    Idle,
    Initiated,
    Offered,
    Accepted,
    Completed,
    Rejected
};

USTRUCT(BlueprintType)
struct FTradeOffer
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
    FString OfferingPlayer;

    UPROPERTY(BlueprintReadWrite, Category = "MP_Trading")
    TArray<FMP_InventoryStruct> OfferedItems;
};

UCLASS(Blueprintable)
class MP_INVENTORY_API UMP_TradingManager : public UObject
{
	GENERATED_BODY()

public:
    UMP_TradingManager();

    UFUNCTION(BlueprintCallable, Category = "MP_Trading")
    void InitiateTrade(FString PlayersA, FString PlayersB);

    UFUNCTION(BlueprintCallable, Category = "MP_Trading")
    void MakeOffer(FString Player, FMP_InventoryStruct Item);

    UFUNCTION(BlueprintCallable, Category = "MP_Trading")
    void AcceptOffer();

    UFUNCTION(BlueprintCallable, Category = "MP_Trading")
    void RejectOffer();

    UFUNCTION(BlueprintCallable, Category = "MP_Trading")
    void EndTrade();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Trading")
    ETradeState GetTradeState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Trading")
    FTradeOffer GetOfferFromPlayer(FString Player) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Trading")
    bool IsPlayerInTrade(FString Player) const;

private:
    UPROPERTY()
    FString PlayerA;

    UPROPERTY()
    FString PlayerB;

    UPROPERTY()
    FTradeOffer OfferA;

    UPROPERTY()
    FTradeOffer OfferB;

    UPROPERTY()
    ETradeState CurrentState;

    void ResetTrade();
	
};
