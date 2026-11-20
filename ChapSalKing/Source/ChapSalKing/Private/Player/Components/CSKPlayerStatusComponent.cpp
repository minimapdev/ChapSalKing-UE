#include "Player/Components/CSKPlayerStatusComponent.h"

#include "Player/CSKPlayerCharacter.h"
#include "Engine/World.h"

UCSKPlayerStatusComponent::UCSKPlayerStatusComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCSKPlayerStatusComponent::InitializeStatus()
{
	CacheOwnerCSKCharacter();
}

bool UCSKPlayerStatusComponent::TryApplyStatus(ECSKStatusType Type, float DurationSeconds, ECSKAffinity SourceAffinity)
{
	UWorld* World = GetWorld();
	if (!World || DurationSeconds <= 0.0f || IsImmuneToStatusFrom(SourceAffinity))
	{
		return false;
	}

	const float CurrentTime = World->GetTimeSeconds();
	const float NewEndTime = CurrentTime + DurationSeconds;
	FCSKActiveStatus& Status = ActiveStatuses.FindOrAdd(Type);
	Status.EndTime = FMath::Max(Status.EndTime, NewEndTime);
	const float RemainingDuration = FMath::Max(0.0f, Status.EndTime - CurrentTime);

	FTimerDelegate ExpireDelegate;
	ExpireDelegate.BindUObject(this, &UCSKPlayerStatusComponent::HandleStatusExpired, Type);
	World->GetTimerManager().SetTimer(
		Status.ExpireTimer,
		ExpireDelegate,
		RemainingDuration,
		false);

	OnStatusChanged.Broadcast(Type, true, RemainingDuration);
	NotifyStatusApplied(Type, RemainingDuration);
	return true;
}

bool UCSKPlayerStatusComponent::HasStatus(ECSKStatusType Type) const
{
	return GetRemainingTime(Type) > 0.0f;
}

float UCSKPlayerStatusComponent::GetRemainingTime(ECSKStatusType Type) const
{
	const UWorld* World = GetWorld();
	const FCSKActiveStatus* Status = ActiveStatuses.Find(Type);
	if (!World || !Status)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, Status->EndTime - World->GetTimeSeconds());
}

bool UCSKPlayerStatusComponent::IsImmuneToKnockback() const
{
	return OwnerCSKCharacter
		&& OwnerCSKCharacter->IsLeapAttackActive();
}

bool UCSKPlayerStatusComponent::IsImmuneToHitReaction() const
{
	return OwnerCSKCharacter
		&& OwnerCSKCharacter->IsChapSalKing();
}

bool UCSKPlayerStatusComponent::IsImmuneToStatusFrom(ECSKAffinity SourceAffinity) const
{
	if (!OwnerCSKCharacter)
	{
		return true;
	}

	return SourceAffinity != ECSKAffinity::None
		&& OwnerCSKCharacter->GetAffinity() == SourceAffinity;
}

void UCSKPlayerStatusComponent::HandleStatusExpired(ECSKStatusType Type)
{
	if (GetRemainingTime(Type) > 0.0f)
	{
		return;
	}

	ActiveStatuses.Remove(Type);
	OnStatusChanged.Broadcast(Type, false, 0.0f);
}

void UCSKPlayerStatusComponent::NotifyStatusApplied(ECSKStatusType Type, float DurationSeconds)
{
	switch (Type)
	{
	case ECSKStatusType::Confusion:
		OwnerCSKCharacter->BP_OnConfused(DurationSeconds);
		break;

	case ECSKStatusType::VisionBlocked:
		OwnerCSKCharacter->BP_OnScreenObstructed(DurationSeconds);
		break;
	}
}
