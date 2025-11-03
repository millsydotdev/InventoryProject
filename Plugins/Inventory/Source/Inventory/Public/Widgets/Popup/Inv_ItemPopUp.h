// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inv_ItemPopUp.generated.h"

////////////////////////////
// Forward Declarations
class USizeBox;
class UButton;
class USlider;
class UTextBlock;

////////////////////////////
// Delegates
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnPopupMenuSplit, int32, SplitAmount, int32, Index);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPopupMenuDrop, int32, Index);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPopupMenuConsume, int32, Index);

/**
 * The Item Popup widget shows up when right-clicking on the item in the inventory grid.
 */
UCLASS()
class INVENTORY_API UInv_ItemPopUp : public UUserWidget
{
	GENERATED_BODY()

public:
	//~Begin UUserWidget Interface
	void NativeOnInitialized() override;
	void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	//~End UUserWidget Interface

	//These are broadcast in button callbacks.
	FOnPopupMenuSplit OnSplit;
	FOnPopupMenuDrop OnDrop;
	FOnPopupMenuConsume OnConsume;

	int32 GetGridIndex() const { return GridIndex; }
	void SetGridIndex(const int32 InGridIndex) { GridIndex = InGridIndex; }
	
	void CollapseSplitButton() const;
	void CollapseConsumeButton() const;
	void SetSliderParams(const float Max, const float Value) const;
	
	FVector2D GetBoxSize() const;
	
private:
	//Represents the index of the item, which info this popup is displaying.
	int32 GridIndex{INDEX_NONE};

	int32 GetSplitAmount() const;
	
	//******* Bound Widgets *******//
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Split;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Drop;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Consume;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> Slider_Split;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_SplitAmount;
	//******* Bound Widgets *******//

	UFUNCTION()
	void SplitButtonClicked();

	UFUNCTION()
	void DropButtonClicked();

	UFUNCTION()
	void ConsumeButtonClicked();

	UFUNCTION()
	void SliderValueChanged(float InValue);

	
};
