// Copyright 2026 UVSquare. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "MP_ItemDefinition.generated.h"

/**
 * Base class for all inventory items.
 * Managed natively by the UAssetManager.
 */
UCLASS(BlueprintType, Const)
class MP_INVENTORY_API UMP_ItemDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // Core Display
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Display")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Display", meta = (MultiLine = true))
    FText Description;

    // Soft References for pure async loading (Zero memory bloat on startup)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Media")
    TSoftObjectPtr<UTexture2D> Icon;

    // Helper to safely and synchronously load the icon directly in Blueprint without async nodes
    UFUNCTION(BlueprintCallable, Category = "Item|Media")
    UTexture2D* GetLoadedIcon() const
    {
        return Icon.LoadSynchronous();
    }

    // 3D asset reference (mesh, actor, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Definition")
    TSoftObjectPtr<UObject> Model;

    // (Optional) Array for multiple material instances
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory|Definition")
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials;

    // Economy & Gameplay
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Economy")
    float BasePrice = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Gameplay")
    FGameplayTagContainer Tags;

    // How many of this specific item can fit in one slot? (0 = Infinite)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Rules")
    int32 MaxStackSize = 1;

    // How heavy is one unit of this item?
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Rules")
    float PerItemWeight = 0.0f;

    // Override this to tell the Asset Manager how to categorize this item
    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        // This makes the ID look like "Item:Sword_Iron" automatically
        return FPrimaryAssetId(FName("Item"), GetFName());
    }
};
