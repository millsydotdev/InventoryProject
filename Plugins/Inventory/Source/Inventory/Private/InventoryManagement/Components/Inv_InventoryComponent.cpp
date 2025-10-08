// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagement/Components/Inv_InventoryComponent.h"

#include "Items/Components/Inv_ItemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Inventory/InventoryBase/Inv_InventoryBase.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Fragments/Inv_ItemFragment.h"

UInv_InventoryComponent::UInv_InventoryComponent() : InventoryList(this)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
	
	bInventoryMenuOpen = false;
}

void UInv_InventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryList);
}

void UInv_InventoryComponent::TryAddItem(UInv_ItemComponent* ItemComponent)
{
	FInv_SlotAvailabilityResult Result = InventoryMenu->HasRoomForItem(ItemComponent);

	UInv_InventoryItem* FoundItem = InventoryList.FindFirstItemByTag(ItemComponent->GetItemManifest().GetItemType());
	Result.Item = FoundItem;

	if (Result.TotalRoomToFill == 0)
	{
		NoRoomInInventory.Broadcast();
		return;
	}

	//if we're trying to add item of the type that already exists and it's stackable
	if (Result.Item.IsValid() && Result.bStackable)
	{
		//Add stacks to the item that already exists in the inventory. We only want to update the stack count,
		//not create a new item of this type.
		OnStackChange.Broadcast(Result);
		Server_AddStacksToItem(ItemComponent, Result.TotalRoomToFill, Result.Remainder);
	}
	else if (Result.TotalRoomToFill > 0) //if the item doesn't exist in the inventory we should check if we have room for it
	{
		// This item type does not exist in the inventory. Create a new one and update all pertinent slots.
		Server_AddNewItem(ItemComponent, Result.bStackable ? Result.TotalRoomToFill : 0);
	}
}

void UInv_InventoryComponent::Server_AddNewItem_Implementation(UInv_ItemComponent* ItemComponent, int32 StackCount)
{
	//add Item to Fast Array Serializer
	UInv_InventoryItem* NewItem = InventoryList.AddEntry(ItemComponent);
	NewItem->SetTotalStackCount(StackCount);
	
	if (GetOwner()->GetNetMode() == NM_ListenServer || GetOwner()->GetNetMode() == NM_Standalone)
	{
		OnInventoryItemAdded.Broadcast(NewItem);
	}
	
	//Destroy owning actor of the actor component.
	ItemComponent->PickedUp();
}

void UInv_InventoryComponent::Server_AddStacksToItem_Implementation(UInv_ItemComponent* ItemComponent, int32 StackCount, int32 Remainder)
{
	const FGameplayTag& ItemType = IsValid(ItemComponent) ? ItemComponent->GetItemManifest().GetItemType() : FGameplayTag::EmptyTag;
	UInv_InventoryItem* Item = InventoryList.FindFirstItemByTag(ItemType);
	if (!IsValid(Item)) return;

	Item->SetTotalStackCount(Item->GetTotalStackCount() + StackCount);

	//Destroy the item if the remainder is 0
	//else update the stack count for the world pickup
	if (Remainder == 0)
	{
		ItemComponent->PickedUp();
	}
	else if (FInv_StackableFragment* StackableFragment = ItemComponent->GetItemManifest().GetFragmentOfTypeMutable<FInv_StackableFragment>())
	{
		StackableFragment->SetStackCount(Remainder);
	}
}



void UInv_InventoryComponent::ToggleInventoryMenu()
{
	if (bInventoryMenuOpen)
	{
		CloseInventoryMenu();
	}
	else
	{
		OpenInventoryMenu();
	}
}

void UInv_InventoryComponent::AddRepSubObj(UObject* SubObj)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && IsValid(SubObj))
	{
		AddReplicatedSubObject(SubObj);
	}
}


void UInv_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	ConstructInventory();
}

void UInv_InventoryComponent::ConstructInventory()
{
	OwningPlayerController = Cast<APlayerController>(GetOwner());
	checkf(OwningPlayerController.IsValid(), TEXT("Inventory Component needs to have Player Controller as Owner"));

	if (!OwningPlayerController->IsLocalController()) return;

	InventoryMenu = CreateWidget<UInv_InventoryBase>(OwningPlayerController.Get(), InventoryMenuClass);
	InventoryMenu->AddToViewport();
	
	CloseInventoryMenu();
}

void UInv_InventoryComponent::OpenInventoryMenu()
{
	if (!IsValid(InventoryMenu)) return;

	InventoryMenu->SetVisibility(ESlateVisibility::Visible);
	bInventoryMenuOpen = true;

	if (!OwningPlayerController.IsValid()) return;

	FInputModeGameAndUI InputModeGameAndUI;
	OwningPlayerController->SetInputMode(InputModeGameAndUI);
	OwningPlayerController->SetShowMouseCursor(true);
}

void UInv_InventoryComponent::CloseInventoryMenu()
{
	if (!IsValid(InventoryMenu)) return;

	InventoryMenu->SetVisibility(ESlateVisibility::Collapsed);
	bInventoryMenuOpen = false;

	if (!OwningPlayerController.IsValid()) return;

	FInputModeGameOnly InputModeGameOnly;
	OwningPlayerController->SetInputMode(InputModeGameOnly);
	OwningPlayerController->SetShowMouseCursor(false);
}


