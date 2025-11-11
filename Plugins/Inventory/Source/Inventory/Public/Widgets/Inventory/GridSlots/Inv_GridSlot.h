// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inv_GridSlot.generated.h"

//Data Accessor macro for creating getter and setter 
#define DATA_ACCESSOR(type, parameter) \
	type Get##parameter() const {return parameter; } \
	void Set##parameter(type In##parameter) {parameter = In##parameter; }

class UInv_InventoryItem;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGridSlotEvent, int32, GridIndex, const FPointerEvent&, MouseEvent);

UENUM(BlueprintType)
enum class EInv_GridSlotState : uint8
{
	Unoccupied,
	Occupied,
	Selected,
	GrayedOut
};

/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_GridSlot : public UUserWidget
{
	GENERATED_BODY()
public:
	
	//~Begin UUserWidget Interface
	//Override a set of functions that are called on mouse click, hover and unhover events (https://www.udemy.com/course/unreal-engine-5-inventory-systems/learn/lecture/50371187#notes)
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	//~End UUserWidget Interface
	
	
	DATA_ACCESSOR(int32, TileIndex);
	DATA_ACCESSOR(int32, StackCount);
	DATA_ACCESSOR(int32, UpperLeftIndex);
	DATA_ACCESSOR(bool, bIsAvailable);
	
	void SetGridSlotState(const EInv_GridSlotState InNewState);
	
	TWeakObjectPtr<UInv_InventoryItem> GetInventoryItem() const { return InventoryItem; }
	void SetInventoryItem(UInv_InventoryItem* Item);

	FGridSlotEvent OnGridSlotClicked;
	FGridSlotEvent OnGridSlotHovered;
	FGridSlotEvent OnGridSlotUnhovered;

protected:
	
	//current grid slot state
	EInv_GridSlotState GridSlotState = EInv_GridSlotState::Unoccupied;

private:
	int32 TileIndex{INDEX_NONE};
	int32 StackCount{0};
	int32 UpperLeftIndex{INDEX_NONE};
	TWeakObjectPtr<UInv_InventoryItem> InventoryItem;

	bool bIsAvailable{true};

	UPROPERTY(EditAnywhere, Category="Style")
	TMap<EInv_GridSlotState, FSlateBrush> StateBrushes;
	
	//******* Bound Widgets *******//
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_GridSlot;
	//******* Bound Widgets *******//
	
};
