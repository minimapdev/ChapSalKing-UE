#include "Player/Components/CSKPlayerStatusVFXComponent.h"

#include "Components/CapsuleComponent.h"
#include "Framework/CSKGameInstance.h"
#include "Framework/CSKProjectSettingsData.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerMoveComponent.h"


UCSKPlayerStatusVFXComponent::UCSKPlayerStatusVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCSKPlayerStatusVFXComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACSKPlayerCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	Catalog = Settings ? Settings->StatusVFXCatalog : nullptr;
	if (!Catalog)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CSK] StatusVFXCatalog is not assigned in ProjectSettingsData."));
	}

	if (UCSKPlayerStatusComponent* Status = OwnerCharacter->GetStatusComponent())
	{
		Status->OnStatusChanged.AddDynamic(
			this,
			&UCSKPlayerStatusVFXComponent::HandleStatusChanged);
	}

	if (UCSKPlayerMoveComponent* Move = OwnerCharacter->GetMoveComponent())
	{
		Move->OnSpeedEffectsChanged.AddDynamic(
			this,
			&UCSKPlayerStatusVFXComponent::HandleSpeedEffectsChanged);

		bool bHasSlow = false;
		bool bHasHaste = false;
		float EffectiveFactor = 1.0f;
		Move->GetSpeedEffectState(bHasSlow, bHasHaste, EffectiveFactor);
		HandleSpeedEffectsChanged(bHasSlow, bHasHaste, EffectiveFactor);
	}
}

void UCSKPlayerStatusVFXComponent::PlayOneShot(ECSKStatusVisualType Type)
{
	if (!Catalog || !OwnerCharacter)
	{
		return;
	}

	const FCSKStatusVFXEntry* Entry = Catalog->Find(Type);
	if (!Entry || !Entry->ApplySystem)
	{
		return;
	}

	PlayApplyEffect(*Entry);
}

void UCSKPlayerStatusVFXComponent::HandleStatusChanged(
	ECSKStatusType Type,
	bool bActive,
	float)
{
	if (Type != ECSKStatusType::Confusion)
	{
		return;
	}

	SetEffectActive(ECSKStatusVisualType::Confusion, bActive);
}

void UCSKPlayerStatusVFXComponent::HandleSpeedEffectsChanged(
	bool bHasSlow,
	bool bHasHaste,
	float)
{
	SetEffectActive(ECSKStatusVisualType::Slow, bHasSlow);
	SetEffectActive(ECSKStatusVisualType::Haste, bHasHaste);

}

void UCSKPlayerStatusVFXComponent::SetEffectActive(ECSKStatusVisualType Type, bool bActive)
{
	if (!Catalog || !OwnerCharacter)
	{
		return;
	}

	UNiagaraComponent* Existing = ActiveComponents.FindRef(Type);
	if (!bActive)
	{
		if (Existing)
		{
			Existing->Deactivate();
		}
		return;
	}

	const FCSKStatusVFXEntry* Entry = Catalog->Find(Type);
	if (!Entry || !Entry->LoopSystem)
	{
		return;
	}

	if (Existing)
	{
		if (!Existing->IsActive())
		{
			PlayApplyEffect(*Entry);
			Existing->Activate(true);
		}
		return;
	}

	PlayApplyEffect(*Entry);
	if (UNiagaraComponent* NewComponent = CreateLoopComponent(*Entry))
	{
		ActiveComponents.Add(Type, NewComponent);
	}
}

UNiagaraComponent* UCSKPlayerStatusVFXComponent::CreateLoopComponent(
	const FCSKStatusVFXEntry& Entry) const
{
	UCapsuleComponent* Capsule = OwnerCharacter
		? OwnerCharacter->GetCapsuleComponent()
		: nullptr;
	if (!Capsule || !Entry.LoopSystem)
	{
		return nullptr;
	}

	UNiagaraComponent* Component = UNiagaraFunctionLibrary::SpawnSystemAttached(
		Entry.LoopSystem,
		Capsule,
		NAME_None,
		Entry.RelativeLocation,
		Entry.RelativeRotation,
		EAttachLocation::KeepRelativeOffset,
		false,
		true);
	if (Component)
	{
		Component->SetRelativeScale3D(Entry.Scale);
	}
	return Component;
}

void UCSKPlayerStatusVFXComponent::PlayApplyEffect(const FCSKStatusVFXEntry& Entry) const
{
	if (!OwnerCharacter || !Entry.ApplySystem)
	{
		return;
	}

	UNiagaraComponent* Component = UNiagaraFunctionLibrary::SpawnSystemAttached(
		Entry.ApplySystem,
		OwnerCharacter->GetCapsuleComponent(),
		NAME_None,
		Entry.RelativeLocation,
		Entry.RelativeRotation,
		EAttachLocation::KeepRelativeOffset,
		true,
		true);
	if (Component)
	{
		Component->SetRelativeScale3D(Entry.Scale);
	}
}
