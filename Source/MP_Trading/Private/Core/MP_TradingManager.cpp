// Copyright 2026 UVSquare. All Rights Reserved.


#include "Core/MP_TradingManager.h"
#include "Core/MP_AnalyticsManager.h"
#include "Interfaces/MP_TradeNotification_I.h"
#include "Utils/MP_Inventory_BFL.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"


TWeakObjectPtr<UMP_TradingManager> UMP_TradingManager::SingletonInstance = nullptr;

UMP_TradingManager::UMP_TradingManager()
{
	// Initialize empty trade maps
	ActiveTrades.Empty();
	PendingTrades.Empty();
}

void UMP_TradingManager::PrintAllActiveTradeIds() const
{
    if (ActiveTrades.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No active trades."));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Current Active Trade IDs:"));
    for (const auto& Pair : ActiveTrades)
    {
        UE_LOG(LogTemp, Warning, TEXT(" - %s"), *Pair.Key);
    }
}

UMP_TradingManager* UMP_TradingManager::GetTradingManager(UObject* WorldContextObject)
{
    UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("TradingManager::GetTradingManager - No valid world from context %s!"), *GetNameSafe(WorldContextObject));
        return nullptr;
    }

    // Always get the GameMode as the Outer for the singleton
    AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase >();
    if (!GameMode)
    {
        UE_LOG(LogTemp, Error, TEXT("TradingManager::GetTradingManager - No GameMode found in world %s!"), *GetNameSafe(World));
        return nullptr;
    }

    // Attach the singleton pointer to the GameMode (as a UPROPERTY or private member if you want to GC it cleanly)
    // But for plugin singleton pattern, use a static map of World* to instance pointer:
    static TMap<UWorld*, TWeakObjectPtr<UMP_TradingManager>> SingletonMap;

    if (SingletonMap.Contains(World) && SingletonMap[World].IsValid())
    {
		UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::GetTradingManager - instance Found: %p in World: %s"), SingletonMap[World].Get(), *World->GetName());
        return SingletonMap[World].Get();
    }

    UMP_TradingManager* NewInstance = NewObject<UMP_TradingManager>(GameMode);
    SingletonMap.Add(World, NewInstance);
    return NewInstance;
}

// ---- TRADE REQUEST FLOW ----

void UMP_TradingManager::RequestTrade(const FString& PlayerAId, const FString& PlayerBId)
{
    UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::InitiateTrade instance address: %p | World: %s | ActiveTrades.Num: %d"), this, *GetWorld()->GetName(), ActiveTrades.Num());

    if (PlayerAId == PlayerBId)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::InitiateTrade - Invalid trade initiation: Both Player ID is Same"));
        return;
    }
    if (IsPlayerInTrade(PlayerAId) || IsPlayerInTrade(PlayerBId))
    {
		UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::RequestTrade - Player %s or %s is already in an active trade session"), *PlayerAId, *PlayerBId);
        return; // One trade per player
    }

    FString NewID= FGuid::NewGuid().ToString();

    // Generate new trade session but do NOT add to ActiveTrades yet
    FMP_TradeSession PendingSession;
	PendingSession.TradeId = NewID;
    PendingSession.PlayerAId = PlayerAId;
    PendingSession.PlayerBId = PlayerBId;
    PendingSession.CurrentStatus = ETradeSessionStatus::Pending;
	PendingSession.Offers.Empty(); // Start with no offers

	// Add to pending trades
    PendingTrades.Add(PendingSession.TradeId, PendingSession);
	UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::RequestTrade - Temp Trade session created with ID: %s"), *PendingSession.TradeId);

    // Notify Player B (the receiver)
    NotifyParticipant(PlayerBId, "TradeRequest", PendingSession,nullptr , false);
}

