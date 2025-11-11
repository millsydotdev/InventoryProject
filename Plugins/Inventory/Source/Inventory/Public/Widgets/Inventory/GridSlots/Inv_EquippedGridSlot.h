// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Inv_GridSlot.h"
#include "Inv_EquippedGridSlot.generated.h"

class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedGridSlotClicked, UInv_EquippedGridSlot*, GridSlot, const FGameplayTag&, EquipmentTypeTag);

/**
 * Special class for handling character equipment.
 */
UCLASS()
class INVENTORY_API UInv_EquippedGridSlot : public UInv_GridSlot
{
	GENERATED_BODY()

public:
	//~Begin UUserWidget Interface
	virtual void NativePreConstruct() override;
	//~End UUserWidget Interface
	
	//~Begin UInv_GridSlot Interface
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	//~End UInv_GridSlot Interface

	FOnEquippedGridSlotClicked OnEquippedGridSlotClicked;

	FGameplayTag& GetEquipmentTypeTag() { return EquipmentTypeTag; }

private:
	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (Categories = "GameItems.Equipment"))
	FGameplayTag EquipmentTypeTag;

	// Brush for the grayed out icon, can be set on an instance of this equipped grid slot
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FSlateBrush Brush_GrayedOutIcon;

	//******* Bound Widgets *******//

	// Icon that is shown as an indication of the item type that can be placed there.
	// Only visible when there is no equipment item placed there. 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_GrayedOutIcon;
	
	//******* Bound Widgets *******//
};
