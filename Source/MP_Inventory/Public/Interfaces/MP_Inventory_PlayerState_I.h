// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
//#include "GameFramework/PlayerState.h"
#include "MP_Inventory_PlayerState_I.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMP_Inventory_PlayerState_I : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MP_INVENTORY_API IMP_Inventory_PlayerState_I
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Inventory")
	void OnAcceptItemOffer(int32 Trade_ID, APlayerState* UserId, FName ItemID);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Inventory")
	void OnRequestItemOffer(int32 Trade_ID, APlayerState* UserId, FName ItemID);


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Inventory")
	void OnAcceptMakeOffer(int32 Trade_ID, APlayerState* UserId, FName ItemIDA, FName ItemIDB);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Inventory")
	void OnRequestExchageOffer(int32 Trade_ID, APlayerState* PlayerID, FName ItemIDA, FName ItemIDB);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Inventory")
	void OnAcceptTrade(int32 Trade_ID, APlayerState* PlayerID);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MP_Inventory")
	void OnRequestTrade(APlayerState *PlayerID);
};
