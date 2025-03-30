// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/MP_TradingManager.h"
#include "Subsystem/MP_InventorySubsystem.h"

UMP_TradingManager::UMP_TradingManager()
{
    CurrentState = ETradeState::Idle;
    OfferA.OfferingPlayer = "";
    OfferB.OfferingPlayer = "";
}

void UMP_TradingManager::InitiateTrade(FString PlayersA, FString PlayersB)
{
    if (CurrentState != ETradeState::Idle || PlayerA.IsEmpty() || PlayerB.IsEmpty() || PlayerA == PlayerB)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::InitiateTrade - Invalid trade initiation"));
        return;
    }

    this->PlayerA = PlayerA;
    this->PlayerB = PlayerB;
    OfferA.OfferingPlayer = PlayerA;
    OfferB.OfferingPlayer = PlayerB;
    CurrentState = ETradeState::Initiated;

    UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::InitiateTrade - Trade initiated between %s and %s"), *PlayerA, *PlayerB);
}

void UMP_TradingManager::MakeOffer(FString Player, FMP_InventoryStruct Item)
{
    if (CurrentState != ETradeState::Initiated && CurrentState != ETradeState::Offered)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::MakeOffer - Trade not initiated"));
        return;
    }

    if (Player != PlayerA && Player != PlayerB)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::MakeOffer - Player %s not in trade"), *Player);
        return;
    }

    if (Item.ItemID.IsNone() || Item.Quantity <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::MakeOffer - Invalid item offered by %s"), *Player);
        return;
    }

    if (Player == PlayerA)
    {
        OfferA.OfferedItems.Add(Item);
    }
    else
    {
        OfferB.OfferedItems.Add(Item);
    }

    CurrentState = ETradeState::Offered;
    UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::MakeOffer - %s offered item %s"), *Player, *Item.ItemID.ToString());
}

void UMP_TradingManager::AcceptOffer()
{
    if (CurrentState != ETradeState::Offered)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::AcceptOffer - No offer to accept"));
        return;
    }

    CurrentState = ETradeState::Accepted;

    // Here we’d transfer items via Subsystem or Component—placeholder for now
    if (UMP_InventorySubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UMP_InventorySubsystem>())
    {
        for (const FMP_InventoryStruct& Item : OfferA.OfferedItems)
        {
            Subsystem->AddItem(Item); // Placeholder—needs PlayerB’s inventory
            Subsystem->RemoveItem(Item, Item.Quantity); // Placeholder—needs PlayerA’s inventory
        }
        for (const FMP_InventoryStruct& Item : OfferB.OfferedItems)
        {
            Subsystem->AddItem(Item); // Placeholder—needs PlayerA’s inventory
            Subsystem->RemoveItem(Item, Item.Quantity); // Placeholder—needs PlayerB’s inventory
        }
    }

    UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::AcceptOffer - Trade accepted"));
    EndTrade();
}

void UMP_TradingManager::RejectOffer()
{
    if (CurrentState != ETradeState::Offered)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::RejectOffer - No offer to reject"));
        return;
    }

    CurrentState = ETradeState::Rejected;
    UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::RejectOffer - Trade rejected"));
    ResetTrade();
}

void UMP_TradingManager::EndTrade()
{
    if (CurrentState == ETradeState::Accepted || CurrentState == ETradeState::Completed)
    {
        CurrentState = ETradeState::Completed;
        UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::EndTrade - Trade completed"));
    }
    ResetTrade();
}

FTradeOffer UMP_TradingManager::GetOfferFromPlayer(FString Player) const
{
    if (Player == PlayerA) return OfferA;
    if (Player == PlayerB) return OfferB;
    return FTradeOffer();
}

bool UMP_TradingManager::IsPlayerInTrade(FString Player) const
{
    return Player == PlayerA || Player == PlayerB;
}

void UMP_TradingManager::ResetTrade()
{
    PlayerA = "";
    PlayerB = "";
    OfferA.OfferedItems.Empty();
    OfferB.OfferedItems.Empty();
    CurrentState = ETradeState::Idle;
}

