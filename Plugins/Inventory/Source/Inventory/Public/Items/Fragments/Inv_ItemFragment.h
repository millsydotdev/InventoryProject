#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Inv_FragmentTags.h"
#include "StructUtils/InstancedStruct.h"
#include "Inv_ItemFragment.generated.h"

class AInv_EquipActor;
class UInv_CompositeBase;
class APlayerController;

/*
* Base Item Fragment (scroll down to see children)
*/
USTRUCT(BlueprintType)
struct FInv_ItemFragment
{
	GENERATED_BODY()

	FInv_ItemFragment() {}

	/*
	 * The only reason why there is a destructor, is because we need to be able to derive children from this struct,
	 * and since we're adding a destructor, we need to add all 5 special member functions, including destructor and all the listed below,
	 * to be precise about the implementation (but we're also defaulting 4 of them to the default behaviour).
	 */

	virtual ~FInv_ItemFragment() {}

	//copy-constructor
	FInv_ItemFragment(const FInv_ItemFragment&) = default;

	//copy assignment operator
	FInv_ItemFragment& operator=(const FInv_ItemFragment&) = default;

	//move constructor
	FInv_ItemFragment(FInv_ItemFragment&&) = default;

	//move assignment operator
	FInv_ItemFragment& operator=(FInv_ItemFragment&&) = default;

	
	FGameplayTag GetFragmentTag() const { return FragmentTag; }

	virtual void Manifest() {};
	
protected:
	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (Categories = "FragmentTags")) //Categories specify the "FragmentTags" tag
	FGameplayTag FragmentTag;
	
};

/*
* Inventory Item Fragment (Responsible for the description widget showing the info of the item - so specifically for assimilation into a widget.)
*/
USTRUCT(BlueprintType)
struct FInv_InventoryItemFragment : public FInv_ItemFragment
{
	GENERATED_BODY()
public:
	virtual void Assimilate(UInv_CompositeBase* Composite) const;

protected:
	bool MatchesWidgetTag(const UInv_CompositeBase* Composite) const;
};


/*
* Grid Item Fragment (mainly responsible for the size being taken by the item in the inventory)
*/
USTRUCT(BlueprintType)
struct FInv_GridFragment :public FInv_ItemFragment
{
	GENERATED_BODY()
	
public:
	//default constructor for this fragment (setting up tag, but can be done in bp as well)
	FInv_GridFragment() { FragmentTag = FragmentTags::GridFragment; }
	
	FIntPoint GetGridSize() const { return GridSize; }
	void SetGridSize(const FIntPoint& NewGridSize) { GridSize = NewGridSize; }
	float GetGridPadding() const { return GridPadding; }
	void SetGridPadding(const float NewGridPadding) { GridPadding = NewGridPadding; }

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FIntPoint GridSize{1,1};

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	float GridPadding{0.f};
};

/*
* Stackable Item Fragment
*/
USTRUCT(BlueprintType)
struct FInv_StackableFragment : public FInv_ItemFragment
{
	GENERATED_BODY()
public:
	//default constructor for this fragment (setting up tag, but can be done in bp as well)
	FInv_StackableFragment() { FragmentTag = FragmentTags::StackableFragment; }
	
	int32 GetMaxStackSize() const { return MaxStackSize; }
	int32 GetStackCount() const { return StackCount; }

	void SetStackCount(const int32 NewStackCount) { StackCount = NewStackCount; }
	
private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MaxStackSize{1};

	//currently more like item count?
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 StackCount{1};
	
};

/*
* Image Item Fragment (for any image icon we need)
*/
USTRUCT(BlueprintType)
struct FInv_ImageFragment : public FInv_InventoryItemFragment
{
	GENERATED_BODY()
public:
	//default constructor for this fragment (setting up tag, but can be done in bp as well)
	FInv_ImageFragment() { FragmentTag = FragmentTags::IconFragment; }
	
	UTexture2D* GetIcon() const { return Icon; }

	//~Begin FInv_InventoryItemFragment Interface
	virtual void Assimilate(UInv_CompositeBase* Composite) const override;
	//~End FInv_InventoryItemFragment Interface
	
private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TObjectPtr<UTexture2D> Icon{nullptr};
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FVector2D IconSize{44.f,44.f};
	
};

/*
* Text Item Fragment (for any text we need)
*/
USTRUCT(BlueprintType)
struct FInv_TextFragment : public FInv_InventoryItemFragment
{
	GENERATED_BODY()
public:
	//default constructor for this fragment (NOT setting up tag here, done in bp)
	FInv_TextFragment() {}
	
	FText GetText() const { return FragmentText; }
	void SetText(const FText& NewText) { FragmentText = NewText; }

	//~Begin FInv_ItemFragment Interface
	virtual void Assimilate(UInv_CompositeBase* Composite) const override;
	//~End FInv_ItemFragment Interface
	
private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FText FragmentText;
	
};

