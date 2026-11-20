#include "Player/Components/CSKEatableComponent.h"

#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKChapSalKingComponent.h"
#include "Player/Components/CSKPlayerSoftBodyComponent.h"
#include "Player/Components/CSKPlayerIngredientComponent.h"
#include "Player/Components/CSKPlayerScaleComponent.h"
#include "Framework/CSKGameState.h"

#include "AI/CSKBotAIController.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"

UCSKEatableComponent::UCSKEatableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCSKEatableComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor = GetOwner();
}

void UCSKEatableComponent::BindInput(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent || !StruggleAction)
	{
		return;
	}

	EnhancedInputComponent->BindAction(StruggleAction, ETriggerEvent::Triggered, this, &UCSKEatableComponent::HandleStruggleInput);
}

void UCSKEatableComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsBeingEaten())
	{
		UpdateEatReaction(DeltaTime);
	}
}

bool UCSKEatableComponent::CanBeEatenBy(AActor* EaterActor) const
{
	if (!bCanBeEaten || !OwnerActor || !EaterActor)
	{
		return false;
	}

	if (OwnerActor == EaterActor)
	{
		return false;
	}

	return true;
}

bool UCSKEatableComponent::TryStartEatReaction(const FCSKEatContext& Context)
{
	if (!CanBeEatenBy(Context.EaterActor) || IsBeingEaten())
	{
		BP_OnEatRejected(Context);
		return false;
	}

	CurrentContext = Context;
	EatPullStartLocation = OwnerActor->GetActorLocation();
	StolenMassGrams = 0.0f;
	LastStruggleSign = 0;
	SetStruggleProgress(0.0f);
	DisableOwnerEatCollision();

	if (ACSKPlayerCharacter* OwnerCSKCharacter = Cast<ACSKPlayerCharacter>(OwnerActor))
	{
		OwnerCSKCharacter->NotifyBeingEatenStarted();
	}

	if (bLockCharacterControl)
	{
		LockOwnerControl();
	}

	if (ACharacter* OwnerCharacter = GetOwnerCharacter())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController()))
		{
			PlayerController->SetViewTargetWithBlend(Context.EaterActor, 0.2f);
		}
	}

	SetEatenState(ECSKEatenState::PullingToMouth);
	BP_OnEatPullStarted(CurrentContext);

	return true;
}

void UCSKEatableComponent::CancelEatReaction()
{
	if (!IsBeingEaten())
	{
		return;
	}

	FinishEatReaction();
}

void UCSKEatableComponent::SetEatenState(ECSKEatenState NewState)
{
	EatenState = NewState;
	EatenStateElapsedTime = 0.0f;

	SetComponentTickEnabled(EatenState != ECSKEatenState::None);
	OnEatenStateChanged.Broadcast(EatenState);
}

void UCSKEatableComponent::UpdateEatReaction(float DeltaTime)
{
	EatenStateElapsedTime += DeltaTime;

	switch (EatenState)
	{
	case ECSKEatenState::PullingToMouth:
		UpdatePullingToMouth(DeltaTime);
		break;

	case ECSKEatenState::HeldInBelly:
		UpdateHeldInBelly(DeltaTime);
		break;

	case ECSKEatenState::EjectedRecovering:
		UpdateEjectedRecovering(DeltaTime);
		break;

	case ECSKEatenState::StuckToKing:
		UpdateStuckToKing(DeltaTime);
		break;

	case ECSKEatenState::None:
	default:
		break;
	}
}

void UCSKEatableComponent::UpdatePullingToMouth(float DeltaTime)
{
	if (!OwnerActor || !CurrentContext.EaterActor)
	{
		FinishEatReaction();
		return;
	}

	const FVector EatTargetLocation = GetEatTargetLocation();
	const float PullDuration = FMath::Max(CurrentContext.EatSettings.PullDuration, KINDA_SMALL_NUMBER);
	const float PullAlpha = FMath::Clamp(EatenStateElapsedTime / PullDuration, 0.0f, 1.0f);
	const float SmoothPullAlpha = FMath::SmoothStep(0.0f, 1.0f, PullAlpha);
	const FVector PulledLocation = FMath::Lerp(EatPullStartLocation, EatTargetLocation, SmoothPullAlpha);

	OwnerActor->SetActorLocation(PulledLocation, false);

	if (PullAlpha >= 1.0f)
	{
		OwnerActor->SetActorLocation(EatTargetLocation, false);
		HideOwnerInBelly();
		SetEatenState(ECSKEatenState::HeldInBelly);
		OnEatConsumed.Broadcast(this, CurrentContext);
		BP_OnEatConsumed(CurrentContext);
	}
}

