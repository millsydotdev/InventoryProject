// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/GridSlots/Inv_GridSlot.h"
#include "Items/Inv_InventoryItem.h"
#include "IAutomationReport.h"
#include "Components/Image.h"

void UInv_GridSlot::SetGridSlotState(const EInv_GridSlotState InNewState)
{
	GridSlotState = InNewState;

	if (const FSlateBrush* Brush = StateBrushes.Find(InNewState))
	{
		Image_GridSlot->SetBrush(*Brush);
	}
	else
	{
		ensureMsgf(false, TEXT("Missing brush for %s"),
		  *UEnum::GetValueAsString(InNewState));
	}
}

void UInv_GridSlot::SetInventoryItem(UInv_InventoryItem* Item)
{
	InventoryItem = Item;
}
