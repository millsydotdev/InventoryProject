// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inv_EquipmentComponent.generated.h"

struct FInv_EquipmentFragment;
struct FInv_ItemManifest;
class AInv_EquipActor;
class UInv_InventoryItem;
class UInv_InventoryComponent;
class APlayerController;
class USkeletalMeshComponent;

/*
 * Equipment Component - responsible for spawning in actors associated with inventory items and
 * attaching them to the owning pawn's skeletal mesh.
 * 
 */
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORY_API UInv_EquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	//called in OnBeginPlay
	void InitInventoryComponent();
	
	TWeakObjectPtr<APlayerController> OwningPlayerController;

	TWeakObjectPtr<USkeletalMeshComponent> OwningSkeletalMesh;
	
	TWeakObjectPtr<UInv_InventoryComponent> InventoryComponent;

	TArray<TObjectPtr<AInv_EquipActor>> EquippedActors;
	
	UFUNCTION()
	void OnItemEquipped(UInv_InventoryItem* EquippedItem);

	UFUNCTION()
	void OnItemUnequipped(UInv_InventoryItem* UnequippedItem);

	AInv_EquipActor* SpawnEquipActor(FInv_EquipmentFragment* EquipmentFragment, const FInv_ItemManifest& Manifest, USkeletalMeshComponent* AttachMesh);
	
	
	
};
