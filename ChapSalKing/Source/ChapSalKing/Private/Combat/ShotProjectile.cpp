#include "Combat/ShotProjectile.h"
#include "Combat/ShotEffects/CSKShotEffect.h"
#include "Combat/CSKCombatStatics.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

AShotProjectile::AShotProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(30.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AShotProjectile::HandleBeginOverlap);
	RootComponent = CollisionComponent;

	ProjectileVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileVFXComponent"));
	ProjectileVFXComponent->SetupAttachment(CollisionComponent);
	ProjectileVFXComponent->SetAutoActivate(false);

	MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
	MovementComponent->bRotationFollowsVelocity = true;
	MovementComponent->bShouldBounce = false;
	MovementComponent->ProjectileGravityScale = 0.0f;
	MovementComponent->OnProjectileStop.AddDynamic(this, &AShotProjectile::HandleProjectileStopped);
}

void AShotProjectile::InitShot(
	const FCSKShotParams& InParams,
	AActor* InAttacker,
	const FVector& InDirection,
	const TArray<TObjectPtr<UCSKShotEffect>>& InHitEffects,
	ECSKAffinity InSourceAffinity,
	const FCSKShotVFX& InShotVFX
)
{
	Params = InParams;
	Attacker = InAttacker;
	HitEffects = InHitEffects;
	SourceAffinity = InSourceAffinity;
	ShotVFX = InShotVFX;
	bImpactHandled = false;

	if (Attacker)
	{
		CollisionComponent->IgnoreActorWhenMoving(Attacker, true);
	}

	const FVector Direction = InDirection.GetSafeNormal();
	MovementComponent->Velocity = Direction * Params.Speed;

	ProjectileVFXComponent->SetAsset(ShotVFX.ProjectileSystem);
	if (ShotVFX.ProjectileSystem)
	{
		ProjectileVFXComponent->Activate(true);
	}

	SetLifeSpan(Params.LifeSeconds);

	BP_OnShotSpawned();
}

float AShotProjectile::GetCollisionRadius() const
{
	return CollisionComponent ? CollisionComponent->GetScaledSphereRadius() : 0.0f;
}

FCSKShotEffectContext AShotProjectile::MakeEffectContext(AActor* Victim, const FVector& HitLocation) const
{
	FCSKShotEffectContext Context;
	Context.Attacker = Attacker;
	Context.Victim = Victim;
	Context.HitLocation = HitLocation;
	Context.HitDirection = MovementComponent->Velocity.GetSafeNormal();
	Context.World = GetWorld();
	Context.SourceAffinity = SourceAffinity;
	return Context;
}

void AShotProjectile::HandleBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	// 오버랩 바인딩이 생성자에 있어 스폰 등록 시점(InitShot 전)의 오버랩이 먼저 들어온다.
	// 이때 Attacker가 아직 없어 자기제외 필터를 통과하므로 무시해야 한다.
	if (!Attacker)
	{
		return;
	}

	if (!OtherActor || OtherActor == this || OtherActor == Attacker)
	{
		return;
	}

	if (bImpactHandled)
	{
		return;
	}

	bImpactHandled = true;

	const FVector HitLocation = bFromSweep ? FVector(SweepResult.ImpactPoint) : GetActorLocation();
	const FCSKShotEffectContext EffectContext = MakeEffectContext(OtherActor, HitLocation);

	const FCSKCombatHitContext HitContext = FCSKCombatStatics::ApplyKnockbackHit(
		Attacker,
		OtherActor,
		EffectContext.HitDirection,
		Params.KnockbackStrength,
		Params.KnockbackUpwardStrength,
		Params.TargetScaleResistancePower,
		Params.DropFraction
	);

	BP_OnShotHit(HitContext);

	if (ShotVFX.HitSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			ShotVFX.HitSystem,
			HitLocation,
			EffectContext.HitDirection.Rotation());
	}

	UCSKShotEffect::DispatchCharacterHit(HitEffects, EffectContext);

	Destroy();
}

void AShotProjectile::HandleProjectileStopped(const FHitResult& ImpactResult)
{
	ProcessSurfaceImpact(ImpactResult);
}

void AShotProjectile::ProcessSurfaceImpact(const FHitResult& Hit)
{
	if (bImpactHandled)
	{
		return;
	}

	bImpactHandled = true;

	if (!Hit.bBlockingHit)
	{
		Destroy();
		return;
	}

	FCSKShotEffectContext EffectContext = MakeEffectContext(nullptr, Hit.ImpactPoint);
	if (EffectContext.HitDirection.IsNearlyZero())
	{
		EffectContext.HitDirection = (Hit.TraceEnd - Hit.TraceStart).GetSafeNormal();
		if (EffectContext.HitDirection.IsNearlyZero())
		{
			EffectContext.HitDirection = -Hit.ImpactNormal;
		}
	}

	UCSKShotEffect::DispatchSurfaceImpact(HitEffects, EffectContext);

	BP_OnSurfaceImpact(Hit.ImpactPoint, Hit.ImpactNormal);

	if (ShotVFX.HitSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			ShotVFX.HitSystem,
			Hit.ImpactPoint,
			FRotationMatrix::MakeFromZ(Hit.ImpactNormal).Rotator());
	}

	Destroy();
}
