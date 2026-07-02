// Copyright 2026 UVSquare. All Rights Reserved.


#include "Core/MP_AnalyticsManager.h"
#include "Engine/World.h"

TWeakObjectPtr<UMP_AnalyticsManager> UMP_AnalyticsManager::SingletonInstance = nullptr;

UMP_AnalyticsManager* UMP_AnalyticsManager::Get(UObject* WorldContextObject)
{
    if (SingletonInstance.IsValid())
        return SingletonInstance.Get();

    UObject* Outer = WorldContextObject ? WorldContextObject : GetTransientPackage();
    UMP_AnalyticsManager* NewInstance = NewObject<UMP_AnalyticsManager>(Outer);
    //NewInstance->AddToRoot();
    SingletonInstance = NewInstance;
    return NewInstance;
}

// ------ Trade Session Management ------

void UMP_AnalyticsManager::AddTradeSession(const FString& TradeId, const FString& PlayerAId, const FString& PlayerBId)
{
    FMP_TradeSession Session;
    Session.PlayerAId = PlayerAId;
    Session.PlayerBId = PlayerBId;
    TradesList.Add(TradeId, Session);
}

FMP_TradeSession UMP_AnalyticsManager::GetTradeSession(const FString& TradeId) const
{
    const FMP_TradeSession* Found = TradesList.Find(TradeId);
    return Found ? *Found : FMP_TradeSession();
}

// ------ Trade Record and History Management ------

void UMP_AnalyticsManager::AddPlayerTradeRecord(const FString& PlayerId, const FMP_ItemTradeRecord& Record)
{
    PlayerTradeHistory.FindOrAdd(PlayerId).TradeRecords.Add(Record);

    FMP_ItemMetadata& Meta = ItemMetadataMap.FindOrAdd(Record.ItemID); // Always valid ref

    Meta.TradeHistory.Add(Record);       // Update item metadata as well
    Meta.AllOwners.Add(PlayerId);        // Track all owners for item
    Meta.TradeVolume += 1;               // Update trade volume for item
    CalculateAndUpdateItemMetadata(Record.ItemID); // Recalculate metadata after adding trade
}


FMP_PlayerTradeHistory UMP_AnalyticsManager::GetPlayerTradeHistory(const FString& PlayerId) const
{
    const FMP_PlayerTradeHistory* Found = PlayerTradeHistory.Find(PlayerId);
    return Found ? *Found : FMP_PlayerTradeHistory();
}

FMP_ItemTradeRecord UMP_AnalyticsManager::GetLastTradeForPlayer(const FString& PlayerId) const
{
    const FMP_PlayerTradeHistory* Found = PlayerTradeHistory.Find(PlayerId);
    if (Found && Found->TradeRecords.Num() > 0)
        return Found->TradeRecords.Last();
    return FMP_ItemTradeRecord();
}


void UMP_AnalyticsManager::AddItemTradeRecord(const FName ItemID, const FMP_ItemTradeRecord& Record)
{
    ItemTradeHistory.FindOrAdd(ItemID).TradeRecords.Add(Record);
}

FMP_PlayerTradeHistory UMP_AnalyticsManager::GetItemTradeHistory(const FName ItemID) const
{
    const FMP_PlayerTradeHistory* Found = ItemTradeHistory.Find(ItemID);
    return Found ? *Found : FMP_PlayerTradeHistory();
}

FMP_ItemTradeRecord UMP_AnalyticsManager::GetLastTradeForItem(const FName ItemID) const
{
    const FMP_PlayerTradeHistory* Found = ItemTradeHistory.Find(ItemID);
    if (Found && Found->TradeRecords.Num() > 0)
        return Found->TradeRecords.Last();
    return FMP_ItemTradeRecord();
}


// ------ Metadata Analytics ------

void UMP_AnalyticsManager::AddOrUpdateItemMetadata(const FName ItemID, const FMP_ItemMetadata& Metadata)
{
    ItemMetadataMap.Add(ItemID, Metadata);
}

FMP_ItemMetadata UMP_AnalyticsManager::GetItemMetadata(const FName ItemID) const
{
    const FMP_ItemMetadata* Found = ItemMetadataMap.Find(ItemID);
    return Found ? *Found : FMP_ItemMetadata();
}


// ------ Analytics Calculation ------

