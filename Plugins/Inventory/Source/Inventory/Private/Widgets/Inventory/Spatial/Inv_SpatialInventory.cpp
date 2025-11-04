// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Public/Widgets/Inventory/Spatial/Inv_SpatialInventory.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Inventory/Public/Widgets/Inventory/Spatial/Inv_InventoryGrid.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Inventory.h"

void UInv_SpatialInventory::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	Button_Equippables->OnClicked.AddDynamic(this, &ThisClass::ShowEquippables);
	Button_Craftables->OnClicked.AddDynamic(this, &ThisClass::ShowCraftables);
	Button_Consumables->OnClicked.AddDynamic(this, &ThisClass::ShowConsumables);

	Grid_Equippables->SetOwningCanvasPanel(CanvasPanel);
	Grid_Craftables->SetOwningCanvasPanel(CanvasPanel);
	Grid_Consumables->SetOwningCanvasPanel(CanvasPanel);
	
	ShowEquippables();
}

FReply UInv_SpatialInventory::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	ActiveGrid->DropItem();
	return FReply::Handled();
}

FInv_SlotAvailabilityResult UInv_SpatialInventory::HasRoomForItem(UInv_ItemComponent* ItemComponent) const
{
	switch (UInv_InventoryStatics::GetItemCategoryFromItemComponent(ItemComponent))
	{
	case EInv_ItemCategory::Equippable:
		return Grid_Equippables->HasRoomForItem(ItemComponent);
	case EInv_ItemCategory::Consumable:
		return Grid_Consumables->HasRoomForItem(ItemComponent);
	case EInv_ItemCategory::Craftable:
		return Grid_Craftables->HasRoomForItem(ItemComponent);
	default:
		UE_LOG(LogInventory, Error, TEXT("Item Component doesn't have a valid Item Category."));
		return FInv_SlotAvailabilityResult();
	}
}

void UInv_SpatialInventory::ShowEquippables()
{
	SetActiveGrid(Grid_Equippables, Button_Equippables);
}

void UInv_SpatialInventory::ShowCraftables()
{
	SetActiveGrid(Grid_Craftables, Button_Craftables);
}

void UInv_SpatialInventory::ShowConsumables()
{
	SetActiveGrid(Grid_Consumables, Button_Consumables);
}

void UInv_SpatialInventory::SetActiveGrid(UInv_InventoryGrid* Grid, UButton* DisableButton)
{
	if (ActiveGrid.IsValid()) ActiveGrid->SetMouseCursorWidgetByVisibilityType(EInv_MouseCursorVisibilityType::Hidden);
	ActiveGrid = Grid;
	if (ActiveGrid.IsValid()) ActiveGrid->SetMouseCursorWidgetByVisibilityType(EInv_MouseCursorVisibilityType::Visible);
	
	Button_Equippables->SetIsEnabled(true);
	Button_Craftables->SetIsEnabled(true);
	Button_Consumables->SetIsEnabled(true);
	DisableButton->SetIsEnabled(false);

	Switcher->SetActiveWidget(Grid);
}
