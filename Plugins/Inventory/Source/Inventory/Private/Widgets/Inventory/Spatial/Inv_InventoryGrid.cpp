// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Public/Widgets/Inventory/Spatial/Inv_InventoryGrid.h"

#include "Inventory.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/GridSlots/Inv_GridSlot.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"
#include "Widgets/Inventory/SlottedItems/Inv_SlottedItem.h"
#include "Widgets/Utils/Inv_WidgetUtils.h"

void UInv_InventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ConstructGrid();

	InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	InventoryComponent->OnInventoryItemAdded.AddDynamic(this, &ThisClass::AddItem);
	InventoryComponent->OnStackChange.AddDynamic(this, &ThisClass::AddStacks);
}

void UInv_InventoryGrid::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	const FVector2D CanvasPos = UInv_WidgetUtils::GetWidgetPosition(CanvasPanel);
	const FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());

	if (CursorExitedCanvas(CanvasPos, UInv_WidgetUtils::GetWidgetSize(CanvasPanel), MousePos))
	{
		return;
	}
	
	UpdateTileParameters(CanvasPos, MousePos);
}

bool UInv_InventoryGrid::CursorExitedCanvas(const FVector2D& CanvasPosition, const FVector2D& BoundarySize,
	const FVector2D& Location)
{
	bLastMouseWithinCanvas = bMouseWithinCanvas;
	bMouseWithinCanvas = UInv_WidgetUtils::IsWithinBounds(CanvasPosition, BoundarySize, Location);
	if (!bMouseWithinCanvas && bLastMouseWithinCanvas)
	{
		UnHighlightSlots(LastHighlightedGridIndex, LastHighlightedDimensions);
		return true;
	}
	return false;
}

void UInv_InventoryGrid::HighlightSlots(const int32 Index, const FIntPoint& Dimensions)
{
	if (!bMouseWithinCanvas) return;

	UnHighlightSlots(LastHighlightedGridIndex, LastHighlightedDimensions);
	
	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns,
	[&](UInv_GridSlot* GridSlot)
		{
			GridSlot->SetGridSlotState(EInv_GridSlotState::Occupied);
		}
	);
	LastHighlightedDimensions = Dimensions;
	LastHighlightedGridIndex = Index;
	
}

void UInv_InventoryGrid::UnHighlightSlots(const int32 Index, const FIntPoint& Dimensions)
{
	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns,
	[&](UInv_GridSlot* GridSlot)
		{
			if (GridSlot->GetbIsAvailable())
			{
				GridSlot->SetGridSlotState(EInv_GridSlotState::Unoccupied);
			}
			else
			{
				GridSlot->SetGridSlotState(EInv_GridSlotState::Occupied);
			}
		}
	);
}

void UInv_InventoryGrid::ChangeHoverType(const int32 Index, const FIntPoint& Dimensions,
	EInv_GridSlotState GridSlotState)
{
	UnHighlightSlots(LastHighlightedGridIndex, LastHighlightedDimensions);

	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns,
	[GridSlotState](UInv_GridSlot* GridSlot)
		{
			GridSlot->SetGridSlotState(GridSlotState);
		}
	);
	LastHighlightedDimensions = Dimensions;
	LastHighlightedGridIndex = Index;
}

void UInv_InventoryGrid::ClearHoverItem()
{
	if (!IsValid(HoverItem)) return;

	HoverItem->SetInventoryItem(nullptr);
	HoverItem->SetIsStackable(false);
	HoverItem->SetGridIndex(INDEX_NONE);
	HoverItem->UpdateStackCount(0);
	static FSlateBrush EmptyBrush = static_cast<FSlateBrush>(FSlateNoResource());
	HoverItem->SetImageBrush(EmptyBrush);

	HoverItem->RemoveFromParent();
	HoverItem = nullptr;

	//Show mouse cursor
	SetMouseCursorWidgetByVisibilityType(EInv_MouseCursorVisibilityType::Visible);
}

