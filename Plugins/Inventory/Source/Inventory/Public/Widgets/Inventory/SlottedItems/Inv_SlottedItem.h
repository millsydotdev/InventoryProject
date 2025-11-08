// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailTreeNode.h"
#include "Blueprint/UserWidget.h"
#include "Inv_SlottedItem.generated.h"

//Data Accessor macro for creating getter and setter 
#define DATA_ACCESSOR(type, parameter) \
	type Get##parameter() const {return parameter; } \
	void Set##parameter(type In##parameter) {parameter = In##parameter; }


class UInv_InventoryItem;
class UImage;
class UTextBlock;

//delegate to subscribe to in the inventory grid, when we need to determine what item has been clicked to move it around in the inventory.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSlottedItemClicked, int32, GridIndex, const FPointerEvent&, MouseEvent);

DECLARE_DYNAMIC_DELEGATE_OneParam(FSlottedItemHovered, UInv_InventoryItem*, Item);
DECLARE_DYNAMIC_DELEGATE(FSlottedItemUnhovered);

/**
 * Widget that gets displayed as an item in the inventory.
 */
UCLASS()
class INVENTORY_API UInv_SlottedItem : public UUserWidget
{
	GENERATED_BODY()
	
public:
	//~Begin UUserWidget Interface
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	//function overrides for item description
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	//~End UUserWidget Interface
	
	DATA_ACCESSOR(bool, bIsStackable);
	DATA_ACCESSOR(int32, GridIndex);
	DATA_ACCESSOR(FIntPoint, GridDimensions);
	
	UImage* GetImageIcon() const {return Image_Icon; }

	UInv_InventoryItem* GetInventoryItem() const {return InventoryItem.Get();}
	void SetInventoryItem(UInv_InventoryItem* Item);
	
	void SetImageBrush(const FSlateBrush& Brush) const;

	void UpdateStackCountText(int32 StackCount);

	FSlottedItemClicked OnSlottedItemClicked;

	FSlottedItemHovered OnSlottedItemHovered;
	FSlottedItemUnhovered OnSlottedItemUnhovered;

private:
	//******* Bound Widgets *******//
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_StackCount;
	//******* Bound Widgets *******//

	int32 GridIndex;
	FIntPoint GridDimensions;
	TWeakObjectPtr<UInv_InventoryItem> InventoryItem;
	bool bIsStackable{false};
};
