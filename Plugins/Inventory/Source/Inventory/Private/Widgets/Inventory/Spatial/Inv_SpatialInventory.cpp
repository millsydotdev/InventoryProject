// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Public/Widgets/Inventory/Spatial/Inv_SpatialInventory.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Inventory/Public/Widgets/Inventory/Spatial/Inv_InventoryGrid.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Inventory.h"
#include "Blueprint/WidgetTree.h"
#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "Items/Inv_InventoryItem.h"
#include "Widgets/Inventory/GridSlots/Inv_EquippedGridSlot.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"
#include "Widgets/Inventory/SlottedItems/Inv_EquippedSlottedItem.h"

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

	WidgetTree->ForEachWidget([this](UWidget* Widget)
		{
			if (UInv_EquippedGridSlot* EquippedGridSlot = Cast<UInv_EquippedGridSlot>(Widget); IsValid(EquippedGridSlot))
			{
				EquippedGridSlots.Add(EquippedGridSlot);
				
				EquippedGridSlot->OnEquippedGridSlotClicked.AddDynamic(this, &ThisClass::EquippedGridSlotClicked);
			}
		}
	);
}

void UInv_SpatialInventory::EquippedGridSlotClicked(UInv_EquippedGridSlot* EquippedGridSlot,
	const FGameplayTag& EquippedTypeTag)
{
	//Check if we can equip the hover item
	if (!CanEquipHoverItem(EquippedGridSlot, EquippedTypeTag)) return;

	//Create equipped slotted item and add it to the equipped grid slot (call EquippedGridSlot->OnItemEquipped())
	UInv_EquippedSlottedItem* EquippedSlottedItem = EquippedGridSlot->OnItemEquipped(GetHoverItem()->GetInventoryItem(), EquippedTypeTag, GetTileSize());
	EquippedSlottedItem->OnEquippedSlottedItemClicked.AddDynamic(this, &ThisClass::EquippedSlottedItemClicked);

	//Inform the server that we equipped an item (and unequip item as well)
	UInv_InventoryComponent* InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	check(IsValid(InventoryComponent));

	InventoryComponent->Server_EquipSlotClicked(GetHoverItem()->GetInventoryItem(), nullptr);
	if (GetOwningPlayer()->GetNetMode() != NM_DedicatedServer)
	{
		InventoryComponent->OnItemEquipped.Broadcast(GetHoverItem()->GetInventoryItem());
	}

	//Clear the Hover Item
	Grid_Equippables->ClearHoverItem();
}

void UInv_SpatialInventory::EquippedSlottedItemClicked(UInv_EquippedSlottedItem* EquippedSlottedItem)
{
	// Remove the item description
	Grid_Equippables->OnSlottedItemUnhovered();

	if (IsValid(GetHoverItem()) && GetHoverItem()->IsStackable()) return;

	// Get item to equip
	UInv_InventoryItem* ItemToEquip = IsValid(GetHoverItem()) ? GetHoverItem()->GetInventoryItem() : nullptr;
	
	// Get item to unequip
	UInv_InventoryItem* ItemToUnequip = EquippedSlottedItem->GetInventoryItem();
	
	// Get the equipped grid slot holding this item
	UInv_EquippedGridSlot* EquippedGridSlot = FindSlotWithEquippedItem(ItemToUnequip);
	
	// Clear the equipped grid slot of this item
	ClearSlotOfItem(EquippedGridSlot);

	// Assign prev. equipped item as hover item
	Grid_Equippables->AssignHoverItem(ItemToUnequip);
	
	// Remove the equipped slotted item from the equipped grid slot ()
	RemoveEquippedSlottedItem(EquippedSlottedItem);
	
	// Make a new equipped slotted item (for the item we held in hover item)
	MakeEquippedSlottedItem(EquippedSlottedItem, EquippedGridSlot, ItemToEquip);

	// Broadcast delegates for OnItemEquipped/OnItemUnequipped
	UInv_InventoryComponent* InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	check(IsValid(InventoryComponent));

	InventoryComponent->Server_EquipSlotClicked(ItemToEquip, ItemToUnequip);
	if (GetOwningPlayer()->GetNetMode() != NM_DedicatedServer)
	{
		InventoryComponent->OnItemEquipped.Broadcast(ItemToEquip);
		InventoryComponent->OnItemUnequipped.Broadcast(ItemToUnequip);
	}
}

void UInv_SpatialInventory::ClearSlotOfItem(UInv_EquippedGridSlot* EquippedGridSlot)
{
	if (IsValid(EquippedGridSlot))
	{
		EquippedGridSlot->SetInventoryItem(nullptr);
		EquippedGridSlot->SetEquippedSlottedItem(nullptr);
	}
}

