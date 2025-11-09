// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inv_CompositeBase.h"
#include "Inv_Composite.generated.h"

/**
 *  Composite class has multiple children that might be composites themselves, but inevitably when we get to the bottom of the hierarchy,
 *  it's all going to come down to leafs.
 */
UCLASS()
class INVENTORY_API UInv_Composite : public UInv_CompositeBase
{
	GENERATED_BODY()
public:
	//~Begin UUserWidget Interface
	virtual void NativeOnInitialized() override;
	//~End UUserWidget Interface

	virtual void Collapse() override;

	virtual void ApplyFunction(FuncType Function) override;
	
private:
	TArray<TObjectPtr<UInv_CompositeBase>> Children;
};
