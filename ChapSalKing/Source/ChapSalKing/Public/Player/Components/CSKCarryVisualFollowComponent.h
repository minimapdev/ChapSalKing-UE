#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CSKCarryVisualFollowComponent.generated.h"

class ACSKPlayerCharacter;

USTRUCT(BlueprintType)
struct FCSKCarryVisualFollowSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Follow")
	FName FollowSocketName = TEXT("Top");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Follow")
	FVector FollowOffset = FVector(0.0f, 0.0f, 25.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Follow")
	float FollowInterpSpeed = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Follow")
	float RotationInterpSpeed = 20.0f;

	// 운반물이 운반자에서 떨어질 수 있는 최대 거리.
	// 도약·널뛰기처럼 고속 이동할 때 보간만으로는 수백 유닛 뒤처져 떨어져 보인다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Follow", meta = (ClampMin = "1.0", Units = "cm"))
	float MaxFollowLag = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wobble")
	bool bEnablePitchWobble = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wobble")
	float PitchWobbleAmount = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wobble")
	float PitchWobbleSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wobble")
	bool bEnableYawWobble = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wobble")
	float YawWobbleAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wobble")
	float YawWobbleSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wobble")
	bool bEnableRollWobble = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wobble")
	float RollWobbleAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wobble")
	float RollWobbleSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wobble")
	float MoveSpeedForMaxWobble = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wobble")
	float MoveWobbleSpeedAdd = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wobble")
	float MoveWobbleAmountAdd = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wobble")
	float MoveAlphaInterpSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	float JumpLiftOffsetZ = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	float JumpLiftUpInterpSpeed = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	float JumpLiftDownInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	float JumpLiftMinVelocityZ = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	float DashLiftOffsetZ = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	float BodySlamLiftOffsetZ = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	float EatLiftOffsetZ = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	float ActionLiftInterpSpeed = 12.0f;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHAPSALKING_API UCSKCarryVisualFollowComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCSKCarryVisualFollowComponent();

	void StartFollow(
		ACSKPlayerCharacter* InCarrierCharacter,
		USceneComponent* InVisualComponent,
		const FCSKCarryVisualFollowSettings& InSettings);

	void StopFollow();

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	TObjectPtr<ACSKPlayerCharacter> CarrierCharacter;

	UPROPERTY()
	TObjectPtr<USceneComponent> VisualComponent;

	FCSKCarryVisualFollowSettings Settings;
	FRotator VisualBaseRotation = FRotator::ZeroRotator;
	float PitchWobblePhase = 0.0f;
	float YawWobblePhase = 0.0f;
	float RollWobblePhase = 0.0f;
	float MoveAlpha = 0.0f;
	float CurrentJumpLiftZ = 0.0f;
	float CurrentActionLiftZ = 0.0f;

	void UpdateFollow(float DeltaTime);
	void UpdateVisualWobble(float DeltaTime);
	float CalculateTargetActionLiftZ(float CarrierScale) const;
};
