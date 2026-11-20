#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "CSKCameraShakeSubsystem.generated.h"

class APlayerCameraManager;
class UCameraShakeBase;
struct FCSKCameraShakeEntry;

UCLASS()
class CHAPSALKING_API UCSKCameraShakeSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "CSK|Camera Shake")
	UCameraShakeBase* PlayShake(FGameplayTag ShakeTag, float Scale = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "CSK|Camera Shake")
	UCameraShakeBase* PlayShakeAtLocation(FGameplayTag ShakeTag, FVector SourceLocation, float Scale = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "CSK|Camera Shake")
	void StopShake(UCameraShakeBase* ShakeInstance, bool bImmediately = false);

private:
	const FCSKCameraShakeEntry* FindEntry(const FGameplayTag& ShakeTag) const;
	APlayerCameraManager* GetCameraManager() const;
	UCameraShakeBase* PlayResolvedShake(const FGameplayTag& ShakeTag, const FCSKCameraShakeEntry& Entry, float FinalScale);

	TMap<FGameplayTag, double> LastPlayTimes;
};
