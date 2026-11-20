#include "Player/Components/CSKChapSalKingComponent.h"

#include "Player/CSKPlayerCharacter.h"
#include "Player/Components/CSKPlayerIngredientComponent.h"
#include "Player/Components/CSKPlayerScaleComponent.h"
#include "Player/Components/CSKPlayerMoveComponent.h"
#include "Player/Components/CSKPlayerSoftBodyComponent.h"
#include "Player/Components/CSKEatableComponent.h"
#include "Ingredient/IngredientData.h"
#include "Combat/ShotEffects/CSKShotEffect.h"
#include "Camera/CSKCameraShakeStatics.h"
#include "Camera/CSKCameraShakeTags.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Framework/CSKGameState.h"
#include "Framework/CSKGameInstance.h"
#include "Framework/CSKProjectSettingsData.h"
#include "Framework/CSKGameMode.h"

#include "DrawDebugHelpers.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

UCSKChapSalKingComponent::UCSKChapSalKingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCSKChapSalKingComponent::InitializeChapSalKing()
{
	CacheOwnerCSKCharacter();
	InitStatData();
}

void UCSKChapSalKingComponent::TransformToChapSalKing()
{
	if (!OwnerCSKCharacter || bIsChapSalKing)
	{
		return;
	}

	const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	if (!Settings)
	{
		return;
	}

	CurrentTteokData = nullptr;
	const TOptional<ETteokType> SelectedType = ResolveTteokTypeFromIngredient();
	if (SelectedType.IsSet())
	{
		const TObjectPtr<UTteokData>* Found = Settings->TteokDataMap.Find(SelectedType.GetValue());
		if (!Found || !*Found)
		{
			UE_LOG(LogTemp, Error, TEXT("[CSK] TransformToChapSalKing: TteokDataMap에 %s 데이터가 없습니다."),
				*UEnum::GetValueAsString(SelectedType.GetValue()));
			return;
		}

		CurrentTteokData = *Found;
	}

	bIsChapSalKing = true;

	if (UCSKPlayerIngredientComponent* IngredientComponent = OwnerCSKCharacter->GetIngredientComponent())
	{
		IngredientComponent->ClearIngredient();
	}

	OwnerCSKCharacter->ApplyChapSalKingBody(CurrentTteokData);

	StartKingSession();
}

TOptional<ETteokType> UCSKChapSalKingComponent::ResolveTteokTypeFromIngredient() const
{
	const UCSKPlayerIngredientComponent* IngredientComponent =
		OwnerCSKCharacter ? OwnerCSKCharacter->GetIngredientComponent() : nullptr;
	const UIngredientData* Ingredient =
		IngredientComponent ? IngredientComponent->GetCurrentIngredient() : nullptr;

	if (!Ingredient)
	{
		return {};
	}

	switch (Ingredient->Affinity)
	{
	case ECSKAffinity::RedBean:
		return ETteokType::Chapssal;
	case ECSKAffinity::SoyPowder:
		return ETteokType::Injeolmi;
	case ECSKAffinity::Honey:
		return ETteokType::Kkul;
	case ECSKAffinity::Rainbow:
		return ETteokType::Mujigae;
	default:
		return {};
	}
}