bool UInv_InventoryGrid::IsSameStackable(const UInv_InventoryItem* ClickedInventoryItem) const
{
	const bool bIsSameItem = ClickedInventoryItem == HoverItem->GetInventoryItem();
	const bool bIsStackable = ClickedInventoryItem->IsStackable();
	return bIsSameItem && bIsStackable;
}

void UInv_InventoryGrid::SwapWithHoverItem(UInv_InventoryItem* ClickedInventoryItem, const int32 GridIndex)
{
	if(!IsValid(HoverItem)) return;

	UInv_InventoryItem* TempInventoryItem = HoverItem->GetInventoryItem();
	const int32 TempStackCount = HoverItem->GetStackCount();
	const bool bTempIsStackable = HoverItem->IsStackable();

	//Keep the same prev grid index

	AssignHoverItem(ClickedInventoryItem, GridIndex,HoverItem->GetGridIndex());
	RemoveItemFromGrid(ClickedInventoryItem, GridIndex);
	AddItemAtIndex(TempInventoryItem, ItemDropIndex, bTempIsStackable, TempStackCount);
	UpdateGridSlots(TempInventoryItem, ItemDropIndex, bTempIsStackable, TempStackCount);
}

bool UInv_InventoryGrid::ShouldSwapStackCounts(const int32 RoomInClickedSlot, const int32 HoveredStackCount,
	const int32 MaxStackSize) const
{
	return RoomInClickedSlot == 0 && HoveredStackCount < MaxStackSize;
}

void UInv_InventoryGrid::SwapStackCounts(const int32 ClickedStackCount, const int32 HoveredStackCount,
	const int32 Index)
{
	UInv_GridSlot* GridSlot = GridSlots[Index];
	GridSlot->SetStackCount(HoveredStackCount);

	UInv_SlottedItem* ClickedSlottedItem = SlottedItems.FindChecked(Index);
	ClickedSlottedItem->UpdateStackCountText(HoveredStackCount);

	HoverItem->UpdateStackCount(ClickedStackCount);
}

void UInv_InventoryGrid::ConsumeHoverItemStacks(const int32 ClickedStackCount, const int32 HoveredStackCount,
	const int32 Index)
{
	const int32 AmountToTransfer = HoveredStackCount;
	const int32 NewClickedStackCount = ClickedStackCount + AmountToTransfer;

	GridSlots[Index]->SetStackCount(NewClickedStackCount);
	SlottedItems.FindChecked(Index)->UpdateStackCountText(NewClickedStackCount);
	ClearHoverItem();
	
	const FInv_GridFragment* GridFragment = GridSlots[Index]->GetInventoryItem()->GetItemManifest().GetFragmentOfType<FInv_GridFragment>();
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);
	HighlightSlots(Index, Dimensions);
}

void UInv_InventoryGrid::FillInStack(const int32 FillAmount, const int32 Remainder, const int32 Index)
{
	UInv_GridSlot* GridSlot = GridSlots[Index];
	const int32 NewStackCount = GridSlot->GetStackCount() + FillAmount;

	GridSlot->SetStackCount(NewStackCount);

	UInv_SlottedItem* ClickedSlottedItem = SlottedItems.FindChecked(Index);
	ClickedSlottedItem->UpdateStackCountText(NewStackCount);

	HoverItem->UpdateStackCount(Remainder);
}

void UInv_InventoryGrid::SetMouseCursorWidgetByVisibilityType(const EInv_MouseCursorVisibilityType& MouseCursor)
{
	// Use Find to safely access widget class
	TSubclassOf<UUserWidget>* WidgetClassPtr = MouseCursorWidgetClassMap.Find(MouseCursor);
	APlayerController* OwningPlayer = GetOwningPlayer();
 
	if (!WidgetClassPtr || !IsValid(*WidgetClassPtr) || !IsValid(OwningPlayer))
	{
		return; // Early out if missing class or player
	}
 
	// Safely find existing widget
	UUserWidget** WidgetPtr = MouseCursorWidgetMap.Find(MouseCursor);
	if (!WidgetPtr || !IsValid(*WidgetPtr))
	{
		// Create widget and add to map only if it doesn't exist or is invalid
		UUserWidget* NewWidget = CreateWidget<UUserWidget>(OwningPlayer, *WidgetClassPtr);
		if (IsValid(NewWidget))
		{
			MouseCursorWidgetMap.Add(MouseCursor, NewWidget);
			WidgetPtr = MouseCursorWidgetMap.Find(MouseCursor);
		}
	}
 
	// Set mouse cursor widget safely
	if (WidgetPtr && IsValid(*WidgetPtr))
	{
		OwningPlayer->SetMouseCursorWidget(EMouseCursor::Default, *WidgetPtr);
	}
}