void UCSKEatableComponent::UpdateHeldInBelly(float DeltaTime)
{
	if (!OwnerActor || !CurrentContext.EaterActor)
	{
		FinishEatReaction();
		return;
	}

	OwnerActor->SetActorLocation(GetEatTargetLocation(), false);

	if (EatenStateElapsedTime >= CurrentContext.EatSettings.BellyHoldDuration)
	{
		if (CurrentContext.EatSettings.bStickToKingOnEat)
		{
			AttachToKingSurface();
		}
		else
		{
			EjectOwner();
		}
	}
}

void UCSKEatableComponent::UpdateEjectedRecovering(float DeltaTime)
{
	const UCharacterMovementComponent* Movement = GetOwnerMovement();

	const bool bMinRecoverElapsed =
		EatenStateElapsedTime >= CurrentContext.EatSettings.RecoverMinDuration;

	const bool bMaxRecoverElapsed =
		EatenStateElapsedTime >= CurrentContext.EatSettings.RecoverMaxDuration;

	const bool bGrounded =
		Movement && Movement->IsMovingOnGround();

	if (bMinRecoverElapsed && (bGrounded || bMaxRecoverElapsed))
	{
		FinishEatReaction();
	}
}

void UCSKEatableComponent::UpdateStuckToKing(float DeltaTime)
{
	const UWorld* World = GetWorld();
	const ACSKGameState* GameState = World ? World->GetGameState<ACSKGameState>() : nullptr;
	if (GameState && GameState->IsMatchFinished())
	{
		return;
	}

	// 봇은 성격의 연타 속도로 자동 몸부림
	if (const ACharacter* OwnerCharacter = GetOwnerCharacter())
	{
		if (const ACSKBotAIController* BotController = Cast<ACSKBotAIController>(OwnerCharacter->GetController()))
		{
			AddStruggleFlips(BotController->GetPersonality().StruggleFlipsPerSecond * DeltaTime);
		}
	}

	SetStruggleProgress(StruggleProgress - CurrentContext.EatSettings.StruggleDecayFlipsPerSecond * DeltaTime);

	if (StruggleProgress >= GetRequiredStruggleFlips())
	{
		EscapeFromKing();
	}
}

void UCSKEatableComponent::HandleStruggleInput(const FInputActionValue& Value)
{
	if (EatenState != ECSKEatenState::StuckToKing)
	{
		return;
	}

	const float AxisX = Value.Get<FVector2D>().X;
	const int32 Sign = AxisX > 0.5f ? 1 : (AxisX < -0.5f ? -1 : 0);
	if (Sign == 0 || Sign == LastStruggleSign)
	{
		return;
	}

	LastStruggleSign = Sign;
	AddStruggleFlips(1.0f);

	// 전환마다 붙은 몸이 그 방향으로 출렁인다
	if (const ACSKPlayerCharacter* OwnerCSKCharacter = Cast<ACSKPlayerCharacter>(OwnerActor))
	{
		if (UCSKPlayerSoftBodyComponent* SoftBody = OwnerCSKCharacter->GetSoftBodyComponent())
		{
			const FVector SwayDirection = CurrentContext.EaterActor
				? CurrentContext.EaterActor->GetActorRightVector() * static_cast<float>(Sign)
				: FVector::RightVector * static_cast<float>(Sign);
			SoftBody->AddHitImpulse(SwayDirection, 0.6f);
		}
	}
}

void UCSKEatableComponent::AddStruggleFlips(float Flips)
{
	SetStruggleProgress(StruggleProgress + Flips);
}

