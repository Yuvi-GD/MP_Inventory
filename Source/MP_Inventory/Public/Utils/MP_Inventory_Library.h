// Copyright UVSquare, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/MP_InventoryComponent.h" 
#include "Core/MP_InventoryManager.h" 
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MP_Inventory_Library.generated.h"

/**
 * Static utility library providing globally accessible Blueprint nodes.
 * Facilitates safe retrieval of inventory components, execution of asynchronous tasks,
 * and common helper operations without requiring direct actor references.
 */
UCLASS()
class MP_INVENTORY_API UMP_Inventory_Library : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/** Safely retrieves the inventory component attached to the given actor, if one exists. */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "MP_Inventory|FLibrary")
	static UMP_InventoryComponent* GetInventoryByActor(AActor* Actor);

	/** Looks up an inventory globally by its exact InventoryID via the Item Registry. */
	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static UMP_InventoryComponent* GetInventoryByID(UObject* WorldContextObject, FName InventoryID);

	/** Finds the local player's Inventory Manager (from their PlayerController). */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static UMP_InventoryManager* GetInventoryManager(UObject* WorldContextObject);

	/** Retrieves the PlayerState associated with a specific UniqueNetId. */
	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static APlayerState* FindPlayerStateByUniqueNetId(UObject* WorldContextObject, const FUniqueNetIdRepl& TargetUniqueNetId);

	/** Converts a UniqueNetId to a standard FString format. */
	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static FString UniqueNetIdToString(UObject* WorldContextObject, const FUniqueNetIdRepl& TargetUniqueNetId);

	/** 
	 * Calculates a highly optimized, safe ground location for dropping items by tracing downwards.
	 * @param TargetActor The actor (usually the player) dropping the item.
	 * @param ForwardOffset Distance in front of the actor to drop.
	 * @param RightOffset Distance to the right/left of the actor to drop.
	 * @param DebugDrawDuration | -1 = Persistent, 0 = No Debug, > 0 = Duration in seconds.
	 * @return The safe ground location to feed into the DropItem function.
	 */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "MP_Inventory|Utils")
	static FVector GetSafeDropLocation(AActor* TargetActor, float ForwardOffset = 150.0f, float DebugDrawDuration = 0.0f);

	/** Locates an inventory component belonging to a specific persistent player ID (EOS, Steam, etc). */
	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static UMP_InventoryComponent* FindInventoryComponentByUniqueId(UObject* WorldContextObject, const FString& TargetPersistentPlayerId);

	/** Converts an FName directly into a GameplayTag. */
	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static FGameplayTag FNameToGameplayTag(UObject* WorldContextObject, const FName TagName);

	/** Retrieves all currently registered GameplayTags in the project. */
	UFUNCTION(BlueprintPure, Category = "MP_Inventory|FLibrary", meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject"))
	static TArray<FGameplayTag> GetAllGameplayTags(UObject* WorldContextObject);
	
	// Helper to easily style an Image widget for rounded corners and hover states without Slate caching bugs
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "MP_Inventory|UI")
	static FSlateBrush GetImageStyle(class UImage* TargetImage);
};