void UInv_InventoryGrid::UpdateTileParameters(const FVector2D CanvasPosition, const FVector2D MousePosition)
{
	if (!bMouseWithinCanvas) return;
	
	LastTileParameters = TileParameters;
	
	// Calc the tile quadrant, tile index and coords
	const FIntPoint HoveredTileCoords{
		static_cast<int32>(FMath::FloorToInt((MousePosition.X - CanvasPosition.X) / TileSize)),
		static_cast<int32>(FMath::FloorToInt((MousePosition.Y - CanvasPosition.Y) / TileSize))
	};
	
	TileParameters.TileCoordinates = HoveredTileCoords;
	TileParameters.TileIndex = UInv_WidgetUtils::GetIndexFromPosition(HoveredTileCoords, Columns);
	
	EInv_TileQuadrant HoveredTileQuadrant{EInv_TileQuadrant::None};
	//init HoveredTileQuadrant
	{
		//calculate relative position within the current tile
		const float TileLocalX = FMath::Fmod(MousePosition.X - CanvasPosition.X, TileSize);
		const float TileLocalY = FMath::Fmod(MousePosition.Y - CanvasPosition.Y, TileSize);

		//determine which quadrant the mouse is in
		const bool bIsTop = TileLocalY < TileSize / 2.f; //Top if Y is in the upper half
		const bool bIsLeft = TileLocalX < TileSize / 2.f; //Left if X is in the left half
		
		if (bIsTop && bIsLeft) HoveredTileQuadrant = EInv_TileQuadrant::TopLeft;
		else if (bIsTop && !bIsLeft) HoveredTileQuadrant = EInv_TileQuadrant::TopRight;
		else if (!bIsTop && bIsLeft) HoveredTileQuadrant = EInv_TileQuadrant::BottomLeft;
		else if (!bIsTop && !bIsLeft) HoveredTileQuadrant = EInv_TileQuadrant::BottomRight;
	}
	
	TileParameters.TileQuadrant = HoveredTileQuadrant;
	
	// Handle highlight/unhighlight grid slots
	OnTileParametersUpdated(TileParameters);
}

void UInv_InventoryGrid::OnTileParametersUpdated(FInv_TileParameters& Parameters)
{
	if (!IsValid(HoverItem)) return;

	//Get Hover Item's dimensions
	const FIntPoint Dimensions = HoverItem->GetGridDimensions();
	
	//calc the starting coords for highlighting
	const FIntPoint StartingCoordinate = CalculateStartingCoordinate(Parameters.TileCoordinates, Dimensions, Parameters.TileQuadrant);
	ItemDropIndex = UInv_WidgetUtils::GetIndexFromPosition(StartingCoordinate, Columns);

	CurrentSpaceQueryResult = CheckHoverPosition(StartingCoordinate, Dimensions);

	if (CurrentSpaceQueryResult.bHasSpace)
	{
		HighlightSlots(ItemDropIndex, Dimensions);
		return;
	}
	
	UnHighlightSlots(LastHighlightedGridIndex, LastHighlightedDimensions);
	
	//only true if we have a single item that we can swap with
	if (CurrentSpaceQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentSpaceQueryResult.UpperLeftIndex))
	{
		//There is a single item in this space. We can swap, or add stacks.
		const FInv_GridFragment* GridFragment = GetFragmentByTag<FInv_GridFragment>(CurrentSpaceQueryResult.ValidItem.Get(), FragmentTags::GridFragment);
		if (!GridFragment) return;
		ChangeHoverType(CurrentSpaceQueryResult.UpperLeftIndex, GridFragment->GetGridSize(), EInv_GridSlotState::GrayedOut);
	}
}

