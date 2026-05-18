// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/MP_InventoryAnalytics.h"
#include "Engine/World.h"

TWeakObjectPtr<UMP_InventoryAnalytics> UMP_InventoryAnalytics::SingletonInstance = nullptr;

UMP_InventoryAnalytics* UMP_InventoryAnalytics::Get(UObject* WorldContextObject)
{
    if (SingletonInstance.IsValid())
        return SingletonInstance.Get();

    UObject* Outer = WorldContextObject ? WorldContextObject : GetTransientPackage();
    UMP_InventoryAnalytics* NewInstance = NewObject<UMP_InventoryAnalytics>(Outer);
    //NewInstance->AddToRoot();
    SingletonInstance = NewInstance;
    return NewInstance;
}

// ------ Trade Session Management ------

void UMP_InventoryAnalytics::AddTradeSession(const FString& TradeId, const FString& PlayerAId, const FString& PlayerBId)
{
    FMP_TradeSession Session;
    Session.PlayerAId = PlayerAId;
    Session.PlayerBId = PlayerBId;
    TradesList.Add(TradeId, Session);
}

FMP_TradeSession UMP_InventoryAnalytics::GetTradeSession(const FString& TradeId) const
{
    const FMP_TradeSession* Found = TradesList.Find(TradeId);
    return Found ? *Found : FMP_TradeSession();
}

// ------ Trade Record and History Management ------

void UMP_InventoryAnalytics::AddPlayerTradeRecord(const FString& PlayerId, const FMP_ItemTradeRecord& Record)
{
    PlayerTradeHistory.FindOrAdd(PlayerId).TradeRecords.Add(Record);

    FMP_ItemMetadata& Meta = ItemMetadataMap.FindOrAdd(Record.ItemID); // Always valid ref

    Meta.TradeHistory.Add(Record);       // Update item metadata as well
    Meta.AllOwners.Add(PlayerId);        // Track all owners for item
    Meta.TradeVolume += 1;               // Update trade volume for item
    CalculateAndUpdateItemMetadata(Record.ItemID); // Recalculate metadata after adding trade
}


FMP_PlayerTradeHistory UMP_InventoryAnalytics::GetPlayerTradeHistory(const FString& PlayerId) const
{
    const FMP_PlayerTradeHistory* Found = PlayerTradeHistory.Find(PlayerId);
    return Found ? *Found : FMP_PlayerTradeHistory();
}

FMP_ItemTradeRecord UMP_InventoryAnalytics::GetLastTradeForPlayer(const FString& PlayerId) const
{
    const FMP_PlayerTradeHistory* Found = PlayerTradeHistory.Find(PlayerId);
    if (Found && Found->TradeRecords.Num() > 0)
        return Found->TradeRecords.Last();
    return FMP_ItemTradeRecord();
}


void UMP_InventoryAnalytics::AddItemTradeRecord(const FName ItemID, const FMP_ItemTradeRecord& Record)
{
    ItemTradeHistory.FindOrAdd(ItemID).TradeRecords.Add(Record);
}

FMP_PlayerTradeHistory UMP_InventoryAnalytics::GetItemTradeHistory(const FName ItemID) const
{
    const FMP_PlayerTradeHistory* Found = ItemTradeHistory.Find(ItemID);
    return Found ? *Found : FMP_PlayerTradeHistory();
}

FMP_ItemTradeRecord UMP_InventoryAnalytics::GetLastTradeForItem(const FName ItemID) const
{
    const FMP_PlayerTradeHistory* Found = ItemTradeHistory.Find(ItemID);
    if (Found && Found->TradeRecords.Num() > 0)
        return Found->TradeRecords.Last();
    return FMP_ItemTradeRecord();
}


// ------ Metadata Analytics ------

void UMP_InventoryAnalytics::AddOrUpdateItemMetadata(const FName ItemID, const FMP_ItemMetadata& Metadata)
{
    ItemMetadataMap.Add(ItemID, Metadata);
}

FMP_ItemMetadata UMP_InventoryAnalytics::GetItemMetadata(const FName ItemID) const
{
    const FMP_ItemMetadata* Found = ItemMetadataMap.Find(ItemID);
    return Found ? *Found : FMP_ItemMetadata();
}


// ------ Analytics Calculation ------

void UMP_InventoryAnalytics::CalculateAndUpdateItemMetadata(const FName ItemID)
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

int32 UMP_InventoryAnalytics::GetTradeVolumeForItem(const FName ItemID, int32 TimeWindowSeconds) const
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

TSet<FString> UMP_InventoryAnalytics::GetUniqueOwnersForItem(const FName ItemID) const
{
    const FMP_ItemMetadata* Meta = ItemMetadataMap.Find(ItemID);
    return Meta ? Meta->AllOwners : TSet<FString>();
}

void UMP_InventoryAnalytics::PrintPlayerTradeHistory(const FString& PlayerId) const
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

void UMP_InventoryAnalytics::PrintItemTradeHistory(const FName ItemID) const
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
void UMP_InventoryAnalytics::UpdateItemMetadata(const FMP_ItemTradeRecord& Record, const FString& FromPlayerId, const FString& ToPlayerId)
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
