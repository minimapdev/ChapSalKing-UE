#include "Player/CSKPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"

#include "Framework/CSKGameInstance.h"
#include "Framework/CSKProjectSettingsData.h"
#include "Framework/CSKGameState.h"
#include "Engine/World.h"
#include "Player/CSKPlayerStatData.h"
#include "Dough/DoughDataRegistry.h"
#include "Dough/DoughData.h"
#include "Dough/DoughBlob.h"
#include "Props/Mortar/PestleItem.h"

#include "Tteok/TteokData.h"
#include "Ingredient/IngredientData.h"

#include "Player/Components/CSKChapSalKingComponent.h"
#include "Player/Components/CSKPlayerStatusComponent.h"
#include "Player/Components/CSKPlayerStatusVFXComponent.h"
#include "Player/Components/CSKPlayerVisualComponent.h"
#include "Player/Components/CSKPlayerMoveComponent.h"
#include "Player/Components/CSKPlayerSoftBodyComponent.h"
#include "Player/Components/CSKPlayerScaleComponent.h"
#include "Player/Components/CSKPlayerAttackComponent.h"
#include "Player/Components/CSKPlayerShootComponent.h"
#include "Player/Components/CSKPlayerLeapComponent.h"
#include "Player/Components/CSKPlayerArcFlightComponent.h"
#include "Player/Components/CSKPlayerIngredientComponent.h"
#include "Player/Components/CSKEatableComponent.h"
#include "AI/CSKBotAIController.h"

ACSKPlayerCharacter::ACSKPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	InitCharacterSettings();
	InitCameraComponents();

	PestleAttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PestleAttachPoint"));
	PestleAttachPoint->SetupAttachment(GetCapsuleComponent());

	ShootMuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("ShootMuzzlePoint"));
	ShootMuzzlePoint->SetupAttachment(GetCapsuleComponent());
	ShootMuzzlePoint->SetRelativeLocation(FVector(40.0f, 0.0f, 40.0f));

	AttackHitboxComponent = CreateDefaultSubobject<USphereComponent>(TEXT("AttackHitboxComponent"));
	AttackHitboxComponent->SetupAttachment(GetCapsuleComponent());
	AttackHitboxComponent->SetSphereRadius(120.0f);
	AttackHitboxComponent->SetRelativeLocation(FVector::ZeroVector);
	AttackHitboxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackHitboxComponent->SetGenerateOverlapEvents(false);
	AttackHitboxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	AttackHitboxComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AttackHitboxComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	AttackHitboxComponent->SetHiddenInGame(true);

	ChapSalKingComponent = CreateDefaultSubobject<UCSKChapSalKingComponent>(TEXT("ChapSalKingComponent"));
	StatusComponent = CreateDefaultSubobject<UCSKPlayerStatusComponent>(TEXT("StatusComponent"));
	StatusVFXComponent = CreateDefaultSubobject<UCSKPlayerStatusVFXComponent>(TEXT("StatusVFXComponent"));
	VisualComponent = CreateDefaultSubobject<UCSKPlayerVisualComponent>(TEXT("VisualComponent"));
	MoveComponent = CreateDefaultSubobject<UCSKPlayerMoveComponent>(TEXT("MoveComponent"));
	SoftBodyComponent = CreateDefaultSubobject<UCSKPlayerSoftBodyComponent>(TEXT("SoftBodyComponent"));
	ScaleComponent = CreateDefaultSubobject<UCSKPlayerScaleComponent>(TEXT("ScaleComponent"));
	AttackComponent = CreateDefaultSubobject<UCSKPlayerAttackComponent>(TEXT("AttackComponent"));
	ShootComponent = CreateDefaultSubobject<UCSKPlayerShootComponent>(TEXT("ShootComponent"));
	EatableComponent = CreateDefaultSubobject<UCSKEatableComponent>(TEXT("EatableComponent"));
	LeapComponent = CreateDefaultSubobject<UCSKPlayerLeapComponent>(TEXT("LeapComponent"));
	ArcFlightComponent = CreateDefaultSubobject<UCSKPlayerArcFlightComponent>(TEXT("ArcFlightComponent"));
	IngredientComponent = CreateDefaultSubobject<UCSKPlayerIngredientComponent>(TEXT("IngredientComponent"));
}