FIntPoint UInv_InventoryGrid::CalculateStartingCoordinate(const FIntPoint& Coordinate, const FIntPoint Dimensions,
	const EInv_TileQuadrant Quadrant) const
{
	const int32 HasEvenWidth = Dimensions.X % 2 == 0 ? 1 : 0;
	const int32 HasEvenHeight = Dimensions.Y % 2 == 0 ? 1 : 0;

	FIntPoint StartingCoordinate;
	switch(Quadrant)
	{
	case EInv_TileQuadrant::TopLeft:
		StartingCoordinate.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X);
		StartingCoordinate.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y);
		break;
	case EInv_TileQuadrant::TopRight:
		StartingCoordinate.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X) + HasEvenWidth;
		StartingCoordinate.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y);
		break;
	case EInv_TileQuadrant::BottomLeft:
		StartingCoordinate.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X);
		StartingCoordinate.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y) + HasEvenHeight;
		break;
	case EInv_TileQuadrant::BottomRight:
		StartingCoordinate.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X) + HasEvenWidth;
		StartingCoordinate.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y) + HasEvenHeight;
		break;
	default:
		UE_LOG(LogInventory, Error, TEXT("Invalid quadrant!"));
		return FIntPoint(-1, -1);
	}
	return StartingCoordinate;
}

