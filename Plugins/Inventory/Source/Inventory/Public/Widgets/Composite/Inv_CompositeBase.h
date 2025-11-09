// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "Inv_CompositeBase.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_CompositeBase : public UUserWidget
{
	GENERATED_BODY()
public:
	FGameplayTag GetFragmentTag() {return FragmentTag;};
	void SetFragmentTag(FGameplayTag tag) {FragmentTag = tag;};

	virtual void Collapse() { SetVisibility(ESlateVisibility::Collapsed); };
	void Expand() { SetVisibility(ESlateVisibility::Visible); };

	using FuncType = TFunction<void(UInv_CompositeBase*)>;
	virtual void ApplyFunction(FuncType Function) {};
	
private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag FragmentTag;
};