void UCSKChapSalKingComponent::PlayLandingImpact()
{
	UWorld* World = GetWorld();
	if (!bIsChapSalKing || !World)
	{
		return;
	}

	if (UCSKPlayerSoftBodyComponent* SoftBody = OwnerCSKCharacter->GetSoftBodyComponent())
	{
		SoftBody->AddBodySlamImpulse(1.0f);
	}

	// 예측점 오차·경사 대응: 실제 바닥을 트레이스해 임팩트 기준점으로 삼는다
	const FCSKPlayerLeapSettings& LeapSettings = PlayerStatData->LeapSettings;
	const float HalfHeight = OwnerCSKCharacter->GetSimpleCollisionHalfHeight();
	const FVector ActorLocation = OwnerCSKCharacter->GetActorLocation();

	FVector GroundLocation = ActorLocation - FVector(0.0f, 0.0f, HalfHeight);
	FVector GroundNormal = FVector::UpVector;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCSKCharacter);

	FHitResult GroundHit;
	if (World->LineTraceSingleByChannel(
			GroundHit,
			ActorLocation,
			ActorLocation - FVector(0.0f, 0.0f, HalfHeight + 300.0f),
			LeapSettings.LeapTraceChannel,
			Params))
	{
		GroundLocation = GroundHit.ImpactPoint;
		GroundNormal = GroundHit.ImpactNormal.GetSafeNormal();
	}

	UCSKCameraShakeStatics::PlayCameraShakeAtLocation(
		this,
		CSKCameraShakeTags::King_LeapLanding,
		GroundLocation);

	UTteokData* Tteok = CurrentTteokData;

	FCSKShotEffectContext Context;
	Context.Attacker = OwnerCSKCharacter;
	Context.HitLocation = GroundLocation;
	Context.HitDirection = OwnerCSKCharacter->GetActorForwardVector();
	Context.World = World;
	Context.SourceAffinity = Tteok ? Tteok->Affinity : ECSKAffinity::None;
	Context.AreaRadius = LeapSettings.LandingIndicatorRadius;

	const FCSKPlayerLeapLandingVFXSettings& LandingVFX =
		PlayerStatData->LeapLandingVFXSettings;
	if (LandingVFX.System)
	{
		const FVector SpawnLocation =
			GroundLocation + GroundNormal * LandingVFX.ZOffset;
		const float RadiusScale =
			Context.AreaRadius / FMath::Max(LandingVFX.ReferenceRadius, 1.0f);
		const float MasterScale = LandingVFX.MasterScale * RadiusScale;

		if (UNiagaraComponent* NiagaraComponent =
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				LandingVFX.System,
				SpawnLocation,
				FRotationMatrix::MakeFromZ(GroundNormal).Rotator(),
				FVector::OneVector,
				true,
				false))
		{
			NiagaraComponent->SetVariableFloat(
				FName(TEXT("User.00_Master_Scale")),
				MasterScale);

			const FVector BoundsExtent(Context.AreaRadius);
			NiagaraComponent->SetSystemFixedBounds(
				FBox(-BoundsExtent, BoundsExtent));
			NiagaraComponent->Activate(true);
		}
	}

	if (Tteok)
	{
		UCSKShotEffect::DispatchSurfaceImpact(
			Tteok->LandingEffects,
			Context);

		if (UCSKGameInstance::IsDrawDebugEnabled(this))
		{
			DrawDebugCircle(
				World,
				Context.HitLocation + FVector(0.0f, 0.0f, 15.0f),
				Context.AreaRadius, 48, FColor::Magenta, false, 2.5f, 0, 8.0f,
				FVector(1.0f, 0.0f, 0.0f), FVector(0.0f, 1.0f, 0.0f), false);
		}
	}
}

void UCSKChapSalKingComponent::RevertFromChapSalKing()
{
	if (!OwnerCSKCharacter || !bIsChapSalKing)
	{
		return;
	}

	bIsChapSalKing = false;
	CurrentTteokData = nullptr;

	EndKingSession();

	OwnerCSKCharacter->RevertChapSalKingBody();
}

void UCSKChapSalKingComponent::StartKingSession()
{
	if (ACSKGameState* GameState = GetCSKGameState())
	{
		GameState->SetChapSalKing(OwnerCSKCharacter);
	}

	StartKingDurationTimer();
}

void UCSKChapSalKingComponent::EndKingSession()
{
	ClearKingDurationTimer();
	CancelOngoingEatReactions();
	OccupiedStickSockets.Reset();

	if (ACSKGameState* GameState = GetCSKGameState())
	{
		GameState->ClearChapSalKing(OwnerCSKCharacter);
	}
}

