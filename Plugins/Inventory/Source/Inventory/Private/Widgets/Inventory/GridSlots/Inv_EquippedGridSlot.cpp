// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/GridSlots/Inv_EquippedGridSlot.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Widgets/Inventory/SlottedItems/Inv_EquippedSlottedItem.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"


void UInv_EquippedGridSlot::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	SetGridSlotState(GridSlotState);
	Image_GrayedOutIcon->SetBrush(Brush_GrayedOutIcon);
}

void UInv_EquippedGridSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!GetbIsAvailable()) return;
	
	if (UInv_HoverItem* HoverItem = UInv_InventoryStatics::GetHoverItem(GetOwningPlayer()); IsValid(HoverItem))
	{
		// check for a partial match with the EquipmentTypeTag
		// because more specific tag (like item type tag) checking against a more broad tag (like EquipmentTypeTag) will return true.
		if (HoverItem->GetItemType().MatchesTag(EquipmentTypeTag))
		{
			SetGridSlotState(EInv_GridSlotState::Occupied);
			Image_GrayedOutIcon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UInv_EquippedGridSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	if (!GetbIsAvailable()) return;
	
	if (UInv_HoverItem* HoverItem = UInv_InventoryStatics::GetHoverItem(GetOwningPlayer()); IsValid(HoverItem))
	{
		// check for a partial match with the EquipmentTypeTag
		// because more specific tag (like item type tag) checking against a more broad tag (like EquipmentTypeTag) will return true.
		if (HoverItem->GetItemType().MatchesTag(EquipmentTypeTag))
		{
			SetGridSlotState(EInv_GridSlotState::Unoccupied);
			Image_GrayedOutIcon->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

FReply UInv_EquippedGridSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	OnEquippedGridSlotClicked.Broadcast(this, EquipmentTypeTag);
	return FReply::Handled();
}

UInv_EquippedSlottedItem* UInv_EquippedGridSlot::OnItemEquipped(UInv_InventoryItem* Item,
	const FGameplayTag& EquipmentTag, float TileSize)
{
	// Check the Equipment Type Tag
	if (!EquipmentTag.MatchesTagExact(EquipmentTypeTag)) return nullptr;
	
	// Create Equipped Slotted Item widget
	EquippedSlottedItem = CreateWidget<UInv_EquippedSlottedItem>(GetOwningPlayer(), EquippedSlottedItemClass);
	
	// Set equipped slotted item's properties
	EquippedSlottedItem->SetInventoryItem(Item);
	
	// Set slotted item's equipment type tag
	EquippedSlottedItem->SetEquipmentTypeTag(EquipmentTag);
	
	// Hide stack count widget on the slotted item
	EquippedSlottedItem->UpdateStackCountText(0);
	
	// Set inventory item on this class (the equipped grid slot class)
	SetInventoryItem(Item);
	
	// Set the image brush on the equipped slotted item
	const FInv_ImageFragment* ImageFragment = GetFragmentByTag<FInv_ImageFragment>(Item, FragmentTags::IconFragment);
	if (!ImageFragment) return nullptr;
	FSlateBrush IconBrush = UWidgetBlueprintLibrary::MakeBrushFromTexture(ImageFragment->GetIcon());

	// Get grid dimensions
	const FInv_GridFragment* GridFragment = GetFragmentByTag<FInv_GridFragment>(Item, FragmentTags::GridFragment);
	if (!GridFragment) return nullptr;
	
	// Calc the draw size for equipped slotted item
	const FIntPoint GridDimensions = GridFragment->GetGridSize();
	const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2;
	const FVector2D DrawSize = GridDimensions * IconTileWidth;
	IconBrush.ImageSize = DrawSize;
	
	EquippedSlottedItem->SetImageBrush(IconBrush);

	// Add the slotted item to the child of this widget's overlay
	Overlay_Root->AddChildToOverlay(EquippedSlottedItem);
	
	FGeometry OverlayGeometry = Overlay_Root->GetCachedGeometry();
	auto OverlayPos = OverlayGeometry.Position;
	auto OverlaySize = OverlayGeometry.Size;

	const float LeftPadding = OverlaySize.X / 2.f - DrawSize.X / 2.f;
	const float TopPadding = OverlaySize.Y / 2.f - DrawSize.Y / 2.f;
	UOverlaySlot* OverlaySlot = UWidgetLayoutLibrary::SlotAsOverlaySlot(EquippedSlottedItem);

	OverlaySlot->SetPadding(FMargin(LeftPadding, TopPadding));
	
	// return the Equipped Slotted Item widget
	return EquippedSlottedItem;
}
