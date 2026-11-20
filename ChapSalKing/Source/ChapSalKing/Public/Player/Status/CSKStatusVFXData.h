#pragma once

#include "CoreMinimal.h"
#include "Framework/CSKDataAsset.h"
#include "CSKStatusVFXData.generated.h"

class UNiagaraSystem;

UENUM(BlueprintType)
enum class ECSKStatusVisualType : uint8
{
	Confusion,
	Slow,
	Haste,
	DoughPickup
};

USTRUCT(BlueprintType)
struct FCSKStatusVFXEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> LoopSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> ApplySystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transform")
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transform")
	FRotator RelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transform")
	FVector Scale = FVector::OneVector;
};

UCLASS(BlueprintType)
class CHAPSALKING_API UCSKStatusVFXCatalog : public UCSKDataAsset
{
	GENERATED_BODY()

public:
	UCSKStatusVFXCatalog();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status VFX")
	TMap<ECSKStatusVisualType, FCSKStatusVFXEntry> Effects;

	const FCSKStatusVFXEntry* Find(ECSKStatusVisualType Type) const
	{
		return Effects.Find(Type);
	}

protected:
	virtual void PostLoad() override;

private:
	void EnsureDefaultEntries();
};