FName UCSKChapSalKingComponent::ReserveStickSocket()
{
	const FCSKChapSalKingSettings& KingSettings = PlayerStatData->ChapSalKingSettings;
	const int32 SocketCount = FMath::Max(KingSettings.StickSocketCount, 1);

	for (int32 Index = 0; Index < SocketCount; ++Index)
	{
		const FName SocketName = MakeStickSocketName(KingSettings.StickSocketBaseName, Index);
		if (!OccupiedStickSockets.Contains(SocketName))
		{
			OccupiedStickSockets.Add(SocketName);
			return SocketName;
		}
	}

	return NAME_None;
}

void UCSKChapSalKingComponent::ReleaseStickSocket(FName SocketName)
{
	OccupiedStickSockets.Remove(SocketName);
}

FName UCSKChapSalKingComponent::MakeStickSocketName(FName BaseName, int32 Index)
{
	return FName(*FString::Printf(TEXT("%s%02d"), *BaseName.ToString(), Index + 1));
}

void UCSKChapSalKingComponent::CancelOngoingEatReactions()
{
	for (TActorIterator<ACSKPlayerCharacter> It(GetWorld()); It; ++It)
	{
		UCSKEatableComponent* Eatable = It->FindComponentByClass<UCSKEatableComponent>();
		if (Eatable && Eatable->IsBeingEatenBy(OwnerCSKCharacter))
		{
			// 붙은 채 세션이 끝나면 몸부림 게이지 비율만큼 돌려주고 푼다
			Eatable->ReleaseFromKingSession();
		}
	}
}

ACSKGameState* UCSKChapSalKingComponent::GetCSKGameState() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetGameState<ACSKGameState>() : nullptr;
}


void UCSKChapSalKingComponent::StartKingDurationTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Duration = GetKingSessionDuration();
	if (Duration <= 0.0f)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		KingDurationTimerHandle,
		this,
		&UCSKChapSalKingComponent::OnKingDurationExpired,
		Duration,
		false
	);
}

void UCSKChapSalKingComponent::ClearKingDurationTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(KingDurationTimerHandle);
	}
}

void UCSKChapSalKingComponent::OnKingDurationExpired()
{
	RevertFromChapSalKing();
}

float UCSKChapSalKingComponent::GetKingSessionDuration() const
{
	const UCSKProjectSettingsData* Settings = UCSKGameInstance::GetProjectSettings(this);
	return Settings ? FMath::Max(Settings->KingSettings.KingSessionDuration, 0.0f) : 0.0f;
}

float UCSKChapSalKingComponent::GetKingSessionTimeRemaining() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0.0f;
	}

	const float Remaining = World->GetTimerManager().GetTimerRemaining(KingDurationTimerHandle);
	return FMath::Max(Remaining, 0.0f);
}

