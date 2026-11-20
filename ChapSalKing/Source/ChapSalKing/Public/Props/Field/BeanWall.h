#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Props/Field/CSKAreaRadiusReceiver.h"
#include "BeanWall.generated.h"

class UBoxComponent;
class UBeanWallData;
class UInstancedStaticMeshComponent;

// 찹쌀떡 착지 팥알 벽: 원형 세그먼트 블로킹(킹 포함 전원+탄), 닿으면 중심 방향으로 튕겨냄
UCLASS()
class CHAPSALKING_API ABeanWall : public AActor, public ICSKAreaRadiusReceiver
{
	GENERATED_BODY()

public:
	ABeanWall();

	virtual void SetAreaRadiusOverride(float Radius) override
	{
		AreaRadiusOverride = Radius;
	}

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void HandlePadOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Wall")
	void BP_OnWallSpawned(float WallRadius, float WallHeight, int32 SegmentCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Wall")
	void BP_OnWallDespawning();

private:
	void BuildWallVisuals(float WallRadius, float WallHeight, int32 CollisionSegmentCount);
	void UpdateWallVisuals(float AgeSeconds);
	void SetWallCollisionEnabled(bool bEnabled);

	UPROPERTY(EditAnywhere, Category = "Wall")
	TObjectPtr<UBeanWallData> WallData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall|Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInstancedStaticMeshComponent> WallVisuals;

	UPROPERTY(EditAnywhere, Category = "Wall|Visual")
	bool bFitVisualHeightToWallHeight = true;

	UPROPERTY(EditAnywhere, Category = "Wall|Visual")
	FVector VisualScaleMultiplier = FVector::OneVector;

	UPROPERTY(EditAnywhere, Category = "Wall|Visual", meta = (Units = "cm"))
	float VisualRadiusOffset = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Wall|Visual", meta = (Units = "cm"))
	float VisualGroundOffset = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Wall|Visual", meta = (Units = "deg"))
	float VisualYawOffset = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Wall|Visual", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "45.0", Units = "deg"))
	float VisualRandomYaw = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Wall|Visual", meta = (ClampMin = "0.1", UIMin = "0.5", UIMax = "2.0"))
	float VisualRandomScaleMin = 0.9f;

	UPROPERTY(EditAnywhere, Category = "Wall|Visual", meta = (ClampMin = "0.1", UIMin = "0.5", UIMax = "2.0"))
	float VisualRandomScaleMax = 1.1f;

	UPROPERTY(EditAnywhere, Category = "Wall|Visual", meta = (Units = "cm"))
	FVector VisualCollisionPadding = FVector(5.0f, 5.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Wall|Animation", meta = (ClampMin = "0.01", UIMin = "0.05", UIMax = "1.0", Units = "s"))
	float RiseDuration = 0.22f;

	UPROPERTY(EditAnywhere, Category = "Wall|Animation", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "0.5", Units = "s"))
	float RiseStagger = 0.06f;

	UPROPERTY(EditAnywhere, Category = "Wall|Animation", meta = (ClampMin = "0.01", UIMin = "0.05", UIMax = "1.0", Units = "s"))
	float SinkDuration = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Wall|Animation", meta = (ClampMin = "1.0", UIMin = "1.0", UIMax = "2.0"))
	float UndergroundDepthMultiplier = 1.1f;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBoxComponent>> BlockingSegments;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBoxComponent>> BouncePads;

	TMap<TWeakObjectPtr<AActor>, float> LastBounceTimes;

	TArray<FTransform> VisualTargetTransforms;
	TArray<float> VisualRiseDepths;
	TArray<float> VisualRiseDelays;

	float AreaRadiusOverride = 0.0f;
	float SpawnTimeSeconds = 0.0f;
	float EffectiveLifeSeconds = 0.0f;
	bool bWallCollisionEnabled = false;
	bool bDespawning = false;
};
