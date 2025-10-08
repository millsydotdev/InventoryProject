// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Public/Widgets/Inventory/Spatial/Inv_InventoryGrid.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/GridSlots/Inv_GridSlot.h"
#include "Widgets/Inventory/SlottedItems/Inv_SlottedItem.h"
#include "Widgets/Utils/Inv_WidgetUtils.h"

void UInv_InventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ConstructGrid();

	InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	InventoryComponent->OnInventoryItemAdded.AddDynamic(this, &ThisClass::AddItem);
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const UInv_ItemComponent* ItemComponent)
{
	return HasRoomForItem(ItemComponent->GetItemManifest());
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const UInv_InventoryItem* Item)
{
	return HasRoomForItem(Item->GetItemManifest());
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const FInv_ItemManifest& ItemManifest)
{
	/*
	*	Here is a rough breakdown of what we're going to do to determine FInv_SlotAvailabilityResult struct.
	*/
	
	//Determine if the item is stackable.
	//Determine how many stacks to add

	//for each Grid Slot:
		//if we don't have anymore to fill - break out of the loop
		//is this index claimed yet?
		//can item fit here? (i.e. is it not out of bound of the grid?)
		//Is there room at this index (i.e. are there no other items in the way?)
		//check any other important conditions - for each 2D over a 2D range
			//is the index claimed
			//Has valid item?
			//is this slot an upper left slot?
			//is this item the same type as the item we're trying to add?
			//if so is this a stackable item?
			//if stackable, is this slot at the max stack size already?
		//how much to fill
		//update amount left to fill
	//how much is the remainder?

	
	/*
	*	The actual logic:
	*/
	
	FInv_SlotAvailabilityResult Result;
	
	//Determine if the item is stackable.
	const FInv_StackableFragment* StackableFragment = ItemManifest.GetFragmentOfType<FInv_StackableFragment>();
	Result.bStackable = StackableFragment != nullptr;

	//Determine how many items to add
	const int32 MaxStackSize = Result.bStackable ? StackableFragment->GetMaxStackSize() : 1;
	int32 AmountToFill = Result.bStackable ? StackableFragment->GetStackCount() : 1; //amount to fill will get smaller as we allocate space for each amount of item we're trying to add

	//we can only have unique indices inside a default TSet
	TSet<int32> CheckedIndices;

	//for each Grid Slot:
	for (const auto& GridSlot : GridSlots)
	{
		//in case we don't have anymore to fill - break out of the loop
		if (AmountToFill == 0) break;
		
		//is this index claimed yet?
		if (IsIndexClaimed(CheckedIndices, GridSlot->GetTileIndex())) continue;

		//is it not out of bound of the grid? Prevents adding the items that can stick out of the grid
		if (!IsInGridBounds(GridSlot->GetTileIndex(), GetItemDimensions(ItemManifest))) continue;
		
		//can item fit here? 
		//TentativelyClaimed - collection of indices that may be claimed.
		TSet<int32> TentativelyClaimed;
		if (!HasRoomAtIndex(GridSlot, GetItemDimensions(ItemManifest), CheckedIndices, TentativelyClaimed, ItemManifest.GetItemType(), MaxStackSize))
		{
			continue;
		}
		
		//how much to fill
		const int32 AmountToFillInSlot = DetermineFillAmountForSlot(Result.bStackable, MaxStackSize, AmountToFill, GridSlot);
		if (AmountToFillInSlot == 0) continue;

		CheckedIndices.Append(TentativelyClaimed);
		
		//update amount left to fill
		Result.TotalRoomToFill += AmountToFillInSlot;
		Result.SlotAvailabilities.Emplace(
			FInv_SlotAvailability{
				HasValidItem(GridSlot) ? GridSlot->GetUpperLeftIndex() : GridSlot->GetTileIndex(),
				Result.bStackable ? AmountToFillInSlot : 0,
				HasValidItem(GridSlot)
			}
		);

		AmountToFill -= AmountToFillInSlot;
		
		//how much is the remainder?
		Result.Remainder = AmountToFill;
		
		if (AmountToFill == 0) return Result;
	}
	
	
	/*
	*	Debug Test
	*/
	// Result.TotalRoomToFill = 7;
	// Result.bStackable = true;
	//
	// FInv_SlotAvailability SlotAvailability;
	// SlotAvailability.AmountToFill = 2;
	// SlotAvailability.Index = 0;
	//
	// Result.SlotAvailabilities.Add(MoveTemp(SlotAvailability));
	
	return Result;
}

