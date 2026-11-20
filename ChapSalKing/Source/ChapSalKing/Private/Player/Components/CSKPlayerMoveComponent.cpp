#include "Player/Components/CSKPlayerMoveComponent.h"
#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerSoftBodyComponent.h"
#include "Player/CSKPlayerStatData.h"
#include "Player/Components/CSKPlayerScaleComponent.h"
#include "Player/Components/CSKPlayerLeapComponent.h"
#include "Player/Components/CSKPlayerStatusComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"


UCSKPlayerMoveComponent::UCSKPlayerMoveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f;
}

void UCSKPlayerMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateWalkPuff();
}

void UCSKPlayerMoveComponent::InitializeMove()
{
	CacheOwnerComponents();
	InitStatData();
	BindScaleEvents();
	SetComponentTickEnabled(PlayerStatData && PlayerStatData->WalkPuffSettings.System != nullptr);

	if (OwnerMovement)
	{
		ApplyMovementStats(GetCurrentBodyScale());
	}
}

void UCSKPlayerMoveComponent::RegisterInputMappingContext()
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	APlayerController* PlayerController = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
	if (!PlayerController)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (Subsystem && DefaultMappingContext)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, MappingPriority);
	}
}

const FCSKPlayerMovementSettings& UCSKPlayerMoveComponent::GetMovementSettings() const
{
	return PlayerStatData->MovementSettings;
}

void UCSKPlayerMoveComponent::BindInput(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent가 아닙니다."));
		return;
	}

	if (MoveAction)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &UCSKPlayerMoveComponent::Move);
	}

	if (LookAction)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &UCSKPlayerMoveComponent::Look);
	}

	if (JumpAction)
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &UCSKPlayerMoveComponent::StartJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &UCSKPlayerMoveComponent::StopJump);
	}

	if (DashAction)
	{
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &UCSKPlayerMoveComponent::Dash);
	}

}

void UCSKPlayerMoveComponent::BindScaleEvents()
{
	if (!OwnerScaleComponent)
	{
		return;
	}

	OwnerScaleComponent->OnScaleChanged.AddDynamic(
		this,
		&UCSKPlayerMoveComponent::HandleScaleChanged
	);
}

void UCSKPlayerMoveComponent::HandleScaleChanged(float BodyScale, float AppliedScale)
{
	ApplyMovementStats(BodyScale);
}

float UCSKPlayerMoveComponent::GetCurrentBodyScale() const
{
	return OwnerScaleComponent
		? OwnerScaleComponent->GetBodyScale()
		: 1.0f;
}

void UCSKPlayerMoveComponent::ApplyMovementStats(float BodyScale)
{
	if (!OwnerMovement)
	{
		return;
	}

	const FCSKPlayerMovementSettings& M = PlayerStatData->MovementSettings;
	const float DefaultBodyScale = PlayerStatData->ScaleSettings.DefaultBodyScale;

	// 크기 반비례: 작을수록 빠름.
	const float SafeScale = FMath::Max(BodyScale, KINDA_SMALL_NUMBER);
	const float ScaleSpeedMul = FMath::Clamp(
		FMath::Pow(DefaultBodyScale / SafeScale, M.MoveSpeedScalePower),
		M.MinMoveSpeedMultiplier,
		M.MaxMoveSpeedMultiplier);

	float SpeedFactorProduct = 1.0f;
	for (const TPair<FName, float>& Factor : SpeedFactors)
	{
		SpeedFactorProduct *= Factor.Value;
	}

	// 멀티 seam: MaxWalkSpeed 변경은 서버 권위 + owning client 동기화 필요 지점 (CMC 속도 불일치 = 러버밴딩)
	OwnerMovement->MaxWalkSpeed = M.WalkSpeed * ScaleSpeedMul * SpeedFactorProduct;
}

namespace
{
	const FName EatBoostFactorKey(TEXT("EatBoost"));
}

