// Copyright UVSquare, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/MP_InventoryComponent.h" 
// #include "MP_Inventory_PlayerState.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MP_Inventory_BFL.generated.h"

/**
 * Static utility library providing globally accessible Blueprint nodes.
 * Facilitates safe retrieval of inventory components, execution of asynchronous tasks,
 * and common helper operations without requiring direct actor references.
 */
UCLASS()
class MP_INVENTORY_API UMP_Inventory_BFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "MP_Inventory|FLibrary")
	static UMP_InventoryComponent* GetInventoryByActor(AActor* Actor);

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static UMP_InventoryComponent* GetInventoryComponent(UObject* WorldContextObject);


	//UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	//static AMP_Inventory_PlayerState* GetInventoryPlayerState(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static APlayerState* FindPlayerStateByUniqueNetId(UObject* WorldContextObject, const FUniqueNetIdRepl& TargetUniqueNetId);

	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static FString UniqueNetIdToString(UObject* WorldContextObject, const FUniqueNetIdRepl& TargetUniqueNetId);

	//UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	//static AMP_Inventory_PlayerState* FindPlayerStateByPersistentPlayerId(UObject* WorldContextObject, const FString& TargetPersistentPlayerId);

	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static UMP_InventoryComponent* FindInventoryComponentByUniqueId(UObject* WorldContextObject, const FString& TargetPersistentPlayerId);


	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static FGameplayTag FNameToGameplayTag(UObject* WorldContextObject, const FName TagName);

	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static TArray<FGameplayTag> GetAllGameplayTags(UObject* WorldContextObject);

	
	// Helper to easily style an Image widget for rounded corners and hover states without Slate caching bugs
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "MP_Inventory|UI")
	static FSlateBrush GetImageStyle(class UImage* TargetImage);
};
