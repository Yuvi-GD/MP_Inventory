// Copyright UVSquare, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MP_InventoryComponent.h" 
#include "MP_Inventory_PlayerState.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MP_Inventory_BFL.generated.h"

/**
 * A Blueprint Function Library for the MP_Inventory plugin, providing utility functions to interact with inventory components and player states.
 */
UCLASS()
class MP_INVENTORY_API UMP_Inventory_BFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "MP_Inventory|FLibrary")
	static UMP_InventoryComponent* GetInventoryByActor(AActor* Actor);


	//UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	//static AMP_Inventory_PlayerState* GetInventoryPlayerState(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static APlayerState* FindPlayerStateByUniqueNetId(UObject* WorldContextObject, const FUniqueNetIdRepl& TargetUniqueNetId);

	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static FString UniqueNetIdToString(UObject* WorldContextObject, const FUniqueNetIdRepl& TargetUniqueNetId);

	//UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	//static APlayerState* FindPlayerStateByPersistentPlayerId(UObject* WorldContextObject, const FString& TargetPersistentPlayerId);

	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static UMP_InventoryComponent* FindInventoryComponentByUniqueId(UObject* WorldContextObject, const FString& TargetPersistentPlayerId);


	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static FGameplayTag FNameToGameplayTag(UObject* WorldContextObject, const FName TagName);

	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static TArray<FGameplayTag> GetAllGameplayTags(UObject* WorldContextObject);
};