/*
* Labeled Number Item Fragment - for label containing name and value.
*/
USTRUCT(BlueprintType)
struct FInv_LabeledNumberFragment : public FInv_InventoryItemFragment
{
	GENERATED_BODY()
public:
	//default constructor for this fragment (NOT setting up tag here, done in bp)
	FInv_LabeledNumberFragment() {}

	//~Begin FInv_ItemFragment Interface
	virtual void Manifest() override;
	//~End FInv_ItemFragment Interface

	// When manifesting for the first time this fragment will randomize, but once equipped and dropped the item should
	// retain the same value, and randomization should not occur.
	bool bRandomizeOnManifest = true;

	//~Begin FInv_ItemFragment Interface
	virtual void Assimilate(UInv_CompositeBase* Composite) const override;
	//~End FInv_ItemFragment Interface

	float GetValue() const { return Value; }
	
private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FText Text_Label{};

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	float Value = 0.f;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float MinValue = 0.f;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float MaxValue = 0.f;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	bool bCollapseLabel = false;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	bool bCollapseValue = false;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MinFractionalDigits = 1;
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MaxFractionalDigits = 1;
	
};

/*
* Consumable Fragments Modifiers
*/

//Base
USTRUCT(BlueprintType)
struct FInv_ConsumeModifierFragment : public FInv_LabeledNumberFragment
{
	GENERATED_BODY()

	virtual void OnConsume(APlayerController* PC) {};
};

//Health Potion
USTRUCT(BlueprintType)
struct FInv_HealthPotionFragment : public FInv_ConsumeModifierFragment
{
	GENERATED_BODY()
public:
	
	virtual void OnConsume(APlayerController* PC) override;
};

//Mana Potion
USTRUCT(BlueprintType)
struct FInv_ManaPotionFragment : public FInv_ConsumeModifierFragment
{
	GENERATED_BODY()
public:
	
	virtual void OnConsume(APlayerController* PC) override;
};

/*
* Base Consumable Item Fragment - contains consume modifiers
*/
USTRUCT(BlueprintType)
struct FInv_ConsumableFragment : public FInv_InventoryItemFragment
{
	GENERATED_BODY()
public:
	FInv_ConsumableFragment() { FragmentTag = FragmentTags::ConsumableFragment; };

	//call these functions on all the child fragments
	virtual void OnConsume(APlayerController* PC);
	
	//~Begin FInv_ItemFragment Interface
	virtual void Assimilate(UInv_CompositeBase* Composite) const override;
	//~End FInv_ItemFragment Interface

	//~Begin FInv_ItemFragment Interface
	virtual void Manifest() override;
	//~End FInv_ItemFragment Interface

private:
	//subfragments for all the types of additional consume effects
	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FInv_ConsumeModifierFragment>> ConsumeModifiers;
};


/*
* Equip Modifier Fragments
*/

//Base
USTRUCT(BlueprintType)
struct FInv_EquipModifierFragment : public FInv_LabeledNumberFragment
{
	GENERATED_BODY()
public:
	
	virtual void OnEquip(APlayerController* PC) {};
	virtual void OnUnequip(APlayerController* PC) {};
};


//Strength
USTRUCT(BlueprintType)
struct FInv_StrengthModifierFragment : public FInv_EquipModifierFragment
{
	GENERATED_BODY()
public:
	
	virtual void OnEquip(APlayerController* PC) override;
	virtual void OnUnequip(APlayerController* PC) override;
};


/*
* Base Equipment Fragment - contains equip modifiers
*/
USTRUCT(BlueprintType)
struct FInv_EquipmentFragment : public FInv_InventoryItemFragment
{
	GENERATED_BODY()
public:
	//default constructor for this fragment (NOT setting up tag here, done in bp)
	FInv_EquipmentFragment() {}

	//Called first
	void OnEquip(APlayerController* PC);
	void OnUnequip(APlayerController* PC);

	//Called in equipment component
	AInv_EquipActor* SpawnAttachedActor(USkeletalMeshComponent* AttachMesh) const;
	void DestroyAttachedActor() const;

	FGameplayTag GetEquipmentType() const { return EquipmentType; }
	void SetEquippedActor(AInv_EquipActor* EquipActor);

	//~Begin FInv_ItemFragment Interface
	virtual void Manifest() override;
	//~End FInv_ItemFragment Interface

	//~Begin FInv_InventoryItemFragment Interface
	virtual void Assimilate(UInv_CompositeBase* Composite) const override;
	//~End FInv_InventoryItemFragment Interface
	
private:
	//subfragments for all the types of additional equip effects
	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FInv_EquipModifierFragment>> EquipModifiers;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<AInv_EquipActor> EquipActorClass = nullptr;

	TWeakObjectPtr<AInv_EquipActor> EquippedActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FName SocketAttachPoint = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag EquipmentType = FGameplayTag::EmptyTag;

	bool bEquipped = false;
};