void ACSKPlayerCharacter::ReceiveCombatHit_Implementation(const FCSKCombatHitContext& HitContext)
{
	if (bSequenceLocked)
	{
		return;
	}

	if (StatusComponent->IsImmuneToHitReaction())
	{
		return;
	}

	if (SoftBodyComponent)
	{
		SoftBodyComponent->AddHitImpulse(HitContext.HitDirection, 1.0f);
	}

	if (LeapComponent)
	{
		LeapComponent->CancelAiming();
	}

	if (CarriedPestleItem)
	{
		CarriedPestleItem->DropFromCarrier(HitContext.HitDirection);
	}

	if (ACSKBotAIController* BotAI = Cast<ACSKBotAIController>(GetController()))
	{
		BotAI->ApplyStagger();
	}

	if (HitContext.DropFraction > 0.0f && ScaleComponent)
	{
		const float DropAmount = ScaleComponent->GetMass() * HitContext.DropFraction;
		const float Removed = ScaleComponent->RemoveMass(DropAmount);
		if (Removed > 0.0f)
		{
			ADoughBlob::SpawnBlob(this, GetActorLocation(), DoughType, Removed, HitContext.HitDirection);
		}
	}
}

bool ACSKPlayerCharacter::RequestCombatAction_Implementation(const FCSKCombatRequest& Request)
{
	switch (Request.ActionType)
	{
	case ECSKCombatActionType::BasicAttack:
		return AttackComponent ? AttackComponent->TryAttack() : false;

	default:
		return false;
	}
}

void ACSKPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CameraBoom)
	{
		BaseCameraArmLength = CameraBoom->TargetArmLength;
	}
}

void ACSKPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitDoughData();
	ApplyDoughData();

	if (ChapSalKingComponent)
	{
		ChapSalKingComponent->InitializeChapSalKing();
	}

	if (StatusComponent)
	{
		StatusComponent->InitializeStatus();
	}

	if (ScaleComponent)
	{
		ScaleComponent->OnScaleTransitionFinished.AddDynamic(
			this,
			&ACSKPlayerCharacter::HandleScaleTransitionFinished
		);
	}

	if (MoveComponent)
	{
		MoveComponent->InitializeMove();
	}

	if (AttackComponent)
	{
		AttackComponent->InitializeAttack();
	}

	if (ShootComponent)
	{
		ShootComponent->InitializeShoot();
	}

	if (LeapComponent)
	{
		LeapComponent->InitializeLeap();
	}
}

void ACSKPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 플레이어·봇 공통 — 화면에 뜨는 이름은 반죽 이름이다
	if (APlayerState* PS = GetPlayerState())
	{
		if (PlayerDoughData)
		{
			PS->SetPlayerName(PlayerDoughData->DisplayName.ToString());
		}
	}
}

void ACSKPlayerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (MoveComponent)
	{
		MoveComponent->RegisterInputMappingContext();
	}

	if (SoftBodyComponent)
	{
		SoftBodyComponent->UpdateRemoteTickThrottle();
	}

	// 초기 질량은 possess 전에 정해져서 PlayerState에 밀어넣을 수 없었다.
	// 링크가 맺어지는 이 시점에 한 번 밀어넣는다 (리스폰·폰 교체도 이 경로를 탄다)
	if (ScaleComponent)
	{
		ScaleComponent->PushScoreToPlayerState();
	}
}

void ACSKPlayerCharacter::ApplyScaleCameraDistance(float AppliedScale, float DistanceScalePower)
{
	if (!CameraBoom)
	{
		return;
	}

	const float CameraDistanceMultiplier = FMath::Pow(AppliedScale, DistanceScalePower);
	CameraBoom->TargetArmLength = BaseCameraArmLength * CameraDistanceMultiplier;
}