void UMP_TradingManager::RespondTrade(const FString& TradeId, const bool bAccepted)
{
    FMP_TradeSession* Session = PendingTrades.Find(TradeId);
    if (!Session)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MP_TradingManager] RespondTrade: TradeId '%s' not found in pending."), *TradeId);
        return;
    }

    if (bAccepted)
    {
        // At this point, both players agree—create session and TradeId
        // BP/UI should supply PlayerAId (initiator) and PlayerBId (acceptor)
        Session->CurrentStatus = ETradeSessionStatus::Accepted;

        ActiveTrades.Add(Session->TradeId, *Session);
	    PendingTrades.Remove(TradeId);

        // Notify both that trade started
        NotifyParticipant(Session->PlayerAId, "TradeStarted", *Session, nullptr, true);
        UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::RespondTrade - Trade session created with ID: %s. ActiveTrades count: %d"), *Session->TradeId, ActiveTrades.Num());
    }
    // Here we assume TradeId is empty for a new session, so reconstruct IDs
    else
    {
        // Notify PlayerA of rejection (PlayerA is always the initiator)
        Session->CurrentStatus = ETradeSessionStatus::Rejected;
        NotifyParticipant(Session->PlayerAId, "TradeRejected", *Session, nullptr, false);
        
        // Analytics should be happen for rejected trades
        UMP_AnalyticsManager* Analytics = UMP_AnalyticsManager::Get(this);
        // Mark session as rejected; can add a custom flag/field in your analytics code.
        Analytics->AddTradeSession(Session->TradeId, Session->PlayerAId, Session->PlayerBId);
        
		PendingTrades.Remove(TradeId);
		UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::RespondTrade - Trade session %s rejected by PlayerB"), *TradeId);
        return;
    }
}

// ---- OFFER FLOW ----

void UMP_TradingManager::SubmitOffer(const FString& TradeId, const FMP_TradeOffer& Offer)
{
    FMP_TradeSession* Session = ActiveTrades.Find(TradeId);
    if (!Session)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MP_TradingManager] SubmitOffer: TradeId '%s' not active."), *TradeId);
        return;
    }

    if (Offer.Status != ETradeSessionStatus::Bargain && Offer.Status != ETradeSessionStatus::Pending)
    {
		UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::SubmitOffer - Cannot submit offer in current Offre Status: %s"), *UEnum::GetValueAsString(Offer.Status));
        return;
    }

    FMP_TradeOffer TempOffer = Offer;

    if (Session->CurrentStatus == ETradeSessionStatus::Pending)
    {
        FString NewID;
        bool bDuplicate;
        do
        {
            NewID = FGuid::NewGuid().ToString();
            bDuplicate = false;
            for (const FMP_TradeOffer& Offers : ActiveTrades[TradeId].Offers)
            {
                if (Offers.OfferId == NewID)
                {
                    bDuplicate = true;
                    break; // Early out on match
                }
            }
        } while (bDuplicate);

        TempOffer.OfferId = NewID;
        TempOffer.OfferTime = FDateTime::UtcNow();
    }


    // Handle bargaining/counter-offer
    if (Offer.Status == ETradeSessionStatus::Bargain)
    {  
        // Mark previous offer as abandoned
        for (FMP_TradeOffer& PrevOffer : Session->Offers)
        {
            if (PrevOffer.OfferId == Offer.OfferId)
            {
                PrevOffer.Status = ETradeSessionStatus::Cancelled; // Or a specific "Abandoned" status
                break;
            }
        }

        // Swap logic for exchange counter-offer
        FString TempId = Session->PlayerAId;
        Session->PlayerAId = Session->PlayerBId;
        Session->PlayerBId = TempId;
        if (Offer.OfferType == ETradeOfferType::Exchange)
        {
            TempOffer.ItemsOffered.Swap(0,1);
        }
    }

    Session->Offers.Add(TempOffer);
	UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::SubmitOffer - Offer submitted by %s with ID: %s"), *Offer.PlayerId, *TempOffer.OfferId);

    // Notify other participant
    FString OtherId = GetOtherParticipant(*Session, Offer.PlayerId);
    NotifyParticipant(OtherId, TEXT("TradeOfferReceived"), *Session, &TempOffer, false);
}