FInv_SpaceQueryResult UInv_InventoryGrid::CheckHoverPosition(const FIntPoint& Position,
	const FIntPoint& Dimensions)
{
	FInv_SpaceQueryResult Result;
	
	//are the dimensions within grid bounds?
	if (!IsInGridBounds(UInv_WidgetUtils::GetIndexFromPosition(Position, Columns), Dimensions)) return Result; //if we dont have room
	
	Result.bHasSpace = true;
	
	//if more than one of the indices are occupied with the same item, we need to see if they all have the same upper left index.
	TSet<int32> OccupiedUpperLeftIndices;
	UInv_InventoryStatics::ForEach2D(GridSlots, UInv_WidgetUtils::GetIndexFromPosition(Position, Columns), Dimensions, Columns,
		[&](const UInv_GridSlot* GridSlot)
		{
			if (GridSlot->GetInventoryItem().IsValid())
			{
				OccupiedUpperLeftIndices.Add(GridSlot->GetUpperLeftIndex());
				Result.bHasSpace = false;
			}
		}
	);
	
	//if so, is there only one item in the way (can we swap?)
	if (OccupiedUpperLeftIndices.Num() == 1) //single item at position - it's valid for swapping or combining
	{
		const int32 Index = *OccupiedUpperLeftIndices.CreateConstIterator();
		Result.ValidItem = GridSlots[Index]->GetInventoryItem();
		Result.UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex();
	}
	
	return Result;
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
	SlottedItem->OnSlottedItemClicked.AddDynamic(this, &ThisClass::OnSlottedItemClicked);

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

void UInv_InventoryGrid::AddStacks(FInv_SlotAvailabilityResult& Result)
{
	if (!MatchesCategory(Result.Item.Get())) return;

	for (const auto& Availability : Result.SlotAvailabilities)
	{
		if (Availability.bItemAtIndex)
		{
			const auto& GridSlot = GridSlots[Availability.Index];
			const auto& SlottedItem = SlottedItems.FindChecked(Availability.Index);
			SlottedItem->UpdateStackCountText(GridSlot->GetStackCount() + Availability.AmountToFill);
			GridSlot->SetStackCount(GridSlot->GetStackCount() + Availability.AmountToFill);
		}
		else
		{
			AddItemAtIndex(Result.Item.Get(), Availability.Index, Result.bStackable, Availability.AmountToFill);
			UpdateGridSlots(Result.Item.Get(), Availability.Index, Result.bStackable, Availability.AmountToFill);
		}
	}
}

void UInv_InventoryGrid::OnSlottedItemClicked(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	UE_LOG(LogInventory, Log, TEXT("OnSlottedItemClicked"));
	
	check(GridSlots.IsValidIndex(GridIndex));
	
	UInv_InventoryItem* ClickedInventoryItem = GridSlots[GridIndex]->GetInventoryItem().Get();
	
	if (!IsValid(HoverItem) && IsLeftMouseClick(MouseEvent))
	{
		PickUp(ClickedInventoryItem, GridIndex);
		return;
	}

	//Do the hovered item and the clicked inventory item share a type, and are they stackable?
	if (IsSameStackable(ClickedInventoryItem))
	{
		const int32 ClickedStackCount = GridSlots[GridIndex]->GetStackCount();
		const FInv_StackableFragment* StackableFragment = ClickedInventoryItem->GetItemManifest().GetFragmentOfType<FInv_StackableFragment>();
		const int32 MaxStackSize = StackableFragment->GetMaxStackSize();
		const int32 RoomInClickedSlot = MaxStackSize - ClickedStackCount;
		const int32 HoveredStackCount = HoverItem->GetStackCount();
		
		//Should swap stack counts? (Room in the clicked slot is 0 && HoveredStackCount < MaxStackSize)
		if (ShouldSwapStackCounts(RoomInClickedSlot, HoveredStackCount, MaxStackSize))
		{
			//Swap stack counts
			SwapStackCounts(ClickedStackCount, HoveredStackCount, GridIndex);
			return;
		}
		
		//Should consume hover item's stacks? (Room in the clicked slot >= HoverStackCount)
		if (RoomInClickedSlot >= HoveredStackCount)
		{
			//Consume hover item's stacks
			ConsumeHoverItemStacks(ClickedStackCount, HoveredStackCount, GridIndex);
			return;
		}
		
		//Should fill in the stacks of the clicked item? (and not consume hover item?)
		if (RoomInClickedSlot < HoveredStackCount)
		{
			FillInStack(RoomInClickedSlot, HoveredStackCount - RoomInClickedSlot, GridIndex);
			return;
		}
		
		//Clicked slot is already full - do nothing
		if (RoomInClickedSlot == 0)
		{
			return;
		}
	}
	
	//Swap with the hover item.
	SwapWithHoverItem(ClickedInventoryItem, GridIndex);
}

bool UInv_InventoryGrid::IsLeftMouseClick(const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
}

bool UInv_InventoryGrid::IsRightMouseClick(const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::RightMouseButton;
}

void UInv_InventoryGrid::PickUp(UInv_InventoryItem* ClickedInventoryItem, const int32 GridIndex)
{
	AssignHoverItem(ClickedInventoryItem, GridIndex, GridIndex);
	RemoveItemFromGrid(ClickedInventoryItem, GridIndex);
	//Remove clicked item from the grid
}

void UInv_InventoryGrid::AssignHoverItem(UInv_InventoryItem* InventoryItem)
{
	if (!IsValid(HoverItem))
	{
		HoverItem = CreateWidget<UInv_HoverItem>(GetOwningPlayer(), HoverItemClass);
	}
	const FInv_GridFragment* GridFragment = GetFragmentByTag<FInv_GridFragment>(InventoryItem, FragmentTags::GridFragment);
	const FInv_ImageFragment* ImageFragment = GetFragmentByTag<FInv_ImageFragment>(InventoryItem, FragmentTags::IconFragment);
	if (!GridFragment || !ImageFragment) return;

	const FVector2D DrawSize = GetDrawSize(GridFragment);

	FSlateBrush IconBrush;
	IconBrush.SetResourceObject(ImageFragment->GetIcon());
	IconBrush.DrawAs = ESlateBrushDrawType::Image;
	IconBrush.ImageSize = DrawSize * UWidgetLayoutLibrary::GetViewportScale(this);

	HoverItem->SetImageBrush(IconBrush);
	HoverItem->SetGridDimensions(GridFragment->GetGridSize());
	HoverItem->SetInventoryItem(InventoryItem);
	HoverItem->SetIsStackable(InventoryItem->IsStackable());

	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, HoverItem);
}