void UCSKPlayerMoveComponent::ApplySpeedBoost(float Multiplier, float DurationSeconds)
{
	UWorld* World = GetWorld();
	if (!World || Multiplier <= 1.0f || DurationSeconds <= 0.0f)
	{
		return;
	}

	SetSpeedFactor(EatBoostFactorKey, Multiplier);

	World->GetTimerManager().SetTimer(
		SpeedBoostTimerHandle,
		this,
		&UCSKPlayerMoveComponent::EndSpeedBoost,
		DurationSeconds,
		false
	);
}

void UCSKPlayerMoveComponent::EndSpeedBoost()
{
	ClearSpeedFactor(EatBoostFactorKey);
}

void UCSKPlayerMoveComponent::SetSpeedFactor(FName SourceKey, float Factor)
{
	SpeedFactors.Add(SourceKey, Factor);
	ApplyMovementStats(GetCurrentBodyScale());
	BroadcastSpeedEffects();
}

void UCSKPlayerMoveComponent::ClearSpeedFactor(FName SourceKey)
{
	if (SpeedFactors.Remove(SourceKey) == 0)
	{
		return;
	}
	ApplyMovementStats(GetCurrentBodyScale());
	BroadcastSpeedEffects();
}

void UCSKPlayerMoveComponent::GetSpeedEffectState(
	bool& bOutHasSlow,
	bool& bOutHasHaste,
	float& OutEffectiveFactor) const
{
	bOutHasSlow = false;
	bOutHasHaste = false;
	OutEffectiveFactor = 1.0f;

	for (const TPair<FName, float>& Pair : SpeedFactors)
	{
		const float Factor = Pair.Value;
		OutEffectiveFactor *= Factor;
		bOutHasSlow |= Factor < 1.0f - KINDA_SMALL_NUMBER;
		bOutHasHaste |= Factor > 1.0f + KINDA_SMALL_NUMBER;
	}
}

void UCSKPlayerMoveComponent::BroadcastSpeedEffects()
{
	bool bHasSlow = false;
	bool bHasHaste = false;
	float EffectiveFactor = 1.0f;
	GetSpeedEffectState(bHasSlow, bHasHaste, EffectiveFactor);
	OnSpeedEffectsChanged.Broadcast(bHasSlow, bHasHaste, EffectiveFactor);
}

void UCSKPlayerMoveComponent::Move(const FInputActionValue& Value)
{
	if (!OwnerCSKCharacter->GetController() || IsInputLocked())
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();
	if (OwnerCSKCharacter->GetStatusComponent()->HasStatus(ECSKStatusType::Confusion))
	{
		MovementVector *= -1.0f;
	}

	const FRotator ControlRotation = OwnerCSKCharacter->GetController()->GetControlRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	OwnerCSKCharacter->AddMovementInput(ForwardDirection, MovementVector.Y);
	OwnerCSKCharacter->AddMovementInput(RightDirection, MovementVector.X);
}

void UCSKPlayerMoveComponent::Look(const FInputActionValue& Value)
{
	if (!OwnerCSKCharacter || OwnerCSKCharacter->IsBeingEaten())
	{
		return;
	}

	if (UCSKPlayerLeapComponent* Leap = OwnerCSKCharacter->GetLeapComponent())
	{
		if (Leap->IsAiming())
		{
			return;
		}
	}

	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	OwnerCSKCharacter->AddControllerYawInput(LookAxisVector.X);
	OwnerCSKCharacter->AddControllerPitchInput(LookAxisVector.Y);
}

bool UCSKPlayerMoveComponent::TryJump()
{
	if (!OwnerCSKCharacter->CanJump() || IsInputLocked())
	{
		return false;
	}

	OwnerCSKCharacter->Jump();
	bPendingJumpLandingVFX = true;

	if (PlayerStatData)
	{
		PlayGroundVFX(PlayerStatData->JumpStartVFXSettings);
	}

	return true;
}