float UCSKChapSalKingComponent::GetKingSessionTimeRemainingRatio() const
{
	const float Duration = GetKingSessionDuration();
	if (Duration <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(GetKingSessionTimeRemaining() / Duration, 0.0f, 1.0f);
}

void UCSKChapSalKingComponent::EatAttack(const FVector& AttackDirection)
{
	if (!OwnerCSKCharacter || !bIsChapSalKing)
	{
		return;
	}

	const FVector SafeAttackDirection = AttackDirection.GetSafeNormal();
	if (SafeAttackDirection.IsNearlyZero())
	{
		return;
	}

	const FCSKPlayerEatAttackSettings& Settings = GetEatAttackSettings();
	const FVector EatSweepStart = GetEatSweepStart(SafeAttackDirection, Settings);
	const FVector EatSweepEnd = GetEatSweepEnd(SafeAttackDirection, Settings);
	const float ScaledEatSweepRadius = GetScaledEatSweepRadius(Settings);

	if (UCSKPlayerSoftBodyComponent* SoftBody = OwnerCSKCharacter->GetSoftBodyComponent())
	{
		SoftBody->AddEatImpulse(1.0f);
	}

	if (UCSKGameInstance::IsDrawDebugEnabled(this))
	{
		DrawDebugSphere(
			GetWorld(),
			EatSweepStart,
			ScaledEatSweepRadius,
			24,
			FColor::Yellow,
			false,
			Settings.DebugDrawDuration,
			0,
			2.0f
		);

		DrawDebugSphere(
			GetWorld(),
			EatSweepEnd,
			ScaledEatSweepRadius,
			24,
			FColor::Yellow,
			false,
			Settings.DebugDrawDuration,
			0,
			2.0f
		);

		DrawDebugLine(
			GetWorld(),
			EatSweepStart,
			EatSweepEnd,
			FColor::Orange,
			false,
			Settings.DebugDrawDuration,
			0,
			3.0f
		);
	}

	BP_OnEatAttackStarted(SafeAttackDirection);

	TArray<UCSKEatableComponent*> EatTargets =
		FindEatTargets(SafeAttackDirection, Settings, EatSweepStart, EatSweepEnd, ScaledEatSweepRadius);

	for (UCSKEatableComponent* TargetEatable : EatTargets)
	{
		if (!TargetEatable)
		{
			continue;
		}

		AActor* TargetActor = TargetEatable->GetOwner();
		if (!TargetActor)
		{
			continue;
		}

		if (UCSKGameInstance::IsDrawDebugEnabled(this))
		{
			DrawDebugSphere(
				GetWorld(),
				TargetActor->GetActorLocation(),
				80.0f,
				16,
				FColor::Green,
				false,
				Settings.DebugDrawDuration,
				0,
				4.0f
			);

			DrawDebugLine(
				GetWorld(),
				EatSweepEnd,
				TargetActor->GetActorLocation(),
				FColor::Green,
				false,
				Settings.DebugDrawDuration,
				0,
				2.0f
			);
		}

		FCSKEatContext EatContext;
		EatContext.EaterActor = OwnerCSKCharacter;
		EatContext.EatDirection = SafeAttackDirection;
		EatContext.EatSettings = Settings;

		TargetEatable->OnEatConsumed.RemoveAll(this);
		TargetEatable->OnEatConsumed.AddUObject(
			this,
			&UCSKChapSalKingComponent::HandleEatTargetConsumed
		);

		if (!TargetEatable->TryStartEatReaction(EatContext))
		{
			TargetEatable->OnEatConsumed.RemoveAll(this);
		}
	}

	BP_OnEatTargetsFound(EatTargets);
}

TArray<UCSKEatableComponent*> UCSKChapSalKingComponent::FindEatTargets(
	const FVector& AttackDirection,
	const FCSKPlayerEatAttackSettings& Settings,
	const FVector& SweepStart,
	const FVector& SweepEnd,
	float SweepRadius
) const
{
	TArray<UCSKEatableComponent*> EatTargets;

	const UWorld* World = GetWorld();
	if (!World)
	{
		return EatTargets;
	}

	const FCollisionShape EatShape = FCollisionShape::MakeSphere(FMath::Max(SweepRadius, 1.0f));

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ChapSalKingEatOverlap), false, OwnerCSKCharacter);

	TArray<FHitResult> HitResults;
	World->SweepMultiByObjectType(
		HitResults,
		SweepStart,
		SweepEnd,
		FQuat::Identity,
		ObjectQueryParams,
		EatShape,
		QueryParams
	);

	TArray<UCSKEatableComponent*> Candidates;

	for (const FHitResult& Result : HitResults)
	{
		AActor* HitActor = Result.GetActor();
		if (!HitActor)
		{
			continue;
		}

		UCSKEatableComponent* Eatable = HitActor->FindComponentByClass<UCSKEatableComponent>();
		if (!CanEatTarget(Eatable, AttackDirection, Settings))
		{
			continue;
		}

		Candidates.AddUnique(Eatable);
	}

	Candidates.Sort(
		[this, AttackDirection](const UCSKEatableComponent& A, const UCSKEatableComponent& B)
		{
			return CalculateEatTargetScore(&A, AttackDirection) > CalculateEatTargetScore(&B, AttackDirection);
		}
	);

	const int32 MaxTargets = FMath::Max(Settings.MaxTargets, 0);
	for (int32 Index = 0; Index < Candidates.Num() && Index < MaxTargets; ++Index)
	{
		EatTargets.Add(Candidates[Index]);
	}

	return EatTargets;
}
bool UCSKChapSalKingComponent::CanEatTarget(
	const UCSKEatableComponent* TargetEatable,
	const FVector& AttackDirection,
	const FCSKPlayerEatAttackSettings& Settings
) const
{
	if (!OwnerCSKCharacter || !TargetEatable)
	{
		return false;
	}

	AActor* TargetActor = TargetEatable->GetOwner();
	if (!TargetActor || TargetActor == OwnerCSKCharacter)
	{
		return false;
	}

	if (!TargetEatable->CanBeEatenBy(OwnerCSKCharacter))
	{
		return false;
	}

	FVector DirectionToTarget = TargetActor->GetActorLocation() - OwnerCSKCharacter->GetActorLocation();
	DirectionToTarget.Z = 0.0f;

	if (DirectionToTarget.IsNearlyZero())
	{
		return true;
	}

	DirectionToTarget = DirectionToTarget.GetSafeNormal();

	const FVector FlatAttackDirection = FVector(AttackDirection.X, AttackDirection.Y, 0.0f).GetSafeNormal();
	const float ForwardDot = FVector::DotProduct(FlatAttackDirection, DirectionToTarget);

	return ForwardDot >= Settings.MinForwardDot;
}

