// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"

#include "Inv_EquipActor.generated.h"

UCLASS(Blueprintable)
class INVENTORY_API AInv_EquipActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInv_EquipActor();

	FGameplayTag GetEquipmentTag() { return EquipmentType; };
	void SetEquipmentType(FGameplayTag InEquipmentType) { EquipmentType = InEquipmentType; };

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag EquipmentType;

};
