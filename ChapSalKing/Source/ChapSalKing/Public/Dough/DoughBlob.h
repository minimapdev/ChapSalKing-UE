#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Dough/DoughTypes.h"
#include "DoughBlob.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UDoughBlobData;
class ACSKPlayerCharacter;

UCLASS()
class CHAPSALKING_API ADoughBlob : public AActor
{
	GENERATED_BODY()

public:
	ADoughBlob();

	static ADoughBlob* SpawnBlob(
		const UObject* WorldContext,
		const FVector& Location,
		EDoughCharacterType DoughType,
		float Grams,
		const FVector& LaunchDirection);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blob", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> BlobMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blob", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> PickupCollision;

	UPROPERTY()
	TObjectPtr<UDoughBlobData> BlobData;

	UPROPERTY(VisibleInstanceOnly, Category = "Blob")
	EDoughCharacterType DoughType = EDoughCharacterType::White;

	UPROPERTY(VisibleInstanceOnly, Category = "Blob")
	float Grams = 0.0f;

	bool bLanded = false;

	FVector RoundScale = FVector::OneVector;
	FVector SquashTargetScale = FVector::OneVector;
	float SquashElapsed = 0.0f;
	bool bSquashing = false;

	float LandBottomZ = 0.0f;
	float LandPivotOffset = 0.0f;

	FTimerHandle PickupLockoutTimerHandle;

	void ApplyVisual();
	void Launch(const FVector& LaunchDirection);
	void SetPickupEnabled(bool bEnabled);

	UFUNCTION()
	void FinishPickupLockout();

	UFUNCTION()
	void HandleMeshHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	UFUNCTION()
	void OnPickupOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};
