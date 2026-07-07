// Copyright 2026 UVSquare. All Rights Reserved.


#include "Utils/MP_Inventory_Library.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "MP_Inventory.h"
#include "GameplayTagsManager.h"
#include "Components/Image.h"
#include "Styling/SlateBrush.h"

UMP_InventoryComponent* UMP_Inventory_Library::GetInventoryByActor(AActor* Actor)
{
    // Step 1: Check if the actor is valid
    if (!Actor)
    {
        UE_LOG(LogMPInventory, Warning, TEXT("GetMPInventoryByActor - Actor is invalid."));
        return nullptr;
    }

    // Step 2: If no valid PlayerState, check if the actor itself has the Inventory component
    UMP_InventoryComponent* MP_Inventory = Actor->FindComponentByClass<UMP_InventoryComponent>();
    if (!MP_Inventory)
    {
        UE_LOG(LogMPInventory, Warning, TEXT("GetMPInventoryByActor - No MP_Inventory component found on actor or player state."));
    }

    return MP_Inventory; // This will be nullptr if nothing was found
}

UMP_InventoryComponent* UMP_Inventory_Library::GetInventoryByID(UObject* WorldContextObject, FName InventoryID)
{
    if (!WorldContextObject) {
        UE_LOG(LogMPInventory, Warning, TEXT("GetMPInventoryByID - WorldContextObject is invalid."));
        return nullptr;
	}
    
	UWorld* World = WorldContextObject->GetWorld();
    if (!World) {
        UE_LOG(LogMPInventory, Warning, TEXT("GetMPInventoryByID - World is invalid."));
        return nullptr;
	}

	if (UMP_ItemRegistry* Registry = World->GetGameInstance()->GetSubsystem<UMP_ItemRegistry>())
	{
		return Registry->GetInventoryByInventoryID(InventoryID);
	}

	UE_LOG(LogMPInventory, Warning, TEXT("GetMPInventoryByID - InventoryComponent not found with ID %s."), *InventoryID.ToString());
    return nullptr;
}

UMP_InventoryManager* UMP_Inventory_Library::GetInventoryManager(UObject* WorldContextObject)
{
    if (!WorldContextObject) {
        UE_LOG(LogMPInventory, Warning, TEXT("GetMPInventoryByID - WorldContextObject is invalid."));
        return nullptr;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World) {
        UE_LOG(LogMPInventory, Warning, TEXT("GetMPInventoryByID - World is invalid."));
        return nullptr;
    }

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
    if (PlayerController)
    {
        if (UMP_InventoryManager* InventoryManager = PlayerController->FindComponentByClass<UMP_InventoryManager>())
        {
            return InventoryManager;
        }
        else
        {
            UE_LOG(LogMPInventory, Warning, TEXT("GetInventoryManager - No UMP_InventoryManager component found on PlayerController."));
        }
    }
    else
    {
        UE_LOG(LogMPInventory, Warning, TEXT("GetInventoryManager - No PlayerController found."));
	}
    return nullptr;
}

APlayerState* UMP_Inventory_Library::FindPlayerStateByUniqueNetId(UObject* WorldContextObject, const FUniqueNetIdRepl& TargetUniqueNetId)
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

FString UMP_Inventory_Library::UniqueNetIdToString(UObject* WorldContextObject, const FUniqueNetIdRepl& TargetUniqueNetId)
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

UMP_InventoryComponent* UMP_Inventory_Library::FindInventoryComponentByUniqueId(UObject* WorldContextObject, const FString& TargetPersistentPlayerId)
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
		if (Inventory && Inventory->OwnerID.ToString() == TargetPersistentPlayerId)
		{
			return Inventory;
		}
    }
    return nullptr;
}


FGameplayTag UMP_Inventory_Library::FNameToGameplayTag(UObject* WorldContextObject, const FName TagName)
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

TArray<FGameplayTag> UMP_Inventory_Library::GetAllGameplayTags(UObject* WorldContextObject)
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

FSlateBrush UMP_Inventory_Library::GetImageStyle(class UImage* TargetImage)
{
	FSlateBrush Brush = FSlateBrush();
	if (IsValid(TargetImage))
	{
        Brush = TargetImage->Brush;
	}

	return Brush;
}

FVector UMP_Inventory_Library::GetSafeDropLocation(AActor* TargetActor, float ForwardOffset, float DebugDrawDuration)
{
	if (!TargetActor)
	{
		return FVector::ZeroVector;
	}

	FVector ActorLoc = TargetActor->GetActorLocation();
	FVector ForwardVec = TargetActor->GetActorForwardVector();

	// Calculate the target X/Y position
	FVector TargetXY = ActorLoc + (ForwardVec * ForwardOffset);

	// Setup the trace vertically: 100 units above the target point, down to 500 units below it
	FVector TraceStart = TargetXY + FVector(0.0f, 0.0f, 100.0f);
	FVector TraceEnd = TargetXY + FVector(0.0f, 0.0f, -500.0f);

	UWorld* World = TargetActor->GetWorld();
	if (!World)
	{
		return TargetXY;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(TargetActor);

	FHitResult HitResult;
	bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	// Handle Debug Drawing
	if (DebugDrawDuration != 0.0f)
	{
		bool bPersistent = (DebugDrawDuration < 0.0f);
		float LifeTime = bPersistent ? -1.0f : DebugDrawDuration;
		
		DrawDebugLine(World, TraceStart, TraceEnd, bHit ? FColor::Green : FColor::Red, bPersistent, LifeTime, 0, 2.0f);
		if (bHit)
		{
			DrawDebugSphere(World, HitResult.ImpactPoint, 10.0f, 12, FColor::Yellow, bPersistent, LifeTime);
		}
	}

	if (bHit)
	{
		// Slightly offset the Z so it doesn't clip perfectly into flat geometry
		return HitResult.ImpactPoint; 
	}

	// Fallback if we drop it over a cliff/void
	return TargetXY;
}