void UMP_AnalyticsManager::CalculateAndUpdateItemMetadata(const FName ItemID)
{
    FMP_ItemMetadata* Meta = ItemMetadataMap.Find(ItemID);
    if (!Meta)
        return;

    // Example: calculate demand/supply, price, etc.
    int32 NumTrades = Meta->TradeHistory.Num();

    float DemandFactor = (Meta->TotalItemSold > 0)
        ? (float(NumTrades) / float(Meta->TotalItemSold))
        : 0.f;
    float SupplyFactor = (Meta->TotalExistingItems > 0)
        ? (float(Meta->TotalItemVolume) / float(Meta->TotalExistingItems))
        : 0.f;

    // Final price formula
    float NewPrice = Meta->CurrentPrice * (1.f + (DemandFactor * Meta->ControlFactor) - (SupplyFactor * Meta->DecayRate));
    Meta->CurrentPrice = FMath::Clamp(NewPrice, 0.1f, 999999.0f);
    Meta->PriceHistory.Add(Meta->CurrentPrice);

    // Optionally update any other analytics fields here...
}

// ------ Utility/Queries ------

int32 UMP_AnalyticsManager::GetTradeVolumeForItem(const FName ItemID, int32 TimeWindowSeconds) const
{
    const FMP_PlayerTradeHistory* History = ItemTradeHistory.Find(ItemID);
    if (!History) return 0;

    int32 Volume = 0;
    FDateTime Now = FDateTime::UtcNow();
    for (const FMP_ItemTradeRecord& Rec : History->TradeRecords)
    {
        if ((Now - Rec.TradeTime).GetTotalSeconds() <= TimeWindowSeconds)
            Volume++;
    }
    return Volume;
}

TSet<FString> UMP_AnalyticsManager::GetUniqueOwnersForItem(const FName ItemID) const
{
    const FMP_ItemMetadata* Meta = ItemMetadataMap.Find(ItemID);
    return Meta ? Meta->AllOwners : TSet<FString>();
}

void UMP_AnalyticsManager::PrintPlayerTradeHistory(const FString& PlayerId) const
{
    const FMP_PlayerTradeHistory* History = PlayerTradeHistory.Find(PlayerId);
    if (!History) return;

    UE_LOG(LogTemp, Log, TEXT("Trade history for player %s:"), *PlayerId);
    for (const FMP_ItemTradeRecord& Rec : History->TradeRecords)
    {
        UE_LOG(LogTemp, Log, TEXT("TradeId: %s | Item: %s | Qty: %d | Price: %.2f | Type: %d | Time: %s"),
            *Rec.TradeId, *Rec.ItemID.ToString(), Rec.Quantity, Rec.TradePrice, (int32)Rec.TradeType, *Rec.TradeTime.ToString());
    }
}

void UMP_AnalyticsManager::PrintItemTradeHistory(const FName ItemID) const
{
    const FMP_PlayerTradeHistory* History = ItemTradeHistory.Find(ItemID);
    if (!History) return;

    UE_LOG(LogTemp, Log, TEXT("Trade history for item %s:"), *ItemID.ToString());
    for (const FMP_ItemTradeRecord& Rec : History->TradeRecords)
    {
        UE_LOG(LogTemp, Log, TEXT("TradeId: %s | Qty: %d | Price: %.2f | Type: %d | Time: %s"),
            *Rec.TradeId, Rec.Quantity, Rec.TradePrice, (int32)Rec.TradeType, *Rec.TradeTime.ToString());
    }
}

// Internal: update all item metadata (analytics, price, demand, supply, etc)
void UMP_AnalyticsManager::UpdateItemMetadata(const FMP_ItemTradeRecord& Record, const FString& FromPlayerId, const FString& ToPlayerId)
{
    FMP_ItemMetadata& Meta = ItemMetadataMap.FindOrAdd(Record.ItemID);

    // Update trade history (capped size if needed)
    Meta.TradeHistory.Add(Record);

    // Update owners
    Meta.AllOwners.Add(FromPlayerId);
    Meta.AllOwners.Add(ToPlayerId);

    // Update total volume, trade count, etc.
    Meta.TotalExistingItems += Record.Quantity;
    Meta.TotalItemSold += Record.Quantity;
    Meta.TradeVolume += 1;

    // Update price history and recalculate price if needed
    if (Meta.IsPriceDynamic)
    {
        float DemandFactor = Meta.TotalItemSold > 0 ? ((float)Meta.TradeVolume / Meta.TotalItemSold) : 0.0f;
        float SupplyFactor = Meta.TotalExistingItems > 0 ? ((float)Meta.TotalItemVolume / Meta.TotalExistingItems) : 0.0f;
        float NewPrice = Meta.CurrentPrice * (1.f + (DemandFactor * Meta.ControlFactor) - (SupplyFactor * Meta.DecayRate));
        Meta.CurrentPrice = FMath::Clamp(NewPrice, 0.1f, 999999.0f);
        Meta.PriceHistory.Add(Meta.CurrentPrice);
    }
}
