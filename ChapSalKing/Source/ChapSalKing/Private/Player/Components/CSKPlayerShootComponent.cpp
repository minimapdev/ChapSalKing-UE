#include "Player/Components/CSKPlayerShootComponent.h"

#include "Camera/CSKCameraShakeStatics.h"
#include "Camera/CSKCameraShakeTags.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerIngredientComponent.h"
#include "Player/Components/CSKPlayerLeapComponent.h"
#include "Player/Components/CSKPlayerSoftBodyComponent.h"
#include "Ingredient/IngredientData.h"
#include "Combat/ShotProjectile.h"

#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/SceneComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Engine/World.h"
#include "NiagaraFunctionLibrary.h"

UCSKPlayerShootComponent::UCSKPlayerShootComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCSKPlayerShootComponent::InitializeShoot()
{
	CacheOwnerComponents();
	InitStatData();

	if (USpringArmComponent* Boom = OwnerCSKCharacter->GetCameraBoom())
	{
		Boom->AddTickPrerequisiteComponent(this);
	}
}

void UCSKPlayerShootComponent::CacheOwnerComponents()
{
	CacheOwnerCSKCharacter();

	OwnerIngredientComponent = OwnerCSKCharacter->GetIngredientComponent();
}

void UCSKPlayerShootComponent::BindInput(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}

	if (AimAction)
	{
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &UCSKPlayerShootComponent::StartAiming);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &UCSKPlayerShootComponent::StopAiming);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AimAction is not assigned."));
	}
}

void UCSKPlayerShootComponent::StartAiming(const FInputActionValue& Value)
{
	if (OwnerCSKCharacter->IsChapSalKing())
	{
		if (UCSKPlayerLeapComponent* Leap = OwnerCSKCharacter->GetLeapComponent())
		{
			Leap->BeginAiming();
		}
		return;
	}

	if (!OwnerIngredientComponent->HasIngredient())
	{
		return;
	}

	bIsAiming = true;
	SetComponentTickEnabled(true);

	OwnerCSKCharacter->BP_OnAimChanged(true);
	BroadcastShootState();
}

void UCSKPlayerShootComponent::StopAiming(const FInputActionValue& Value)
{
	if (OwnerCSKCharacter->IsChapSalKing())
	{
		if (UCSKPlayerLeapComponent* Leap = OwnerCSKCharacter->GetLeapComponent())
		{
			Leap->ConfirmLeap();
		}
		return;
	}

	CancelAiming();
}

void UCSKPlayerShootComponent::CancelAiming()
{
	if (!bIsAiming)
	{
		return;
	}

	bIsAiming = false;

	OwnerCSKCharacter->BP_OnAimChanged(false);
	BroadcastShootState();
}

bool UCSKPlayerShootComponent::IsShootReady() const
{
	const UWorld* World = GetWorld();
	return World && World->GetTimeSeconds() >= NextShootReadyTime;
}

float UCSKPlayerShootComponent::GetCooldownRatio() const
{
	const UWorld* World = GetWorld();
	if (!World || LastCooldownDuration <= 0.0f)
	{
		return 0.0f;
	}

	const float Remaining = NextShootReadyTime - World->GetTimeSeconds();
	return FMath::Clamp(Remaining / LastCooldownDuration, 0.0f, 1.0f);
}

void UCSKPlayerShootComponent::BroadcastShootState()
{
	OnShootStateChanged.Broadcast(bIsAiming, GetCooldownRatio());
}

bool UCSKPlayerShootComponent::TryShoot()
{
	if (!bIsAiming || !IsShootReady() || OwnerCSKCharacter->IsInputLocked())
	{
		return false;
	}

	ShootShot();
	return true;
}

void UCSKPlayerShootComponent::ShootShot()
{
	USceneComponent* AimMuzzle = OwnerCSKCharacter->GetShootMuzzlePoint();
	if (!AimMuzzle)
	{
		return;
	}

	const FVector MuzzleLocation = AimMuzzle->GetComponentLocation();
	const FVector CameraAimPoint = GetCameraAimPoint();

	const UIngredientData* Ingredient = OwnerIngredientComponent ? OwnerIngredientComponent->GetCurrentIngredient() : nullptr;
	const AShotProjectile* ProjectileCDO = Ingredient && Ingredient->ProjectileClass
		? Ingredient->ProjectileClass->GetDefaultObject<AShotProjectile>()
		: nullptr;
	const float ProjectileRadius = ProjectileCDO ? ProjectileCDO->GetCollisionRadius() : 0.0f;

	FireShot(ResolveMuzzleAimPoint(MuzzleLocation, CameraAimPoint, ProjectileRadius));
}

bool UCSKPlayerShootComponent::TryShootAt(const FVector& TargetPoint)
{
	if (OwnerCSKCharacter->IsChapSalKing() || !IsShootReady() || OwnerCSKCharacter->IsInputLocked())
	{
		return false;
	}

	return FireShot(TargetPoint);
}