void UCSKPlayerMoveComponent::HandleLanded(const FHitResult& Hit)
{
	if (!bPendingJumpLandingVFX)
	{
		return;
	}

	bPendingJumpLandingVFX = false;
	if (PlayerStatData)
	{
		PlayGroundVFX(PlayerStatData->JumpLandingVFXSettings, &Hit);
	}
}

void UCSKPlayerMoveComponent::StartJump(const FInputActionValue& Value)
{
	if (IsInputLocked())
	{
		return;
	}

	if (!OwnerCSKCharacter->CanJump())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || World->GetTimerManager().IsTimerActive(JumpAnticipationTimerHandle))
	{
		return;
	}

	float JumpAnticipationTime = 0.0f;

	if (UCSKPlayerSoftBodyComponent* SoftBody = OwnerCSKCharacter->GetSoftBodyComponent())
	{
		SoftBody->NotifyJumpRequested();
		JumpAnticipationTime = SoftBody->GetJumpAnticipationTime();
	}

	if (JumpAnticipationTime <= KINDA_SMALL_NUMBER)
	{
		ExecuteJump();
		return;
	}

	World->GetTimerManager().SetTimer(
		JumpAnticipationTimerHandle,
		this,
		&UCSKPlayerMoveComponent::ExecuteJump,
		JumpAnticipationTime,
		false
	);
}

void UCSKPlayerMoveComponent::StopJump(const FInputActionValue& Value)
{
	if (!IsInputLocked())
	{
		OwnerCSKCharacter->StopJumping();
	}
}

bool UCSKPlayerMoveComponent::CanDash() const
{
	const UWorld* World = GetWorld();
	if (!World || World->GetTimeSeconds() < NextDashReadyTime || IsInputLocked())
	{
		return false;
	}
	const UCharacterMovementComponent* Movement = OwnerCSKCharacter->GetCharacterMovement();
	const FCSKPlayerMovementSettings& Settings = GetMovementSettings();
	if (!Movement || (!Settings.bCanDashInAir && Movement->IsFalling()))
	{
		return false;
	}
	return true;
}

bool UCSKPlayerMoveComponent::TryDash(const FVector& WorldDirection)
{
	if (!CanDash())
	{
		return false;
	}

	FVector DashDirection = WorldDirection;
	DashDirection.Z = 0.0f;
	DashDirection = DashDirection.GetSafeNormal();
	if (DashDirection.IsNearlyZero())
	{
		return false;
	}

	const FCSKPlayerMovementSettings& Settings = GetMovementSettings();

	OwnerCSKCharacter->LaunchCharacter(DashDirection * Settings.DashStrength, true, false);
	PlayDashVFX(DashDirection);

	if (const UWorld* World = GetWorld())
	{
		NextDashReadyTime = World->GetTimeSeconds() + Settings.DashCooldown;
	}

	if (UCSKPlayerSoftBodyComponent* SoftBody = OwnerCSKCharacter->GetSoftBodyComponent())
	{
		SoftBody->AddDashImpulse(Settings.DashSoftBodyStrength);
	}
	return true;
}

void UCSKPlayerMoveComponent::PlayDashVFX(const FVector& DashDirection)
{
	const FCSKPlayerDashVFXSettings& VFX = PlayerStatData->DashVFXSettings;
	if (!VFX.System)
	{
		return;
	}

	const float CharacterScale =
		FMath::Max(OwnerCSKCharacter->GetActorScale3D().GetAbsMax(), 0.01f);
	const FVector SpawnLocation =
		OwnerCSKCharacter->GetActorLocation()
		- DashDirection * (VFX.RearOffset * CharacterScale)
		+ FVector::UpVector * (VFX.ZOffset * CharacterScale);
	const FRotator SpawnRotation =
		(DashDirection.Rotation() + VFX.RotationOffset).GetNormalized();

	UNiagaraComponent* NiagaraComponent =
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			VFX.System,
			OwnerCSKCharacter->GetRootComponent(),
			NAME_None,
			SpawnLocation,
			SpawnRotation,
			EAttachLocation::KeepWorldPosition,
			true,
			false);
	if (!NiagaraComponent)
	{
		return;
	}

	NiagaraComponent->SetVariableFloat(
		FName(TEXT("User.Scale")),
		CharacterScale * VFX.ScaleMultiplier);
	NiagaraComponent->Activate(true);
}

