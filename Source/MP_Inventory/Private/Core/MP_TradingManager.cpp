// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/MP_TradingManager.h"
#include "Framework/MP_InventorySubsystem.h"
#include "Framework/MP_Inventory_PlayerState.h"
//#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Framework/MP_Inventory_BFL.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/MP_Inventory_PlayerState_I.h"
#include "Interfaces/MP_Inventory_PlayerController_I.h"


UMP_TradingManager::UMP_TradingManager()
{
    OfferA;
    OfferB;
    ActiveTrades;
}

void UMP_TradingManager::RequestTrade(APlayerState* PlayerA, APlayerState* PlayerB)
{
    if (PlayerA == PlayerB)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::InitiateTrade - Invalid trade initiation: Both Player ID is Same"));
        return ;
    }
    if (!PlayerB || !PlayerA)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::InitiateTrade - Invalid trade initiation: One of the Player ID is not Valid"));
        return ;
    }

    AMP_Inventory_PlayerState *PlayerStateA = Cast<AMP_Inventory_PlayerState>(PlayerA);
    AMP_Inventory_PlayerState *PlayerStateB = Cast<AMP_Inventory_PlayerState>(PlayerB);

    if(PlayerStateA && PlayerStateB)
    {
        PlayerStateB->OnRequestTrade(PlayerStateA);
    }
    else
    {
        if (PlayerB->Implements<UMP_Inventory_PlayerState_I>())
        {
            IMP_Inventory_PlayerState_I::Execute_OnRequestTrade(PlayerB, PlayerA);
        }
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::PlayerState Check == %s "), *PlayerB->GetName());
    }
    return;
}

int32  UMP_TradingManager::AcceptTrade(APlayerState* PlayerA, APlayerState* PlayerB)
{
    if (PlayerA == PlayerB)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::InitiateTrade - Invalid trade initiation: Both Player ID is Same"));
        return -1;
    }
    if (!PlayerB || !PlayerA)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::InitiateTrade - Invalid trade initiation: One of the Player ID is not Valid"));
        return -1;
    }

    AMP_Inventory_PlayerState* PlayerStateA = Cast<AMP_Inventory_PlayerState>(PlayerA);
    AMP_Inventory_PlayerState* PlayerStateB = Cast<AMP_Inventory_PlayerState>(PlayerB);
    
    int32 Tradeid;

    
    /* Only for check that trade has been done before
    TArray<FTradePlayer> Playerlist;
    TradesList.GenerateValueArray(Playerlist);
    
    int32 Tempid = -1;
    
    for (FTradePlayer Playerid: Playerlist)
    {
        Tempid++;
        if (Playerid.PlayerA == PlayerStateA->PersistentPlayerId || Playerid.PlayerB == PlayerStateB->PersistentPlayerId)
        {
            Tradeid =Tempid;
            break;
        }
    }*/

    if (PlayerStateA && PlayerStateB)
    {
        Tradeid = TradesList.GetMaxIndex() + 1;   
        FTradePlayer PlayersId;
        PlayersId.PlayerA = PlayerStateA->PersistentPlayerId;
        PlayersId.PlayerB = PlayerStateB->PersistentPlayerId;

        PlayerStateA->OnAcceptTrade(Tradeid, PlayerStateB);
        TradesList.Add(Tradeid, PlayersId);
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::InitiateTrade - Trade initiated between Player A and Player B With Trade id %d"), Tradeid);
    }
    else
    {
        Tradeid = ActiveTrades.GetMaxIndex() + 1;
        FTradeSession PlayersId;

        PlayersId.PlayerA = PlayerA->GetUniqueId();
        PlayersId.PlayerB = PlayerB->GetUniqueId();

        if (PlayerA->Implements<UMP_Inventory_PlayerState_I>())
        {
            IMP_Inventory_PlayerState_I::Execute_OnAcceptTrade(PlayerA, Tradeid, PlayerB);
        }
        ActiveTrades.Add(Tradeid,PlayersId);
        UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::InitiateTrade - Trade initiated between Player A and Player B"));
    }
    return Tradeid;

    //int32 Tradeid = 11;
}

