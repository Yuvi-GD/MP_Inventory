// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Database/MP_InventoryStruct.h"
#include "UObject/NoExportTypes.h"
#include "MP_InventoryAnalytics.generated.h"

/**
 * 
 */
UCLASS()
class MP_INVENTORY_API UMP_InventoryAnalytics : public UObject
{
	GENERATED_BODY()
	
public:
    // Singleton getter
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Analytics", meta = (WorldContext = "WorldContextObject"))
    static UMP_InventoryAnalytics* Get(UObject* WorldContextObject);

    // Trade session management
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Analytics")
    void AddTradeSession(const FString& TradeId, const FString& PlayerAId, const FString& PlayerBId);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Analytics")
    FMP_TradeSession GetTradeSession(const FString& TradeId) const;

    // Trade records and history
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Analytics")
    void AddPlayerTradeRecord(const FString& PlayerId, const FMP_ItemTradeRecord& Record);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Analytics")
    FMP_PlayerTradeHistory GetPlayerTradeHistory(const FString& PlayerId) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Analytics")
    FMP_ItemTradeRecord GetLastTradeForPlayer(const FString& PlayerId) const;

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Analytics")
    void AddItemTradeRecord(const FName ItemID, const FMP_ItemTradeRecord& Record);

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Analytics")
    FMP_PlayerTradeHistory GetItemTradeHistory(const FName ItemID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Analytics")
    FMP_ItemTradeRecord GetLastTradeForItem(const FName ItemID) const;


    // Metadata analytics
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Analytics")
    void AddOrUpdateItemMetadata(const FName ItemID, const FMP_ItemMetadata& Metadata);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Analytics")
    FMP_ItemMetadata GetItemMetadata(const FName ItemID) const;

    // Analytics calculation (price, demand, supply, volume, etc)
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Analytics")
    void CalculateAndUpdateItemMetadata(const FName ItemID);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Analytics")
    int32 GetTradeVolumeForItem(const FName ItemID, int32 TimeWindowSeconds) const;

        UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MP_Inventory|Analytics")
    TSet<FString> GetUniqueOwnersForItem(const FName ItemID) const;

    // Print/debug
    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Analytics")
    void PrintPlayerTradeHistory(const FString& PlayerId) const;

    UFUNCTION(BlueprintCallable, Category = "MP_Inventory|Analytics")
    void PrintItemTradeHistory(const FName ItemID) const;

protected:
    // PlayerId → All trades for this player
    UPROPERTY()
    TMap<FString, FMP_PlayerTradeHistory> PlayerTradeHistory;

    // ItemID → All trades involving this item
    UPROPERTY()
    TMap<FName, FMP_PlayerTradeHistory> ItemTradeHistory;

    // ItemID → Metadata/analytics (price, demand, trade history, owners, etc)
    UPROPERTY()
    TMap<FName, FMP_ItemMetadata> ItemMetadataMap;

    // TradeId → PlayerId, for quick lookup if needed
    UPROPERTY()
    TMap<FString, FMP_TradeSession> TradesList;

private:
    static TWeakObjectPtr<UMP_InventoryAnalytics> SingletonInstance;

    // Internal: calculate and update item metadata (price, demand, supply, etc)
    void UpdateItemMetadata(const FMP_ItemTradeRecord& Record, const FString& FromPlayerId, const FString& ToPlayerId);
};
