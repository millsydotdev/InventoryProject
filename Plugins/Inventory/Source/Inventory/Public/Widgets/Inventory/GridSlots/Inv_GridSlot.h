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
	DATA_ACCESSOR(int32, TileIndex);
	DATA_ACCESSOR(int32, StackCount);
	DATA_ACCESSOR(int32, UpperLeftIndex);
	DATA_ACCESSOR(bool, bIsAvailable);
	
	void SetGridSlotState(const EInv_GridSlotState InNewState);
	
	TWeakObjectPtr<UInv_InventoryItem> GetInventoryItem() const { return InventoryItem; }
	void SetInventoryItem(UInv_InventoryItem* Item);

private:
	int32 TileIndex;
	int32 StackCount;
	int32 UpperLeftIndex{INDEX_NONE};
	TWeakObjectPtr<UInv_InventoryItem> InventoryItem;

	bool bIsAvailable;

	UPROPERTY(EditDefaultsOnly, Category="Style")
	TMap<EInv_GridSlotState, FSlateBrush> StateBrushes;
	
	//current grid slot state
	EInv_GridSlotState GridSlotState;
	
	//******* Bound Widgets *******//
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_GridSlot;
	//******* Bound Widgets *******//
	
};
