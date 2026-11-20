#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CSKCameraShakeTypes.generated.h"

class UCameraShakeBase;

USTRUCT(BlueprintType)
struct FCSKCameraShakeEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shake")
	FGameplayTag ShakeTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> ShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shake", meta = (ClampMin = "0.0"))
	float BaseScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shake", meta = (ClampMin = "0.0", Units = "s"))
	float RetriggerInterval = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shake|Spatial", meta = (ClampMin = "0.0", Units = "cm"))
	float InnerRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shake|Spatial", meta = (ClampMin = "0.0", Units = "cm"))
	float OuterRadius = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shake|Spatial", meta = (ClampMin = "0.01"))
	float FalloffExponent = 1.0f;
};

USTRUCT(BlueprintType)
struct FCSKCameraShakeReference
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shake", meta = (Categories = "CameraShake"))
	FGameplayTag ShakeTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shake", meta = (ClampMin = "0.0"))
	float Scale = 1.0f;
};