void UMP_TradingManager::RespondOffer(const FString& TradeId, const FString& OfferId, bool bAccept)
{
    UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::RespondOffer instance address: %p | World: %s | ActiveTrades.Num: %d"), this, *GetWorld()->GetName(), ActiveTrades.Num());
    FMP_TradeSession* Session = ActiveTrades.Find(TradeId);
    if (!Session)
    {
		UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::RespondOffer - TradeId %s not found in active trades. ActiveTrades count: %d"), *TradeId, ActiveTrades.Num());
        return;
    }

    // Find the offer by OfferId using pointer to the element, method is  `FindByPredicate` and its returning a pointer to the first element that matches the predicate
    FMP_TradeOffer* LatestOffer = Session->Offers.FindByPredicate([&](const FMP_TradeOffer& O)
    {
        return O.OfferId == OfferId;
    });

    if (!LatestOffer)
    {
		UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::RespondOffer - OfferId %s not found in trade %s"), *OfferId, *TradeId);
        return;
    }

    FString OtherId = GetOtherParticipant(*Session, LatestOffer->PlayerId);

    if (!bAccept)
    {
        LatestOffer->Status = ETradeSessionStatus::Rejected;

        // Log a trade record for the offerer with TradeType=None
        UMP_AnalyticsManager* Analytics = UMP_AnalyticsManager::Get(this);

        for (const FMP_InventoryItem& Item : LatestOffer->ItemsOffered)
        {
            FMP_ItemTradeRecord Record;
            Record.TradeId = Session->TradeId;
            Record.ItemID = Item.ItemID;
            Record.Quantity = Item.Quantity;
            Record.TradeTime = FDateTime::UtcNow();
            Record.TradeType = EInventoryDelta::None;
            Analytics->AddPlayerTradeRecord(LatestOffer->PlayerId, Record);
        }

        NotifyParticipant(OtherId, TEXT("TradeOfferRejected"), *Session, nullptr, false);
        return;
    }

    // Accept: Only the non-offering participant needs to call this
    Session->CurrentStatus = ETradeSessionStatus::Accepted;
    if (CompleteTrade(TradeId, *LatestOffer))
    {
		LatestOffer->Status = ETradeSessionStatus::Completed;
		NotifyParticipant(LatestOffer->PlayerId, TEXT("TradeCompleted"), *Session, LatestOffer, true);
		NotifyParticipant(OtherId, TEXT("TradeCompleted"), *Session, LatestOffer, true);
	}
    else
    {
        LatestOffer->Status = ETradeSessionStatus::Cancelled;
        NotifyParticipant(LatestOffer->PlayerId, TEXT("TradeOfferFailed"), *Session, nullptr, false);
        NotifyParticipant(OtherId, TEXT("TradeOfferFailed"), *Session, nullptr, false);

    }
}


// End/cancel a trade (timeout, exit, abort)
void UMP_TradingManager::EndTrade(const FString& TradeId)
{
    FMP_TradeSession* Session = ActiveTrades.Find(TradeId);
    if (!Session)
    {
		UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::EndTrade - TradeId %s not found in active trades. ActiveTrades count: %d"), *TradeId, ActiveTrades.Num());
        return;
    }

    Session->CurrentStatus = ETradeSessionStatus::Cancelled;
    NotifyParticipant(Session->PlayerAId, "TradeEnded", *Session, nullptr, false);
    NotifyParticipant(Session->PlayerBId, "TradeEnded", *Session, nullptr, false);

    // --- Add to analytics before removal ---
    UMP_AnalyticsManager* Analytics = UMP_AnalyticsManager::Get(this);
    Analytics->AddTradeSession(Session->TradeId, Session->PlayerAId, Session->PlayerBId);

    // Remove from active trades
    ActiveTrades.Remove(TradeId);
    UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::EndTrade - Trade %s ended. ActiveTrades count: %d"), *TradeId, ActiveTrades.Num());
}


// ---- HELPERS ----

UMP_InventoryComponent* UMP_TradingManager::GetInventoryComponent(const FString& PlayerId) const  
{  
   UMP_InventoryComponent* Comp = UMP_Inventory_BFL::FindInventoryComponentByUniqueId(GetWorld(), PlayerId);
   if (Comp && Comp->IsValidLowLevel())
   {
	   return Comp;
   }
   UE_LOG(LogTemp, Warning, TEXT("Inventory component not found or is invalid for player %s"), *PlayerId);  
   return nullptr;
}   



FMP_TradeSession UMP_TradingManager::GetTradeSession(const FString& TradeId) const
{
    const FMP_TradeSession* Session = ActiveTrades.Find(TradeId);
    return Session ? *Session : FMP_TradeSession();
}

