// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MP_InventoryComponent.h" 
#include "Net/UnrealNetwork.h"
#include "MP_Inventory_PlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MP_INVENTORY_API AMP_Inventory_PlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	UMP_InventoryComponent* MP_Inventory;

	AMP_Inventory_PlayerState();

	UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "MP_Inventory|PlayerState")
	FString PersistentPlayerId;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MP_Inventory|PlayerState")
	int32 TradeId;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MP_Inventory|PlayerState")
	FString OtherPlayerId;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MP_Inventory|PlayerState")
	AMP_Inventory_PlayerState* OtherUserId;

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Inventory|PlayerState")
	void OnAcceptItemOffer(int32 Trade_ID, AMP_Inventory_PlayerState* UserId, FName ItemID);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Inventory|PlayerState")
	void OnRequestItemOffer(int32 Trade_ID, AMP_Inventory_PlayerState* UserId, FName ItemID);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Inventory|PlayerState")
	void OnAcceptExchageOffer(int32 Trade_ID, AMP_Inventory_PlayerState* UserId, FName ItemIDA, FName ItemIDB);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Inventory|PlayerState")
	void OnRequestExchageOffer(int32 Trade_ID, AMP_Inventory_PlayerState* UserId, FName ItemIDA, FName ItemIDB);

	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "MP_Inventory|PlayerState")
	void OnAcceptTrade(int32 Trade_ID, AMP_Inventory_PlayerState* UserId);

	//Calling From Server and Run on Server as well
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Inventory|PlayerState")
	void OnRequestTrade(AMP_Inventory_PlayerState* UserId);

};
