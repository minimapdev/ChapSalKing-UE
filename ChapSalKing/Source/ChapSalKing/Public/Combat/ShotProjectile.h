#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Combat/CSKShotTypes.h"
#include "Combat/CSKCombatTypes.h"
#include "Combat/CSKAffinityTypes.h"
#include "ShotProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
class UCSKShotEffect;
struct FCSKShotEffectContext;

UCLASS()
class CHAPSALKING_API AShotProjectile : public AActor
{
	GENERATED_BODY()

public:
	AShotProjectile();

	void InitShot(
		const FCSKShotParams& InParams,
		AActor* InAttacker,
		const FVector& InDirection,
		const TArray<TObjectPtr<UCSKShotEffect>>& InHitEffects,
		ECSKAffinity InSourceAffinity,
		const FCSKShotVFX& InShotVFX
	);

	float GetCollisionRadius() const;

protected:
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
	void HandleProjectileStopped(const FHitResult& ImpactResult);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Shot")
	void BP_OnShotSpawned();

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Shot")
	void BP_OnShotHit(const FCSKCombatHitContext& HitContext);

	UFUNCTION(BlueprintImplementableEvent, Category = "CSK|Shot")
	void BP_OnSurfaceImpact(const FVector& ImpactPoint, const FVector& ImpactNormal);

private:
	FCSKShotEffectContext MakeEffectContext(AActor* Victim, const FVector& HitLocation) const;
	void ProcessSurfaceImpact(const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProjectileMovementComponent> MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shot|VFX", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraComponent> ProjectileVFXComponent;

	FCSKShotParams Params;

	UPROPERTY()
	FCSKShotVFX ShotVFX;

	UPROPERTY()
	TObjectPtr<AActor> Attacker;

	UPROPERTY()
	TArray<TObjectPtr<UCSKShotEffect>> HitEffects;

	ECSKAffinity SourceAffinity = ECSKAffinity::None;
	bool bImpactHandled = false;
};
