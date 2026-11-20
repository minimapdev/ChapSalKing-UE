#include "Camera/CSKCameraShakeSubsystem.h"

#include "Camera/CameraShakeBase.h"
#include "Camera/CSKCameraShakeCatalog.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Framework/CSKGameInstance.h"
#include "Framework/CSKProjectSettingsData.h"
#include "GameFramework/PlayerController.h"

const FCSKCameraShakeEntry* UCSKCameraShakeSubsystem::FindEntry(const FGameplayTag& ShakeTag) const
{
	if (!ShakeTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Camera Shake tag."));
		return nullptr;
	}

	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	const UCSKGameInstance* GameInstance = LocalPlayer
		? Cast<UCSKGameInstance>(LocalPlayer->GetGameInstance())
		: nullptr;
	const UCSKProjectSettingsData* ProjectSettings = GameInstance
		? GameInstance->GetProjectSettingsData()
		: nullptr;
	const UCSKCameraShakeCatalog* Catalog = ProjectSettings
		? ProjectSettings->CameraShakeCatalog
		: nullptr;

	if (!Catalog)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraShakeCatalog is not assigned in ProjectSettings."));
		return nullptr;
	}

	const FCSKCameraShakeEntry* Entry = Catalog->FindEntry(ShakeTag);
	if (!Entry)
	{
		UE_LOG(LogTemp, Warning, TEXT("Camera Shake tag is not registered: %s"), *ShakeTag.ToString());
		return nullptr;
	}

	return Entry;
}

APlayerCameraManager* UCSKCameraShakeSubsystem::GetCameraManager() const
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	APlayerController* PlayerController = LocalPlayer
		? LocalPlayer->GetPlayerController(GetWorld())
		: nullptr;

	return PlayerController ? PlayerController->PlayerCameraManager : nullptr;
}

UCameraShakeBase* UCSKCameraShakeSubsystem::PlayShake(FGameplayTag ShakeTag, float Scale)
{
	const FCSKCameraShakeEntry* Entry = FindEntry(ShakeTag);
	if (!Entry)
	{
		return nullptr;
	}

	const float FinalScale = FMath::Max(Entry->BaseScale * Scale, 0.0f);
	return PlayResolvedShake(ShakeTag, *Entry, FinalScale);
}

UCameraShakeBase* UCSKCameraShakeSubsystem::PlayShakeAtLocation(
	FGameplayTag ShakeTag,
	FVector SourceLocation,
	float Scale)
{
	const FCSKCameraShakeEntry* Entry = FindEntry(ShakeTag);
	if (!Entry)
	{
		return nullptr;
	}

	const APlayerCameraManager* CameraManager = GetCameraManager();
	if (!CameraManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerCameraManager is not available for spatial Camera Shake playback."));
		return nullptr;
	}

	const float InnerRadius = FMath::Max(Entry->InnerRadius, 0.0f);
	const float OuterRadius = FMath::Max(Entry->OuterRadius, InnerRadius);
	const float Distance = FVector::Distance(CameraManager->GetCameraLocation(), SourceLocation);

	float DistanceScale = 0.0f;
	if (Distance <= InnerRadius)
	{
		DistanceScale = 1.0f;
	}
	else if (Distance < OuterRadius && !FMath::IsNearlyEqual(InnerRadius, OuterRadius))
	{
		const float DistanceAlpha = FMath::Clamp(
			(Distance - InnerRadius) / (OuterRadius - InnerRadius),
			0.0f,
			1.0f);
		const float SafeFalloffExponent = FMath::Max(Entry->FalloffExponent, 0.01f);
		DistanceScale = FMath::Pow(1.0f - DistanceAlpha, SafeFalloffExponent);
	}

	const float FinalScale = FMath::Max(Entry->BaseScale * Scale * DistanceScale, 0.0f);
	return PlayResolvedShake(ShakeTag, *Entry, FinalScale);
}

void UCSKCameraShakeSubsystem::StopShake(UCameraShakeBase* ShakeInstance, bool bImmediately)
{
	APlayerCameraManager* CameraManager = GetCameraManager();
	if (!CameraManager || !IsValid(ShakeInstance))
	{
		return;
	}

	CameraManager->StopCameraShake(ShakeInstance, bImmediately);
}

UCameraShakeBase* UCSKCameraShakeSubsystem::PlayResolvedShake(
	const FGameplayTag& ShakeTag,
	const FCSKCameraShakeEntry& Entry,
	float FinalScale)
{
	if (!Entry.ShakeClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Camera Shake class is not assigned: %s"), *ShakeTag.ToString());
		return nullptr;
	}

	APlayerCameraManager* CameraManager = GetCameraManager();
	if (!CameraManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerCameraManager is not available for Camera Shake playback."));
		return nullptr;
	}

	if (FinalScale <= 0.0f)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	const double CurrentTime = World ? World->GetTimeSeconds() : 0.0;
	if (Entry.RetriggerInterval > 0.0f)
	{
		if (const double* LastPlayTime = LastPlayTimes.Find(ShakeTag))
		{
			const double ElapsedTime = CurrentTime - *LastPlayTime;
			if (ElapsedTime >= 0.0 && ElapsedTime < Entry.RetriggerInterval)
			{
				return nullptr;
			}
		}
	}

	UCameraShakeBase* StartedShake = CameraManager->StartCameraShake(Entry.ShakeClass, FinalScale);
	if (StartedShake && Entry.RetriggerInterval > 0.0f)
	{
		LastPlayTimes.Add(ShakeTag, CurrentTime);
	}

	return StartedShake;
}