void UMP_TradingManager::RequestItemOffer(int32 Trade_ID, APlayerState* PlayerID, FName ItemID)
{
    //ActiveTrades.Find(Trade_ID);
    if (!ActiveTrades.Contains(Trade_ID))
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::RequestItemOffer - Player %d not in trade"), Trade_ID);
        return;
    }

    if (ItemID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::RequestItemOffer - Invalid item offered by %d"), Trade_ID);
        return;
    }

    FItemList Items;
    Items.SendItemID = ItemID;

    AMP_Inventory_PlayerState* PlayerStateIDA = Cast<AMP_Inventory_PlayerState>(PlayerID);
    if (PlayerStateIDA)
    {
        FPlayerTradeOffer TCurrentOffer;
        FTradePlayer CurrentTrade;
        CurrentTrade = TradesList[Trade_ID];
        if (CurrentTrade.PlayerA == PlayerStateIDA->PersistentPlayerId) // checking TradeID with Player existance
        {
            if (PlayerTradeA.Contains(Trade_ID))// Checking TradeID Existe in OfferContainer
            {
                if (PlayerTradeA[Trade_ID].OfferingPlayer == PlayerStateIDA->PersistentPlayerId) // Conforming Player
                {
                    PlayerTradeA[Trade_ID].OfferedItems.Add(Items);
                }
            }
            else
            {
                TCurrentOffer.OfferingPlayer = PlayerStateIDA->PersistentPlayerId;
                TCurrentOffer.OfferedItems.Add(Items);
                PlayerTradeA.Add(Trade_ID, TCurrentOffer);
            }
            AMP_Inventory_PlayerState* PlayerStateIDB = UMP_Inventory_BFL::FindPlayerStateByPersistentPlayerId(this, CurrentTrade.PlayerB);
            PlayerStateIDB->OnRequestItemOffer(Trade_ID, PlayerStateIDA, ItemID);
        }
        else if (CurrentTrade.PlayerB == PlayerStateIDA->PersistentPlayerId)
        {

            if (PlayerTradeB.Contains(Trade_ID))
            {
                if (PlayerTradeB[Trade_ID].OfferingPlayer == PlayerStateIDA->PersistentPlayerId)
                {
                    PlayerTradeB[Trade_ID].OfferedItems.Add(Items);
                }
            }
            else
            {
                TCurrentOffer.OfferingPlayer = PlayerStateIDA->PersistentPlayerId;
                TCurrentOffer.OfferedItems.Add(Items);
                PlayerTradeB.Add(Trade_ID, TCurrentOffer);
            }

            AMP_Inventory_PlayerState* PlayerStateIDB = UMP_Inventory_BFL::FindPlayerStateByPersistentPlayerId(this, CurrentTrade.PlayerA);
            PlayerStateIDB->OnRequestItemOffer(Trade_ID, PlayerStateIDA, ItemID);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Player ID is not matching with any of the PlayerID in the Given TradeID"));
        }
    }
    else
    {
        FExchangeTradeOffer TCurrentOffer;
        FTradeSession CurrentSession;
        CurrentSession = ActiveTrades[Trade_ID];
        if (CurrentSession.PlayerA == PlayerID->GetUniqueId()) // checking TradeID with Player existance
        {
            if (PlayerOfferA.Contains(Trade_ID)) // Checking TradeID Existe in OfferContainer
            {
                if (PlayerOfferA[Trade_ID].OfferingPlayer == PlayerID->GetUniqueId())// Conforming Player
                {
                    PlayerOfferA[Trade_ID].OfferedItems.Add(Items);
                }
            }
            else
            {
                TCurrentOffer.OfferingPlayer = PlayerID->GetUniqueId();
                TCurrentOffer.OfferedItems.Add(Items);
                PlayerOfferA.Add(Trade_ID, TCurrentOffer);
            }

            FUniqueNetIdRepl PlayerIDB;
            PlayerIDB = CurrentSession.PlayerB.GetUniqueNetId();

            APlayerState* PlayerState = UGameplayStatics::GetPlayerStateFromUniqueNetId(this, PlayerIDB);
            if (PlayerState->Implements<UMP_Inventory_PlayerState_I>())
            {
                IMP_Inventory_PlayerState_I::Execute_OnRequestItemOffer(PlayerState, Trade_ID, PlayerID, ItemID);
            }
        }
        else if (CurrentSession.PlayerB == PlayerID->GetUniqueId()) // checking TradeID with Player existance
        {

            if (PlayerOfferB.Contains(Trade_ID)) // Checking TradeID Existe in OfferContainer
            {
                if (PlayerOfferB[Trade_ID].OfferingPlayer == PlayerID->GetUniqueId())// Conforming Player
                {
                    PlayerOfferB[Trade_ID].OfferedItems.Add(Items);
                }
            }
            else
            {
                TCurrentOffer.OfferingPlayer = PlayerID->GetUniqueId();
                TCurrentOffer.OfferedItems.Add(Items);
                PlayerOfferB.Add(Trade_ID, TCurrentOffer);
            }

            FUniqueNetIdRepl PlayerIDB;
            PlayerIDB = CurrentSession.PlayerB.GetUniqueNetId();

            APlayerState* PlayerState = UGameplayStatics::GetPlayerStateFromUniqueNetId(this, PlayerIDB);
            if (PlayerState->Implements<UMP_Inventory_PlayerState_I>())
            {
                IMP_Inventory_PlayerState_I::Execute_OnRequestItemOffer(PlayerState, Trade_ID, PlayerID, ItemID);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Player ID is not matching with any of the PlayerID in the Given TradeID"));
        }
        UE_LOG(LogTemp, Error, TEXT("PlayerStateIDA Cast failed back to base Player State"));
    }
    UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::RequestItemOffer - %d offered item %s"), Trade_ID, *ItemID.ToString());
}