float UCSKEatableComponent::GetRequiredStruggleFlips() const
{
	const FCSKPlayerEatAttackSettings& Settings = CurrentContext.EatSettings;
	return Settings.StruggleEscapeSeconds * Settings.StruggleReferenceFlipsPerSecond;
}

float UCSKEatableComponent::GetStruggleRatio() const
{
	const float Required = GetRequiredStruggleFlips();
	return Required > 0.0f ? FMath::Clamp(StruggleProgress / Required, 0.0f, 1.0f) : 0.0f;
}

void UCSKEatableComponent::SetStruggleProgress(float NewProgress)
{
	const float PreviousRatio = GetStruggleRatio();
	StruggleProgress = FMath::Max(NewProgress, 0.0f);

	const float NewRatio = GetStruggleRatio();
	if (!FMath::IsNearlyEqual(PreviousRatio, NewRatio))
	{
		OnStruggleChanged.Broadcast(NewRatio);
	}
}

void UCSKEatableComponent::RefundStolenMass(float Fraction)
{
	const float RefundRequest = StolenMassGrams * FMath::Clamp(Fraction, 0.0f, 1.0f);
	if (RefundRequest <= 0.0f)
	{
		return;
	}

	const ACSKPlayerCharacter* EaterCharacter = Cast<ACSKPlayerCharacter>(CurrentContext.EaterActor);
	const ACSKPlayerCharacter* OwnerCSKCharacter = Cast<ACSKPlayerCharacter>(OwnerActor);
	UCSKPlayerScaleComponent* EaterScale = EaterCharacter ? EaterCharacter->GetScaleComponent() : nullptr;
	UCSKPlayerScaleComponent* OwnerScale = OwnerCSKCharacter ? OwnerCSKCharacter->GetScaleComponent() : nullptr;
	if (!EaterScale || !OwnerScale)
	{
		return;
	}

	const float Refunded = EaterScale->RemoveMass(RefundRequest);
	if (Refunded > 0.0f)
	{
		OwnerScale->AddMass(Refunded);
	}
	StolenMassGrams = 0.0f;
}

void UCSKEatableComponent::EscapeFromKing()
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter || !CurrentContext.EaterActor)
	{
		FinishEatReaction();
		return;
	}

	RefundStolenMass(1.0f);

	// 붙어 있던 표면의 바깥 방향으로 사출 (여럿이 붙었으면 서로 다른 방향으로 흩어진다)
	FVector OutwardDirection = FVector::ZeroVector;
	if (const USkeletalMeshComponent* DoughMesh = OwnerCharacter->GetMesh())
	{
		OutwardDirection = DoughMesh->GetComponentLocation() - CurrentContext.EaterActor->GetActorLocation();
		OutwardDirection.Z = 0.0f;
		OutwardDirection = OutwardDirection.GetSafeNormal();
	}
	if (OutwardDirection.IsNearlyZero())
	{
		OutwardDirection = -CurrentContext.EaterActor->GetActorForwardVector();
	}

	DetachFromKing();
	RestoreOwnerBellyHiddenState();

	if (UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Falling);
	}

	const FCSKPlayerEatAttackSettings& Settings = CurrentContext.EatSettings;
	OwnerCharacter->LaunchCharacter(
		OutwardDirection * Settings.StruggleEscapeOutwardStrength
		+ FVector::UpVector * Settings.StruggleEscapeUpwardStrength,
		true, true);

	if (UWorld* World = GetWorld())
	{
		if (ACSKGameState* GameState = World->GetGameState<ACSKGameState>())
		{
			const ACSKPlayerCharacter* EaterCharacter = Cast<ACSKPlayerCharacter>(CurrentContext.EaterActor);
			GameState->BroadcastMatchEvent(FCSKMatchEventInfo(
				ECSKMatchEventType::DoughEscaped,
				OwnerCharacter->GetPlayerState(),
				EaterCharacter ? EaterCharacter->GetPlayerState() : nullptr));
		}
	}

	SetEatenState(ECSKEatenState::EjectedRecovering);
	BP_OnEscaped(CurrentContext);
}

