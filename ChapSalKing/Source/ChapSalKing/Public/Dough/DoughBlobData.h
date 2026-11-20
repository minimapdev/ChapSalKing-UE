#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "DoughBlobData.generated.h"

class UStaticMesh;
class UMaterialInterface;

UCLASS(BlueprintType)
class CHAPSALKING_API UDoughBlobData : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blob|Visual")
	TObjectPtr<UStaticMesh> BlobMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blob|Visual")
	TObjectPtr<UMaterialInterface> BlobMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blob|Visual")
	FName ColorParameterName = TEXT("BaseColor");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blob|Visual", meta = (ClampMin = "1.0", Units = "g"))
	float GramsPerBlobScale = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blob|Visual", meta = (ClampMin = "0.01"))
	float MinBlobScale = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blob|Visual", meta = (ClampMin = "0.01"))
	float MaxBlobScale = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blob|Pickup", meta = (ClampMin = "1.0"))
	float PickupRadius = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blob|Pickup", meta = (ClampMin = "0.0", Units = "s"))
	float PickupLockout = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blob|Launch", meta = (ClampMin = "0.0"))
	float ImpulseStrength = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blob|Launch", meta = (ClampMin = "0.0"))
	float ImpulseUp = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blob|Launch", meta = (ClampMin = "0.0"))
	float LinearDamping = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blob|Land", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GroundNormalZThreshold = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blob|Land", meta = (ClampMin = "0.1"))
	float LandSquashXY = 1.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blob|Land", meta = (ClampMin = "0.1"))
	float LandSquashZ = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blob|Land", meta = (ClampMin = "0.0", Units = "s"))
	float LandSquashDuration = 0.1f;
};
