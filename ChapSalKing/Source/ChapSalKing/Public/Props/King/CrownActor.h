#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrownActor.generated.h"

class USkeletalMeshComponent;
class UAnimSequence;
class UCSKCarryVisualFollowComponent;
class ACSKPlayerCharacter;
class UCrownData;

UCLASS()
class CHAPSALKING_API ACrownActor : public AActor
{
	GENERATED_BODY()

public:
	ACrownActor();

	UFUNCTION(BlueprintCallable, Category = "CSK|Crown")
	void InitializeCrown(ACSKPlayerCharacter* InOwnerCharacter);

	UFUNCTION(BlueprintCallable, Category = "CSK|Crown")
	void RemoveCrown();

	UFUNCTION(BlueprintPure, Category = "CSK|Crown")
	ACSKPlayerCharacter* GetCrownOwner() const
	{
		return CrownOwnerCharacter;
	}

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Crown", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> CrownMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CSK|Crown", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCSKCarryVisualFollowComponent> CarryVisualFollowComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CSK|Crown", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCrownData> CrownData;

	UPROPERTY(VisibleInstanceOnly, Category = "CSK|Crown")
	TObjectPtr<ACSKPlayerCharacter> CrownOwnerCharacter;

	FTimerHandle DespawnAnimationTimerHandle;
	bool bIsRemoving = false;

	void InitCrownData();
	void ApplyCrownData();
	void PlayCrownAnimation(UAnimSequence* Animation, bool bLooping, float PlayRate = 1.0f);
	void PlaySpawnAnimation();
	void PlayDespawnAnimation();
	void FinishRemoveCrown();
};