FString UMP_TradingManager::GetActiveTradeForPlayer(const FString& PlayerId) const
{
    for (const auto& Pair : ActiveTrades)
    {
        const FMP_TradeSession& Session = Pair.Value;
        if ((Session.PlayerAId == PlayerId || Session.PlayerBId == PlayerId)
            && Session.CurrentStatus != ETradeSessionStatus::Completed
            && Session.CurrentStatus != ETradeSessionStatus::Cancelled
            && Session.CurrentStatus != ETradeSessionStatus::Rejected)
            return Session.TradeId;
    }
    return FString();
}

bool UMP_TradingManager::IsPlayerInTrade(const FString& PlayerId) const
{
    return GetActiveTradeForPlayer(PlayerId).IsEmpty();
}

// ---- PRIVATE HELPERS ----

// Main trade mutation and analytics logic
bool UMP_TradingManager::CompleteTrade(const FString& TradeId, const FMP_TradeOffer& Offer)
{
    FMP_TradeSession* Session = ActiveTrades.Find(TradeId);
    if (!Session) return false;

    // Get Item Definition Storage for price lookups
    UMP_ItemRegistry* Storage = Cast<UMP_ItemRegistry>(GetWorld()->GetGameInstance()->GetSubsystem<UMP_ItemRegistry>());

    FString FromId = Offer.PlayerId;                   // Offeror
    FString ToId = GetOtherParticipant(*Session, FromId); // Offeree

    float ItemA_Value = Offer.ItemsOffered.IsValidIndex(0) ? Storage->GetItemDefinitionByName(Offer.ItemsOffered[0].ItemID)->BasePrice : 0.0f;
    float ItemB_Value = Offer.ItemsOffered.IsValidIndex(1) ? Storage->GetItemDefinitionByName(Offer.ItemsOffered[1].ItemID)->BasePrice : 0.0f;

    float TotalPrice = Offer.OfferType == ETradeOfferType::Exchange ? ItemA_Value + ItemB_Value : Offer.ValueOffered;

	float Price = Offer.OfferType == ETradeOfferType::Exchange ? ItemA_Value - ItemB_Value : Offer.ValueOffered;

    float TradeFee = FMath::Abs(TotalPrice * 0.026f);

	float RecivePrice = FMath::Abs(Price) - TradeFee;

    bool bPaidFeeA = false;
    bool bPaidFeeB = false;

	// 2. Validate payment (via interface), charge fee and difference
    if (Offer.OfferType == ETradeOfferType::Exchange)
    {
        if (Price < TradeFee)
        {
            bPaidFeeB = ProcessPayment(ToId, Price+(TradeFee/2), TradeId);
            bPaidFeeA = ProcessPayment(FromId, -(Price-(TradeFee / 2)), TradeId);
        }
		else if (Price > 0) // Price is positive for seller
        {
			bPaidFeeB = ProcessPayment(ToId, Price, TradeId);
            bPaidFeeA = ProcessPayment(FromId, -RecivePrice, TradeId);
        }
		else if (Price < 0) // Price is negative for buyer
        {
            bPaidFeeA = ProcessPayment(FromId, -Price, TradeId);
			bPaidFeeB = ProcessPayment(ToId, -RecivePrice, TradeId);
        }
        else
        {
            bPaidFeeA = ProcessPayment(FromId, TradeFee/2, TradeId);
            bPaidFeeB = ProcessPayment(ToId, TradeFee/2, TradeId);
        }
    }

	else
	{
		if (Offer.OfferType == ETradeOfferType::Sell)
		{
			// For sell, only seller pays the fee
			bPaidFeeB = ProcessPayment(ToId, Price, TradeId);
			bPaidFeeA = ProcessPayment(FromId, -RecivePrice, TradeId);
		}
		else if (Offer.OfferType == ETradeOfferType::Buy)
		{
			// For buy, only buyer pays the fee
			bPaidFeeA = ProcessPayment(FromId, Price, TradeId);
			bPaidFeeB = ProcessPayment(ToId, -RecivePrice, TradeId);
		}
	}


    if (!bPaidFeeA || !bPaidFeeB )
    {
        NotifyParticipant(FromId, TEXT("PaymentFailed"), *Session, nullptr, false);
        NotifyParticipant(ToId, TEXT("PaymentFailed"), *Session, nullptr, false);
		UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::CompleteTrade - Payment failed for trade %s"), *TradeId);
        return false;
    }
    else
    {
        // 3. Validate both sides have the items and quantities
        if (!TransferItems(Offer, FromId, ToId))
        {
            NotifyParticipant(FromId, TEXT("TradeFailed"), *Session, nullptr, false);
            NotifyParticipant(ToId, TEXT("TradeFailed"), *Session, nullptr, false);
			UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::CompleteTrade - Transfer items failed for trade %s"), *TradeId);
            return false;
        }

    }

    // 4. Analytics
    UMP_AnalyticsManager* Analytics = UMP_AnalyticsManager::Get(this);

    if (Offer.OfferType == ETradeOfferType::Exchange)
    {
        FMP_ItemTradeRecord RecordA;
        RecordA.TradeId = Session->TradeId;
        RecordA.ItemID = Offer.ItemsOffered[0].ItemID;
        RecordA.Quantity = Offer.ItemsOffered[0].Quantity;
        RecordA.TradePrice = ItemA_Value;
        RecordA.TradeTime = FDateTime::UtcNow();
        RecordA.TradeType = EInventoryDelta::Added;
		Analytics->AddPlayerTradeRecord(FromId, RecordA);

        FTimerHandle DelayHandle;
        GetWorld()->GetTimerManager().SetTimer(DelayHandle, [this, ToId, TradeId = Session->TradeId, ItemB = Offer.ItemsOffered[1], ItemB_Value, Analytics]()
            {
                FMP_ItemTradeRecord RecordB;
                RecordB.TradeId = TradeId;
                RecordB.ItemID = ItemB.ItemID;
                RecordB.Quantity = ItemB.Quantity;
                RecordB.TradePrice = ItemB_Value;
                RecordB.TradeTime = FDateTime::UtcNow();
                RecordB.TradeType = EInventoryDelta::Added;
                Analytics->AddPlayerTradeRecord(ToId, RecordB);
            }, 0.5f, false);
	}
	else if (Offer.OfferType == ETradeOfferType::Sell)
	{
		for (const FMP_InventoryItem& Item : Offer.ItemsOffered)
		{
			FMP_ItemTradeRecord Record;
			Record.TradeId = Session->TradeId;
			Record.ItemID = Item.ItemID;
			Record.Quantity = Item.Quantity;
			Record.TradePrice = Storage->GetItemDefinitionByName(Item.ItemID)->BasePrice;
			Record.TradeTime = FDateTime::UtcNow();
			Record.TradeType = EInventoryDelta::Added;
			Analytics->AddPlayerTradeRecord(FromId, Record);
		}
    }
    else if (Offer.OfferType == ETradeOfferType::Buy)
    {
		for (const FMP_InventoryItem& Item : Offer.ItemsOffered)
		{
			FMP_ItemTradeRecord Record;
			Record.TradeId = Session->TradeId;
			Record.ItemID = Item.ItemID;
			Record.Quantity = Item.Quantity;
			Record.TradePrice = Storage->GetItemDefinitionByName(Item.ItemID)->BasePrice;
			Record.TradeTime = FDateTime::UtcNow();
			Record.TradeType = EInventoryDelta::Added;
			Analytics->AddPlayerTradeRecord(ToId, Record);
		}
    }
	return true;

}

