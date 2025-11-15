
#include "Items/Fragments/Inv_ItemFragment.h"

#include "EquipmentManagement/EquipActor/Inv_EquipActor.h"
#include "Widgets/Composite/Inv_CompositeBase.h"
#include "Widgets/Composite/Leafs/Inv_Leaf_Image.h"
#include "Widgets/Composite/Leafs/Inv_Leaf_LabeledValue.h"
#include "Widgets/Composite/Leafs/Inv_Leaf_Text.h"

void FInv_InventoryItemFragment::Assimilate(UInv_CompositeBase* Composite) const
{
	//see if the tag matches with the widget tag and if so call expand on it (see the widget)
	if (!MatchesWidgetTag(Composite)) return;
	Composite->Expand();
}

bool FInv_InventoryItemFragment::MatchesWidgetTag(const UInv_CompositeBase* Composite) const
{
	return Composite->GetFragmentTag().MatchesTagExact(GetFragmentTag());
}

void FInv_ImageFragment::Assimilate(UInv_CompositeBase* Composite) const
{
	//see if the tag matches with the widget tag and if so call expand on it (see the widget)
	FInv_InventoryItemFragment::Assimilate(Composite); 
	if (!MatchesWidgetTag(Composite)) return;

	if (UInv_Leaf_Image* Image = Cast<UInv_Leaf_Image>(Composite); IsValid(Image))
	{
		Image->SetImage(Icon);
		Image->SetBoxSize(IconSize);
		Image->SetImageSize(IconSize);
	}
	
}

void FInv_TextFragment::Assimilate(UInv_CompositeBase* Composite) const
{
	FInv_InventoryItemFragment::Assimilate(Composite);
	if (!MatchesWidgetTag(Composite)) return;

	if (UInv_Leaf_Text* Text = Cast<UInv_Leaf_Text>(Composite); IsValid(Text))
	{
		Text->SetText(FragmentText);
	}
}

void FInv_LabeledNumberFragment::Manifest()
{
	FInv_InventoryItemFragment::Manifest();

	if (bRandomizeOnManifest)
	{
		Value = FMath::RandRange(MinValue, MaxValue);
		bRandomizeOnManifest = false;
	}
}

void FInv_LabeledNumberFragment::Assimilate(UInv_CompositeBase* Composite) const
{
	FInv_InventoryItemFragment::Assimilate(Composite);
	if (!MatchesWidgetTag(Composite)) return;

	if (UInv_Leaf_LabeledValue* LabeledValue = Cast<UInv_Leaf_LabeledValue>(Composite); IsValid(LabeledValue))
	{
		LabeledValue->SetText_Label(Text_Label, bCollapseLabel);

		FNumberFormattingOptions Options;
		Options.MinimumFractionalDigits = MinFractionalDigits;
		Options.MaximumFractionalDigits = MaxFractionalDigits;
		LabeledValue->SetText_Value(FText::AsNumber(Value, &Options), bCollapseLabel);
	}
}

void FInv_ConsumableFragment::OnConsume(APlayerController* PC)
{
	for (auto& Modifier : ConsumeModifiers)
	{
		auto& ModifierRef = Modifier.GetMutable();
		ModifierRef.OnConsume(PC);
	}
}

void FInv_ConsumableFragment::Assimilate(UInv_CompositeBase* Composite) const
{
	FInv_InventoryItemFragment::Assimilate(Composite);
	for (const auto& Modifier : ConsumeModifiers)
	{
		const auto& ModifierRef = Modifier.Get();
		ModifierRef.Assimilate(Composite);
	}
}

void FInv_ConsumableFragment::Manifest()
{
	FInv_InventoryItemFragment::Manifest();
	for (auto& Modifier : ConsumeModifiers)
	{
		auto& ModifierRef = Modifier.GetMutable();
		ModifierRef.Manifest();
	}
}

void FInv_StrengthModifierFragment::OnEquip(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Strength increased by %f."), GetValue()));
}

void FInv_StrengthModifierFragment::OnUnequip(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Strength decreased by %f."), GetValue()));
}

void FInv_EquipmentFragment::OnEquip(APlayerController* PC)
{
	if (bEquipped) return;
	
	for (auto& Modifier : EquipModifiers)
	{
		Modifier.GetMutable().OnEquip(PC);
	}

	bEquipped = true;
}

void FInv_EquipmentFragment::OnUnequip(APlayerController* PC)
{
	if (!bEquipped) return;
	
	for (auto& Modifier : EquipModifiers)
	{
		Modifier.GetMutable().OnUnequip(PC);
	}

	bEquipped = false;
}

AInv_EquipActor* FInv_EquipmentFragment::SpawnAttachedActor(USkeletalMeshComponent* AttachMesh) const
{
	if (!IsValid(EquipActorClass) || !IsValid(AttachMesh)) return nullptr;
	
	AInv_EquipActor* SpawnedActor = AttachMesh->GetWorld()->SpawnActor<AInv_EquipActor>(EquipActorClass);
	SpawnedActor->AttachToComponent(AttachMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketAttachPoint);
	
	return SpawnedActor;
}

void FInv_EquipmentFragment::DestroyAttachedActor() const
{
	if (EquippedActor.IsValid())
	{
		EquippedActor->Destroy();
	}
}

void FInv_EquipmentFragment::SetEquippedActor(AInv_EquipActor* EquipActor)
{
	EquippedActor = EquipActor;
}

void FInv_EquipmentFragment::Manifest()
{
	FInv_InventoryItemFragment::Manifest();
	for (auto& Modifier : EquipModifiers)
	{
		auto& ModifierRef = Modifier.GetMutable();
		ModifierRef.Manifest();
	}
}

void FInv_EquipmentFragment::Assimilate(UInv_CompositeBase* Composite) const
{
	FInv_InventoryItemFragment::Assimilate(Composite);
	
	for (const auto& Modifier : EquipModifiers)
	{
		Modifier.Get().Assimilate(Composite);
	}
}

void FInv_HealthPotionFragment::OnConsume(APlayerController* PC)
{
	//Get stats comp from the PC or the PC->GetPawn()
	//or get Ability System Component and apply a Gameplay Effect
	//or call an interface func for healing

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Health potion consumed! Healing %f player HP."), GetValue()));
}

void FInv_ManaPotionFragment::OnConsume(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Mana potion consumed! Mana replenished by %f."), GetValue()));
}