void UInv_InventoryGrid::AddItem(UInv_InventoryItem* Item)
{
	if (!MatchesCategory(Item)) return;

	FInv_SlotAvailabilityResult Result = HasRoomForItem(Item);

	AddItemToIndices(Result, Item);
}

void UInv_InventoryGrid::AddItemToIndices(const FInv_SlotAvailabilityResult& Result, UInv_InventoryItem* NewItem)
{
	for (const auto& Availability : Result.SlotAvailabilities)
	{
		AddItemAtIndex(NewItem, Availability.Index, Result.bStackable, Availability.AmountToFill);
		UpdateGridSlots(NewItem, Availability.Index, Result.bStackable, Availability.AmountToFill);
	}
	
}

void UInv_InventoryGrid::AddItemAtIndex(UInv_InventoryItem* Item, const int32 Index, const bool bStackable,
	const int32 StackAmount)
{
	//get grid fragment so we know how much space the item is going to take
	const FInv_GridFragment* GridFragment = GetFragmentByTag<FInv_GridFragment>(Item, FragmentTags::GridFragment);
	
	//get image fragment to have icon
	const FInv_ImageFragment* ImageFragment = GetFragmentByTag<FInv_ImageFragment>(Item, FragmentTags::IconFragment);

	if (!GridFragment || !ImageFragment) return;
	
	UInv_SlottedItem* SlottedItem = CreateSlottedItem(Item, bStackable, StackAmount, GridFragment, ImageFragment, Index);

	//Add slotted item to the canvas panel
	AddSlottedItemToCanvas(Index, GridFragment, SlottedItem);
	
	//Store the newly created widget in a container
	SlottedItems.Add(Index, SlottedItem);
}

UInv_SlottedItem* UInv_InventoryGrid::CreateSlottedItem(UInv_InventoryItem* Item, const bool bStackable,
	const int32 StackAmount, const FInv_GridFragment* GridFragment, const FInv_ImageFragment* ImageFragment,
	const int32 Index)
{
	UInv_SlottedItem* SlottedItem = CreateWidget<UInv_SlottedItem>(GetOwningPlayer(), SlottedItemClass);
	SlottedItem->SetInventoryItem(Item);

	SetSlottedItemImage(SlottedItem, GridFragment, ImageFragment);

	SlottedItem->SetGridIndex(Index);
	SlottedItem->SetbIsStackable(bStackable);
	const int32 StackUpdateAmount = bStackable ? StackAmount : 0;
	SlottedItem->UpdateStackCountText(StackUpdateAmount);

	return SlottedItem;
}

void UInv_InventoryGrid::SetSlottedItemImage(const UInv_SlottedItem* SlottedItem, const FInv_GridFragment* GridFragment,
                                             const FInv_ImageFragment* ImageFragment) const
{
	FSlateBrush Brush;
	Brush.SetResourceObject(ImageFragment->GetIcon());
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.ImageSize = GetDrawSize(GridFragment);
	
	SlottedItem->SetImageBrush(Brush);
}