// Moves/removes items from one inventory to another
bool UMP_TradingManager::TransferItems(const FMP_TradeOffer& Offer, const FString& FromId, const FString& ToId) const
{
    UMP_InventoryComponent* CompFrom = GetInventoryComponent(FromId);
    UMP_InventoryComponent* CompTo = GetInventoryComponent(ToId);

    if (!CompFrom || !CompTo) return false;

 //   if (Offer.OfferType == ETradeOfferType::Exchange)
 //   {
	//	//For exchange, we assume items are swapped between two parties
	//		if (Offer.ItemsOffered.Num() != 2)
	//			return false; // Invalid exchange offer

	//	// Ensure both parties have the items they are offering
	//	FMP_InventoryItem ItemA = Offer.ItemsOffered[0];
	//	FMP_InventoryItem ItemB = Offer.ItemsOffered[1];

 //       FMP_InventoryItem OwnedItemA = CompFrom->GetItemByItemID(ItemA.ItemID);
	//	FMP_InventoryItem OwnedItemB = CompTo->GetItemByItemID(ItemB.ItemID);
	//	
 //       if (OwnedItemA.Quantity < ItemA.Quantity || OwnedItemB.Quantity < ItemB.Quantity)
	//		return false; // Not enough items to exchange
	//	if (ItemA.ItemID == ItemB.ItemID)
	//		return false; // Cannot exchange same item

	//	// Remove items from FromId's inventory
	//	CompFrom->RemoveItemByID(ItemA.ItemID, ItemA.Quantity);
	//	CompTo->RemoveItemByID(ItemB.ItemID, ItemB.Quantity);

	//	// Add items to ToId's inventory
	//	CompTo->AddItem(ItemA.ItemID, ItemA.Quantity);
	//	CompFrom->AddItem(ItemB.ItemID, ItemB.Quantity);
 //       return true;
 //   }
	//// For other offer types (buy/sell), we assume items are only from one side
 //   else if (Offer.OfferType == ETradeOfferType::Sell)
 //   {
 //       // Validate all items/quantities exist
 //       for (const FMP_InventoryItem& Item : Offer.ItemsOffered)
 //       {
 //           FMP_InventoryItem OwnedItem = CompFrom->GetItemByItemID(Item.ItemID);
 //           if (OwnedItem.Quantity > Item.Quantity)
 //           {
 //               CompFrom->RemoveItemByID(Item.ItemID, Item.Quantity);
 //               CompTo->AddItem(Item.ItemID, Item.Quantity);
	//			UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::TransferItems - Adding item %s with Quantity: %d"), *Item.ItemID.ToString(), Item.Quantity);
 //               return true;
 //           }
 //       }
 //   }
 //   else if (Offer.OfferType == ETradeOfferType::Buy)
 //   {
 //       // Validate all items/quantities exist
 //       for (const FMP_InventoryItem& Item : Offer.ItemsOffered)
 //       {
 //           FMP_InventoryItem OwnedItem = CompTo->GetItemByItemID(Item.ItemID);
 //           if (OwnedItem.Quantity > Item.Quantity)
 //           {
 //               CompTo->RemoveItemByID(Item.ItemID, Item.Quantity);
 //               CompFrom->AddItem(Item.ItemID, Item.Quantity);
	//			UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::TransferItems - Adding item %s with Quantity: %d"), *Item.ItemID.ToString(), Item.Quantity);
 //               return true;
 //           }
 //       }
 //   }
	return false; // If we reach here, something went wrong
}