void UCSKPlayerMoveComponent::Dash(const FInputActionValue& Value)
{
	TryDash(GetDashDirection(OwnerCSKCharacter));
}

void UCSKPlayerMoveComponent::ExecuteJump()
{
	TryJump();
}

bool UCSKPlayerMoveComponent::IsInputLocked() const
{
	return OwnerCSKCharacter->IsInputLocked();
}

void UCSKPlayerMoveComponent::UpdateWalkPuff()
{
	if (!PlayerStatData || !OwnerCSKCharacter || !OwnerMovement)
	{
		return;
	}

	const FCSKPlayerWalkPuffSettings& Puff = PlayerStatData->WalkPuffSettings;
	if (!Puff.System)
	{
		return;
	}

	const FVector CurrentLocation = OwnerCSKCharacter->GetActorLocation();
	const float Speed2D = OwnerMovement->Velocity.Size2D();
	if (!OwnerMovement->IsMovingOnGround() || Speed2D < Puff.MinSpeed)
	{
		LastWalkPuffLocation = CurrentLocation;
		bHasWalkPuffLocation = true;
		return;
	}

	if (!bHasWalkPuffLocation)
	{
		LastWalkPuffLocation = CurrentLocation;
		bHasWalkPuffLocation = true;
		return;
	}

	if (FVector::Dist2D(CurrentLocation, LastWalkPuffLocation) < Puff.SpawnDistance)
	{
		return;
	}

	const FVector MoveDirection = OwnerMovement->Velocity.GetSafeNormal2D();
	const UCapsuleComponent* Capsule = OwnerCSKCharacter->GetCapsuleComponent();
	const float RearOffset = Capsule
		? Capsule->GetScaledCapsuleRadius() * Puff.SpawnRearOffsetRadiusMultiplier
		: 0.0f;
	const FVector SpawnTraceOrigin = CurrentLocation - MoveDirection * RearOffset;

	FHitResult GroundHit;
	if (!FindWalkPuffGround(SpawnTraceOrigin, GroundHit))
	{
		LastWalkPuffLocation = CurrentLocation;
		return;
	}

	const float CharacterScale = FMath::Max(OwnerCSKCharacter->GetActorScale3D().GetAbsMax(), 0.01f);
	const FRotator GroundRotation = FRotationMatrix::MakeFromZ(GroundHit.ImpactNormal).Rotator();
	UNiagaraComponent* PuffComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		this,
		Puff.System,
		GroundHit.ImpactPoint + GroundHit.ImpactNormal * 2.0f,
		GroundRotation,
		FVector::OneVector);
	if (PuffComponent)
	{
		PuffComponent->SetVariableFloat(TEXT("User.PuffScale"), CharacterScale * Puff.ScaleMultiplier);
	}

	LastWalkPuffLocation = CurrentLocation;
}

bool UCSKPlayerMoveComponent::FindWalkPuffGround(const FVector& TraceOrigin, FHitResult& OutHit) const
{
	const UCapsuleComponent* Capsule = OwnerCSKCharacter ? OwnerCSKCharacter->GetCapsuleComponent() : nullptr;
	UWorld* World = GetWorld();
	if (!Capsule || !World)
	{
		return false;
	}

	const FVector Start = TraceOrigin;
	const FVector End = Start - FVector::UpVector * (Capsule->GetScaledCapsuleHalfHeight() + 30.0f);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WalkPuffGround), false, OwnerCSKCharacter);
	return World->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);
}