void UInv_InventoryGrid::AddSlottedItemToCanvas(const int32 Index, const FInv_GridFragment* GridFragment,
	UInv_SlottedItem* SlottedItem) const
{
	CanvasPanel->AddChild(SlottedItem);

	UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(SlottedItem);
	CanvasSlot->SetSize(GetDrawSize(GridFragment));
	
	const FVector2D DrawPos = UInv_WidgetUtils::GetPositionFromIndex(Index, Columns) * TileSize;
	const FVector2D DrawPosWithPadding = DrawPos + FVector2D(GridFragment->GetGridPadding());

	CanvasSlot->SetPosition(DrawPosWithPadding);
}

void UInv_InventoryGrid::UpdateGridSlots(UInv_InventoryItem* NewItem, const int32 Index, bool bStackableItem, const int32 StackAmount)
{
	check(GridSlots.IsValidIndex(Index));

	if (bStackableItem)
	{
		//Upper left index
		GridSlots[Index]->SetStackCount(StackAmount);
	}

	const FInv_GridFragment* GridFragment = GetFragmentByTag<FInv_GridFragment>(NewItem, FragmentTags::GridFragment);

	//if we don't have a grid fragment - assume the size of 1:1
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);

	//actually add the item
	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns,
		[&](UInv_GridSlot* GridSlot)
		{
			GridSlot->SetInventoryItem(NewItem);
			GridSlot->SetUpperLeftIndex(Index);
			GridSlot->SetbIsAvailable(false);
			//cosmetic - set occupied texture on all the grid slots where the item is
			GridSlot->SetGridSlotState(EInv_GridSlotState::Occupied);
		}
	);
	
}

bool UInv_InventoryGrid::IsIndexClaimed(const TSet<int32>& CheckedIndices, const int32 Index) const
{
	return CheckedIndices.Contains(Index);
}

FIntPoint UInv_InventoryGrid::GetItemDimensions(const FInv_ItemManifest& ItemManifest) const
{
	const FInv_GridFragment* GridFragment = ItemManifest.GetFragmentOfType<FInv_GridFragment>();
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);

	return Dimensions;
}

bool UInv_InventoryGrid::HasRoomAtIndex(const UInv_GridSlot* GridSlot,
										const FIntPoint& Dimensions,
										const TSet<int32>& CheckedIndices,
										TSet<int32>& OutTentativelyClaimed,
										const FGameplayTag& ItemType,
										const int32 MaxStackSize)
{
	//Is there room at this index (i.e. are there no other items in the way?)
	
	bool bHasRoom = true;

	UInv_InventoryStatics::ForEach2D(GridSlots, GridSlot->GetTileIndex(), Dimensions, Columns,
		[&](UInv_GridSlot* SubGridSlot)
		{
			//Check slot constraints
			if (CheckSlotConstraints(GridSlot, SubGridSlot, CheckedIndices, OutTentativelyClaimed, ItemType, MaxStackSize))
			{
				OutTentativelyClaimed.Add(SubGridSlot->GetTileIndex());
			}

			else
			{
				bHasRoom = false;
			}
		}
	);
	
	return bHasRoom;
}

bool UInv_InventoryGrid::CheckSlotConstraints(const UInv_GridSlot* UpperLeftGridSlot,
	const UInv_GridSlot* SubGridSlot,
	const TSet<int32>& CheckedIndices,
	TSet<int32>& OutTentativelyClaimed,
	const FGameplayTag& ItemType,
	const int32 MaxStackSize) const
{
	//is the index claimed
	if (IsIndexClaimed(CheckedIndices, SubGridSlot->GetTileIndex())) return false;
	
	//Has valid item?
	if (!HasValidItem(SubGridSlot))
	{
		OutTentativelyClaimed.Add(SubGridSlot->GetTileIndex());
		return true;
	}

	//is this slot an upper left slot?
	if (!IsUpperLeftGridSlot(UpperLeftGridSlot, SubGridSlot)) return false;
	
	const UInv_InventoryItem* SubItem = SubGridSlot->GetInventoryItem().Get();
		
	//is this a stackable item?
	if (!SubItem->IsStackable()) return false;
		
	//is this item the same type as the item we're trying to add?
	if (!DoesItemTypeMatch(SubItem, ItemType)) return false;
	
	//if stackable, is this slot at the max stack size already?
	if (UpperLeftGridSlot->GetStackCount() >= MaxStackSize) return false;
	
	return true;
}