void UMP_TradingManager::RequestExchageOffer(int32 Trade_ID, APlayerState* PlayerID, FName ItemIDA, FName ItemIDB)
{
    if (!ActiveTrades.Contains(Trade_ID) && !TradesList.Contains(Trade_ID))
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::RequestExchageOffer - Player %d not in trade"), Trade_ID);
        return;
    }
    if (ItemIDA.IsNone() || ItemIDB.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::RequestExchageOffer - Invalid item offered by %d"), Trade_ID);
        return;
    }
    
    FItemList Items;
    Items.SendItemID = ItemIDA;
    Items.ReciveItemID = ItemIDB;

    AMP_Inventory_PlayerState* PlayerStateIDA = Cast<AMP_Inventory_PlayerState>(PlayerID);
    if (PlayerStateIDA)
    {
        FPlayerTradeOffer TCurrentOffer;
        FTradePlayer CurrentTrade;
        CurrentTrade = TradesList[Trade_ID];

        if (CurrentTrade.PlayerA == PlayerStateIDA->PersistentPlayerId)
        {
            if (PlayerTradeA.Contains(Trade_ID))
            {
                if (PlayerTradeA[Trade_ID].OfferingPlayer == PlayerStateIDA->PersistentPlayerId)
                {
                    PlayerTradeA[Trade_ID].OfferedItems.Add(Items);
                }
            }
            else
            {
                TCurrentOffer.OfferingPlayer = PlayerStateIDA->PersistentPlayerId;
                TCurrentOffer.OfferedItems.Add(Items);
                PlayerTradeA.Add(Trade_ID ,TCurrentOffer);
            }
            AMP_Inventory_PlayerState* PlayerStateIDB = UMP_Inventory_BFL::FindPlayerStateByPersistentPlayerId(this,CurrentTrade.PlayerB);
            PlayerStateIDB->OnRequestExchageOffer(Trade_ID, PlayerStateIDA, ItemIDA, ItemIDB);
        }
        else if (CurrentTrade.PlayerB == PlayerStateIDA->PersistentPlayerId)
        {

            if (PlayerTradeB.Contains(Trade_ID))
            {
                if (PlayerTradeB[Trade_ID].OfferingPlayer == PlayerStateIDA->PersistentPlayerId)
                {
                    PlayerTradeB[Trade_ID].OfferedItems.Add(Items);
                }
            }
            else
            {
                TCurrentOffer.OfferingPlayer = PlayerStateIDA->PersistentPlayerId;
                TCurrentOffer.OfferedItems.Add(Items);
                PlayerTradeB.Add(Trade_ID, TCurrentOffer);
            }

            AMP_Inventory_PlayerState* PlayerStateIDB = UMP_Inventory_BFL::FindPlayerStateByPersistentPlayerId(this,CurrentTrade.PlayerA);
            PlayerStateIDB->OnRequestExchageOffer(Trade_ID, PlayerStateIDA, ItemIDA, ItemIDB);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Player ID is not matching with any of the PlayerID in the Given TradeID"));
        }
    }
    else 
    {
        FExchangeTradeOffer TCurrentOffer;
        FTradeSession CurrentSession;
        CurrentSession = ActiveTrades[Trade_ID];

        if (CurrentSession.PlayerA == PlayerID->GetUniqueId()) // checking TradeID with Player existance
        {
            if (PlayerOfferA.Contains(Trade_ID)) // Checking TradeID Existe in OfferContainer
            {
                if (PlayerOfferA[Trade_ID].OfferingPlayer == PlayerID->GetUniqueId())// Conforming Player
                {
                    PlayerOfferA[Trade_ID].OfferedItems.Add(Items);
                }
            }
            else
            {
                TCurrentOffer.OfferingPlayer = PlayerID->GetUniqueId();
                TCurrentOffer.OfferedItems.Add(Items);
                PlayerOfferA.Add(Trade_ID, TCurrentOffer);
            }

            FUniqueNetIdRepl PlayerIDB;
            PlayerIDB = CurrentSession.PlayerB.GetUniqueNetId();

            APlayerState* PlayerState = UGameplayStatics::GetPlayerStateFromUniqueNetId(this, PlayerIDB);
            if (PlayerState->Implements<UMP_Inventory_PlayerState_I>())
            {
                IMP_Inventory_PlayerState_I::Execute_OnRequestExchageOffer(PlayerState, Trade_ID, PlayerID, ItemIDA, ItemIDB);
            }
        }
        else if (CurrentSession.PlayerB == PlayerID->GetUniqueId()) // checking TradeID with Player existance
        {

            if (PlayerOfferB.Contains(Trade_ID)) // Checking TradeID Existe in OfferContainer
            {
                if (PlayerOfferB[Trade_ID].OfferingPlayer == PlayerID->GetUniqueId())// Conforming Player
                {
                    PlayerOfferB[Trade_ID].OfferedItems.Add(Items);
                }
            }
            else
            {
                TCurrentOffer.OfferingPlayer = PlayerID->GetUniqueId();
                TCurrentOffer.OfferedItems.Add(Items);
                PlayerOfferB.Add(Trade_ID, TCurrentOffer);
            }

            FUniqueNetIdRepl PlayerIDB;
            PlayerIDB = CurrentSession.PlayerB.GetUniqueNetId();

            APlayerState* PlayerState = UGameplayStatics::GetPlayerStateFromUniqueNetId(this, PlayerIDB);
            if (PlayerState->Implements<UMP_Inventory_PlayerState_I>())
            {
                IMP_Inventory_PlayerState_I::Execute_OnRequestExchageOffer(PlayerState, Trade_ID, PlayerID, ItemIDA, ItemIDB);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Player ID is not matching with any of the PlayerID in the Given TradeID"));
        }
        UE_LOG(LogTemp, Error, TEXT("PlayerStateIDA Cast failed back to base Player State"));
    }
 
    return;
}

