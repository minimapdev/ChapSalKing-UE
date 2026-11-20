#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CSKCameraShakeStatics.generated.h"

class UCameraShakeBase;

UCLASS()
class CHAPSALKING_API UCSKCameraShakeStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "CSK|Camera Shake",
		meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject")
	)
	static UCameraShakeBase* PlayCameraShake(
		const UObject* WorldContextObject,
		FGameplayTag ShakeTag,
		float Scale = 1.0f
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "CSK|Camera Shake",
		meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject")
	)
	static UCameraShakeBase* PlayCameraShakeAtLocation(
		const UObject* WorldContextObject,
		FGameplayTag ShakeTag,
		FVector SourceLocation,
		float Scale = 1.0f
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "CSK|Camera Shake",
		meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject")
	)
	static void StopCameraShake(
		const UObject* WorldContextObject,
		UCameraShakeBase* ShakeInstance,
		bool bImmediately = false
	);
};
