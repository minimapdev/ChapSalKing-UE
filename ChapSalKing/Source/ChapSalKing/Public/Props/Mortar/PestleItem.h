#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PestleItem.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class ACSKPlayerCharacter;
class UPestleItemData;
class UCSKCarryVisualFollowComponent;
class UNiagaraComponent;

UCLASS()
class CHAPSALKING_API APestleItem : public AActor
{
	GENERATED_BODY()

public:
	APestleItem();

	UFUNCTION(BlueprintCallable, Category = "CSK|Pestle")
	void DropFromCarrier(const FVector& PreferredDirection);

	bool IsCarried() const
	{
		return CarrierCharacter != nullptr;
	}

	bool IsCarriedBy(const ACSKPlayerCharacter* Character) const;

	ACSKPlayerCharacter* GetCarrierCharacter() const
	{
		return CarrierCharacter;
	}

	void PlantInMortar(const FTransform& PlantTransform);

	void PrepareForMortarSequence();

	bool IsPlanted() const
	{
		return bPlantedInMortar;
	}

	// 복주머니 안으로 들어간다: 절구에서 뽑고, 든 사람에게서 떼고, 숨긴다.
	// 액터는 매치 내내 하나로 유지되므로 파괴하지 않는다 — 주머니가 열릴 때 RevealAt으로 돌아온다
	void StashIntoPouch();

	// 주머니가 열렸다. 그 자리에 드러나고 곧바로 주울 수 있게 된다
	void RevealAt(const FVector& Location);

	bool IsStashed() const
	{
		return bStashed;
	}

protected:
	virtual void BeginPlay() override;

	virtual void FellOutOfWorld(const UDamageType& DmgType) override;

	// 절구에서 뽑혀 나가는 순간. 꽂혀 있던 자리(달 절구)에 소멸 연출을 남길 때 쓴다
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Pestle")
	void BP_OnPestleUnplanted(const FVector& MortarLocation);

	// 복주머니에서 나온 순간. 등장 연출은 BP가 소유한다
	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Pestle")
	void BP_OnPestleRevealed(const FVector& RevealLocation);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Pestle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PestleBody;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Pestle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PestleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Pestle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> PickupCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Pestle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCSKCarryVisualFollowComponent> CarryVisualFollowComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Pestle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraComponent> BeaconEffectComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Pestle", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPestleItemData> PestleData;

	UPROPERTY(VisibleInstanceOnly, Category = "CSK|Pestle")
	TObjectPtr<ACSKPlayerCharacter> CarrierCharacter;

	FVector ReturnLocation = FVector::ZeroVector;

	FTimerHandle PickupLockoutTimerHandle;

	void FinishPickupLockout();
	FVector GetRandomDropDirection(const FVector& PreferredDirection) const;
	void LaunchWithPhysics(const FVector& Direction, const struct FCSKPhysicsLaunchSettings& Launch);

	bool bPlantedInMortar = false;
	bool bStashed = false;

	UFUNCTION()
	void OnPickupOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	bool CanBePickedUpBy(const ACSKPlayerCharacter* Character) const;
	void InitPestleData();
	void SetPickupEnabled(bool bEnabled);
	void SetBeaconActive(bool bActive);
};
