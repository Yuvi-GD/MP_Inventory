// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MP_InventoryComponent.h" 
#include "MP_Inventory_PlayerState.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MP_Inventory_BFL.generated.h"

/**
 * 
 */
UCLASS()
class MP_INVENTORY_API UMP_Inventory_BFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "MP_Inventory|FLibrary")
	static UMP_InventoryComponent* GetInventoryByActor(AActor* Actor);


	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static AMP_Inventory_PlayerState* GetInventoryPlayerState(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static APlayerState* FindPlayerStateByUniqueNetId(UObject* WorldContextObject, const FUniqueNetIdRepl& TargetUniqueNetId);

	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static FString UniqueNetIdToString(UObject* WorldContextObject, const FUniqueNetIdRepl& TargetUniqueNetId);

	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static AMP_Inventory_PlayerState* FindPlayerStateByPersistentPlayerId(UObject* WorldContextObject, const FString& TargetPersistentPlayerId);

};
