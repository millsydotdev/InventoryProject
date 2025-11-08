// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/Inv_GridTypes.h"
#include "Inv_InventoryGrid.generated.h"

class UInv_ItemDescription;
class UInv_ItemPopUp;
enum class EInv_GridSlotState : uint8;
class UInv_HoverItem;
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
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;
	//~End UUserWidget Interface

	bool CursorExitedCanvas(const FVector2D& CanvasPosition, const FVector2D& BoundarySize, const FVector2D& Location);
	void HighlightSlots(const int32 Index, const FIntPoint& Dimensions);
	void UnHighlightSlots(const int32 Index, const FIntPoint& Dimensions);
	void ChangeHoverType(const int32 Index, const FIntPoint& Dimensions, EInv_GridSlotState GridSlotState);

	bool IsSameStackable(const UInv_InventoryItem* ClickedInventoryItem) const;
	void SwapWithHoverItem(UInv_InventoryItem* ClickedInventoryItem, const int32 GridIndex);
	bool ShouldSwapStackCounts(const int32 RoomInClickedSlot, const int32 HoveredStackCount, const int32 MaxStackSize) const;
	void SwapStackCounts(const int32 ClickedStackCount, const int32 HoveredStackCount, const int32 Index);
	void ConsumeHoverItemStacks(const int32 ClickedStackCount, const int32 HoveredStackCount, const int32 Index);
	void FillInStack(const int32 FillAmount, const int32 Remainder, const int32 Index);

	bool HasHoverItem() const;
	UInv_ItemDescription* GetOrCreateItemDescription();
	void SetItemDescriptionSizeAndPosition(UInv_ItemDescription* Description, UCanvasPanel* Canvas) const;
	
	void SetMouseCursorWidgetByVisibilityType(const EInv_MouseCursorVisibilityType& MouseCursor);

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TMap<EInv_MouseCursorVisibilityType, TSubclassOf<UUserWidget>> MouseCursorWidgetClassMap;
	
	//update TileParameters and LastTileParameters
	void UpdateTileParameters(const FVector2D CanvasPosition, const FVector2D MousePosition);
	//determines what is actually happening when tile params are updated (change slots hover state, etc.)
	//concerned only with the situation when we have a hover item to highlight the range of grid slots this item can take
	//is NOT concerned with highlighting just one grid slot when we hover our mouse over it
	void OnTileParametersUpdated(FInv_TileParameters& Parameters);
	//Calculate starting coordinate for highlighting
	FIntPoint CalculateStartingCoordinate(const FIntPoint& Coordinate, const FIntPoint Dimensions, const EInv_TileQuadrant Quadrant) const;
	FInv_SpaceQueryResult CheckHoverPosition(const FIntPoint& Position, const FIntPoint& Dimensions);

	FInv_SlotAvailabilityResult HasRoomForItem(const UInv_ItemComponent* ItemComponent);
	
	EInv_ItemCategory GetItemCategory() const { return ItemCategory; }
	
	UFUNCTION()
	void AddItem(UInv_InventoryItem* Item);

	void SetOwningCanvasPanel(UCanvasPanel* InCanvasPanel);

	//Drop the current hover item.
	void DropItem();