bool UInv_InventoryGrid::IsUpperLeftGridSlot(const UInv_GridSlot* GridSlot, const UInv_GridSlot* SubGridSlot) const
{
	return SubGridSlot->GetUpperLeftIndex() == GridSlot->GetTileIndex();
}

bool UInv_InventoryGrid::HasValidItem(const UInv_GridSlot* GridSlot) const
{
	return GridSlot->GetInventoryItem().IsValid();
}

bool UInv_InventoryGrid::DoesItemTypeMatch(const UInv_InventoryItem* SubItem, const FGameplayTag& ItemType) const
{
	return SubItem->GetItemManifest().GetItemType().MatchesTagExact(ItemType);
}

bool UInv_InventoryGrid::IsInGridBounds(const int32 StartIndex, const FIntPoint& ItemDimensions)
{
	if (StartIndex < 0 || StartIndex >= GridSlots.Num()) return false;

	const int32 EndColumn = (StartIndex % Columns) + ItemDimensions.X;
	const int32 EndRow = (StartIndex / Columns) + ItemDimensions.Y;

	return EndColumn <= Columns && EndRow <= Rows;
}

int32 UInv_InventoryGrid::DetermineFillAmountForSlot(const bool bStackable,
													const int32 MaxStackSize,
													const int32 AmountToFill,
													const UInv_GridSlot* GridSlot) const
{
	//calc room in the slot
	const int32 RoomInSlot = MaxStackSize - GetStackAmount(GridSlot);
	
	//if stackable, need the min betweenAmount to fill and RoomInSlot
	return bStackable ? FMath::Min(AmountToFill, RoomInSlot) : 1;
}

int32 UInv_InventoryGrid::GetStackAmount(const UInv_GridSlot* GridSlot) const
{
	int32 CurrentSlotStackCount = GridSlot->GetStackCount();
	//If we are at the slot that doesn't hold the stack count, we must get the stack count
	if (const int UpperLeftIndex = GridSlot->GetUpperLeftIndex(); UpperLeftIndex != INDEX_NONE)
	{
		UInv_GridSlot* UpperLeftGridSlot = GridSlots[UpperLeftIndex];
		CurrentSlotStackCount = UpperLeftGridSlot->GetStackCount();
	}
	return CurrentSlotStackCount;
}

FVector2D UInv_InventoryGrid::GetDrawSize(const FInv_GridFragment* GridFragment) const
{
	//accounting for left padding and right padding
	const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2;
	
	return GridFragment->GetGridSize() * IconTileWidth;
}

void UInv_InventoryGrid::ConstructGrid()
{
	//determine the array size right from the start (performance)
	GridSlots.Reserve(Rows * Columns);

	for (int32 j = 0; j<Rows; ++j)
	{
		for (int32 i = 0; i<Columns; ++i)
		{
			UInv_GridSlot* GridSlot = CreateWidget<UInv_GridSlot>(this, GridSlotClass);
			
			CanvasPanel->AddChildToCanvas(GridSlot);

			//get index in the array, corresponding to 2D coords i and j
			const FIntPoint Pos = FIntPoint(i, j);
			int32 Index = UInv_WidgetUtils::GetIndexFromPosition(Pos, Columns);
			GridSlot->SetTileIndex(Index);

			UCanvasPanelSlot* GridCanvasPanelSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridSlot);
			GridCanvasPanelSlot->SetSize(FVector2D(TileSize));
			GridCanvasPanelSlot->SetPosition(Pos * TileSize);

			GridSlots.Add(GridSlot);
		}
	}
}

bool UInv_InventoryGrid::MatchesCategory(const UInv_InventoryItem* Item) const
{
	return Item->GetItemManifest().GetItemCategory() == ItemCategory;
}
