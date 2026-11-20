#pragma once

#include "CoreMinimal.h"
#include "Player/CSKPlayerComponentBase.h"
#include "Combat/CSKAffinityTypes.h"
#include "TimerManager.h"
#include "CSKPlayerStatusComponent.generated.h"

UENUM(BlueprintType)
enum class ECSKStatusType : uint8
{
	Confusion,
	VisionBlocked
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FCSKStatusChangedSignature,
	ECSKStatusType, Type,
	bool, bActive,
	float, RemainingDuration);

USTRUCT()
struct FCSKActiveStatus
{
	GENERATED_BODY()

	float EndTime = 0.0f;
	FTimerHandle ExpireTimer;
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHAPSALKING_API UCSKPlayerStatusComponent : public UCSKPlayerComponentBase
{
	GENERATED_BODY()

public:
	UCSKPlayerStatusComponent();

	void InitializeStatus();

	UFUNCTION(BlueprintCallable, Category = "CSK|Status")
	bool TryApplyStatus(ECSKStatusType Type, float DurationSeconds, ECSKAffinity SourceAffinity = ECSKAffinity::None);

	UFUNCTION(BlueprintPure, Category = "CSK|Status")
	bool HasStatus(ECSKStatusType Type) const;

	UFUNCTION(BlueprintPure, Category = "CSK|Status")
	float GetRemainingTime(ECSKStatusType Type) const;

	bool IsImmuneToKnockback() const;

	bool IsImmuneToHitReaction() const;

	bool IsImmuneToStatusFrom(ECSKAffinity SourceAffinity) const;

	UPROPERTY(BlueprintAssignable, Category = "CSK|Status")
	FCSKStatusChangedSignature OnStatusChanged;

private:
	void NotifyStatusApplied(ECSKStatusType Type, float DurationSeconds);

	void HandleStatusExpired(ECSKStatusType Type);
	TMap<ECSKStatusType, FCSKActiveStatus> ActiveStatuses;
};