void UCSKEatableComponent::ReleaseFromKingSession()
{
	if (EatenState == ECSKEatenState::StuckToKing)
	{
		RefundStolenMass(FMath::Pow(GetStruggleRatio(), CurrentContext.EatSettings.StruggleRefundCurvePower));
	}

	FinishEatReaction();
}

void UCSKEatableComponent::LockOwnerControl()
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter)
	{
		return;
	}

	if (AController* Controller = OwnerCharacter->GetController())
	{
		Controller->SetIgnoreMoveInput(true);
		Controller->SetIgnoreLookInput(true);

		if (ACSKBotAIController* BotController = Cast<ACSKBotAIController>(Controller))
		{
			BotController->StopMovement();
			BotController->PushLogicPause(CSKLogicPauseReasons::BeingEaten);
		}
	}

	if (UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}
}

void UCSKEatableComponent::UnlockOwnerControl()
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter)
	{
		return;
	}

	if (AController* Controller = OwnerCharacter->GetController())
	{
		Controller->SetIgnoreMoveInput(false);
		Controller->SetIgnoreLookInput(false);

		if (ACSKBotAIController* BotController = Cast<ACSKBotAIController>(Controller))
		{
			BotController->PopLogicPause(CSKLogicPauseReasons::BeingEaten);
		}
	}

	if (UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement())
	{
		if (Movement->MovementMode == MOVE_None)
		{
			Movement->SetMovementMode(MOVE_Falling);
		}
	}
}

void UCSKEatableComponent::DisableOwnerEatCollision()
{
	UCapsuleComponent* Capsule = GetOwnerCapsule();
	if (!Capsule || bOwnerEatCollisionDisabled)
	{
		return;
	}

	SavedPawnCollisionResponse = Capsule->GetCollisionResponseToChannel(ECC_Pawn);
	Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	OwnerActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}

		CameraCollisionComponents.Add(PrimitiveComponent);
		SavedCameraCollisionResponses.Add(PrimitiveComponent->GetCollisionResponseToChannel(ECC_Camera));
		PrimitiveComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}

	bOwnerEatCollisionDisabled = true;
}

void UCSKEatableComponent::RestoreOwnerEatCollision()
{
	UCapsuleComponent* Capsule = GetOwnerCapsule();
	if (!Capsule || !bOwnerEatCollisionDisabled)
	{
		return;
	}

	Capsule->SetCollisionResponseToChannel(ECC_Pawn, SavedPawnCollisionResponse);

	for (int32 Index = 0; Index < CameraCollisionComponents.Num(); ++Index)
	{
		UPrimitiveComponent* PrimitiveComponent = CameraCollisionComponents[Index].Get();
		if (!PrimitiveComponent || !SavedCameraCollisionResponses.IsValidIndex(Index))
		{
			continue;
		}

		PrimitiveComponent->SetCollisionResponseToChannel(ECC_Camera, SavedCameraCollisionResponses[Index]);
	}

	CameraCollisionComponents.Reset();
	SavedCameraCollisionResponses.Reset();
	bOwnerEatCollisionDisabled = false;
}

void UCSKEatableComponent::HideOwnerInBelly()
{
	if (!OwnerActor || bOwnerHiddenInBelly)
	{
		return;
	}

	bSavedOwnerHiddenInGame = OwnerActor->IsHidden();
	OwnerActor->SetActorHiddenInGame(true);
	bOwnerHiddenInBelly = true;
}

void UCSKEatableComponent::RestoreOwnerBellyHiddenState()
{
	if (!OwnerActor || !bOwnerHiddenInBelly)
	{
		return;
	}

	OwnerActor->SetActorHiddenInGame(bSavedOwnerHiddenInGame);
	bOwnerHiddenInBelly = false;
}