void UInv_InventoryGrid::AssignHoverItem(UInv_InventoryItem* InventoryItem,
										const int32 GridIndex,
										const int32 PreviousGridIndex)
{
	AssignHoverItem(InventoryItem);

	HoverItem->SetGridIndex(PreviousGridIndex);
	HoverItem->UpdateStackCount(InventoryItem->IsStackable() ? GridSlots[GridIndex]->GetStackCount() : 0);
}

void UInv_InventoryGrid::RemoveItemFromGrid(UInv_InventoryItem* InventoryItem, const int32 GridIndex)
{
	const FInv_GridFragment* GridFragment = GetFragmentByTag<FInv_GridFragment>(InventoryItem, FragmentTags::GridFragment);
	if (!GridFragment) return;

	UInv_InventoryStatics::ForEach2D(GridSlots, GridIndex, GridFragment->GetGridSize(), Columns,
		[&](UInv_GridSlot* GridSlot)
		{
			GridSlot->SetInventoryItem(nullptr);
			GridSlot->SetUpperLeftIndex(INDEX_NONE);
			GridSlot->SetGridSlotState(EInv_GridSlotState::Unoccupied);
			GridSlot->SetbIsAvailable(true);
			GridSlot->SetStackCount(0);
		}
	);

	if (SlottedItems.Contains(GridIndex))
	{
		TObjectPtr<UInv_SlottedItem> FoundSlottedItem;
		SlottedItems.RemoveAndCopyValue(GridIndex, FoundSlottedItem);
		FoundSlottedItem->RemoveFromParent();
	}
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
			
			GridSlot->OnGridSlotClicked.AddDynamic(this, &UInv_InventoryGrid::OnGridSlotClicked);
			GridSlot->OnGridSlotHovered.AddDynamic(this, &UInv_InventoryGrid::OnGridSlotHovered);
			GridSlot->OnGridSlotUnhovered.AddDynamic(this, &UInv_InventoryGrid::OnGridSlotUnhovered);
		}
	}
}

void UInv_InventoryGrid::OnGridSlotClicked(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	//if we're not holding a hover item - return
	if (!IsValid(HoverItem)) return;
	
	if (!GridSlots.IsValidIndex(ItemDropIndex)) return;

	//if we've clicked at the location that has a slotted item
	if (CurrentSpaceQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentSpaceQueryResult.UpperLeftIndex))
	{
		//forward request to on slotted item clicked
		OnSlottedItemClicked(CurrentSpaceQueryResult.UpperLeftIndex, MouseEvent);
		return;
	}

	//if there is no valid item at this index
	auto GridSlot = GridSlots[ItemDropIndex];
	if (!GridSlot->GetInventoryItem().IsValid())
	{
		//Put item down at this index.
		AddItemAtIndex(HoverItem->GetInventoryItem(), ItemDropIndex, HoverItem->IsStackable(), HoverItem->GetStackCount());
		UpdateGridSlots(HoverItem->GetInventoryItem(), ItemDropIndex, HoverItem->IsStackable(), HoverItem->GetStackCount());
		ClearHoverItem();
	}
	
}

void UInv_InventoryGrid::OnGridSlotHovered(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (IsValid(HoverItem)) return;

	UInv_GridSlot* GridSlot = GridSlots[GridIndex];
	if (GridSlot->GetbIsAvailable())
	{
		GridSlot->SetGridSlotState(EInv_GridSlotState::Occupied);
	}
}

void UInv_InventoryGrid::OnGridSlotUnhovered(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (IsValid(HoverItem)) return;

	UInv_GridSlot* GridSlot = GridSlots[GridIndex];
	if (GridSlot->GetbIsAvailable())
	{
		GridSlot->SetGridSlotState(EInv_GridSlotState::Unoccupied);
	}
}

bool UInv_InventoryGrid::MatchesCategory(const UInv_InventoryItem* Item) const
{
	return Item->GetItemManifest().GetItemCategory() == ItemCategory;
}