void UCSKPlayerMoveComponent::BeginLeapMovement()
{
	bPendingJumpLandingVFX = false;

	if (OwnerMovement)
	{
		OwnerMovement->StopMovementImmediately();
		OwnerMovement->SetMovementMode(MOVE_None);
	}
}

void UCSKPlayerMoveComponent::PlayGroundVFX(
	const FCSKPlayerGroundVFXSettings& Settings,
	const FHitResult* SurfaceHit) const
{
	if (!Settings.System || !OwnerCSKCharacter)
	{
		return;
	}

	FVector SurfaceLocation = OwnerCSKCharacter->GetActorLocation();
	FVector SurfaceNormal = FVector::UpVector;
	bool bHasSurface = false;

	if (SurfaceHit && SurfaceHit->bBlockingHit)
	{
		SurfaceLocation = SurfaceHit->ImpactPoint;
		SurfaceNormal = SurfaceHit->ImpactNormal.GetSafeNormal();
		bHasSurface = !SurfaceNormal.IsNearlyZero();
	}
	else if (OwnerMovement && OwnerMovement->CurrentFloor.IsWalkableFloor())
	{
		const FHitResult& FloorHit = OwnerMovement->CurrentFloor.HitResult;
		SurfaceLocation = FloorHit.ImpactPoint;
		SurfaceNormal = FloorHit.ImpactNormal.GetSafeNormal();
		bHasSurface = !SurfaceNormal.IsNearlyZero();
	}

	if (!bHasSurface)
	{
		const UCapsuleComponent* Capsule = OwnerCSKCharacter->GetCapsuleComponent();
		const float HalfHeight = Capsule ? Capsule->GetScaledCapsuleHalfHeight() : 0.0f;
		SurfaceLocation -= FVector::UpVector * HalfHeight;
		SurfaceNormal = FVector::UpVector;
	}

	const FRotator SurfaceRotation = FRotationMatrix::MakeFromZ(SurfaceNormal).Rotator();
	const FRotator SpawnRotation = SurfaceRotation + Settings.RotationOffset;
	const float CharacterScale =
		FMath::Max(OwnerCSKCharacter->GetActorScale3D().GetAbsMax(), 0.01f);

	const float ScaleInfluence = FMath::Clamp(Settings.CharacterScaleInfluence, 0.0f, 1.0f);
	const float AdjustedCharacterScale = FMath::Lerp(1.0f, CharacterScale, ScaleInfluence);
	const float EffectScale = FMath::Min(
		AdjustedCharacterScale * Settings.ScaleMultiplier,
		FMath::Max(Settings.MaxEffectScale, 0.1f));
	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		this,
		Settings.System,
		SurfaceLocation + SurfaceNormal * Settings.SurfaceOffset,
		SpawnRotation,
		FVector::OneVector,
		true,
		false);
	if (!NiagaraComponent)
	{
		return;
	}

	NiagaraComponent->SetVariableFloat(TEXT("User.Scale"), EffectScale);
	NiagaraComponent->Activate(true);
}

void UCSKPlayerMoveComponent::EndLeapMovement()
{
	if (OwnerMovement)
	{
		OwnerMovement->SetMovementMode(MOVE_Falling);
	}
}

FVector UCSKPlayerMoveComponent::GetDashDirection(const ACharacter* Character) const
{
	if (!Character)
	{
		return FVector::ZeroVector;
	}

	FVector DashDirection = Character->GetActorForwardVector();
	DashDirection.Z = 0.0f;

	return DashDirection.GetSafeNormal();
}

void UCSKPlayerMoveComponent::CacheOwnerComponents()
{
	CacheOwnerCSKCharacter();

	OwnerMovement = OwnerCSKCharacter->GetCharacterMovement();
	OwnerScaleComponent = OwnerCSKCharacter->GetScaleComponent();
}
