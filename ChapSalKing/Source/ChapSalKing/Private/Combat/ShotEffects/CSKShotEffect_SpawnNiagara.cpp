#include "Combat/ShotEffects/CSKShotEffect_SpawnNiagara.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

void UCSKShotEffect_SpawnNiagara::OnCharacterHit(const FCSKShotEffectContext& Context)
{
	Spawn(Context);
}

void UCSKShotEffect_SpawnNiagara::OnSurfaceImpact(const FCSKShotEffectContext& Context)
{
	Spawn(Context);
}

void UCSKShotEffect_SpawnNiagara::Spawn(const FCSKShotEffectContext& Context)
{
	if (!Context.World || !System)
	{
		return;
	}

	const FVector SpawnLocation =
		Context.HitLocation + FVector(0.0f, 0.0f, ZOffset);

	UNiagaraComponent* NiagaraComponent =
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			Context.World,
			System,
			SpawnLocation,
			FRotator::ZeroRotator,
			Scale,
			true,
			false
		);

	if (!NiagaraComponent)
	{
		return;
	}

	if (bUseAreaRadius && Context.AreaRadius > 0.0f)
	{
		const float ParameterValue =
			bConvertRadiusToScale
				? Context.AreaRadius / FMath::Max(BaseRadius, 1.0f)
				: Context.AreaRadius;

		NiagaraComponent->SetVariableFloat(
			RadiusParameterName,
			ParameterValue
		);

		const FVector BoundsExtent(Context.AreaRadius);
		NiagaraComponent->SetSystemFixedBounds(
			FBox(-BoundsExtent, BoundsExtent)
		);
	}

	NiagaraComponent->Activate(true);
}
