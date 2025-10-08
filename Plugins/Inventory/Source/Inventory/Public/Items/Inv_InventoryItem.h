// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Inv_InventoryItem.generated.h"

/**
 * This is a UObject that represents an Item in our Inventory.
 * Contains data about this Item.
 */
UCLASS()
class INVENTORY_API UInv_InventoryItem : public UObject
{
	GENERATED_BODY()
public:
	//~Begin UObject Interface
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; };
	//~End UObject Interface
	
	void SetItemManifest(const FInv_ItemManifest& Manifest);
	
	const FInv_ItemManifest& GetItemManifest() const { return ItemManifest.Get<FInv_ItemManifest>(); }
	FInv_ItemManifest GetItemManifestMutable() { return ItemManifest.GetMutable<FInv_ItemManifest>(); }

	bool IsStackable() const;

private:
	UPROPERTY(VisibleAnywhere, meta = (BaseStruct = "/Script/Inventory.Inv_ItemManifest"), Replicated)
	FInstancedStruct ItemManifest;
	
};

template<typename FragmentType>
const FragmentType* GetFragmentByTag(const UInv_InventoryItem* Item, UPARAM(meta = (Categories = "FragmentTags")) const FGameplayTag& Tag)
{
	if (!IsValid(Item)) return nullptr;

	const FInv_ItemManifest& Manifest = Item->GetItemManifest();
	
	return Manifest.GetFragmentOfTypeByTag<FragmentType>(Tag);
}
