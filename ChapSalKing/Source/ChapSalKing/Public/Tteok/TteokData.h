#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "Combat/CSKAffinityTypes.h"
#include "TteokData.generated.h"

class UCSKShotEffect;
class UMaterialInterface;
class UTexture2D;

UENUM(BlueprintType)
enum class ETteokType : uint8
{
	Chapssal,
	Injeolmi,
	Kkul,
	Mujigae
};

UCLASS(BlueprintType)
class CHAPSALKING_API UTteokData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tteok")
	FText DisplayName;

	// 킹 상태 HUD 표시용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tteok|UI")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, Instanced, Category = "Tteok|Landing")
	TArray<TObjectPtr<UCSKShotEffect>> LandingEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tteok|Visual")
	TObjectPtr<UMaterialInterface> BodyOverlayMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tteok")
	ECSKAffinity Affinity = ECSKAffinity::None;
};
