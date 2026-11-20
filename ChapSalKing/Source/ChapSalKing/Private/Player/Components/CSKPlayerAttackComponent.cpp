#include "Player/Components/CSKPlayerAttackComponent.h"
#include "Player/Components/CSKPlayerSoftBodyComponent.h"
#include "Player/Components/CSKChapSalKingComponent.h"
#include "Player/Components/CSKPlayerShootComponent.h"

#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerScaleComponent.h"
#include "Dough/DoughData.h"
#include "Combat/CSKCombatHitReceiverInterface.h"
#include "Combat/CSKCombatStatics.h"

#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

UCSKPlayerAttackComponent::UCSKPlayerAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCSKPlayerAttackComponent::InitializeAttack()
{
	CacheOwnerComponents();
	InitStatData();

	BodySlamOriginalRadius = AttackHitboxComponent->GetUnscaledSphereRadius();

	AttackHitboxComponent->OnComponentBeginOverlap.AddDynamic(
		this,
		&UCSKPlayerAttackComponent::HandleAttackHitboxBeginOverlap
	);
}

void UCSKPlayerAttackComponent::BindInput(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerInputComponent is not an EnhancedInputComponent."));
		return;
	}

	if (AttackAction)
	{
		EnhancedInputComponent->BindAction(
			AttackAction,
			ETriggerEvent::Started,
			this,
			&UCSKPlayerAttackComponent::Attack
		);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackAction is not assigned."));
	}
}

bool UCSKPlayerAttackComponent::CanAttack() const
{
	const UWorld* World = GetWorld();
	return World
		&& World->GetTimeSeconds() >= NextAttackReadyTime
		&& !OwnerCSKCharacter->IsInputLocked();
}

bool UCSKPlayerAttackComponent::TryAttack()
{
	if (UCSKPlayerShootComponent* Shoot = OwnerCSKCharacter->GetShootComponent())
	{
		if (Shoot->IsAiming())
		{
			return Shoot->TryShoot();
		}
	}

	if (!CanAttack())
	{
		return false;
	}

	if (OwnerCSKCharacter->IsChapSalKing())
	{
		FVector EatDirection = OwnerCSKCharacter->GetActorForwardVector();
		EatDirection.Z = 0.0f;
		EatDirection = EatDirection.GetSafeNormal();

		if (UCSKChapSalKingComponent* ChapSalKingComponent = OwnerCSKCharacter->GetChapSalKingComponent())
		{
			ChapSalKingComponent->EatAttack(EatDirection);
		}

		StartAttackCooldown(GetAttackSettings().Eat.Cooldown);
		return true;
	}

	BodySlamAttack();
	return true;
}

void UCSKPlayerAttackComponent::Attack(const FInputActionValue& Value)
{
	TryAttack();
}

void UCSKPlayerAttackComponent::BodySlamAttack()
{
	const FCSKPlayerAttackSettings& Settings = GetAttackSettings();

	const float ActiveRadius = BodySlamOriginalRadius * FMath::Max(Settings.BodySlamRadiusMultiplier, 1.0f);

	BodySlamHitActors.Reset();
	bBodySlamActive = true;
	if (UCSKPlayerSoftBodyComponent* SoftBody = OwnerCSKCharacter->GetSoftBodyComponent())
	{
		SoftBody->AddBodySlamImpulse(1.0f);
	}
	SpawnBodySlamEffect(PlayerStatData->BodySlamVFXSettings);

	AttackHitboxComponent->SetSphereRadius(ActiveRadius, true);
	AttackHitboxComponent->SetGenerateOverlapEvents(true);
	AttackHitboxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AttackHitboxComponent->UpdateOverlaps();

	ProcessCurrentBodySlamOverlaps();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			BodySlamActiveTimerHandle,
			this,
			&UCSKPlayerAttackComponent::EndBodySlam,
			FMath::Max(Settings.BodySlamActiveDuration, 0.0f),
			false
		);
	}

	StartAttackCooldown(Settings.AttackCooldown);
}

void UCSKPlayerAttackComponent::SpawnBodySlamEffect(const FCSKPlayerBodySlamVFXSettings& Settings) const
{
	if (!OwnerCSKCharacter || !Settings.System)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UNiagaraComponent* EffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		Settings.System,
		OwnerCSKCharacter->GetRootComponent(),
		NAME_None,
		OwnerCSKCharacter->GetActorLocation(),
		FRotator::ZeroRotator,
		EAttachLocation::KeepWorldPosition,
		true,
		false);
	if (!EffectComponent)
	{
		return;
	}

	const float EffectScale = Settings.ScaleMultiplier * GetCurrentAppliedScale();
	EffectComponent->SetVariableFloat(TEXT("User.Scale"), EffectScale);

	if (const UDoughData* DoughData = OwnerCSKCharacter->GetPlayerDoughData())
	{
		EffectComponent->SetVariableLinearColor(TEXT("User.InnerColor"), DoughData->BodySlamInnerColor);
		EffectComponent->SetVariableLinearColor(TEXT("User.OuterColor"), DoughData->BodySlamOuterColor);
	}

	EffectComponent->Activate(true);
}

