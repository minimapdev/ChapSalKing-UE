#pragma once

#include "CoreMinimal.h"
#include "Camera/CSKCameraShakeTypes.h"
#include "Framework/CSKDataAsset.h"
#include "CSKCameraShakeCatalog.generated.h"

UCLASS(BlueprintType)
class CHAPSALKING_API UCSKCameraShakeCatalog : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	const FCSKCameraShakeEntry* FindEntry(const FGameplayTag& ShakeTag) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shake")
	TArray<FCSKCameraShakeEntry> Entries;
};
