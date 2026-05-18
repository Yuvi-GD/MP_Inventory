// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/MP_Inventory_BFL.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h" // Add this include to resolve the incomplete type error
#include "GameplayTagsManager.h"


UMP_InventoryComponent* UMP_Inventory_BFL::GetInventoryByActor(AActor* Actor)
{
    // Step 1: Check if the actor is valid
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetMPInventoryByActor - Actor is invalid."));
        return nullptr;
    }

    // Step 2: If no valid PlayerState, check if the actor itself has the Inventory component
    UMP_InventoryComponent* MP_Inventory = Actor->FindComponentByClass<UMP_InventoryComponent>();
    if (!MP_Inventory)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetMPInventoryByActor - No MP_Inventory component found on actor or player state."));
    }

    return MP_Inventory; // This will be nullptr if nothing was found
}

//AMP_Inventory_PlayerState* UMP_Inventory_BFL::GetInventoryPlayerState(const UObject* WorldContextObject)
//{
//    if (!WorldContextObject) {
//        return nullptr;
//    }
//
//    UWorld* World = WorldContextObject->GetWorld();
//    if (!World) {
//        return nullptr;
//    }
//
//    APlayerController* PlayerCtrlr = UGameplayStatics::GetPlayerController(WorldContextObject,0);
//
//    return Cast<AMP_Inventory_PlayerState>(PlayerCtrlr->PlayerState);
//}

APlayerState* UMP_Inventory_BFL::FindPlayerStateByUniqueNetId(UObject* WorldContextObject, const FUniqueNetIdRepl& TargetUniqueNetId)
{
    if (!WorldContextObject || !TargetUniqueNetId.IsValid())
    {
        return nullptr;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // Loop through all Player Controllers in the world
    for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PlayerController = Iterator->Get();
        if (!PlayerController)
        {
            continue;
        }

        APlayerState* PlayerState = PlayerController->PlayerState;
        if (!PlayerState)
        {
            continue;
        }

        // Get the Unique Net ID from the Player State
        const FUniqueNetIdRepl CurrentUniqueNetId = PlayerState->GetUniqueId();
        if (CurrentUniqueNetId == TargetUniqueNetId)
        {
            return PlayerState;
        }
    }

    // Return nullptr if no matching Player State is found
    return nullptr;
}

FString UMP_Inventory_BFL::UniqueNetIdToString(UObject* WorldContextObject, const FUniqueNetIdRepl& TargetUniqueNetId)
{
    if (!WorldContextObject)
    {
        return FString();
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return FString();
    }

    return TargetUniqueNetId.GetUniqueNetId()->ToString();
}

//APlayerState* UMP_Inventory_BFL::FindPlayerStateByPersistentPlayerId(UObject* WorldContextObject, const FString& TargetPersistentPlayerId)
//{
//    if (!WorldContextObject || TargetPersistentPlayerId.IsEmpty())
//    {
//        return nullptr;
//    }
//
//    UWorld* World = WorldContextObject->GetWorld();
//    if (!World)
//    {
//        return nullptr;
//    }
//
//    // Loop through all Player Controllers in the world
//    for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
//    {
//        APlayerController* PlayerController = Iterator->Get();
//        if (!PlayerController && !PlayerController->PlayerState)
//        {
//            continue;
//        }
//
//        // Get the Unique Net ID from the Player State
//        //const FString CurrentPersistentPlayerId = PlayerState->PersistentPlayerId;
//        //if (CurrentPersistentPlayerId == TargetPersistentPlayerId)
//        //{
//        //    return PlayerState;
//        //}
//    }
//
//    // Return nullptr if no matching Player State is found
//    return nullptr;
//}

UMP_InventoryComponent* UMP_Inventory_BFL::FindInventoryComponentByUniqueId(UObject* WorldContextObject, const FString& TargetPersistentPlayerId)
{
    if (!WorldContextObject || TargetPersistentPlayerId.IsEmpty())
    {
        return nullptr;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return nullptr;
    }

    for (APlayerState* PS : World->GetGameState()->PlayerArray)
    {
        if (!IsValid(PS)) continue;
        auto* Inventory = PS->FindComponentByClass<UMP_InventoryComponent>();
		if (Inventory->UniqueId == TargetPersistentPlayerId)
		{
			return Inventory;
		}
    }
    return nullptr;
}


FGameplayTag UMP_Inventory_BFL::FNameToGameplayTag(UObject* WorldContextObject, const FName TagName)
{
    if (!WorldContextObject)
    {
        return FGameplayTag::EmptyTag;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return FGameplayTag::EmptyTag;
    }

    // Convert FName to GameplayTag using RequestGameplayTag
    FGameplayTag RequestedTag = UGameplayTagsManager::Get().RequestGameplayTag(TagName, false);

    // Validate whether the requested tag is actually registered
    if (RequestedTag.IsValid())
    {
        return RequestedTag;
    }
    else
    {
        return FGameplayTag::EmptyTag; // This handles the missing return path
    }
}

TArray<FGameplayTag> UMP_Inventory_BFL::GetAllGameplayTags(UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return TArray<FGameplayTag>();
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return TArray<FGameplayTag>();
    }

    FString Prefix = "Item.";
    TArray<FGameplayTag> FilteredTags;
    FGameplayTagContainer AllTags;
    UGameplayTagsManager::Get().RequestAllGameplayTags(AllTags, false);

    for (const FGameplayTag& Tag : AllTags)
    {
        if (Tag.ToString().StartsWith(Prefix))
        {
            FilteredTags.Add(Tag);
        }
    }
    return FilteredTags;
}