float UCSKChapSalKingComponent::CalculateEatTargetScore(const UCSKEatableComponent* TargetEatable, const FVector& AttackDirection) const
{
	if (!OwnerCSKCharacter || !TargetEatable || !TargetEatable->GetOwner())
	{
		return -FLT_MAX;
	}

	const AActor* TargetActor = TargetEatable->GetOwner();

	FVector ToTarget = TargetActor->GetActorLocation() - OwnerCSKCharacter->GetActorLocation();
	ToTarget.Z = 0.0f;

	const float Distance = ToTarget.Size();
	const FVector DirectionToTarget = ToTarget.IsNearlyZero()
		? AttackDirection
		: ToTarget.GetSafeNormal();

	const FVector FlatAttackDirection = FVector(AttackDirection.X, AttackDirection.Y, 0.0f).GetSafeNormal();
	const float ForwardDot = FVector::DotProduct(FlatAttackDirection, DirectionToTarget);

	return ForwardDot * 1000.0f - Distance;
}

void UCSKChapSalKingComponent::HandleEatTargetConsumed(UCSKEatableComponent* EatableComponent, const FCSKEatContext& EatContext)
{
	if (!OwnerCSKCharacter || !EatableComponent || EatContext.EaterActor != OwnerCSKCharacter)
	{
		return;
	}

	EatableComponent->OnEatConsumed.RemoveAll(this);

	const FCSKChapSalKingSettings& KingSettings = PlayerStatData->ChapSalKingSettings;

	if (const ACSKPlayerCharacter* TargetCharacter = Cast<ACSKPlayerCharacter>(EatableComponent->GetOwner()))
	{
		if (ACSKGameState* CSKGameState = GetCSKGameState())
		{
			CSKGameState->BroadcastMatchEvent(FCSKMatchEventInfo(
				ECSKMatchEventType::DoughEaten,
				OwnerCSKCharacter->GetPlayerState(),
				TargetCharacter->GetPlayerState()));
		}

		UCSKPlayerScaleComponent* TargetScale = TargetCharacter->GetScaleComponent();
		UCSKPlayerScaleComponent* OwnerScale = OwnerCSKCharacter->GetScaleComponent();
		if (TargetScale && OwnerScale && KingSettings.EatMassStealFraction > 0.0f)
		{
			// 현재 몸집이 아니라 최저치 위의 여유분에서 뜯는다.
			// 몸집 비율이면 서너 번 만에 바닥이고, 바닥에 닿은 반죽은 더 잃을 게 없어진다
			const float StealAmount = TargetScale->GetLossableMass() * KingSettings.EatMassStealFraction;
			const float Removed = TargetScale->RemoveMass(StealAmount);
			if (Removed > 0.0f)
			{
				OwnerScale->AddMass(Removed);
			}
			EatableComponent->SetStolenMass(Removed);
		}
	}

	if (UCSKPlayerMoveComponent* MoveComponent = OwnerCSKCharacter->GetMoveComponent())
	{
		MoveComponent->ApplySpeedBoost(KingSettings.EatSpeedBoostMultiplier, KingSettings.EatSpeedBoostDuration);
	}
}

