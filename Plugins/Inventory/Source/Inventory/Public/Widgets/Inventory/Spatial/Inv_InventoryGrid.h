// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/Inv_GridTypes.h"
#include "Inv_InventoryGrid.generated.h"

struct FGameplayTag;
struct FInv_ImageFragment;
struct FInv_GridFragment;
class UInv_SlottedItem;
struct FInv_ItemManifest;
class UInv_ItemComponent;
class UInv_InventoryComponent;
class UCanvasPanel;
class UInv_GridSlot;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_InventoryGrid : public UUserWidget
{
	GENERATED_BODY()
public:
	//~Begin UUserWidget Interface
	virtual void NativeOnInitialized() override;
	//~End UUserWidget Interface

	FInv_SlotAvailabilityResult HasRoomForItem(const UInv_ItemComponent* ItemComponent);
	
	EInv_ItemCategory GetItemCategory() const { return ItemCategory; }
	
	UFUNCTION()
	void AddItem(UInv_InventoryItem* Item);
	
private:
	void ConstructGrid();
	
	FInv_SlotAvailabilityResult HasRoomForItem(const UInv_InventoryItem* Item);

	//Does the heavy lifting
	FInv_SlotAvailabilityResult HasRoomForItem(const FInv_ItemManifest& ItemManifest);

	void AddItemToIndices(const FInv_SlotAvailabilityResult& Result, UInv_InventoryItem* NewItem);
	void AddItemAtIndex(UInv_InventoryItem* Item, const int32 Index, const bool bStackable, const int32 StackAmount);

	//create widget to add to the grid and set its properties
	UInv_SlottedItem* CreateSlottedItem(
		UInv_InventoryItem* Item,
		const bool bStackable,
		const int32 StackAmount,
		const FInv_GridFragment* GridFragment,
		const FInv_ImageFragment* ImageFragment,
		const int32 Index
	);
	//set image property
	void SetSlottedItemImage(const UInv_SlottedItem* SlottedItem, const FInv_GridFragment* GridFragment, const FInv_ImageFragment* ImageFragment) const;

	void AddSlottedItemToCanvas(const int32 Index, const FInv_GridFragment* GridFragment, UInv_SlottedItem* SlottedItem) const;
	//actually add the item to the inventory + update to occupied grid slot texture
	void UpdateGridSlots(UInv_InventoryItem* NewItem, const int32 Index, bool bStackableItem, const int32 StackAmount);

	//helper functions for UpdateGridSlots:
	bool IsIndexClaimed(const TSet<int32>& CheckedIndices, const int32 Index) const;
	FIntPoint GetItemDimensions(const FInv_ItemManifest& ItemManifest) const;
	//HasRoomAtIndex checks slot constraints for each square in the two-dimensional range covered
	//by this item. If it finds any that fail check slot constraints, then it returns false,
	//and OutTentativelyClaimed will never be used.
	bool HasRoomAtIndex(const UInv_GridSlot* GridSlot,
		const FIntPoint& Dimensions,
		const TSet<int32>& CheckedIndices,
		TSet<int32>& OutTentativelyClaimed,
		const FGameplayTag& ItemType,
		const int32 MaxStackSize);
	//Checking for slot constraints -
	//Upper left grid slot - is the first grid slot for this ForEach2D iteration
	//SubGridSlot - is one of the grid slots in this ForEach2D iteration that this item should occupy, be it placed in this position
	bool CheckSlotConstraints(const UInv_GridSlot* UpperLeftGridSlot,
								const UInv_GridSlot* SubGridSlot,
								const TSet<int32>& CheckedIndices,
								TSet<int32>& OutTentativelyClaimed,
								const FGameplayTag& ItemType,
								const int32 MaxStackSize) const;
	bool HasValidItem(const UInv_GridSlot* GridSlot) const;
	bool IsUpperLeftGridSlot(const UInv_GridSlot* GridSlot, const UInv_GridSlot* SubGridSlot) const;
	bool DoesItemTypeMatch(const UInv_InventoryItem* SubItem, const FGameplayTag& ItemType) const;
	bool IsInGridBounds(const int32 StartIndex, const FIntPoint& ItemDimensions);
	int32 DetermineFillAmountForSlot(const bool bStackable,
									const int32 MaxStackSize,
									const int32 AmountToFill,
									const UInv_GridSlot* GridSlot) const;
	int32 GetStackAmount(const UInv_GridSlot* GridSlot) const;
	
	bool MatchesCategory(const UInv_InventoryItem* Item) const;
	FVector2D GetDrawSize(const FInv_GridFragment* GridFragment) const;

	UFUNCTION()
	void AddStacks(FInv_SlotAvailabilityResult& Result);
	
	TWeakObjectPtr<UInv_InventoryComponent> InventoryComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Inventory")
	EInv_ItemCategory ItemCategory;
	
	TArray<TObjectPtr<UInv_GridSlot>> GridSlots;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_GridSlot> GridSlotClass;

	//******* Bound Widgets *******//
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;
	//******* Bound Widgets *******//
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_SlottedItem> SlottedItemClass;

	//inventory items widgets
	//index will help us to move, remove things, etc.
	UPROPERTY()
	TMap<int32, TObjectPtr<UInv_SlottedItem>> SlottedItems;
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 Rows;
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 Columns;
	UPROPERTY(EditAnywhere, Category = "Inventory")
	float TileSize;
};