private:
	TWeakObjectPtr<UInv_InventoryComponent> InventoryComponent;
	TWeakObjectPtr<UCanvasPanel> OwningCanvasPanel;
	
	UPROPERTY()
	TMap<EInv_MouseCursorVisibilityType, UUserWidget*> MouseCursorWidgetMap;
	
	//This function creates grid slots widgets, binds callbacks for on grid slot hover, unhover and click events, places slots widgets inside the array and draws them on screen
	void ConstructGrid();
	
	FInv_SlotAvailabilityResult HasRoomForItem(const UInv_InventoryItem* Item);

	//Does the heavy lifting
	FInv_SlotAvailabilityResult HasRoomForItem(const FInv_ItemManifest& ItemManifest);

	void AddItemToIndices(const FInv_SlotAvailabilityResult& Result, UInv_InventoryItem* NewItem);
	void AddItemAtIndex(UInv_InventoryItem* Item, const int32 Index, const bool bStackable, const int32 StackAmount);

	UFUNCTION()
	void OnSlottedItemHovered(UInv_InventoryItem* Item);

	FTimerHandle Description_Timer;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DescriptionTimerDelay = 0.5f;
	
	UFUNCTION()
	void OnSlottedItemUnhovered();
	
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

	UFUNCTION()
	void OnSlottedItemClicked(int32 GridIndex, const FPointerEvent& MouseEvent);
	void CreateItemPopup(const int32 GridIndex);

	UFUNCTION()
	void OnPopupMenuSplit(int32 SplitAmount, int32 Index);

	UFUNCTION()
	void OnPopupMenuDrop(int32 Index);

	UFUNCTION()
	void OnPopupMenuConsume(int32 Index);

	UFUNCTION()
	void OnGridSlotClicked(int32 GridIndex, const FPointerEvent& MouseEvent);

	UFUNCTION()
	void OnGridSlotHovered(int32 GridIndex, const FPointerEvent& MouseEvent);

	UFUNCTION()
	void OnGridSlotUnhovered(int32 GridIndex, const FPointerEvent& MouseEvent);
	
	bool IsLeftMouseClick(const FPointerEvent& MouseEvent) const;
	bool IsRightMouseClick(const FPointerEvent& MouseEvent) const;

	//On item clicked with left mouse button - Assign the hover item and remove the slotted item from the grid
	void PickUp(UInv_InventoryItem* ClickedInventoryItem, const int32 GridIndex);
	void AssignHoverItem(UInv_InventoryItem* InventoryItem);
	void AssignHoverItem(UInv_InventoryItem* InventoryItem, const int32 GridIndex, const int32 PreviousGridIndex);
	void ClearHoverItem();
	void RemoveItemFromGrid(UInv_InventoryItem* InventoryItem, const int32 GridIndex);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Inventory")
	EInv_ItemCategory ItemCategory;
	
	TArray<TObjectPtr<UInv_GridSlot>> GridSlots;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_GridSlot> GridSlotClass;

	UPROPERTY()
	TObjectPtr<UInv_ItemPopUp> ItemPopup;
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_ItemPopUp> ItemPopupClass;

	UPROPERTY()
	TObjectPtr<UInv_ItemDescription> ItemDescription;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_ItemDescription> ItemDescriptionClass;

	//These store information about the position of the mouse within the grid
	//(Specific slot, specific position within this slot)
	//These are constantly changing in NativeTick func
	FInv_TileParameters TileParameters;
	FInv_TileParameters LastTileParameters;

	//******* Bound Widgets *******//
	
	//where the grid slots are located
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;
	//******* Bound Widgets *******//

	//inventory items widgets
	//index will help us to move, remove things, etc.
	UPROPERTY()
	TMap<int32, TObjectPtr<UInv_SlottedItem>> SlottedItems;
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_SlottedItem> SlottedItemClass;

	UPROPERTY()
	TObjectPtr<UInv_HoverItem> HoverItem;
	

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInv_HoverItem> HoverItemClass;
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 Rows;
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 Columns;
	UPROPERTY(EditAnywhere, Category = "Inventory")
	float TileSize;

	// Index were an item would be placed if we click at the valid location
	int32 ItemDropIndex = INDEX_NONE;

	//CurrentSpaceQueryResult is updated as we move our mouse around, performing queries to see if it is hovering over a location
	//where if we clicked there would be a place valid for placing an item or swapping with it
	FInv_SpaceQueryResult CurrentSpaceQueryResult;

	bool bMouseWithinCanvas;
	bool bLastMouseWithinCanvas;

	int32 LastHighlightedGridIndex;
	FIntPoint LastHighlightedDimensions;
};