void UMP_TradingManager::AcceptExchageOffer(int32 Trade_ID, APlayerState* PlayerID, FName ItemIDA, FName ItemIDB)
{
    if (!ActiveTrades.Contains(Trade_ID) && !TradesList.Contains(Trade_ID))
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::AcceptExchageOffer - Player %d not in trade"), Trade_ID);
        return;
    }
    if (ItemIDA.IsNone() || ItemIDB.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::AcceptExchageOffer - Invalid item offered by %d"), Trade_ID);
        return;
    }

    FItemList Items;
    Items.SendItemID = ItemIDA;
    Items.ReciveItemID = ItemIDB;

    AMP_Inventory_PlayerState* PlayerStateIDA = Cast<AMP_Inventory_PlayerState>(PlayerID);
    if (PlayerStateIDA)
    {
        FTradePlayer CurrentTrade;
        CurrentTrade = TradesList[Trade_ID];
        if (CurrentTrade.PlayerA == PlayerStateIDA->PersistentPlayerId)
        {
            AMP_Inventory_PlayerState* PlayerStateIDB = UMP_Inventory_BFL::FindPlayerStateByPersistentPlayerId(this, CurrentTrade.PlayerB);
            UE_LOG(LogTemp, Warning, TEXT("Player ID is A = %s"), *PlayerStateIDB->GetName());
            PlayerStateIDB->OnAcceptExchageOffer(Trade_ID, PlayerStateIDA, ItemIDA, ItemIDB);
        }
        else if (CurrentTrade.PlayerB == PlayerStateIDA->PersistentPlayerId)
        {
            AMP_Inventory_PlayerState* PlayerStateIDB = UMP_Inventory_BFL::FindPlayerStateByPersistentPlayerId(this, CurrentTrade.PlayerA);
            UE_LOG(LogTemp, Warning, TEXT("Player ID is B = %s"), *PlayerStateIDB->GetName());
            PlayerStateIDB->OnAcceptExchageOffer(Trade_ID, PlayerStateIDA, ItemIDA, ItemIDB);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Player ID is not matching with any of the PlayerID in the Given TradeID"));
        }
    }
    else
    {
        // Implementaiton is on going for not Specific player state/.....

        UE_LOG(LogTemp, Error, TEXT("PlayerStateIDA Cast failed no go back... %s "), *PlayerStateIDA->GetName());
    }
    return;
}