//bool UMP_TradingManager::ProcessItemTransfer(const FString& TradeId, const FMP_InventoryItem& Item, const FString& PlayerId, const float Amount) const
//{
//    UMP_InventoryComponent* Comp = GetInventoryComponent(PlayerId);
//    if (!Comp) return false;
//
//    if(Amount < 0) // Buy
//    {
//        if (ProcessPayment(PlayerId, Amount, TradeId))
//        {
//			Comp->AddItem(Item);
//            return true;
//        }
//    }
//	else if (Amount > 0) // Sell
//    {
//        // For sell, player gives item and receives amount
//        FMP_InventoryItem OwnedItem = Comp->GetItemByItemID(Item.ItemID);
//        if (OwnedItem.Quantity > Item.Quantity)
//        {
//            Comp->RemoveItemByID(Item.ItemID, Item.Quantity);
//        }
//        if (!ProcessPayment(PlayerId, Amount, TradeId))
//        {
//            Comp->AddItem(Item);
//			return false; // Payment failed, rollback item transfer
//        }
//		return true;
//	}
//
//    return false;
//}
//
//// Payment interface calls
//bool UMP_TradingManager::ValidateAndProcessPayment(const FString& PlayerId, float Amount, const FString& TradeId) const
//{
//    UMP_InventoryComponent* Comp = GetInventoryComponent(PlayerId);
//    if (!Comp) return false;
//
//	Comp->OnProcessPayment.Broadcast(TradeId, Amount);
//
//    AActor* Owner = Comp->GetOwner();
//    if (!Owner) return false;
//
//    // First, validate payment
//    if (Owner->GetClass()->ImplementsInterface(UMP_TradeNotification_I::StaticClass()))
//    {
//        bool bValid = IMP_TradeNotification_I::Execute_OnValidatePayment(Owner, TradeId, Amount);
//        if (!bValid) return false;
//        // Now process payment
//        IMP_TradeNotification_I::Execute_OnProcessPayment(Owner, TradeId, Amount);
//        return true;
//    }
//    return false;
//}