void UCSKEatableComponent::EjectOwner()
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (!OwnerCharacter || !CurrentContext.EaterActor)
	{
		FinishEatReaction();
		return;
	}

	const FVector EaterForward = CurrentContext.EaterActor->GetActorForwardVector().GetSafeNormal();
	const FVector LaunchVelocity =
		-EaterForward * CurrentContext.EatSettings.EjectBackwardStrength
		+ FVector::UpVector * CurrentContext.EatSettings.EjectUpwardStrength;

	if (UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Falling);
	}

	RestoreOwnerBellyHiddenState();
	OwnerCharacter->LaunchCharacter(LaunchVelocity, true, true);

	SetEatenState(ECSKEatenState::EjectedRecovering);
	BP_OnEatEjected(CurrentContext);
}

void UCSKEatableComponent::AttachToKingSurface()
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	ACSKPlayerCharacter* EaterCharacter = Cast<ACSKPlayerCharacter>(CurrentContext.EaterActor);
	USkeletalMeshComponent* DoughMesh = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;
	USkeletalMeshComponent* KingMesh = EaterCharacter ? EaterCharacter->GetMesh() : nullptr;
	UCSKChapSalKingComponent* KingComponent = EaterCharacter ? EaterCharacter->GetChapSalKingComponent() : nullptr;

	if (!OwnerCharacter || !DoughMesh || !KingMesh || !KingComponent)
	{
		EjectOwner();
		return;
	}

	const FName StickSocket = KingComponent->ReserveStickSocket();
	if (StickSocket.IsNone() || !KingMesh->DoesSocketExist(StickSocket))
	{
		// 빈 소켓이 없거나 아직 메시에 소켓이 정의되지 않았으면 기존 사출로 처리
		if (!StickSocket.IsNone())
		{
			KingComponent->ReleaseStickSocket(StickSocket);
		}
		EjectOwner();
		return;
	}

	// 뱃속에서 다시 드러나 표면으로 나옴
	RestoreOwnerBellyHiddenState();

	// 메시만 소켓에 하드 어태치: Control Rig가 움직이는 소켓을 프레임 밀림 없이 추적.
	// 스케일은 반죽 고유값 유지(KeepWorld), 위치·회전만 소켓에 스냅.
	SavedMeshRelativeTransform = DoughMesh->GetRelativeTransform();
	const FAttachmentTransformRules AttachRules(
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::KeepWorld,
		false);
	DoughMesh->AttachToComponent(KingMesh, AttachRules, StickSocket);

	// Derive the surface normal from the king center, ignoring authored socket rotation.
	// Dough +Z faces outward, so the root bottom (-Z) rests against the surface.
	const FVector SocketLocation = KingMesh->GetSocketLocation(StickSocket);
	FVector SurfaceNormal = (SocketLocation - KingMesh->Bounds.Origin).GetSafeNormal();
	if (SurfaceNormal.IsNearlyZero())
	{
		SurfaceNormal = KingMesh->GetSocketQuaternion(StickSocket).GetUpVector();
	}

	FVector FaceDirection = FVector::VectorPlaneProject(
		EaterCharacter->GetActorForwardVector(),
		SurfaceNormal).GetSafeNormal();
	if (FaceDirection.IsNearlyZero())
	{
		FaceDirection = FVector::VectorPlaneProject(
			EaterCharacter->GetActorRightVector(),
			SurfaceNormal).GetSafeNormal();
	}

	const float MaxFaceAngle = FMath::Max(CurrentContext.EatSettings.StickFaceRandomAngle, 0.0f);
	const float FaceAngle = FMath::FRandRange(-MaxFaceAngle, MaxFaceAngle);
	FaceDirection = FaceDirection.RotateAngleAxis(FaceAngle, SurfaceNormal);

	const FQuat SurfaceRotation = FRotationMatrix::MakeFromZY(
		SurfaceNormal,
		FaceDirection).ToQuat();
	const float EmbedDepth = FMath::Max(CurrentContext.EatSettings.StickEmbedDepth, 0.0f);
	const FVector StuckLocation = SocketLocation - SurfaceNormal * EmbedDepth;
	DoughMesh->SetWorldLocationAndRotation(StuckLocation, SurfaceRotation);

	// Pulling and snapping can create new inertial deformation after the initial reset.
	// Reset again at the final attached transform and immediately evaluate a neutral pose.
	if (ACSKPlayerCharacter* CapturedCharacter = Cast<ACSKPlayerCharacter>(OwnerCharacter))
	{
		if (UCSKPlayerIngredientComponent* Ingredient = CapturedCharacter->GetIngredientComponent())
		{
			if (Ingredient->HasIngredient())
			{
				Ingredient->ClearIngredient();
			}
		}

		if (UCSKPlayerSoftBodyComponent* SoftBody = CapturedCharacter->GetSoftBodyComponent())
		{
			SoftBody->ResetForCapture();
		}
	}
	DoughMesh->InitAnim(true);
	DoughMesh->ResetAnimInstanceDynamics(ETeleportType::ResetPhysics);
	DoughMesh->TickAnimation(0.0f, false);
	DoughMesh->RefreshBoneTransforms();

	AttachedKingComponent = KingComponent;
	AttachedStickSocket = StickSocket;
	bAttachedToKing = true;

	SetEatenState(ECSKEatenState::StuckToKing);
	KingComponent->NotifyDoughStuck(this);
	BP_OnEatStuck(CurrentContext);
}