void UInv_SpatialInventory::RemoveEquippedSlottedItem(UInv_EquippedSlottedItem* EquippedSlottedItem)
{
	if (!IsValid(EquippedSlottedItem)) return;

	// Unbind from the OnEquippedSlottedItemClicked delegate
	if (EquippedSlottedItem->OnEquippedSlottedItemClicked.IsAlreadyBound(this, &ThisClass::EquippedSlottedItemClicked))
	{
		EquippedSlottedItem->OnEquippedSlottedItemClicked.RemoveDynamic(this, &ThisClass::EquippedSlottedItemClicked);
	}
	
	// Remove from equipped slotted item from parent
	EquippedSlottedItem->RemoveFromParent();
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

bool UInv_SpatialInventory::HasHoverItem() const
{
	return Grid_Equippables->HasHoverItem();
}

UInv_HoverItem* UInv_SpatialInventory::GetHoverItem() const
{
	if (!ActiveGrid.IsValid()) return nullptr;
	
	return ActiveGrid->GetHoverItem();
}

float UInv_SpatialInventory::GetTileSize() const
{
	return Grid_Equippables->GetTileSize();
}

bool UInv_SpatialInventory::CanEquipHoverItem(UInv_EquippedGridSlot* EquippedGridSlot,
                                              const FGameplayTag& EquipmentTypeTag) const
{
	if (!IsValid(EquippedGridSlot) || EquippedGridSlot->GetInventoryItem().IsValid()) return false;

	UInv_HoverItem* HoverItem = GetHoverItem();
	if (!IsValid(HoverItem)) return false;

	UInv_InventoryItem* HeldItem = HoverItem->GetInventoryItem();

	return HasHoverItem() &&
		IsValid(HeldItem) &&
		!HoverItem->IsStackable() &&
		HeldItem->GetItemManifest().GetItemCategory() == EInv_ItemCategory::Equippable &&
		HeldItem->GetItemManifest().GetItemType().MatchesTag(EquipmentTypeTag);
}

// void UInv_SpatialInventory::OnItemHovered(UInv_InventoryItem* Item)
// {
// 	//Super::OnItemHovered(Item);
// 	if (HasHoverItem()) return;
// 	
// }
//
// void UInv_SpatialInventory::OnItemUnhovered()
// {
// 	//Super::OnItemUnhovered();
// }
//
// bool UInv_SpatialInventory::HasHoverItem() const
// {
// 	if (Grid_Consumables->HasHoverItem()) return true;
// 	if (Grid_Craftables->HasHoverItem()) return true;
// 	if (Grid_Equippables->HasHoverItem()) return true;
// 	return false;
// }

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
	if (ActiveGrid.IsValid())
	{
		ActiveGrid->SetMouseCursorWidgetByVisibilityType(EInv_MouseCursorVisibilityType::Hidden);
		Grid->OnHide();
	}
	
	Button_Equippables->SetIsEnabled(true);
	Button_Craftables->SetIsEnabled(true);
	Button_Consumables->SetIsEnabled(true);
	DisableButton->SetIsEnabled(false);

	ActiveGrid = Grid;
	ActiveGrid->SetMouseCursorWidgetByVisibilityType(EInv_MouseCursorVisibilityType::Visible);
	
	Switcher->SetActiveWidget(Grid);
}

void UInv_SpatialInventory::MakeEquippedSlottedItem(UInv_EquippedSlottedItem* EquippedSlottedItem,
	UInv_EquippedGridSlot* EquippedGridSlot, UInv_InventoryItem* ItemToEquip)
{
	if (!IsValid(EquippedGridSlot)) return;

	UInv_EquippedSlottedItem* SlottedItem = EquippedGridSlot->OnItemEquipped(
		ItemToEquip,
		EquippedSlottedItem->GetEquipmentTypeTag(),
		GetTileSize()
	);

	if (IsValid(SlottedItem))
	{
		SlottedItem->OnEquippedSlottedItemClicked.AddDynamic(this, &ThisClass::EquippedSlottedItemClicked);
	}
	
	EquippedGridSlot->SetEquippedSlottedItem(SlottedItem);
}

UInv_EquippedGridSlot* UInv_SpatialInventory::FindSlotWithEquippedItem(UInv_InventoryItem* EquippedItem)
{
	auto* FoundEquippedGridSlot = EquippedGridSlots.FindByPredicate(
		[EquippedItem](const UInv_EquippedGridSlot* GridSlot)
			{
				return GridSlot->GetInventoryItem() == EquippedItem;
			}
		);
	return FoundEquippedGridSlot ? *FoundEquippedGridSlot : nullptr;
}
