// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Database/MP_InventoryStruct.h"
#include "InvItemObj.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class MP_INVENTORY_API UInvItemObj : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MP_Inventory", meta = (ExposeOnSpawn = "true"))
	FMP_InventoryItem ItemData;
	
    // Updated property to ensure it is marked as 'Instance Editable'  
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (ExposeOnSpawn = "true"))
	bool bIsEditable= false;
};

/*
FNetReplicationGraphConnection*
FConnectionReplicationActorInfo&
FNewReplicatedActorInfo&
FReplicatorConnectionKey


   virtual void NotifyAddNetworkActor(const FNewReplicatedActorInfo& ActorInfo) override;
   virtual void NotifyConnectionRemoved(FNetReplicationGraphConnection* ConnectionManager) override;
   virtual void NotifyConnectionAdded(FNetReplicationGraphConnection* ConnectionManager) override;
   virtual void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params) override;
   TMap<FNetReplicationGraphConnection*, TSet<FString>> ConnectionToRoomIDs;
   void UnsubscribeConnectionFromRoom(FNetReplicationGraphConnection* ConnectionManager, const FString& RoomID);

*/