void UCSKPlayerAttackComponent::HandleAttackHitboxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!bBodySlamActive)
	{
		return;
	}

	ProcessBodySlamHit(OtherActor, OtherComp);
}

void UCSKPlayerAttackComponent::EndBodySlam()
{
	bBodySlamActive = false;
	AttackHitboxComponent->SetGenerateOverlapEvents(false);
	AttackHitboxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackHitboxComponent->SetSphereRadius(BodySlamOriginalRadius, true);
	BodySlamHitActors.Reset();
}

void UCSKPlayerAttackComponent::ProcessCurrentBodySlamOverlaps()
{
	TArray<UPrimitiveComponent*> OverlappingComponents;
	AttackHitboxComponent->GetOverlappingComponents(OverlappingComponents);

	for (UPrimitiveComponent* OverlappingComponent : OverlappingComponents)
	{
		if (OverlappingComponent)
		{
			ProcessBodySlamHit(OverlappingComponent->GetOwner(), OverlappingComponent);
		}
	}
}

void UCSKPlayerAttackComponent::ProcessBodySlamHit(AActor* HitActor, UPrimitiveComponent* HitComponent)
{
	if (!OwnerCSKCharacter || !HitActor || HitActor == OwnerCSKCharacter)
	{
		return;
	}

	if (BodySlamHitActors.Contains(HitActor))
	{
		return;
	}

	const bool bImplementsHitReceiver = HitActor->GetClass()->ImplementsInterface(UCSKCombatHitReceiverInterface::StaticClass());

	if (!Cast<ACharacter>(HitActor) && !bImplementsHitReceiver)
	{
		return;
	}

	BodySlamHitActors.Add(HitActor);
	SpawnBodySlamHitEffect(HitComponent);

	const FCSKPlayerAttackSettings& Settings = GetAttackSettings();

	FVector KnockbackDirection = HitActor->GetActorLocation() - OwnerCSKCharacter->GetActorLocation();
	KnockbackDirection.Z = 0.0f;
	if (KnockbackDirection.IsNearlyZero())
	{
		KnockbackDirection = OwnerCSKCharacter->GetActorForwardVector();
		KnockbackDirection.Z = 0.0f;
	}
	KnockbackDirection = KnockbackDirection.GetSafeNormal();

	FCSKCombatStatics::ApplyKnockbackHit(
		OwnerCSKCharacter,
		HitActor,
		KnockbackDirection,
		GetScaledBodySlamKnockbackStrength(Settings),
		Settings.BodySlamKnockbackUpwardStrength,
		Settings.BodySlamTargetScaleResistancePower,
		Settings.BodySlamDropFraction
	);
}

void UCSKPlayerAttackComponent::SpawnBodySlamHitEffect(UPrimitiveComponent* HitComponent) const
{
	if (!OwnerCSKCharacter || !HitComponent || !PlayerStatData)
	{
		return;
	}

	UNiagaraSystem* HitSystem = PlayerStatData->BodySlamVFXSettings.HitSystem;
	UWorld* World = GetWorld();
	if (!HitSystem || !World)
	{
		return;
	}

	FVector ClosestPoint;
	if (HitComponent->GetClosestPointOnCollision(OwnerCSKCharacter->GetActorLocation(), ClosestPoint) < 0.0f)
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		HitSystem,
		ClosestPoint);
}

void UCSKPlayerAttackComponent::CacheOwnerComponents()
{
	CacheOwnerCSKCharacter();

	OwnerScaleComponent = OwnerCSKCharacter->GetScaleComponent();
	AttackHitboxComponent = OwnerCSKCharacter->GetAttackHitboxComponent();
	checkf(AttackHitboxComponent, TEXT("%s의 AttackHitboxComponent가 없습니다."), *GetNameSafe(GetOwner()));
}

const FCSKPlayerAttackSettings& UCSKPlayerAttackComponent::GetAttackSettings() const
{
	return PlayerStatData->AttackSettings;
}

float UCSKPlayerAttackComponent::GetCurrentAppliedScale() const
{
	return OwnerScaleComponent
		? OwnerScaleComponent->GetAppliedScale()
		: 1.0f;
}

float UCSKPlayerAttackComponent::CalculateScaledValue(float BaseValue, float ScalePower) const
{
	const float SafeAppliedScale = FMath::Max(GetCurrentAppliedScale(), KINDA_SMALL_NUMBER);
	return BaseValue * FMath::Pow(SafeAppliedScale, ScalePower);
}

float UCSKPlayerAttackComponent::GetScaledBodySlamKnockbackStrength(
	const FCSKPlayerAttackSettings& Settings
) const
{
	return CalculateScaledValue(
		Settings.BodySlamKnockbackStrength,
		Settings.BodySlamKnockbackScalePower
	);
}

void UCSKPlayerAttackComponent::StartAttackCooldown(float Cooldown)
{
	if (const UWorld* World = GetWorld())
	{
		NextAttackReadyTime = World->GetTimeSeconds() + FMath::Max(Cooldown, 0.0f);
	}
}