void UMP_TradingManager::RejectOffer(int32 Trade_ID, FUniqueNetIdRepl PlayerID)
{
    FMP_InventoryItem Curentitem;
    FTradeSession CurrentSession;
    CurrentSession = ActiveTrades[Trade_ID];
    if (CurrentSession.PlayerB == PlayerID)
    {
        UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::RejectOffer - Trade rejected by Player A"));
        Curentitem = OfferA[Trade_ID].OfferedItems.Last();
        OfferA[Trade_ID].OfferedItems.Remove(Curentitem);
        //return Curentitem;
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::RejectOffer - Trade rejected by Player B"));
        Curentitem = OfferB[Trade_ID].OfferedItems.Last();
        OfferB[Trade_ID].OfferedItems.Remove(Curentitem);
        //return Curentitem;
    }
}

void UMP_TradingManager::EndTrade(int32 Trade_ID)
{
    ActiveTrades.Remove(Trade_ID);
    UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::EndTrade - Trade completed"));
}

FPlayerTradeOffer UMP_TradingManager::GetOfferFromPlayer(int32 Trade_ID, FString Player) const
{
    FTradePlayer Players;
    Players = TradesList[Trade_ID];
    if (Player == Players.PlayerA) return PlayerTradeA[Trade_ID];
    if (Player == Players.PlayerB) return PlayerTradeB[Trade_ID];
    return FPlayerTradeOffer();
}

bool UMP_TradingManager::IsPlayerInTrade(FUniqueNetIdRepl PlayerID) const
{
    FTradeSession ActiveTrade;
    TArray<FTradeSession> ActiveTradeValue;
    ActiveTrades.GenerateValueArray(ActiveTradeValue);

    for(FTradeSession Traders:ActiveTradeValue)
    {
        if (Traders.PlayerA == PlayerID || Traders.PlayerB == PlayerID)
        {
            return true;
        }
    }
    return false;
}