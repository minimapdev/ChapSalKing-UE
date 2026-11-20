#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Props/Field/CSKAreaRadiusReceiver.h"
#include "HoneyZone.generated.h"

class UBoxComponent;
class UHoneyZoneData;
class UIngredientData;
class ACSKPlayerCharacter;

UCLASS()
class CHAPSALKING_API AHoneyZone : public AActor, public ICSKAreaRadiusReceiver
{
	GENERATED_BODY()

public:
	AHoneyZone();

	virtual void SetAreaRadiusOverride(float Radius) override
	{
		AreaRadiusOverride = Radius;
	}

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void HandleEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	UFUNCTION()
	void HandleOccupantIngredientChanged(UIngredientData* Ingredient, float GaugeRatio);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Zone")
	void BP_OnZoneSpawned(float ZoneRadius);

private:
	void ApplySpeedFactorTo(ACSKPlayerCharacter& Character) const;

	// 납작한 판(장판) — X/Y=반경, Z는 낮게 고정. 구가 아니라 위로 안 솟음
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> ZoneCollision;

	UPROPERTY(EditAnywhere, Category = "Zone")
	TObjectPtr<UHoneyZoneData> ZoneData;

	UPROPERTY()
	TArray<TObjectPtr<ACSKPlayerCharacter>> Occupants;

	float AreaRadiusOverride = 0.0f;
};
