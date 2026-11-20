#include "Combat/ShotEffects/CSKShotEffect_SpawnActor.h"
#include "Props/Field/CSKAreaRadiusReceiver.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"

void UCSKShotEffect_SpawnActor::OnCharacterHit(const FCSKShotEffectContext& Context)
{
	Spawn(Context);
}

void UCSKShotEffect_SpawnActor::OnSurfaceImpact(const FCSKShotEffectContext& Context)
{
	Spawn(Context);
}

void UCSKShotEffect_SpawnActor::Spawn(const FCSKShotEffectContext& Context)
{
	if (!Context.World || !ActorClass)
	{
		return;
	}

	const FTransform SpawnTransform(FRotator::ZeroRotator, Context.HitLocation);
	AActor* Spawned = Context.World->SpawnActorDeferred<AActor>(
		ActorClass,
		SpawnTransform,
		Context.Attacker,
		Cast<APawn>(Context.Attacker),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Spawned)
	{
		return;
	}

	if (Context.AreaRadius > 0.0f)
	{
		if (ICSKAreaRadiusReceiver* Receiver = Cast<ICSKAreaRadiusReceiver>(Spawned))
		{
			Receiver->SetAreaRadiusOverride(Context.AreaRadius);
		}
	}

	Spawned->FinishSpawning(SpawnTransform);
}
