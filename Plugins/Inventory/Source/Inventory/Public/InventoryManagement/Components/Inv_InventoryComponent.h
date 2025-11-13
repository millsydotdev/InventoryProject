// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryManagement/FastArray/Inv_FastArray.h"
#include "Inv_InventoryComponent.generated.h"


struct FInv_SlotAvailabilityResult;
class UInv_ItemComponent;
class UInv_InventoryItem;
class UInv_InventoryBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStackChange, FInv_SlotAvailabilityResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryItemChange, UInv_InventoryItem*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNoRoomInInventory);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemEquipStatusChanged, UInv_InventoryItem*, Item);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class INVENTORY_API UInv_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInv_InventoryComponent();
	
	//~Begin ActorComponent Interface
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	//~End ActorComponent Interface
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Inventory")
	void TryAddItem(UInv_ItemComponent* ItemComponent);

	UFUNCTION(Server, Reliable) //reliable - guaranteed to hit the server
	void Server_AddNewItem(UInv_ItemComponent* ItemComponent, int32 StackCount);

	UFUNCTION(Server, Reliable) //reliable - guaranteed to hit the server
	void Server_DropItem(UInv_InventoryItem* Item, int32 StackCount);

	UFUNCTION(Server, Reliable) //reliable - guaranteed to hit the server
	void Server_ConsumeItem(UInv_InventoryItem* Item);
	
	void SpawnDroppedItem(UInv_InventoryItem* Item, int32 StackCount);

	UFUNCTION(Server, Reliable) //reliable - guaranteed to hit the server
	void Server_AddStacksToItem(UInv_ItemComponent* ItemComponent, int32 StackCount, int32 Remainder);

	UFUNCTION(Server, Reliable) //reliable - guaranteed to hit the server
	void Server_EquipSlotClicked(UInv_InventoryItem* ItemToEquip, UInv_InventoryItem* ItemToUnequip);

	UFUNCTION(NetMulticast, Reliable) //reliable - guaranteed to hit the server
	void Multicast_EquipSlotClicked(UInv_InventoryItem* ItemToEquip, UInv_InventoryItem* ItemToUnequip);

	void ToggleInventoryMenu();
	
	//presents a method to replicate objects on some actor or actor component
	void AddRepSubObj(UObject* SubObj);

	UInv_InventoryBase* GetInventoryMenu() const { return InventoryMenu; };

	FInventoryItemChange OnInventoryItemAdded;
	FInventoryItemChange OnInventoryItemRemoved;
	FNoRoomInInventory NoRoomInInventory;
	FStackChange OnStackChange;

	FOnItemEquipStatusChanged OnItemEquipped;
	FOnItemEquipStatusChanged OnItemUnequipped;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	TWeakObjectPtr<APlayerController> OwningPlayerController;
	
	void ConstructInventory();

	UPROPERTY(Replicated)
	FInv_InventoryFastArray InventoryList;

	//Widget created in Construct Inventory
	UPROPERTY()
	TObjectPtr<UInv_InventoryBase> InventoryMenu;

	//Class for a widget created in Construct Inventory
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_InventoryBase> InventoryMenuClass;

	bool bInventoryMenuOpen;
	void OpenInventoryMenu();
	void CloseInventoryMenu();
};
