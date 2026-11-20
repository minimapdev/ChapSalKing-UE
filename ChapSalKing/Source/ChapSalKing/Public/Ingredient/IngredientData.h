#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "Combat/CSKShotTypes.h"
#include "Combat/CSKAffinityTypes.h"
#include "IngredientData.generated.h"

class UStaticMesh;
class UTexture2D;
class UMaterialInterface;
class AShotProjectile;
class UCSKShotEffect;

UCLASS(BlueprintType)
class CHAPSALKING_API UIngredientData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredient")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredient")
	ECSKAffinity Affinity = ECSKAffinity::None;

	// HUD 탄약 표시용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredient|UI")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredient|Pickup")
	TObjectPtr<UStaticMesh> PickupMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredient|Gauge", meta = (ClampMin = "1.0"))
	float MaxGauge = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredient|Gauge", meta = (ClampMin = "0.0"))
	float GaugeCostPerShot = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredient|Shot")
	FCSKShotParams ShotParams;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredient|Shot")
	FCSKShotPattern ShotPattern;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredient|Shot")
	TSubclassOf<AShotProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredient|Visual")
	FCSKShotVFX ShotVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredient|Visual")
	TObjectPtr<UMaterialInterface> BodyOverlayMaterial;

	UPROPERTY(EditAnywhere, Instanced, Category = "Ingredient|Effects")
	TArray<TObjectPtr<UCSKShotEffect>> HitEffects;
};
