#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "Inv_FastArray.generated.h"

struct FGameplayTag;
class UInv_ItemComponent;
class UInv_InventoryComponent;
class UInv_InventoryItem;

/* These structs are closely related to the inventory component */

/* A single Entry in the inventory */
USTRUCT(BlueprintType)
struct FInv_InventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
public:
	FInv_InventoryEntry() {}

private:
	friend struct FInv_InventoryFastArray;
	friend UInv_InventoryComponent;
	
	UPROPERTY()
	TObjectPtr<UInv_InventoryItem> Item = nullptr;
};


/* A list of inventory items */
USTRUCT(BlueprintType)
struct FInv_InventoryFastArray : public FFastArraySerializer
{
	GENERATED_BODY()
public:
	FInv_InventoryFastArray() : OwnerComponent(nullptr) {};
	FInv_InventoryFastArray(UActorComponent* InOwnerComponent) : OwnerComponent(InOwnerComponent) {};

	TArray<UInv_InventoryItem*> GetAllInventoryItems() const;

	//~Begin FFastArraySerializer contract
	//Automatically called on the client when the item has been removed
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);

	//Automatically called on the client when the new item has been added
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FInv_InventoryEntry,FInv_InventoryFastArray>(Entries, DeltaParams, *this);
	};

	UInv_InventoryItem* AddEntry(UInv_ItemComponent* ItemComponent);
	UInv_InventoryItem* AddEntry(UInv_InventoryItem* InventoryItem);
	void RemoveEntry(UInv_InventoryItem* InventoryItem);
	//~End FFastArraySerializer contract

	UInv_InventoryItem* FindFirstItemByTag(const FGameplayTag& ItemType);
	
private:
	friend UInv_InventoryComponent;
	
	//Replicated list of items
	UPROPERTY()
	TArray<FInv_InventoryEntry> Entries;
	
	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};


template<>
struct TStructOpsTypeTraits<FInv_InventoryFastArray> : TStructOpsTypeTraitsBase2<FInv_InventoryFastArray>
{
	//sets a flag, that indicates that this particular type should be delta serialized
	enum { WithNetDeltaSerializer = true };
};