void UCSKEatableComponent::DetachFromKing()
{
	if (!bAttachedToKing)
	{
		return;
	}

	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (OwnerCharacter)
	{
		if (USkeletalMeshComponent* DoughMesh = OwnerCharacter->GetMesh())
		{
			// 붙어있던 월드 위치를 캡슐로 옮겨, 떨어질 때 그 자리에서 낙하
			const FVector StuckWorldLocation = DoughMesh->GetComponentLocation();

			DoughMesh->AttachToComponent(
				OwnerCharacter->GetCapsuleComponent(),
				FAttachmentTransformRules::KeepRelativeTransform);
			DoughMesh->SetRelativeTransform(SavedMeshRelativeTransform);

			OwnerActor->SetActorLocation(StuckWorldLocation, false);
		}
	}

	if (UCSKChapSalKingComponent* KingComponent = AttachedKingComponent.Get())
	{
		KingComponent->ReleaseStickSocket(AttachedStickSocket);
	}

	AttachedKingComponent = nullptr;
	AttachedStickSocket = NAME_None;
	bAttachedToKing = false;
}

void UCSKEatableComponent::FinishEatReaction()
{
	const FCSKEatContext FinishedContext = CurrentContext;

	DetachFromKing();

	if (bLockCharacterControl)
	{
		UnlockOwnerControl();
	}
	RestoreOwnerEatCollision();
	RestoreOwnerBellyHiddenState();

	if (ACharacter* OwnerCharacter = GetOwnerCharacter())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(OwnerCharacter->GetController()))
		{
			PlayerController->SetViewTargetWithBlend(OwnerCharacter, 0.2f);
		}
	}

	SetStruggleProgress(0.0f);
	StolenMassGrams = 0.0f;
	LastStruggleSign = 0;

	CurrentContext = FCSKEatContext();
	EatPullStartLocation = FVector::ZeroVector;
	SetEatenState(ECSKEatenState::None);

	OnEatConsumed.Clear();

	BP_OnEatFinished(FinishedContext);
}

FVector UCSKEatableComponent::GetEatTargetLocation() const
{
	if (!OwnerActor || !CurrentContext.EaterActor)
	{
		return OwnerActor ? OwnerActor->GetActorLocation() : FVector::ZeroVector;
	}

	return CurrentContext.EaterActor->GetActorLocation();
}

ACharacter* UCSKEatableComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(OwnerActor);
}

UCapsuleComponent* UCSKEatableComponent::GetOwnerCapsule() const
{
	const ACharacter* OwnerCharacter = GetOwnerCharacter();
	return OwnerCharacter
		? OwnerCharacter->GetCapsuleComponent()
		: nullptr;
}

UCharacterMovementComponent* UCSKEatableComponent::GetOwnerMovement() const
{
	const ACharacter* OwnerCharacter = GetOwnerCharacter();

	return OwnerCharacter
		? OwnerCharacter->GetCharacterMovement()
		: nullptr;
}