void UCSKChapSalKingComponent::NotifyDoughStuck(UCSKEatableComponent* EatableComponent)
{
	if (!bIsChapSalKing || !OwnerCSKCharacter || !EatableComponent
		|| EatableComponent->GetEatenState() != ECSKEatenState::StuckToKing
		|| !EatableComponent->IsBeingEatenBy(OwnerCSKCharacter))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (ACSKGameMode* GameMode = World->GetAuthGameMode<ACSKGameMode>())
	{
		GameMode->TryEndMatchFromKingCapture(OwnerCSKCharacter);
	}
}

const FCSKPlayerEatAttackSettings& UCSKChapSalKingComponent::GetEatAttackSettings() const
{
	return PlayerStatData->AttackSettings.Eat;
}

FVector UCSKChapSalKingComponent::GetEatSweepStart(
	const FVector& AttackDirection,
	const FCSKPlayerEatAttackSettings& Settings
) const
{
	if (!OwnerCSKCharacter)
	{
		return FVector::ZeroVector;
	}

	const FVector FlatAttackDirection = FVector(AttackDirection.X, AttackDirection.Y, 0.0f).GetSafeNormal();
	const FVector BaseLocation = OwnerCSKCharacter->GetActorLocation();

	return BaseLocation
		+ FlatAttackDirection * GetScaledEatSweepStartOffset(Settings)
		+ FVector::UpVector * GetScaledEatHeightOffset(Settings);
}

FVector UCSKChapSalKingComponent::GetEatSweepEnd(
	const FVector& AttackDirection,
	const FCSKPlayerEatAttackSettings& Settings
) const
{
	const FVector FlatAttackDirection = FVector(AttackDirection.X, AttackDirection.Y, 0.0f).GetSafeNormal();
	return GetEatSweepStart(AttackDirection, Settings)
		+ FlatAttackDirection * GetScaledEatSweepLength(Settings);
}

float UCSKChapSalKingComponent::GetOwnerAppliedScale() const
{
	if (!OwnerCSKCharacter)
	{
		return 1.0f;
	}

	const UCSKPlayerScaleComponent* ScaleComponent = OwnerCSKCharacter->GetScaleComponent();
	return ScaleComponent
		? ScaleComponent->GetAppliedScale()
		: 1.0f;
}

float UCSKChapSalKingComponent::GetEatScaleMultiplier(float ScalePower) const
{
	const float SafeAppliedScale = FMath::Max(GetOwnerAppliedScale(), KINDA_SMALL_NUMBER);
	const float SafeScalePower = FMath::Max(ScalePower, 0.0f);

	return FMath::Pow(SafeAppliedScale, SafeScalePower);
}

float UCSKChapSalKingComponent::GetScaledEatSweepRadius(
	const FCSKPlayerEatAttackSettings& Settings
) const
{
	return Settings.SweepRadius * GetEatScaleMultiplier(Settings.SweepRadiusScalePower);
}

float UCSKChapSalKingComponent::GetScaledEatSweepLength(
	const FCSKPlayerEatAttackSettings& Settings
) const
{
	return Settings.SweepLength * GetEatScaleMultiplier(Settings.SweepLengthScalePower);
}

float UCSKChapSalKingComponent::GetScaledEatSweepStartOffset(
	const FCSKPlayerEatAttackSettings& Settings
) const
{
	return Settings.SweepStartOffset * GetEatScaleMultiplier(Settings.SweepOffsetScalePower);
}

float UCSKChapSalKingComponent::GetScaledEatHeightOffset(
	const FCSKPlayerEatAttackSettings& Settings
) const
{
	return Settings.HeightOffset * GetEatScaleMultiplier(Settings.SweepOffsetScalePower);
}
