#include "Combat/ShotEffects/CSKShotEffect_ScatterActors.h"

#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"

void UCSKShotEffect_ScatterActors::OnCharacterHit(const FCSKShotEffectContext& Context)
{
	Scatter(Context);
}

void UCSKShotEffect_ScatterActors::OnSurfaceImpact(const FCSKShotEffectContext& Context)
{
	Scatter(Context);
}

void UCSKShotEffect_ScatterActors::Scatter(const FCSKShotEffectContext& Context)
{
	if (!Context.World || !ActorClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Context.Attacker;
	SpawnParams.Instigator = Cast<APawn>(Context.Attacker);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector Origin = Context.HitLocation + FVector(0.0f, 0.0f, SpawnHeightOffset);

	for (int32 Index = 0; Index < Count; ++Index)
	{
		AActor* Spawned = Context.World->SpawnActor<AActor>(ActorClass, Origin, FRotator::ZeroRotator, SpawnParams);
		if (!Spawned)
		{
			continue;
		}

		UPrimitiveComponent* Body = Cast<UPrimitiveComponent>(Spawned->GetRootComponent());
		if (Body && Body->IsSimulatingPhysics())
		{
			const FVector Direction = FRotator(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f).Vector();
			Body->SetPhysicsLinearVelocity(
				Direction * FMath::FRandRange(HorizontalSpeedMin, HorizontalSpeedMax)
				+ FVector(0.0f, 0.0f, UpwardSpeed)
			);
		}
	}
}
