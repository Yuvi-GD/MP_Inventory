// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/MP_Inventory_PlayerState.h"
#include "Framework/MP_Inventory_BFL.h"


AMP_Inventory_PlayerState::AMP_Inventory_PlayerState()
{
    MP_Inventory = CreateDefaultSubobject<UMP_InventoryComponent>(TEXT("MP_InventoryComponent"));
    // Enable replication for the component
    if (MP_Inventory)
    {
        MP_Inventory->SetIsReplicated(true);
        MP_Inventory->SetAutoActivate(true); // Automatically activate the component
    }
}

void AMP_Inventory_PlayerState::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority() && PersistentPlayerId.IsEmpty())
    {
        // Generate a unique PersistentPlayerId
        //PersistentPlayerId = GenerateUniquePersistentId();
        UE_LOG(LogTemp, Log, TEXT("Assigned PersistentPlayerId: %s"), *PersistentPlayerId);
    }
}

void AMP_Inventory_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMP_Inventory_PlayerState, PersistentPlayerId);
    DOREPLIFETIME(AMP_Inventory_PlayerState, OtherUserId);
}

void AMP_Inventory_PlayerState::OnAcceptItemOffer_Implementation(int32 Trade_ID, AMP_Inventory_PlayerState* UserId, FName ItemID)
{
}

void AMP_Inventory_PlayerState::OnRequestItemOffer_Implementation(int32 Trade_ID, AMP_Inventory_PlayerState* UserId, FName ItemID)
{
}

void AMP_Inventory_PlayerState::OnAcceptExchageOffer_Implementation(int32 Trade_ID, AMP_Inventory_PlayerState* UserId, FName ItemIDA, FName ItemIDB)
{
    if (UserId == OtherUserId)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player ID is matching with the PlayerID in the Player State Accept Make Offer"));
        UMP_InventoryComponent* OtherMP_Inventory = UMP_Inventory_BFL::GetInventoryByActor(UserId);
        FMP_InventoryStruct ItemA = MP_Inventory->GetItemByItemID(ItemIDA);
        FMP_InventoryStruct ItemB = OtherMP_Inventory->GetItemByItemID(ItemIDB);
        MP_Inventory->RemoveItemByID(ItemIDA);
        ItemB.Quantity = 1;
        MP_Inventory->AddItem(ItemB);

        OtherMP_Inventory->RemoveItemByID(ItemIDB);
        ItemA.Quantity = 1;
        OtherMP_Inventory->AddItem(ItemA);
    }
    UE_LOG(LogTemp, Error, TEXT("Player ID is Not matching with the PlayerID in the Player State Accept Make Offer"));
}

void AMP_Inventory_PlayerState::OnRequestExchageOffer_Implementation(int32 Trade_ID, AMP_Inventory_PlayerState* UserId, FName ItemIDA, FName ItemIDB)
{
    if (UserId == OtherUserId)
    {
    }
}

void AMP_Inventory_PlayerState::OnAcceptTrade_Implementation(int32 Trade_ID, AMP_Inventory_PlayerState* UserId)
{
    OtherUserId = UserId;
    TradeId = Trade_ID;
    OtherUserId->TradeId = Trade_ID;
    OtherUserId->OtherUserId = this;
}

void AMP_Inventory_PlayerState::OnRequestTrade_Implementation(AMP_Inventory_PlayerState* UserId)
{
    OtherUserId = UserId;
}