float ACSKPlayerCharacter::GetScaleDesiredCameraArmLength() const
{
	const float AppliedScale = ScaleComponent ? ScaleComponent->GetAppliedScale() : 1.0f;
	const float DistanceScalePower = ScaleComponent ? ScaleComponent->GetCameraDistanceScalePower() : 0.0f;
	return BaseCameraArmLength * FMath::Pow(AppliedScale, DistanceScalePower);
}

bool ACSKPlayerCharacter::IsBeingEaten() const
{
	return EatableComponent && EatableComponent->IsBeingEaten();
}

bool ACSKPlayerCharacter::IsInputLocked() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const ACSKGameState* GameState = World->GetGameState<ACSKGameState>())
		{
			if (GameState->IsMatchFinished())
			{
				return true;
			}
		}
	}

	return IsBeingEaten()
		|| bSequenceLocked
		|| (LeapComponent && LeapComponent->IsLeapAttackActive());
}

void ACSKPlayerCharacter::SetSequenceLocked(bool bLocked)
{
	if (bSequenceLocked == bLocked)
	{
		return;
	}
	bSequenceLocked = bLocked;

	// 잠금 부수효과보다 먼저 알린다 — HUD 억제는 플래그만 보면 되므로 상태 확정 직후가 안전하다
	OnSequenceLockChanged.Broadcast(bSequenceLocked);

	if (bLocked)
	{
		if (LeapComponent)
		{
			LeapComponent->ForceCancel();
		}
		if (ShootComponent)
		{
			ShootComponent->CancelAiming();
		}
		if (AController* SeatedController = GetController())
		{
			SeatedController->SetIgnoreMoveInput(true);
			if (ACSKBotAIController* BotController = Cast<ACSKBotAIController>(SeatedController))
			{
				BotController->StopMovement();
				BotController->PushLogicPause(CSKLogicPauseReasons::SequenceLock);
			}
		}
		if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->DisableMovement();
		}
		return;
	}

	if (AController* SeatedController = GetController())
	{
		SeatedController->SetIgnoreMoveInput(false);
		if (ACSKBotAIController* BotController = Cast<ACSKBotAIController>(SeatedController))
		{
			BotController->PopLogicPause(CSKLogicPauseReasons::SequenceLock);
		}
	}
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		// 시퀀스가 남긴 이동 모드(None/Flying)를 항상 낙하로 복구
		if (MovementComponent->MovementMode == MOVE_None
			|| MovementComponent->MovementMode == MOVE_Flying)
		{
			MovementComponent->SetMovementMode(MOVE_Falling);
		}
	}
}

void ACSKPlayerCharacter::NotifyBeingEatenStarted()
{
	if (LeapComponent)
	{
		LeapComponent->ForceCancel();
	}

	if (ShootComponent)
	{
		ShootComponent->CancelAiming();
	}

	if (SoftBodyComponent)
	{
		SoftBodyComponent->ResetForCapture();
	}

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		CharacterMesh->ResetAnimInstanceDynamics(ETeleportType::ResetPhysics);
	}
}

void ACSKPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (LeapComponent)
	{
		LeapComponent->HandleLanded();
	}

	if (MoveComponent)
	{
		MoveComponent->HandleLanded(Hit);
	}
}

#pragma region Init
void ACSKPlayerCharacter::InitDoughData()
{
	const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	if (!Settings || !Settings->DoughDataRegistry)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProjectSettings/DoughDataRegistry가 비어있습니다."));
		return;
	}

	PlayerDoughData = Settings->DoughDataRegistry->FindByType(DoughType);
	if (!PlayerDoughData)
	{
		UE_LOG(LogTemp, Warning, TEXT("선택한 DoughType의 DoughData를 찾을 수 없습니다."));
	}
}

void ACSKPlayerCharacter::InitCharacterSettings()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

