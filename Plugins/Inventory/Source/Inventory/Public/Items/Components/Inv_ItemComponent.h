// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/Manifest/Inv_ItemManifest.h"
#include "Inv_ItemComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class INVENTORY_API UInv_ItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInv_ItemComponent();

	//~Begin ActorComponent Interface
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	//~End ActorComponent Interface

	//called only on item drop so far, in other cases - have bp classes with item manifests set up.
	void SetItemManifest(FInv_ItemManifest InItemManifest) { ItemManifest = InItemManifest; };
	
	FInv_ItemManifest GetItemManifest() const { return ItemManifest; };

	FString GetPickupMessage() const { return PickupMessage; };

	void PickedUp();
protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnPickedUp();
	
private:
	UPROPERTY(Replicated, EditAnywhere, Category="Inventory")
	FInv_ItemManifest ItemManifest;
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FString PickupMessage;
		
};
