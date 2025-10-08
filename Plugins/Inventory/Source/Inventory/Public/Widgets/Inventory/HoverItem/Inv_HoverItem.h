// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "Inv_HoverItem.generated.h"

class UTextBlock;
class UInv_InventoryItem;
class UImage;

/**
 * The hover item is the item that will appear and follow the mouse
 * when an inventory Item on the grid has been clicked.
 */
UCLASS()
class INVENTORY_API UInv_HoverItem : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetImageBrush(FSlateBrush& ImageBrush) const;
	void UpdateStackCount(const int32 Count) const;

	FGameplayTag GetItemType() const;
	int32 GetStackCount() const {return StackCount;};
	bool IsStackable() const {return bIsStackable;};
	void SetIsStackable(bool NewIsStackable);

	int32 GetGridIndex() const { return GridIndex; };
	void SetGridIndex(const int32 InGridIndex) { GridIndex = InGridIndex; };

	FIntPoint GetGridDimensions() const { return GridDimensions; };
	void SetGridDimensions(const FIntPoint& InGridDimensions) { GridDimensions = InGridDimensions; };

	UInv_InventoryItem* GetInventoryItem() const;
	void SetInventoryItem(UInv_InventoryItem* NewInventoryItem);
	
private:
	//the grid index where we originally took the item
	int32 GridIndex;
	
	FIntPoint GridDimensions;

	TWeakObjectPtr<UInv_InventoryItem> InventoryItem;
	bool bIsStackable{false};
	int32 StackCount{0};
	
	//******* Bound Widgets *******//
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_StackCount;
	//******* Bound Widgets *******//
	
};