bool UCSKPlayerShootComponent::FireShot(const FVector& AimPoint)
{
	UIngredientData* Ingredient = OwnerIngredientComponent->GetCurrentIngredient();
	if (!Ingredient || !Ingredient->ProjectileClass)
	{
		return false;
	}

	USceneComponent* Muzzle = OwnerCSKCharacter->GetShootMuzzlePoint();
	UWorld* World = GetWorld();
	if (!Muzzle || !World)
	{
		return false;
	}

	const FVector SpawnLocation = Muzzle->GetComponentLocation();
	const FVector AimDirection = (AimPoint - SpawnLocation).GetSafeNormal();

	if (Ingredient->ShotVFX.MuzzleSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			Ingredient->ShotVFX.MuzzleSystem,
			SpawnLocation,
			AimDirection.Rotation());
	}

	FVector FaceDirection = AimDirection;
	FaceDirection.Z = 0.0f;
	if (!FaceDirection.IsNearlyZero())
	{
		OwnerCSKCharacter->SetActorRotation(FRotator(0.0f, FaceDirection.Rotation().Yaw, 0.0f));
	}

	bool bSpawnedAnyProjectile = false;
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerCSKCharacter;
		SpawnParams.Instigator = OwnerCSKCharacter;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		const int32 PelletCount = FMath::Max(Ingredient->ShotPattern.PelletCount, 1);
		const float SpreadHalfAngleDeg = Ingredient->ShotPattern.SpreadHalfAngleDeg;

		for (int32 PelletIndex = 0; PelletIndex < PelletCount; ++PelletIndex)
		{
			FVector ShotDirection = AimDirection;
			if (PelletIndex > 0 && SpreadHalfAngleDeg > 0.0f)
			{
				const int32 MaxRank = PelletCount / 2;
				const int32 Rank = (PelletIndex + 1) / 2;
				const float Side = (PelletIndex % 2 == 1) ? 1.0f : -1.0f;
				const float AngleDeg = Side * SpreadHalfAngleDeg * Rank / MaxRank;
				ShotDirection = AimDirection.RotateAngleAxis(AngleDeg, FVector::UpVector);
			}

			AShotProjectile* Projectile = World->SpawnActor<AShotProjectile>(
				Ingredient->ProjectileClass,
				SpawnLocation,
				ShotDirection.Rotation(),
				SpawnParams
			);

			if (Projectile)
			{
				bSpawnedAnyProjectile = true;
				Projectile->InitShot(
					Ingredient->ShotParams,
					OwnerCSKCharacter,
					ShotDirection,
					Ingredient->HitEffects,
					Ingredient->Affinity,
					Ingredient->ShotVFX);
			}
		}
	}

	if (bSpawnedAnyProjectile
		&& OwnerCSKCharacter->IsPlayerControlled()
		&& OwnerCSKCharacter->IsLocallyControlled())
	{
		UCSKCameraShakeStatics::PlayCameraShake(
			this,
			CSKCameraShakeTags::Weapon_IngredientShot);
	}

	if (UCSKPlayerSoftBodyComponent* SoftBody = OwnerCSKCharacter->GetSoftBodyComponent())
	{
		SoftBody->AddBodySlamImpulse(1.0f);
	}

	LastCooldownDuration = FMath::Max(Ingredient->ShotParams.FireCooldown, 0.0f);
	NextShootReadyTime = World->GetTimeSeconds() + LastCooldownDuration;
	BroadcastShootState();

	if (!OwnerIngredientComponent->ConsumeForShot())
	{
		CancelAiming();
	}

	return true;
}

FVector UCSKPlayerShootComponent::GetCameraAimPoint() const
{
	const float TraceDistance = PlayerStatData->AimSettings.AimTraceDistance;

	UWorld* World = GetWorld();
	APlayerController* PlayerController = Cast<APlayerController>(OwnerCSKCharacter->GetController());

	FVector ViewLocation;
	FRotator ViewRotation;
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * TraceDistance;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCSKCharacter);

	FCollisionObjectQueryParams AimObjectTypes;
	AimObjectTypes.AddObjectTypesToQuery(ECC_WorldStatic);
	AimObjectTypes.AddObjectTypesToQuery(ECC_Pawn);

	if (World->LineTraceSingleByObjectType(Hit, ViewLocation, TraceEnd, AimObjectTypes, Params))
	{
		return Hit.ImpactPoint;
	}

	return TraceEnd;
}

FVector UCSKPlayerShootComponent::ResolveMuzzleAimPoint(
	const FVector& MuzzleLocation,
	const FVector& CameraAimPoint,
	float ProjectileRadius) const
{
	UWorld* World = GetWorld();
	if (!World || ProjectileRadius <= 0.0f)
	{
		return CameraAimPoint;
	}

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCSKCharacter);

	FCollisionObjectQueryParams AimObjectTypes;
	AimObjectTypes.AddObjectTypesToQuery(ECC_WorldStatic);
	AimObjectTypes.AddObjectTypesToQuery(ECC_Pawn);

	const FCollisionShape ProjectileShape = FCollisionShape::MakeSphere(ProjectileRadius);
	if (World->SweepSingleByObjectType(
		Hit,
		MuzzleLocation,
		CameraAimPoint,
		FQuat::Identity,
		AimObjectTypes,
		ProjectileShape,
		Params))
	{
		return Hit.Location;
	}

	return CameraAimPoint;
}

void UCSKPlayerShootComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	USpringArmComponent* Boom = OwnerCSKCharacter->GetCameraBoom();
	if (!Boom)
	{
		SetComponentTickEnabled(false);
		return;
	}

	const FCSKPlayerAimSettings& Aim = PlayerStatData->AimSettings;

	const float NormalArmLength = OwnerCSKCharacter->GetScaleDesiredCameraArmLength();
	const float TargetArm = bIsAiming ? NormalArmLength * Aim.AimZoomFactor : NormalArmLength;

	Boom->TargetArmLength = FMath::FInterpTo(Boom->TargetArmLength, TargetArm, DeltaTime, Aim.TransitionSpeed);

	if (!bIsAiming && FMath::IsNearlyEqual(Boom->TargetArmLength, NormalArmLength, Aim.SettleTolerance))
	{
		Boom->TargetArmLength = NormalArmLength;
		SetComponentTickEnabled(false);
	}
}
