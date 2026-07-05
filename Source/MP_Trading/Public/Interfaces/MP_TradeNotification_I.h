// Copyright 2026 UVSquare. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MP_TradeNotification_I.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMP_TradeNotification_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * Outbound communication interface for broadcasting trade-related events to clients.
 * Designed to be implemented by Player Controllers or UI Managers to handle real-time 
 * alerts, incoming trade requests, and state changes during active negotiations.
 */
class MP_TRADING_API IMP_TradeNotification_I
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// Called when a trade request arrives (other player wants to trade with you)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Trade|Notification")
	void OnReceiveTradeRequest(const FString& TradeId, const FString& OtherPlayerId);

	// Called when your trade request is accepted or rejected
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Trade|Notification")
	void OnTradeRequestResponse(const FString& TradeId, bool bAccepted);

	// Called when you receive a new offer (including counter-offer/bargain)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Trade|Notification")
	void OnReceiveTradeOffer(const FString& TradeId, const FMP_TradeOffer& Offer);

	// Called when your offer is accepted or rejected (feedback for UI)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Trade|Notification")
	void OnOfferResponse(const FString& TradeId, bool bAccepted);

	// Called when trade is completed (with final summary for confirmation)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Trade|Notification")
	void OnTradeCompleted(const FString& TradeId, const FMP_TradeSession& Session);

	// Called when trade is cancelled/ended (timeout, exit, abort)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Trade|Notification")
	void OnTradeEnded(const FString& TradeId);

	// Called to validate payment (pass in required amount, return true/false via BP)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Trade|Notification")
	bool OnValidatePayment(const FString& TradeId, float AmountToPay);

	// Called to actually process payment (deduct/add, and show notification in UI)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Trade|Notification")
	void OnProcessPayment(const FString& TradeId, float AmountToPay);

	// (Optional) Called for custom notifications, errors, etc.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Trade|Notification")
	void OnTradeCustomNotification(const FString& TradeId, const FName& Message);




};
