// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Inventory/InventoryBase/Inv_InventoryBase.h"
#include "Inv_SpatialInventory.generated.h"

class UInv_ItemDescription;
class UInv_EquippedSlottedItem;
struct FGameplayTag;
class UInv_EquippedGridSlot;
class UCanvasPanel;
class UButton;
class UWidgetSwitcher;
class UInv_InventoryGrid;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_SpatialInventory : public UInv_InventoryBase
{
	GENERATED_BODY()
public:
	//~Begin UUserWidget Interface
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;
	UInv_ItemDescription* GetOrCreateItemDescription();
	void SetItemDescriptionSizeAndPosition(UInv_ItemDescription* Description, UCanvasPanel* Canvas) const;
	//~End UUserWidget Interface

	//~Begin UInv_InventoryBase Interface
	virtual FInv_SlotAvailabilityResult HasRoomForItem(UInv_ItemComponent* ItemComponent) const override;
	virtual bool HasHoverItem() const override;
	virtual UInv_HoverItem* GetHoverItem() const override;
	virtual float GetTileSize() const override;
	
	UFUNCTION()
	virtual void OnSlottedItemHovered(UInv_InventoryItem* Item) override;
	UFUNCTION()
	virtual void OnSlottedItemUnhovered() override;
	
	//virtual void OnItemHovered(UInv_InventoryItem* Item) override;
	//virtual void OnItemUnhovered() override;
	//virtual bool HasHoverItem() const override;
	//~End UInv_InventoryBase Interface

	bool CanEquipHoverItem(UInv_EquippedGridSlot* EquippedGridSlot, const FGameplayTag& EquipmentTypeTag) const;
	
private:

	TWeakObjectPtr<UInv_InventoryGrid> ActiveGrid;

	UPROPERTY()
	TObjectPtr<UInv_ItemDescription> ItemDescription;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_ItemDescription> ItemDescriptionClass;

	FTimerHandle Description_Timer;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DescriptionTimerDelay = 0.5f;
	
	//******* Bound Widgets *******//
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;

	//Added in NativeOnInitialized
	UPROPERTY()
	TArray<TObjectPtr<UInv_EquippedGridSlot>> EquippedGridSlots;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> Switcher;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInv_InventoryGrid> Grid_Equippables;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInv_InventoryGrid> Grid_Consumables;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInv_InventoryGrid> Grid_Craftables;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Equippables;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Consumables;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Craftables;
	//******* Bound Widgets *******//

	UFUNCTION()
	void ShowEquippables();
	UFUNCTION()
	void ShowCraftables();
	UFUNCTION()
	void ShowConsumables();

	//Called if we click on equipped GRID SLOT (if equipped grid slot doesn't contain an item).
	//Clicked either emptyhanded or with hover item, but do something only if clicked with hover item
	UFUNCTION()
	void EquippedGridSlotClicked(UInv_EquippedGridSlot* EquippedGridSlot, const FGameplayTag& EquippedTypeTag);

	//Called if we click on equipped slotted ITEM (if equipped grid slot contains an item). Clicked either emptyhanded or with hover item
	UFUNCTION()
	void EquippedSlottedItemClicked(UInv_EquippedSlottedItem* EquippedSlottedItem);

	void ClearSlotOfItem(UInv_EquippedGridSlot* EquippedGridSlot);
	void RemoveEquippedSlottedItem(UInv_EquippedSlottedItem* EquippedSlottedItem);

	void SetActiveGrid(UInv_InventoryGrid* Grid, UButton* DisableButton);

	//Create new equipped slotted item for this equipped grid slot
	void MakeEquippedSlottedItem(UInv_EquippedSlottedItem* EquippedSlottedItem, UInv_EquippedGridSlot* EquippedGridSlot, UInv_InventoryItem* ItemToEquip);

	UInv_EquippedGridSlot* FindSlotWithEquippedItem(UInv_InventoryItem* EquippedItem);
};
