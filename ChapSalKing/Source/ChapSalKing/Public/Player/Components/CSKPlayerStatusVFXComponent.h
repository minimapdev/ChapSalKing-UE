#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Player/Components/CSKPlayerStatusComponent.h"
#include "Player/Status/CSKStatusVFXData.h"
#include "CSKPlayerStatusVFXComponent.generated.h"

class ACSKPlayerCharacter;
class UNiagaraComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHAPSALKING_API UCSKPlayerStatusVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCSKPlayerStatusVFXComponent();
	UFUNCTION(BlueprintCallable, Category = "CSK|VFX")
	void PlayOneShot(ECSKStatusVisualType Type);


protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleStatusChanged(ECSKStatusType Type, bool bActive, float RemainingDuration);

	UFUNCTION()
	void HandleSpeedEffectsChanged(bool bHasSlow, bool bHasHaste, float EffectiveFactor);

	void SetEffectActive(ECSKStatusVisualType Type, bool bActive);
	UNiagaraComponent* CreateLoopComponent(const FCSKStatusVFXEntry& Entry) const;
	void PlayApplyEffect(const FCSKStatusVFXEntry& Entry) const;

	UPROPERTY()
	TObjectPtr<ACSKPlayerCharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UCSKStatusVFXCatalog> Catalog;

	UPROPERTY()
	TMap<ECSKStatusVisualType, TObjectPtr<UNiagaraComponent>> ActiveComponents;
};