void ACSKPlayerCharacter::InitCameraComponents()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void ACSKPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (MoveComponent)
	{
		MoveComponent->BindInput(PlayerInputComponent);
	}

	if (AttackComponent)
	{
		AttackComponent->BindInput(PlayerInputComponent);
	}

	if (ShootComponent)
	{
		ShootComponent->BindInput(PlayerInputComponent);
	}

	if (EatableComponent)
	{
		EatableComponent->BindInput(PlayerInputComponent);
	}
}
#pragma endregion

#pragma region Player Visual
void ACSKPlayerCharacter::ApplyDoughData()
{
	if (!PlayerDoughData)
	{
		return;
	}

	if (VisualComponent)
	{
		VisualComponent->ApplyVisualData(PlayerDoughData);
	}
}
#pragma endregion

#pragma region ChapSalKing
void ACSKPlayerCharacter::TransformToChapSalKing()
{
	ChapSalKingComponent->TransformToChapSalKing();
}

void ACSKPlayerCharacter::RevertFromChapSalKing()
{
	ChapSalKingComponent->RevertFromChapSalKing();
}

bool ACSKPlayerCharacter::IsArcFlying() const
{
	return ArcFlightComponent && ArcFlightComponent->IsFlying();
}

bool ACSKPlayerCharacter::IsChapSalKing() const
{
	return ChapSalKingComponent->IsChapSalKing();
}

UTteokData* ACSKPlayerCharacter::GetCurrentTteokData() const
{
	return ChapSalKingComponent->GetCurrentTteokData();
}

// ---- 몸 반영 (ChapSalKingComponent가 호출) ----
void ACSKPlayerCharacter::ApplyChapSalKingBody(UTteokData* TteokData)
{
	bPendingChapSalKingTransformEvent = true;

	if (VisualComponent && TteokData)
	{
		VisualComponent->SetTteokBodyOverlay(TteokData->BodyOverlayMaterial);
	}

	if (ShootComponent)
	{
		ShootComponent->CancelAiming();
	}

	if (ScaleComponent)
	{
		ScaleComponent->SetChapSalKingScaleBonusEnabled(true);
	}
}

void ACSKPlayerCharacter::RevertChapSalKingBody()
{
	bPendingChapSalKingTransformEvent = false;

	if (VisualComponent)
	{
		VisualComponent->SetTteokBodyOverlay(nullptr);
	}

	if (LeapComponent)
	{
		LeapComponent->CancelAiming();
	}

	if (ScaleComponent)
	{
		ScaleComponent->EndChapSalKingScaleBonus();
	}

	BP_OnRevertFromChapSalKing();
}

bool ACSKPlayerCharacter::IsLeapAttackActive() const
{
	return LeapComponent && LeapComponent->IsLeapAttackActive();
}

ECSKAffinity ACSKPlayerCharacter::GetAffinity() const
{
	if (const UTteokData* Tteok = GetCurrentTteokData())
	{
		return Tteok->Affinity;
	}

	if (IngredientComponent)
	{
		if (UIngredientData* Ingredient = IngredientComponent->GetCurrentIngredient())
		{
			return Ingredient->Affinity;
		}
	}

	return ECSKAffinity::None;
}

float ACSKPlayerCharacter::GetKnockbackResistance(float ScaleResistancePower) const
{
	const float AppliedScale = ScaleComponent ? ScaleComponent->GetAppliedScale() : 1.0f;
	const float SafeScale = FMath::Max(AppliedScale, KINDA_SMALL_NUMBER);
	float Resistance = FMath::Pow(SafeScale, FMath::Max(ScaleResistancePower, 0.0f));

	if (IsChapSalKing())
	{
		const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
		if (Settings && Settings->PlayerStatData)
		{
			Resistance *= Settings->PlayerStatData->ChapSalKingSettings.KnockbackResistance;
		}
	}

	return FMath::Max(Resistance, KINDA_SMALL_NUMBER);
}

void ACSKPlayerCharacter::HandleScaleTransitionFinished(float AppliedScale)
{
	if (!bPendingChapSalKingTransformEvent)
	{
		return;
	}

	bPendingChapSalKingTransformEvent = false;
	BP_OnTransformToChapSalKing();
}
#pragma endregion

