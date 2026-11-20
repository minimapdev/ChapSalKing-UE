#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CSKPlayerIngredientComponent.generated.h"

class UIngredientData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCSKIngredientStateChangedSignature, UIngredientData*, Ingredient, float, GaugeRatio);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHAPSALKING_API UCSKPlayerIngredientComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCSKPlayerIngredientComponent();

	void LoadIngredient(UIngredientData* Ingredient);
	void ClearIngredient();
	bool ConsumeForShot();

	UFUNCTION(BlueprintPure, Category = "CSK|Ingredient")
	bool HasIngredient() const
	{
		return CurrentIngredient != nullptr;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Ingredient")
	UIngredientData* GetCurrentIngredient() const
	{
		return CurrentIngredient;
	}

	UFUNCTION(BlueprintPure, Category = "CSK|Ingredient")
	float GetGaugeRatio() const;

	UPROPERTY(BlueprintAssignable, Category = "CSK|Ingredient")
	FCSKIngredientStateChangedSignature OnIngredientStateChanged;

private:
	void BroadcastState();

	// 획득 사실을 전황 피드로 내보낸다. 재료를 쥐는 경로가 늘어도 알림은 여기 한 곳에서 나간다
	void BroadcastAcquiredEvent() const;

	UPROPERTY(VisibleInstanceOnly, Category = "CSK|Ingredient")
	TObjectPtr<UIngredientData> CurrentIngredient;

	UPROPERTY(VisibleInstanceOnly, Category = "CSK|Ingredient")
	float CurrentGauge = 0.0f;
};
