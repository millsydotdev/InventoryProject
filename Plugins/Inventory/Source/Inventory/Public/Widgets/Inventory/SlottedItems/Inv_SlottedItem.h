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

/**
 * Widget that gets displayed as an item in the inventory.
 */
UCLASS()
class INVENTORY_API UInv_SlottedItem : public UUserWidget
{
	GENERATED_BODY()
	
public:
	DATA_ACCESSOR(bool, bIsStackable);
	DATA_ACCESSOR(int32, GridIndex);
	DATA_ACCESSOR(FIntPoint, GridDimensions);
	
	UImage* GetImageIcon() const {return Image_Icon; }

	UInv_InventoryItem* GetInventoryItem() const {return InventoryItem.Get();}
	void SetInventoryItem(UInv_InventoryItem* Item);
	
	void SetImageBrush(const FSlateBrush& Brush) const;

	void UpdateStackCountText(int32 StackCount);

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