FString UMP_TradingManager::GetOtherParticipant(const FMP_TradeSession& Session, const FString& PlayerId) const
{
    return (Session.PlayerAId == PlayerId) ? Session.PlayerBId : Session.PlayerAId;
}

void UMP_TradingManager::NotifyParticipant(const FString& PlayerId, FName NotificationType, const FMP_TradeSession& Session, const FMP_TradeOffer* Offer, const bool Result) const
{
    UMP_InventoryComponent* Comp = GetInventoryComponent(PlayerId);
    if (!Comp)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::NotifyParticipant - Inventory component not found for player %s "), *PlayerId);
        return;
    }

    AActor* Owner = Comp->GetOwner();
    if (!Owner)
    {
		UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::NotifyParticipant - Owner not found for player %s"), *PlayerId);
        return;
    }

    if (Owner->GetClass()->ImplementsInterface(UMP_TradeNotification_I::StaticClass()))
    {
        // Switch and call the correct BP interface function (no else/if ladders)
        if (NotificationType == "TradeRequest")
        {
            IMP_TradeNotification_I::Execute_OnReceiveTradeRequest(Owner, Session.TradeId, GetOtherParticipant(Session,PlayerId));
			UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::NotifyParticipant - Trade request sent to %s for trade %s"), *PlayerId, *Session.TradeId);
        }
        else if (NotificationType == "TradeRejected")
        {
            IMP_TradeNotification_I::Execute_OnTradeRequestResponse(Owner, Session.TradeId, false);
			UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::NotifyParticipant - Trade %s rejected by %s"), *Session.TradeId, *PlayerId);
        }
        else if (NotificationType == "TradeStarted")
        {
            IMP_TradeNotification_I::Execute_OnTradeRequestResponse(Owner, Session.TradeId, true);
			UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::NotifyParticipant - Trade %s started with %s"), *Session.TradeId, *PlayerId);
        }
        else if (NotificationType == "TradeOfferReceived")
        {
            if (Session.Offers.Num() > 0 || Offer)
            {
                IMP_TradeNotification_I::Execute_OnReceiveTradeOffer(Owner, Session.TradeId, *Offer);
				UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::NotifyParticipant - Trade offer received for trade %s by %s"), *Session.TradeId, *PlayerId);
            }
        }
        else if (NotificationType == "TradeOfferRejected")
        {
            IMP_TradeNotification_I::Execute_OnOfferResponse(Owner, Session.TradeId, false);
			UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::NotifyParticipant - Trade offer for trade %s rejected by %s"), *Session.TradeId, *PlayerId);
        }
        else if (NotificationType == "TradeCompleted")
        {
            IMP_TradeNotification_I::Execute_OnTradeCompleted(Owner, Session.TradeId, Session);
			UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::NotifyParticipant - Trade %s completed by %s"), *Session.TradeId, *PlayerId);
        }
        else if (NotificationType == "TradeEnded")
        {
            IMP_TradeNotification_I::Execute_OnTradeEnded(Owner, Session.TradeId);
			UE_LOG(LogTemp, Log, TEXT("UMP_TradingManager::NotifyParticipant - Trade %s ended by %s"), *Session.TradeId, *PlayerId);
        }
        else if (NotificationType == "PaymentFailed")
        {
            IMP_TradeNotification_I::Execute_OnTradeCustomNotification(Owner, Session.TradeId, "Payment failed.");
			UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::NotifyParticipant - Payment failed for trade %s by %s"), *Session.TradeId, *PlayerId);
        }
        else if (NotificationType == "TradeFailed")
        {
            IMP_TradeNotification_I::Execute_OnTradeCustomNotification(Owner, Session.TradeId, "Trade failed. Items missing or not enough funds.");
			UE_LOG(LogTemp, Warning, TEXT("UMP_TradingManager::NotifyParticipant - Trade %s failed for %s"), *Session.TradeId, *PlayerId);
        }
    }
}
