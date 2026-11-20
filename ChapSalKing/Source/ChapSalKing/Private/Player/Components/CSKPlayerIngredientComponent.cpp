#include "Player/Components/CSKPlayerIngredientComponent.h"

#include "Framework/CSKGameState.h"
#include "Ingredient/IngredientData.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"

UCSKPlayerIngredientComponent::UCSKPlayerIngredientComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCSKPlayerIngredientComponent::LoadIngredient(UIngredientData* Ingredient)
{
	if (!Ingredient)
	{
		return;
	}

	CurrentIngredient = Ingredient;
	CurrentGauge = Ingredient->MaxGauge;
	BroadcastState();
	BroadcastAcquiredEvent();
}

void UCSKPlayerIngredientComponent::ClearIngredient()
{
	CurrentIngredient = nullptr;
	CurrentGauge = 0.0f;
	BroadcastState();
}

bool UCSKPlayerIngredientComponent::ConsumeForShot()
{
	if (!CurrentIngredient)
	{
		return false;
	}

	CurrentGauge -= CurrentIngredient->GaugeCostPerShot;

	if (CurrentGauge <= 0.0f)
	{
		ClearIngredient();
		return false;
	}

	BroadcastState();
	return true;
}

float UCSKPlayerIngredientComponent::GetGaugeRatio() const
{
	return CurrentIngredient ? CurrentGauge / CurrentIngredient->MaxGauge : 0.0f;
}

void UCSKPlayerIngredientComponent::BroadcastState()
{
	OnIngredientStateChanged.Broadcast(CurrentIngredient, GetGaugeRatio());
}

void UCSKPlayerIngredientComponent::BroadcastAcquiredEvent() const
{
	ACSKGameState* GameState = GetWorld()->GetGameState<ACSKGameState>();
	if (!GameState)
	{
		return;
	}

	FCSKMatchEventInfo EventInfo(
		ECSKMatchEventType::ItemAcquired, CastChecked<APawn>(GetOwner())->GetPlayerState());
	EventInfo.SubjectName = CurrentIngredient->DisplayName;
	EventInfo.Affinity = CurrentIngredient->Affinity;

	GameState->BroadcastMatchEvent(EventInfo);
}
